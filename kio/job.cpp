/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <assert.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <qtimer.h>
#include <qfile.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kmessagebox.h>

#include <errno.h>

#include "slave.h"
#include "kio/job.h"
#include "scheduler.h"
#include "kmimemagic.h"

#include "kio/renamedlg.h"
#include "kio/skipdlg.h"
#include "kio/observer.h"

using namespace KIO;

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( packedArgs, IO_WriteOnly ); stream

Job::Job(bool showProgressInfo) : QObject(0, "job"), m_error(0)
{
    // All jobs delete themselves after emiting 'result'.

    // Notify the UI Server and get an id
    if (showProgressInfo)
        id = Observer::self()->newJob( this );
}

void Job::addSubjob(Job *job)
{
    kdDebug(7007) << "addSubJob(" << job << ") this = " << this << endl;
    subjobs.append(job);
    connect(job, SIGNAL(result(KIO::Job*)),
	    SLOT(slotResult(KIO::Job*)));
}

void Job::removeSubjob( Job *job )
{
    kdDebug(7007) << "removeSubJob(" << job << ") this = " << this << "  subjobs = " << subjobs.count() << endl;
    subjobs.remove(job);
    if (subjobs.isEmpty())
    {
	emit result(this);
        delete this; // Suicide is painless
    }
}

void Job::kill()
{
  // kill all subjobs
  QListIterator<Job> it( subjobs );
  for ( ; it.current() ; ++it )
     (*it)->kill();
  delete this;
}

void Job::slotResult( Job *job )
{
    // Did job have an error ?
    if ( job->error() && !m_error )
    {
	// Store it in the parent only if first error
	m_error = job->error();
	m_errorText = job->errorText();
    }
    removeSubjob(job);
}

//Job::errorString is implemented in global.cpp

void Job::showErrorDialog( QWidget * parent )
{
  kapp->enableStyles();
  // Show a message box, except for "user canceled"
  if ( m_error != ERR_USER_CANCELED )
      KMessageBox::error( parent, errorString() );
}

SimpleJob::SimpleJob(const KURL& url, int command, const QByteArray &packedArgs,
                     bool showProgressInfo )
  : Job(showProgressInfo), m_slave(0), m_packedArgs(packedArgs),
    m_url(url), m_command(command), m_totalSize(0), m_percent(0)
{
    if (m_url.isMalformed())
    {
        m_error = ERR_MALFORMED_URL;
        m_errorText = m_url.url();
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
    } else
        Scheduler::doJob(this);
}

void SimpleJob::kill()
{
    Scheduler::cancelJob( this ); // deletes the slave if not 0
    m_slave = 0; // -> set to 0
    Job::kill();
}

SimpleJob::~SimpleJob()
{
    if (m_slave) // was running
    {
        m_slave->kill();
        Scheduler::jobFinished( this, m_slave ); // deletes the slave
        m_slave = 0; // -> set to 0
    }
}

void SimpleJob::start(Slave *slave)
{
    m_slave = slave;
    connect( m_slave, SIGNAL( error( int , const QString & ) ),
	     SLOT( slotError( int , const QString & ) ) );

    connect( m_slave, SIGNAL( finished() ),
	     SLOT( slotFinished() ) );

    connect( m_slave, SIGNAL( totalSize( unsigned long ) ),
	     SLOT( slotTotalSize( unsigned long ) ) );

    connect( m_slave, SIGNAL( processedSize( unsigned long ) ),
	     SLOT( slotProcessedSize( unsigned long ) ) );

    connect( m_slave, SIGNAL( speed( unsigned long ) ),
	     SLOT( slotSpeed( unsigned long ) ) );

    m_slave->connection()->send( m_command, m_packedArgs );
}

void SimpleJob::slotFinished( )
{
    // Return slave to the scheduler
    if(m_slave) {
        Scheduler::jobFinished( this, m_slave );
        m_slave = 0;
    }

    if (subjobs.isEmpty())
    {
	emit result(this);
        delete this; // Suicide is painless
    }
}

void SimpleJob::slotError( int error, const QString & errorText )
{
    m_error = error;
    m_errorText = errorText;
    // error terminates the job
    slotFinished();
}

void SimpleJob::slotTotalSize( unsigned long size )
{
  m_totalSize = size;
  emit totalSize( this, size );
}

void SimpleJob::slotProcessedSize( unsigned long size )
{
  emit processedSize( this, size );

  // calculate percents
  uint ipercent = m_percent;

  if ( m_totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (uint)(( (float)size / (float)m_totalSize ) * 100.0);
  
  if ( m_percent > ipercent ) {
    emit percent( this, m_percent );
  }
}

void SimpleJob::slotSpeed( unsigned long bytes_per_second )
{
  emit speed( this, bytes_per_second );
}

SimpleJob *KIO::mkdir( const KURL& url, int permissions )
{
    kdDebug(7007) << "mkdir " << debugString(url.url()) << endl;
    KIO_ARGS << url.path() << permissions;
    SimpleJob * job = new SimpleJob(url, CMD_MKDIR, packedArgs);
    return job;
}

SimpleJob *KIO::rmdir( const KURL& url )
{
    kdDebug(7007) << "rmdir " << debugString(url.url()) << endl;
    KIO_ARGS << url.path() << Q_INT8(false); // isFile is false
    return new SimpleJob(url, CMD_DEL, packedArgs);
}

SimpleJob *KIO::chmod( const KURL& url, int permissions )
{
    kdDebug(7007) << "chmod " << debugString(url.url()) << endl;
    KIO_ARGS << url.path() << permissions;
    SimpleJob * job = new SimpleJob(url, CMD_CHMOD, packedArgs);
    return job;
}

SimpleJob *KIO::special(const KURL& url, const QByteArray & data)
{
    kdDebug(7007) << "special " << debugString(url.url()) << endl;
    SimpleJob * job = new SimpleJob(url, CMD_SPECIAL, data);
    return job;
}

SimpleJob *KIO::mount( bool ro, const char *fstype, const QString& dev, const QString& point )
{
    KIO_ARGS << int(1) << Q_INT8( ro ? 1 : 0 )
             << fstype << dev << point;
    return special( KURL("file:/"), packedArgs );
}

SimpleJob *KIO::unmount( const QString& point )
{
    KIO_ARGS << int(2) << point;
    return special( KURL("file:/"), packedArgs );
}

//////////

StatJob::StatJob( const KURL& url, int command,
                  const QByteArray &packedArgs )
    : SimpleJob(url, command, packedArgs)
{
}

void StatJob::start(Slave *slave)
{
    SimpleJob::start(slave);

    connect( m_slave, SIGNAL( statEntry( const KIO::UDSEntry& ) ),
             SLOT( slotStatEntry( const KIO::UDSEntry & ) ) );
}

void StatJob::slotStatEntry( const KIO::UDSEntry & entry )
{
    kdDebug(7007) << "StatJob::slotStatEntry" << endl;
    m_statResult = entry;
}

StatJob *KIO::stat(const KURL& url )
{
    kdDebug(7007) << "stat " << debugString(url.url()) << endl;
    KIO_ARGS << url.path();
    StatJob * job = new StatJob(url, CMD_STAT, packedArgs);
    return job;
}

//////////

TransferJob::TransferJob( const KURL& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData)
    : SimpleJob(url, command, packedArgs), staticData( _staticData)
{
    m_suspended = false;
}

// Slave sends data
void TransferJob::slotData( const QByteArray &_data)
{
    emit data( this, _data);
}

// Slave got a redirection request
void TransferJob::slotRedirection( const KURL &url)
{
    kdDebug(7007) << "TransferJob::slotRedirection(" << url.url() << ")" << endl;
    m_redirectionURL = url;
    // We'll remember that when the job finishes
}

void TransferJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || m_error )
        SimpleJob::slotFinished();
    else {
        // tell the user that we haven't finished yet
        emit redirection(m_redirectionURL);
        // Honour the redirection
        // We take the approach of "redirecting this same job"
        // Another solution would be to create a subjob, but the same problem
        // happens (unpacking+repacking)
        staticData.truncate(0);
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        // The very tricky part is the packed arguments business
        QString dummyStr;
        QDataStream istream( m_packedArgs, IO_ReadOnly );
        switch( m_command ) {
            case CMD_GET: {
                Q_INT8 iReload;
                istream >> dummyStr >> dummyStr /*do we keep the query?*/ >> iReload;
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << m_url.path() << m_url.query() << iReload;
                break;
            }
            case CMD_PUT: {
                int permissions;
                Q_INT8 iOverwrite, iResume;
                istream >> iOverwrite >> iResume >> permissions >> dummyStr;
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << iOverwrite << iResume << permissions << m_url.path();
                break;
            }
            case CMD_SPECIAL: {
                int specialcmd;
                istream >> specialcmd;
                assert(specialcmd == 1); // you have to switch() here if other cmds are added
                istream >> dummyStr >> dummyStr;
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << specialcmd << m_url.path() << m_url.query();
                break;
            }
        }

        m_slave = 0L;
        Scheduler::doJob(this);
    }
}

// Slave requests data
void TransferJob::slotDataReq()
{
    QByteArray dataForSlave;
    if (!staticData.isEmpty())
    {
       dataForSlave = staticData;
       staticData = QByteArray();
    }
    else
    {
       emit dataReq( this, dataForSlave);
    }
    m_slave->connection()->send( MSG_DATA, dataForSlave );
}

void TransferJob::suspend()
{
    m_suspended = true;
    if (m_slave)
       m_slave->connection()->suspend();
}

void TransferJob::resume()
{
    m_suspended = false;
    if (m_slave)
       m_slave->connection()->resume();
}

void TransferJob::start(Slave *slave)
{
    assert(slave);

    connect( slave, SIGNAL( data( const QByteArray & ) ),
	     SLOT( slotData( const QByteArray & ) ) );

    connect( slave, SIGNAL( dataReq() ),
	     SLOT( slotDataReq() ) );

    connect( slave, SIGNAL( redirection(const KURL &) ),
	     SLOT( slotRedirection(const KURL &) ) );

    SimpleJob::start(slave);
    if (m_suspended)
       slave->connection()->suspend();
}

TransferJob *KIO::get( const KURL& url, bool reload )
{
    // Send decoded path and encoded query
    KIO_ARGS << url.path() << url.query() << Q_INT8( reload ? 1 : 0);
    TransferJob * job = new TransferJob( url, CMD_GET, packedArgs );
    return job;
}

TransferJob *KIO::http_post( const KURL& url, const QByteArray &postData )
{
    assert( url.protocol() == "http" );
    // Send http post command (1), decoded path and encoded query
    KIO_ARGS << (int)1 << url.path() << url.query();
    TransferJob * job = new TransferJob( url, CMD_SPECIAL,
                                         packedArgs, postData );
    return job;
}

TransferJob *KIO::put( const KURL& url, int permissions,
                  bool overwrite, bool resume )
{
    KIO_ARGS << Q_INT8( overwrite ? 1 : 0 ) << Q_INT8( resume ? 1 : 0 ) << permissions << url.path();
    TransferJob * job = new TransferJob( url, CMD_PUT, packedArgs );
    return job;
}

//////////

MimetypeJob::MimetypeJob( const KURL& url, int command,
                  const QByteArray &packedArgs )
    : TransferJob(url, command, packedArgs)
{
}

// Slave sends data
void MimetypeJob::slotData( KIO::Job *, const QByteArray &_data)
{
    if (m_mimetype.isEmpty())
    {
       kdDebug(7007) << "MimetypeJob::slotData() size = " << _data.size() << endl;
       KMimeMagicResult* result = KMimeMagic::self()->findBufferType( _data );

       // If we still did not find it, we must assume the default mime type
       if ( !result || result->mimeType().isEmpty())
          m_mimetype = QString::fromLatin1("application/octet-stream");
       else
          m_mimetype = result->mimeType();
    }
}

void MimetypeJob::start(Slave *slave)
{
    TransferJob::start(slave);

    connect( m_slave, SIGNAL(mimeType( const QString& ) ),
             SLOT( slotMimetype( const QString& ) ) );
}

void MimetypeJob::slotMimetype( const QString& mimetype )
{
    kdDebug(7007) << "MimetypeJob::slotMimetype(" << mimetype << ")" << endl;
    m_mimetype = mimetype;
}

void MimetypeJob::slotFinished( )
{
    kdDebug(7007) << "MimetypeJob::slotFinished()" << endl;
    // Do stuff

    // Return slave to the scheduler
    TransferJob::slotFinished();
}

MimetypeJob *KIO::mimetype(const KURL& url )
{
    kdDebug(7007) << "mimetype " << debugString(url.url()) << endl;
    KIO_ARGS << url.path();
    MimetypeJob * job = new MimetypeJob(url, CMD_MIMETYPE, packedArgs);
    return job;
}

/*
 * The FileCopyJob works according to the famous Bayern
 * 'Alternating Bittburger Protocol': we either drink a beer or we
 * we order a beer, but never both at the same time.
 * Tranlated to io-slaves: We alternate between receiving a block of data
 * and sending it away.
 */
FileCopyJob::FileCopyJob( const KURL& src, const KURL& dest, int permissions,
                          bool move, bool overwrite, bool resume)
    : Job(), m_src(src), m_dest(dest),
      m_permissions(permissions), m_move(move), m_overwrite(overwrite), m_resume(resume)
{
    kdDebug(7007) << "FileCopyJob::FileCopyJob()" << endl;
    m_moveJob = 0;
    m_copyJob = 0;
    m_getJob = 0;
    m_putJob = 0;
    m_delJob = 0;
    if ((src.protocol() == dest.protocol()) &&
        (src.host() == dest.host()) &&
        (src.port() == dest.port()) &&
        (src.user() == dest.user()) &&
        (src.pass() == dest.pass()))
    {
       if (m_move)
       {
          KIO_ARGS << src.path() << dest.path() << (Q_INT8) m_overwrite;
          m_moveJob = new SimpleJob(src, CMD_RENAME, packedArgs);
          addSubjob( m_moveJob );
       }
       else
       {
          startCopyJob();
       }
    }
    else
    {
       m_copyJob = 0;
       startDataPump();
    }
}

void FileCopyJob::startCopyJob()
{
    kdDebug(7007) << "FileCopyJob::startCopyJob()" << endl;
    KIO_ARGS << m_src.path() << m_dest.path() << m_permissions << (Q_INT8) m_overwrite;
    m_copyJob = new SimpleJob(m_src, CMD_COPY, packedArgs);
    addSubjob( m_copyJob );

    connect( m_copyJob, SIGNAL(totalSize( KIO::Job*, unsigned long )),
             this, SIGNAL( totalSize(KIO::Job*, unsigned long)) );

    connect( m_copyJob, SIGNAL(processedSize( KIO::Job*, unsigned long )),
             this, SIGNAL( processedSize(KIO::Job*, unsigned long)) );

    connect( m_copyJob, SIGNAL(percent( KIO::Job*, unsigned long )),
             this, SIGNAL( percent(KIO::Job*, unsigned long)) );
}

void FileCopyJob::startDataPump()
{
    kdDebug(7007) << "FileCopyJob::startDataPump()" << endl;
    m_getJob = get( m_src );
    kdDebug(7007) << "FileCopyJob: m_getJob = " << m_getJob << endl;
    m_putJob = put( m_dest, m_permissions, m_overwrite, m_resume);
    kdDebug(7007) << "FileCopyJob: m_putJob = " << m_putJob << endl;
    m_putJob->suspend();
    addSubjob( m_getJob );
    addSubjob( m_putJob );
    m_getJob->resume(); // Order a beer

    connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
             SLOT( slotData(KIO::Job *, const QByteArray&)));
    connect( m_putJob, SIGNAL(dataReq(KIO::Job *, QByteArray&)),
             SLOT( slotDataReq(KIO::Job *, QByteArray&)));
}

void FileCopyJob::slotData( KIO::Job * job, const QByteArray &data)
{
   kdDebug(7007) << "FileCopyJob::slotData(" << job << ")" << endl;
   kdDebug(7007) << " data size : " << data.size() << endl;
   assert(m_putJob);
   m_getJob->suspend();
   m_putJob->resume(); // Drink the beer
   m_buffer = data;
}

void FileCopyJob::slotDataReq( KIO::Job * job, QByteArray &data)
{
   kdDebug(7007) << "FileCopyJob::slotDataReq(" << job << ")" << endl;
   if (m_getJob)
   {
      m_getJob->resume(); // Order more beer
      m_putJob->suspend();
   }
   data = m_buffer;
   m_buffer = QByteArray();
}

void FileCopyJob::slotResult( KIO::Job *job)
{
   kdDebug(7007) << "FileCopyJob::slotResult(" << job << ")" << endl;
   // Did job have an error ?
   if ( job->error() )
   {
      if ((job == m_moveJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_moveJob = 0;
         startCopyJob();
         removeSubjob(job);
         return;
      }
      if ((job == m_copyJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_copyJob = 0;
         startDataPump();
         removeSubjob(job);
         return;
      }
      m_error = job->error();
      m_errorText = job->errorText();

      emit result( this );
      delete this;
      return;
   }

   if (job == m_moveJob)
   {
      m_moveJob = 0; // Finished
   }

   if (job == m_copyJob)
   {
      m_copyJob = 0;
      if (m_move)
      {
         m_delJob = file_delete( m_src ); // Delete source
         addSubjob(m_delJob);
      }
   }

   if (job == m_getJob)
   {
      m_getJob = 0; // No action required
      if (m_putJob)
         m_putJob->resume();
   }

   if (job == m_putJob)
   {
      kdDebug(7007) << "FileCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      if (m_getJob)
      {
         kdWarning(7007) << "WARNING ! Get still going on..." << endl;
         m_getJob->resume();
      }
      if (m_move)
      {
         m_delJob = file_delete( m_src ); // Delete source
         addSubjob(m_delJob);
      }
   }

   if (job == m_delJob)
   {
      m_delJob = 0; // Finished
   }
   removeSubjob(job);
}

FileCopyJob *KIO::file_copy( const KURL& src, const KURL& dest, int permissions,
                             bool overwrite, bool resume)
{
   return new FileCopyJob( src, dest, permissions, false, overwrite, resume );
}

FileCopyJob *KIO::file_move( const KURL& src, const KURL& dest, int permissions,
                             bool overwrite, bool resume)
{
   return new FileCopyJob( src, dest, permissions, true, overwrite, resume );
}

SimpleJob *KIO::file_delete( const KURL& src)
{
    KIO_ARGS << src.path() << Q_INT8(true); // isFile
    return new SimpleJob(src, CMD_DEL, packedArgs);
}

//////////

bool KIO::link( const KURL::List &srcUrls, const KURL & destDir )
{
    kdDebug(1202) << "destDir = " << destDir.url() << endl;
    bool overwriteExistingFiles = false;
    if ( destDir.isMalformed() )
    {
	KMessageBox::sorry( 0L, i18n( "Malformed URL\n%1" ).arg( destDir.url() ) );
	return false;
    }
    if ( !destDir.isLocalFile() )
    {
	// I can only make links on the local file system.
	KMessageBox::sorry( 0L, i18n( "Can only make links on local file system" ) );
	return false;
    }
    KURL::List::ConstIterator it = srcUrls.begin();
    for ( ; it != srcUrls.end() ; ++it )
    {
	KURL srcUrl( *it );
	if ( srcUrl.isMalformed() )
	{
	    KMessageBox::sorry( 0L, i18n( "Malformed URL\n%1" ).arg( (*it).url() ) );
	    return false;
	}

	// The destination URL is the destination dir + the filename
	KURL destUrl( destDir.url(1) + srcUrl.filename() );
	kdDebug(1202) << "destUrl = " << destUrl.url() << endl;

	// Do we link a file on the local disk?
	if ( srcUrl.isLocalFile() )
	{
	    // Make a symlink
	    if ( symlink( srcUrl.path().local8Bit(), destUrl.path().local8Bit() ) == -1 )
	    {
		// Does the destination already exist ?
		if ( errno == EEXIST )
		{
		    // Are we allowed to overwrite the files ?
		    if ( overwriteExistingFiles )
		    {
			// Try to delete the destination
			if ( unlink( destUrl.path().local8Bit() ) != 0 )
			{
			    KMessageBox::sorry( 0L, i18n( "Could not overwrite\n%1"), destUrl.path() );
			    return false;
			}
		    }
		    else
		    {
			// Ask the user what to do
			// TODO
			KMessageBox::sorry( 0L, i18n( "Destination exists (real dialog box not implemented yet)\n%1"), destUrl.path() );
			return false;
		    }
		}
		else
		{
		    // Some error occured while we tried to symlink
		    KMessageBox::sorry( 0L, i18n( "Failed to make symlink from \n%1\nto\n%2\n" ).
					arg(srcUrl.url()).arg(destUrl.url()) );
		    return false;
		}
	    } // else : no problem
	}
	// Make a link from a file in a tar archive, ftp, http or what ever
	else
	{
	    // Encode slashes and so on
	    QString destPath = destDir.path(1) + KIO::encodeFileName( srcUrl.url() );
	    QFile f( destPath );
	    if ( f.open( IO_ReadWrite ) )
	    {
		f.close(); // kalle
		KSimpleConfig config( destPath ); // kalle
		config.setDesktopGroup();
		config.writeEntry( "URL", srcUrl.url() );
		config.writeEntry( "Type", "Link" );
		QString protocol = srcUrl.protocol();
		if ( protocol == "ftp" )
		    config.writeEntry( "Icon", "ftp" );
		else if ( protocol == "http" )
		    config.writeEntry( "Icon", "www" );
		else if ( protocol == "info" )
		    config.writeEntry( "Icon", "info" );
		else if ( protocol == "mailto" )   // sven:
		    config.writeEntry( "Icon", "kmail" ); // added mailto: support
		else
		    config.writeEntry( "Icon", "unknown" );
		config.sync();
	    }
	    else
	    {
		KMessageBox::sorry( 0L, i18n( "Could not write to\n%1").arg(destPath) );
		return false;
	    }
	}
    }
    return true;
}

//////////

ListJob::ListJob(const KURL& u, bool showProgressInfo, bool _recursive, QString _prefix) :
    SimpleJob(u, CMD_LISTDIR, QByteArray()),
    recursive(_recursive), prefix(_prefix)
{
    // We couldn't set the args when calling the parent constructor,
    // so do it now.
    QDataStream stream( m_packedArgs, IO_WriteOnly ); stream << u.path();

    if (showProgressInfo)
    {
      connect( this, SIGNAL( entries ( KIO::Job *, const KIO::UDSEntryList & ) ),
               Observer::self(), SLOT( slotEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
      connect( this, SIGNAL( totalEntries ( KIO::Job *, unsigned long ) ),
               Observer::self(), SLOT( slotTotalEntries( KIO::Job *, unsigned long ) ) );
    }
}

void ListJob::slotListEntries( const KIO::UDSEntryList& list )
{
    if (recursive) {
	UDSEntryListIterator it(list);

	for (; it.current(); ++it) {
	    bool isDir = false;
	    bool isLink = false;
	    QString filename;
	
	    UDSEntry::ConstIterator it2 = it.current()->begin();
	    for( ; it2 != it.current()->end(); it2++ ) {
		switch( (*it2).m_uds ) {
                    case UDS_FILE_TYPE:
                        isDir = S_ISDIR((*it2).m_long);
                        break;
                    case UDS_NAME:
                        filename = (*it2).m_str;
                        break;
                    case UDS_LINK_DEST:
                        // This is a link !!! Don't follow !
                        isLink = !(*it2).m_str.isEmpty();
                        break;
                    default:
                        break;
		}
	    }
	    if (isDir && !isLink) {
		if (filename != ".." && filename != ".") {
 		    KURL newone = url();
		    newone.addPath(filename);
		    ListJob *job = new ListJob(newone, true, prefix + filename + "/");
		    connect(job, SIGNAL(entries( KIO::Job *,
						 const KIO::UDSEntryList& )),
			    SLOT( gotEntries( KIO::Job*,
					      const KIO::UDSEntryList& )));
		    addSubjob(job);
		}
	    }
	}
    }

    // Not recursive, or top-level of recursive listing : return now (send . and .. as well)
    if (prefix.isNull()) {
	emit entries(this, list);
	return;
    }

    UDSEntryList newlist;

    UDSEntryListIterator it(list);
    for (; it.current(); ++it) {
	
	UDSEntry *newone = new UDSEntry(*it.current());
	UDSEntry::Iterator it2 = newone->begin();
        QString filename;
	for( ; it2 != newone->end(); it2++ ) {
	    if ((*it2).m_uds == UDS_NAME) {
                filename = (*it2).m_str;
		(*it2).m_str = prefix + filename;
            }
	}
        // Avoid returning entries like subdir/. and subdir/..
        if (filename != ".." && filename != ".")
            newlist.append(newone);
    }

    emit entries(this, newlist);

}

void ListJob::gotEntries(KIO::Job *, const KIO::UDSEntryList& list )
{
    // Forward entries received by subjob - faking we received them ourselves
    emit entries(this, list);
}

void ListJob::slotResult( KIO::Job * job )
{
    // If we can't list a subdir, the result is still ok
    // This is why we override Job::slotResult() - to skip error checking
    removeSubjob( job );
}

void ListJob::slotTotalEntries( unsigned long count )
{
    emit totalEntries( this, count );
}

ListJob *KIO::listDir( const KURL& url, bool showProgressInfo )
{
    ListJob * job = new ListJob(url, showProgressInfo);
    return job;
}

ListJob *KIO::listRecursive( const KURL& url, bool showProgressInfo)
{
    ListJob * job = new ListJob(url, showProgressInfo, true);
    return job;
}

void ListJob::start(Slave *slave)
{
    connect( slave, SIGNAL( listEntries( const KIO::UDSEntryList& )),
	     SLOT( slotListEntries( const KIO::UDSEntryList& )));
    connect( slave, SIGNAL( totalEntries( unsigned long ) ),
             SLOT( slotTotalEntries( unsigned long ) ) );
    SimpleJob::start(slave);
}


CopyJob::CopyJob( const KURL::List& src, const KURL& dest, bool move, bool showProgressInfo )
  : Job(), m_move(move),
    destinationState(DEST_NOT_STATED), state(STATE_STATING),
      m_totalSize(0), m_processedSize(0), m_srcList(src), m_dest(dest),
      m_bAutoSkip( false ), m_bOverwriteAll( false )
{
  if ( showProgressInfo ) {
    connect( this, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( percent( KIO::Job*, unsigned int ) ),
	     Observer::self(), SLOT( slotPercent( KIO::Job*, unsigned int ) ) );

    connect( this, SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	     Observer::self(), SLOT( slotCopying( KIO::Job*, const KURL&, const KURL& ) ) );
    connect( this, SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	     Observer::self(), SLOT( slotMoving( KIO::Job*, const KURL&, const KURL& ) ) );
    connect( this, SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
	     Observer::self(), SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

    connect( this, SIGNAL( canResume( KIO::Job*, bool ) ),
	     Observer::self(), SLOT( slotCanResume( KIO::Job*, bool ) ) );
  }
    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest );
    kdDebug(7007) << "CopyJob:stating the dest " << m_dest.url() << endl;
    addSubjob(job);
}

void CopyJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryListIterator it(list);
    for (; it.current(); ++it) {
        UDSEntry::ConstIterator it2 = it.current()->begin();
        struct CopyInfo info;
        QString relName;
        for( ; it2 != it.current()->end(); it2++ ) {
            switch ((*it2).m_uds) {
                case UDS_FILE_TYPE:
                    info.type = (mode_t)((*it2).m_long);
                    break;
                case UDS_NAME:
                    relName = (*it2).m_str;
                    break;
                case UDS_LINK_DEST:
                    info.linkDest = (*it2).m_str;
                    break;
                case UDS_ACCESS:
                    info.permissions = (mode_t)((*it2).m_long);
                    break;
                case UDS_SIZE:
                    info.size = (off_t)((*it2).m_long);
                    m_totalSize += info.size;
                    break;
                case UDS_MODIFICATION_TIME:
                    info.mtime = (time_t)((*it2).m_long);
                case UDS_CREATION_TIME:
                    info.ctime = (time_t)((*it2).m_long);
                default:
                    break;
            }
        }
        if (relName != ".." && relName != ".")
        {
            kdDebug(7007) << "CopyJob::slotEntries " << relName << endl;
            info.uSource = ((SimpleJob *)job)->url();
            if ( m_bCurrentSrcIsDir ) // Only if src is a directory. Otherwise uSource is fine as is
                info.uSource.addPath( relName );
            info.uDest = m_currentDest;
            // Append filename or dirname to destination URL, except for links
            // (This is due to the way ::link is written. Alternatively,
            // we could change that...)
            if ( info.linkDest.isEmpty() && destinationState == DEST_IS_DIR )
                info.uDest.addPath( relName );
            if ( info.linkDest.isEmpty() && (S_ISDIR(info.type)) )
            {
                dirs.append( info ); // Directories
                if (m_move)
                    dirsToRemove.append( info.uSource );
            }
            else
                files.append( info ); // Files and any symlinks
        }
    }
}

void CopyJob::startNextJob()
{
    files.clear();
    dirs.clear();
    KURL::List::Iterator it = m_srcList.begin();
    if (it != m_srcList.end())
    {
        // First, stat the src
        Job * job = KIO::stat( *it );
        kdDebug(7007) << "KIO::stat on " << (*it).url() << endl;
        state = STATE_STATING;
        addSubjob(job);
        // keep src url in the list, just in case we need it later
    } else
    {
        emit result(this);
        delete this;
    }
}

void CopyJob::slotResultStating( Job *job )
{
    kdDebug(7007) << "CopyJob::slotResultStating" << endl;
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        // Probably : src doesn't exist
        Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }

    // Is it a file or a dir ?
    UDSEntry entry = ((StatJob*)job)->statResult();
    bool bDir = false;
    bool bLink = false;
    UDSEntry::ConstIterator it2 = entry.begin();
    for( ; it2 != entry.end(); it2++ ) {
        if ( ((*it2).m_uds) == UDS_FILE_TYPE )
            bDir = S_ISDIR( (mode_t)(*it2).m_long );
        else if ( ((*it2).m_uds) == UDS_LINK_DEST )
            bLink = !((*it2).m_str.isEmpty());
    }

    if ( destinationState == DEST_NOT_STATED )
        // we were stating the dest
    {
        if (job->error())
            destinationState = DEST_DOESNT_EXIST;
        else
            // Treat symlinks to dirs as dirs here, so no test on bLink
            destinationState = bDir ? DEST_IS_DIR : DEST_IS_FILE;
        subjobs.remove( job );
        assert ( subjobs.isEmpty() ); // We should have only one job at a time ...
        startNextJob();
        return;
    }
    // We were stating the current source URL
    m_currentDest = m_dest;
    // Create a dummy list with it, for slotEntries
    UDSEntryList lst;
    lst.append(new UDSEntry(entry));

    // There 6 cases, and all end up calling slotEntries(job, lst) first :
    // 1 - src is a dir, destination is a directory,
    // slotEntries will append the source-dir-name to the destination
    // 2 - src is a dir, destination is a file, ERROR (done later on)
    // 3 - src is a dir, destination doesn't exist, then it's the destination dirname,
    // so slotEntries will use it as destination.

    // 4 - src is a file, destination is a directory,
    // slotEntries will append the filename to the destination.
    // 5 - src is a file, destination is a file, m_dest is the exact destination name
    // 6 - src is a file, destination doesn't exist, m_dest is the exact destination name
    // Tell slotEntries not to alter the src url
    m_bCurrentSrcIsDir = false;
    slotEntries(job, lst);

    KURL srcurl = ((SimpleJob*)job)->url();

    subjobs.remove( job );
    assert ( subjobs.isEmpty() ); // We should have only one job at a time ...

    if ( bDir && !bLink ) // treat symlinks as files here (no recursion)
    {
        kdDebug(7007) << " Source is a directory " << endl;

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
            // Use <desturl>/<directory_copied> as destination, from now on
            m_currentDest.addPath( srcurl.filename() );
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            m_error = ERR_IS_FILE;
            emit result(this);
            return;
        }
        else // (case 3)
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;

        state = STATE_LISTING;
        ListJob *newjob = listRecursive( srcurl );
        connect(newjob, SIGNAL(entries( KIO::Job *,
                                        const KIO::UDSEntryList& )),
                SLOT( slotEntries( KIO::Job*,
                                   const KIO::UDSEntryList& )));
        addSubjob( newjob );
    }
    else
    {
        kdDebug(7007) << " Source is a file (or a symlink) " << endl;

        // Skip the "listing" stage and go directly copying the file
        state = STATE_COPYING_FILES;
        copyNextFile();
    }
}

void CopyJob::slotResultCreatingDirs( Job * job )
{
    // The dir we are trying to create:
    QValueList<CopyInfo>::Iterator it = dirs.begin();
    // Was there an error creating a dir ?
    if ( job->error() )
    {
        m_conflictError = job->error();
        if ( (m_conflictError == ERR_DIR_ALREADY_EXIST)
             || (m_conflictError == ERR_FILE_ALREADY_EXIST) )
        {
            QString oldPath = ((SimpleJob*)job)->url().path( 1 );
            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // We dont want to copy files in this directory, so we put it on the skip list
                m_skipList.append( oldPath );
                dirs.remove( it ); // Move on to next dir
            } else if ( m_bOverwriteAll ) { // overwrite all => just skip
                dirs.remove( it ); // Move on to next dir
            } else
            {
                assert( ((SimpleJob*)job)->url().url() == (*it).uDest.url() );
                subjobs.remove( job );
                assert ( subjobs.isEmpty() ); // We should have only one job at a time ...

                // We need to stat the existing dir, to get its last-modification time
                KURL existingDest( (*it).uDest );
                Job * newJob = KIO::stat( existingDest );
                kdDebug(7007) << "KIO::stat for resolving conflict on " << existingDest.url() << endl;
                state = STATE_CONFLICT_CREATING_DIRS;
                addSubjob(newJob);
                return; // Don't move to next dir yet !
            }
        }
        else
        {
            // Severe error, abort
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
    }
    else // no error : remove from list, to move on to next dir
        dirs.remove( it );

    subjobs.remove( job );
    assert ( subjobs.isEmpty() ); // We should have only one job at a time ...
    createNextDir();
}

void CopyJob::slotResultConflictCreatingDirs( KIO::Job * job )
{
    // We come here after a conflict has been detected and we've stated the existing dir

    // The dir we were trying to create:
    QValueList<CopyInfo>::Iterator it = dirs.begin();
    // Its modification time:
    time_t destmtime = (time_t)-1;
    time_t destctime = (time_t)-1;
    unsigned long destsize = 0;
    UDSEntry entry = ((KIO::StatJob*)job)->statResult();
    KIO::UDSEntry::ConstIterator it2 = entry.begin();
    for( ; it2 != entry.end(); it2++ ) {
        switch ((*it2).m_uds) {
            case UDS_MODIFICATION_TIME:
                destmtime = (time_t)((*it2).m_long);
                break;
            case UDS_CREATION_TIME:
                destctime = (time_t)((*it2).m_long);
                break;
            case UDS_SIZE:
                destsize = (*it2).m_long;
                break;
        }
    }
    subjobs.remove( job );
    assert ( subjobs.isEmpty() ); // We should have only one job at a time ...

    // Always multi and skip (since there are files after that)
    RenameDlg_Mode mode = (RenameDlg_Mode)( M_MULTI | M_SKIP );
    // Overwrite only if the existing thing is a dir (no chance with a file)
    if ( m_conflictError == ERR_DIR_ALREADY_EXIST )
        mode = (RenameDlg_Mode)( mode | M_OVERWRITE );

    QString existingDest = (*it).uDest.path();
    QString newPath;
    RenameDlg_Result r = open_RenameDlg( i18n("Directory already exists"),
                                         (*it).uSource.url(), existingDest, mode, newPath,
                                         (*it).size, destsize,
                                         (*it).ctime, destctime,
                                         (*it).mtime, destmtime );
    switch ( r ) {
        case R_CANCEL:
            m_error = ERR_USER_CANCELED;
            emit result(this);
            delete this;
            return;
        case R_RENAME:
        {
            QString oldPath = (*it).uDest.path( 1 );
            KURL newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            // Change the current one and strip the trailing '/'
            (*it).uDest = newUrl.path( -1 );
            newPath = newUrl.path( 1 ); // With trailing slash
            QValueList<CopyInfo>::Iterator renamedirit = it;
            renamedirit++;
            // Change the name of subdirectories inside the directory
            for( ; renamedirit != dirs.end() ; ++renamedirit )
            {
                QString path = (*renamedirit).uDest.path();
                if ( strncmp( path, oldPath, oldPath.length() ) == 0 )
                    (*renamedirit).uDest.setPath( path.replace( 0, oldPath.length(), newPath ) );
            }
            // Change filenames inside the directory
            QValueList<CopyInfo>::Iterator renamefileit = files.begin();
            for( ; renamefileit != files.end() ; ++renamefileit )
            {
                QString path = (*renamefileit).uDest.path();
                if ( strncmp( path, oldPath, oldPath.length() ) == 0 )
                    (*renamefileit).uDest.setPath( path.replace( 0, oldPath.length(), newPath ) );
            }
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            // Move on to next dir
            dirs.remove( it );
            break;
        case R_OVERWRITE:
            m_overwriteList.append( existingDest );
            // Move on to next dir
            dirs.remove( it );
            break;
        case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            // Move on to next dir
            dirs.remove( it );
            break;
        default:
            assert( 0 );
    }
    state = STATE_CREATING_DIRS;
    createNextDir();
}

void CopyJob::createNextDir()
{
    // Take first dir to create out of list
    QValueList<CopyInfo>::Iterator it = dirs.begin();
    bool bCreateDir = false; // get in the loop
    QString dir = (*it).uDest.path();
    // Is this URL on the skip list or the overwrite list ?
    while( it != dirs.end() && !bCreateDir )
    {
        bCreateDir = true; // we'll create it if it's not in any list

        QStringList::Iterator sit = m_skipList.begin();
        for( ; sit != m_skipList.end() && bCreateDir; sit++ )
            // Is dir a subdirectory of *sit ?
            if ( qstrncmp( *sit, dir, (*sit).length() ) == 0 )
                bCreateDir = false; // skip this dir

        /* Don't look on the overwrite list. If a/ exists, we must still create a/b/
           (David)
        sit = m_overwriteList.begin();
        for( ; sit != m_overwriteList.end() && bCreateDir; sit++ )
            if ( strncmp( *sit, dir, (*sit).length() ) == 0 )
                bCreateDir = false; // overwrite -> it exists
        */

        if ( !bCreateDir ) {
            dirs.remove( it );
            it = dirs.begin();
        }
    }
    if ( bCreateDir ) // any dir to create, finally ?
    {
        // Create the directory - with default permissions so that we can put files into it
        // TODO : change permissions once all is finished
        KIO::Job * newjob = KIO::mkdir( (*it).uDest, -1 );
	emit creatingDir( this, (*it).uDest);
        addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        state = STATE_COPYING_FILES;
        copyNextFile();
    }
}

void CopyJob::slotResultCopyingFiles( Job * job )
{
    // The file we were trying to copy:
    QValueList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        m_conflictError = job->error(); // save for later
        // Existing dest ?
        if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
          || ( m_conflictError == ERR_DIR_ALREADY_EXIST ) )
        {
            // Should we skip automatically ?
            if ( m_bAutoSkip )
                files.remove( it ); // Move on to next file
            else
            {
                subjobs.remove( job );
                assert ( subjobs.isEmpty() );
                // We need to stat the existing file, to get its last-modification time
                KURL existingFile( (*it).uDest );
                Job * newJob = KIO::stat( existingFile );
                kdDebug(7007) << "KIO::stat for resolving conflict on " << existingFile.url() << endl;
                state = STATE_CONFLICT_COPYING_FILES;
                addSubjob(newJob);
                return; // Don't move to next file yet !
            }
        }
        else
        {
            // Go directly to the conflict resolution, there is nothing to stat
            slotResultConflictCopyingFiles( job );
            return;
        }
    } else // no error : remove from list, to move on to next file
        files.remove( it );

    kdDebug() << "" << files.count() << " files remaining" << endl;
    subjobs.remove( job );
    assert ( subjobs.isEmpty() ); // We should have only one job at a time ...
    copyNextFile();
}

void CopyJob::slotResultConflictCopyingFiles( KIO::Job * job )
{
    // We come here after a conflict has been detected and we've stated the existing file
    // The file we were trying to create:
    QValueList<CopyInfo>::Iterator it = files.begin();

    RenameDlg_Result res;
    QString newPath;

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
      || ( m_conflictError == ERR_DIR_ALREADY_EXIST ) )
    {
        // Its modification time:
        time_t destmtime = (time_t)-1;
        time_t destctime = (time_t)-1;
        unsigned long destsize = 0;
        UDSEntry entry = ((KIO::StatJob*)job)->statResult();
        KIO::UDSEntry::ConstIterator it2 = entry.begin();
        for( ; it2 != entry.end(); it2++ ) {
            switch ((*it2).m_uds) {
                case UDS_MODIFICATION_TIME:
                    destmtime = (time_t)((*it2).m_long);
                    break;
                case UDS_CREATION_TIME:
                    destctime = (time_t)((*it2).m_long);
                    break;
                case UDS_SIZE:
                    destsize = (*it2).m_long;
                    break;
            }
        }

        // Offer overwrite only if the existing thing is a file
        RenameDlg_Mode mode = (RenameDlg_Mode)
            ( m_conflictError == ERR_FILE_ALREADY_EXIST ? M_OVERWRITE : 0 );
        if ( files.count() > 0 ) // Not last one
            mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );
        else
            mode = (RenameDlg_Mode) ( mode | M_SINGLE );
        res = open_RenameDlg( m_conflictError == ERR_FILE_ALREADY_EXIST ?
                                i18n("File already exists") : i18n("Already exists as a directory"),
                              (*it).uSource.url(), (*it).uDest.path(), mode, newPath,
                              (*it).size, destsize,
                              (*it).ctime, destctime,
                              (*it).mtime, destmtime );
    }
    else
    {
        SkipDlg_Result skipResult = open_SkipDlg( files.count() > 0,
                                                  job->errorString() );

        // Convert the return code from SkipDlg into a RenameDlg code
        res = ( skipResult == S_SKIP ) ? R_SKIP :
            ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
            R_CANCEL;
    }

    subjobs.remove( job );
    assert ( subjobs.isEmpty() );
    switch ( res ) {
        case R_CANCEL:
            m_error = ERR_USER_CANCELED;
            emit result(this);
            delete this;
            return;
        case R_RENAME:
        {
            KURL newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            (*it).uDest = newUrl;
            // emit renamed ??
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            files.remove( it );
            break;
       case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            break;
        case R_OVERWRITE:
            // Add to overwrite list, so that copyNextFile knows to overwrite
            m_overwriteList.append( (*it).uDest.path() );
            break;
        default:
            assert( 0 );
    }
    state = STATE_COPYING_FILES;
    copyNextFile();
}

void CopyJob::copyNextFile()
{
    kdDebug(7007) << "CopyJob::copyNextFile()" << endl;
    // Take the first file in the list
    QValueList<CopyInfo>::Iterator it = files.begin();
    bool bCopyFile = false; // get into the loop
    QString destFile = (*it).uDest.path();
    // Is this URL on the skip list ?
    while (it != files.end() && !bCopyFile)
    {
        bCopyFile = true;

        QStringList::Iterator sit = m_skipList.begin();
        for( ; sit != m_skipList.end() && bCopyFile; sit++ )
            // Is destFile in *sit (or a subdirectory of *sit) ?
            if ( qstrncmp( *sit, destFile, (*sit).length() ) == 0 )
                bCopyFile = false; // skip this file

        if (!bCopyFile) {
            files.remove( it );
            it = files.begin();
        }
    }

    if (bCopyFile) // any file to create, finally ?
    {
        // Do we set overwrite ?
        bool bOverwrite = m_bOverwriteAll; // yes if overwrite all
        // or if on the overwrite list
        QStringList::Iterator sit = m_overwriteList.begin();
        for( ; sit != m_overwriteList.end() && !bOverwrite; sit++ )
            if ( strncmp( *sit, destFile, (*sit).length() ) == 0 )
                bOverwrite = true;

        KIO::Job * newjob;
        if ( !(*it).linkDest.isEmpty() ) // Copying a symlink
        {
            KURL::List srcList;
            // The "source" is in fact what the existing link points to
            srcList.append( KURL( (*it).linkDest ) );
            if ( KIO::link( srcList, (*it).uDest ) )
            {
                if (m_move)
                {
                    newjob = KIO::del( (*it).uSource );
                }
                else
                {
                    // Done with this one
                    files.remove( it );
                    copyNextFile();
                    return;
                }
            } else
            {
                // Error - move to next file
                files.remove( it );
                copyNextFile();
                return;
            }
        } else if (m_move) // Moving a file
        {
            newjob = KIO::file_move( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false );
            kdDebug() << "CopyJob::copyNextFile : Moving " << (*it).uSource.url() << " to " << (*it).uDest.url() << endl;
	    emit moving( this, (*it).uSource, (*it).uDest );
        }
        else // Copying a file
        {
            newjob = KIO::file_copy( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false );
            kdDebug() << "CopyJob::copyNextFile : Copying " << (*it).uSource.url() << " to " << (*it).uDest.url() << endl;
	    emit copying( this, (*it).uSource, (*it).uDest );
        }
        addSubjob(newjob);
	connect( newjob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
		 this, SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
    }
    else
    {
        if ( m_move ) // moving ? We need to delete dirs
        {
            kdDebug(7007) << "copyNextFile finished, deleting dirs" << endl;
            state = STATE_DELETING_DIRS;
            deleteNextDir();
        } else {
            // When we're done : move on to next src url
            kdDebug(7007) << "copyNextFile finished, moving to next url" << endl;
            m_srcList.remove(m_srcList.begin());
            startNextJob();
        }
    }
}

void CopyJob::deleteNextDir()
{
    if ( !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        // Take first dir to delete out of list - last ones first !
        KURL::List::Iterator it = dirsToRemove.fromLast();
        SimpleJob *job = KIO::rmdir( *it );
        dirsToRemove.remove(it);
        addSubjob( job );
    }
    else // We have finished deleting
    {
        m_srcList.remove(m_srcList.begin()); // done with this url
        startNextJob();
    }
}

void CopyJob::slotProcessedSize( KIO::Job*, unsigned long data_size )
{
  kdDebug(7007) << "CopyJob::slotProcessedSize " << data_size << endl;
  m_processedSize += data_size;
  emit processedSize( this, m_processedSize );

  // calculate percents
  uint ipercent = m_percent;

  if ( m_totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (uint)(( (float)m_processedSize / (float)m_totalSize ) * 100.0);
  
  if ( m_percent > ipercent ) {
    emit percent( this, m_percent );
  }
}

void CopyJob::slotSpeed( KIO::Job*, unsigned long bytes_per_second )
{
  kdDebug(7007) << "CopyJob::slotSpeed " << bytes_per_second << endl;
  emit speed( this, bytes_per_second );
}

void CopyJob::slotResultDeletingDirs( Job * job )
{
    if (job->error())
    {
        // Couldn't remove directory. Well, perhaps it's not empty
        // because the user pressed Skip for a given file in it.
        // Let's not display "Could not remove dir ..." for each of those dir !
    }
    subjobs.remove( job );
    assert ( subjobs.isEmpty() );
    deleteNextDir();
}

void CopyJob::slotResult( Job *job )
{
    kdDebug() << "CopyJob::slotResult() state=" << state << endl;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - subjobs.remove(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_LISTING: // recursive listing finished
            debug("totalSize: %ld files: %d dirs: %d", m_totalSize, files.count(), dirs.count());
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            subjobs.remove( job );
            assert ( subjobs.isEmpty() ); // We should have only one job at a time ...

	    // emit all signals for total numbers
	    emit totalSize( this, m_totalSize );
	    emit totalFiles( this, files.count() );
	    emit totalDirs( this, dirs.count() );

            state = STATE_CREATING_DIRS;
            createNextDir();
            break;
        case STATE_CREATING_DIRS:
            slotResultCreatingDirs( job );
            break;
        case STATE_CONFLICT_CREATING_DIRS:
            slotResultConflictCreatingDirs( job );
            break;
        case STATE_COPYING_FILES:
            slotResultCopyingFiles( job );
            break;
        case STATE_CONFLICT_COPYING_FILES:
            slotResultConflictCopyingFiles( job );
            break;
        case STATE_DELETING_DIRS:
            slotResultDeletingDirs( job );
            break;
        default:
            assert( 0 );
    }
}

CopyJob *KIO::copy(const KURL& src, const KURL& dest, bool showProgressInfo )
{
    KURL::List srcList;
    srcList.append( src );
    CopyJob *job = new CopyJob( srcList, dest, false, showProgressInfo );
    return job;
}

CopyJob *KIO::copy( const KURL::List& src, const KURL& dest, bool showProgressInfo )
{
    CopyJob *job = new CopyJob( src, dest, false, showProgressInfo );
    return job;
}

CopyJob *KIO::move(const KURL& src, const KURL& dest, bool showProgressInfo )
{
  KURL::List srcList;
  srcList.append( src );
  CopyJob *job = new CopyJob( srcList, dest, true, showProgressInfo );
  return job;
}

CopyJob *KIO::move( const KURL::List& src, const KURL& dest, bool showProgressInfo )
{
  CopyJob *job = new CopyJob( src, dest, true, showProgressInfo );
  return job;
}

DeleteJob::DeleteJob( const KURL::List& src, bool shred, bool showProgressInfo )
    : Job(), m_totalSize(0), m_processedSize(0), m_srcList(src), m_shred(shred)
{
  if ( showProgressInfo ) {
    connect( this, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( percent( KIO::Job*, unsigned int ) ),
	     Observer::self(), SLOT( slotPercent( KIO::Job*, unsigned int ) ) );

    connect( this, SIGNAL( deleting( KIO::Job*, const KURL& ) ),
	     Observer::self(), SLOT( slotDeleting( KIO::Job*, const KURL& ) ) );
  }
  startNextJob();
}

void DeleteJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryListIterator it(list);
    for (; it.current(); ++it) {
        UDSEntry::ConstIterator it2 = it.current()->begin();
        bool bDir = false;
        bool bLink = false;
        QString relName;
        for( ; it2 != it.current()->end(); it2++ ) {
            switch ((*it2).m_uds) {
                case UDS_FILE_TYPE:
                    bDir = S_ISDIR((*it2).m_long);
                    break;
                case UDS_NAME:
                    relName = ((*it2).m_str);
                    break;
                case UDS_LINK_DEST:
                    bLink = !(*it2).m_str.isEmpty();
                    break;
                case UDS_SIZE:
                    m_totalSize += (off_t)((*it2).m_long);
                    break;
                default:
                    break;
            }
        }
        assert(!relName.isEmpty());
        if (relName != ".." && relName != ".")
        {
            KURL url = ((SimpleJob *)job)->url(); // assumed to be a dir
            url.addPath( relName );
            kdDebug(7007) << "DeleteJob::slotEntries " << relName << " (" << url.url() << ")" << endl;
            if ( bLink )
                symlinks.append( url );
            else if ( bDir )
                dirs.append( url );
            else
                files.append( url );
        }
    }
}


void DeleteJob::startNextJob()
{
    files.clear();
    symlinks.clear();
    dirs.clear();
    KURL::List::Iterator it = m_srcList.begin();
    if (it != m_srcList.end())
    {
        // Stat first
        KIO::Job * job = KIO::stat( *it );
        kdDebug(7007) << "KIO::stat (DeleteJob) " << (*it).url() << endl;
        state = STATE_STATING;
        addSubjob(job);
        m_srcList.remove(it);
    } else
    {
        emit result(this);
        delete this;
    }
}

void DeleteJob::deleteNextFile()
{
    if ( !files.isEmpty() || !symlinks.isEmpty() )
    {
        // Take first file to delete out of list
        KURL::List::Iterator it = files.begin();
        bool isLink = false;
        if ( it == files.end() ) // No more files
        {
            it = symlinks.begin(); // Pick up a symlink to delete
            isLink = true;
        }
        SimpleJob *job;
        // Use shredding ?
        if ( m_shred && (*it).isLocalFile() && !isLink )
        {
            // KShred your KTie
            KIO_ARGS << int(3) << (*it).path();
            job = KIO::special(KURL("file:/"), packedArgs);
 	    emit deleting( this, *it );
        } else
        {
            // Normal deletion
            job = KIO::file_delete( *it );
	    emit deleting( this, *it );
        }
        if ( isLink ) symlinks.remove(it);
                 else files.remove(it);
        addSubjob(job);
	connect( job, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
		 this, SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
    } else
    {
        state = STATE_DELETING_DIRS;
        deleteNextDir();
    }
}

void DeleteJob::deleteNextDir()
{
    if ( !dirs.isEmpty() ) // some dirs to delete ?
    {
        // Take first dir to delete out of list - last ones first !
        KURL::List::Iterator it = dirs.fromLast();
        SimpleJob *job = KIO::rmdir( *it );
        dirs.remove(it);
        addSubjob( job );
    }
    else // We have finished deleting
        startNextJob();
}

void DeleteJob::slotProcessedSize( KIO::Job*, unsigned long data_size )
{
  kdDebug(7007) << "DeleteJob::slotProcessedSize " << data_size << endl;
  m_processedSize += data_size;
  emit processedSize( this, m_processedSize );

  // calculate percents
  uint ipercent = m_percent;

  if ( m_totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (uint)(( (float)m_processedSize / (float)m_totalSize ) * 100.0);
  
  if ( m_percent > ipercent ) {
    emit percent( this, m_percent );
  }
}

void DeleteJob::slotResult( Job *job )
{
    switch ( state ) {
        case STATE_STATING:
        {
            // Was there an error while stating ?
            if (job->error() )
            {
                // Probably : doesn't exist
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            // Is it a file or a dir ?
            UDSEntry entry = ((StatJob*)job)->statResult();
            bool bDir = false;
            bool bLink = false;
            UDSEntry::ConstIterator it2 = entry.begin();
            for( ; it2 != entry.end(); it2++ ) {
                if ( ((*it2).m_uds) == UDS_FILE_TYPE )
                    bDir = S_ISDIR( (mode_t)(*it2).m_long );
                else if ( ((*it2).m_uds) == UDS_LINK_DEST )
                    bLink = !((*it2).m_str.isEmpty());
            }

            KURL url = ((SimpleJob*)job)->url();

            if (bDir && !bLink)
            {
                // Add toplevel dir in list of dirs
                dirs.append( url );

                subjobs.remove( job );
                assert( subjobs.isEmpty() );

                kdDebug(7007) << " Target is a directory " << endl;
                // List it
                state = STATE_LISTING;
                ListJob *newjob = listRecursive( url );
                connect(newjob, SIGNAL(entries( KIO::Job *,
                                                const KIO::UDSEntryList& )),
                        SLOT( slotEntries( KIO::Job*,
                                           const KIO::UDSEntryList& )));
                addSubjob(newjob);
            }
            else
            {
                subjobs.remove( job );
                assert( subjobs.isEmpty() );

                kdDebug(7007) << " Target is a file (or a symlink) " << endl;
                // Remove it

                state = STATE_DELETING_FILES;
                if ( m_shred && url.isLocalFile() && !bLink )
                {
                    // KShred your KTie
                    KIO_ARGS << int(3) << url.path();
                    SimpleJob *job = KIO::special(KURL("file:/"), packedArgs);
                    addSubjob(job);
                }
                else
                {
                   // Normal deletion
                   SimpleJob *job = KIO::file_delete(url);
                   addSubjob( job );
                }
            }
        }
        break;
        case STATE_LISTING:
            if ( job->error() )
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }
            subjobs.remove( job );
            assert( subjobs.isEmpty() );
            debug("totalSize: %ld files: %d dirs: %d", m_totalSize, files.count(), dirs.count());

	    // emit all signals for total numbers
	    emit totalSize( this, m_totalSize );
	    emit totalFiles( this, files.count() );
	    emit totalDirs( this, dirs.count() );

            state = STATE_DELETING_FILES;
            deleteNextFile();
            break;
        case STATE_DELETING_FILES:
            if ( job->error() )
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }
            subjobs.remove( job );
            assert( subjobs.isEmpty() );
            deleteNextFile();
            break;
        case STATE_DELETING_DIRS:
            if ( job->error() )
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }
            subjobs.remove( job );
            assert( subjobs.isEmpty() );
            deleteNextDir();
            break;
        default:
            assert(0);
    }
}

DeleteJob *KIO::del( const KURL& src, bool shred, bool showProgressInfo )
{
  KURL::List srcList;
  srcList.append( src );
  DeleteJob *job = new DeleteJob( srcList, shred, showProgressInfo );
  return job;
}

DeleteJob *KIO::del( const KURL::List& src, bool shred, bool showProgressInfo )
{
  DeleteJob *job = new DeleteJob( src, shred, showProgressInfo );
  return job;
}

#include "jobclasses.moc"
