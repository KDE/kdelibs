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

// #include "kio/renamedlg.h"
// #include "kio/skipdlg.h"
#include "kio/observer.h"

using namespace KIO;

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( packedArgs, IO_WriteOnly ); stream

Job::Job(bool showProgressInfo) : QObject(0, "job"), m_error(0), m_percent(0),
                                  m_progressId(0), m_speedTimer(0)
{
    // All jobs delete themselves after emiting 'result'.

    // Notify the UI Server and get a progress id
    if ( showProgressInfo )
    {
        kdDebug(7007) << " -- with progress info -- " << endl;
        m_progressId = Observer::self()->newJob( this );
        // Connect global progress info signals
        connect( this, SIGNAL( percent( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotPercent( KIO::Job*, unsigned long ) ) );
        connect( this, SIGNAL( infoMessage( KIO::Job*, const QString & ) ),
                 Observer::self(), SLOT( slotInfoMessage( KIO::Job*, const QString & ) ) );
        connect( this, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
        connect( this, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
        connect( this, SIGNAL( speed( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
    }
}

Job::~Job()
{
    if ( m_progressId ) // Did we get an ID from the observer ?
        Observer::self()->jobFinished( m_progressId );
    delete m_speedTimer;
}

void Job::addSubjob(Job *job)
{
    kdDebug(7007) << "addSubjob(" << job << ") this = " << this << endl;
    subjobs.append(job);

    connect( job, SIGNAL(result(KIO::Job*)),
             SLOT(slotResult(KIO::Job*)) );

    // Forward information from that subjob.
    connect( job, SIGNAL(speed( KIO::Job*, unsigned long )),
             SLOT(slotSpeed(KIO::Job*, unsigned long)) );

    connect( job, SIGNAL(infoMessage( KIO::Job*, const QString & )),
             SLOT(slotInfoMessage(KIO::Job*, const QString &)) );
}

void Job::removeSubjob( Job *job )
{
    kdDebug(7007) << "removeSubjob(" << job << ") this = " << this << "  subjobs = " << subjobs.count() << endl;
    subjobs.remove(job);
    if (subjobs.isEmpty())
    {
	emit result(this);
        delete this; // Suicide is painless
    }
}

void Job::emitPercent( unsigned long processedSize, unsigned long totalSize )
{
  // calculate percents
  unsigned long ipercent = m_percent;

  if ( totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (unsigned long)(( (float)(processedSize) / (float)(totalSize) ) * 100.0);

  if ( m_percent != ipercent ) {
    emit percent( this, m_percent );
    kdDebug(7007) << "Job::emitPercent - percent =  " << (unsigned int) m_percent << endl;
  }
}

void Job::emitSpeed( unsigned long bytes_per_second )
{
  if ( !m_speedTimer )
  {
    m_speedTimer = new QTimer();
    connect( m_speedTimer, SIGNAL( timeout() ), SLOT( slotSpeedTimeout() ) );
  }
  emit speed( this, bytes_per_second );
  m_speedTimer->start( 5000 );   // 5 seconds interval should be enough
}

void Job::kill( bool quietly )
{
  // kill all subjobs
  QListIterator<Job> it( subjobs );
  for ( ; it.current() ; ++it )
     (*it)->kill();

  if ( ! quietly ) {
    emit canceled( this );
  }

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

void Job::slotSpeed( KIO::Job*, unsigned long bytes_per_second )
{
  //kdDebug(7007) << "Job::slotSpeed " << (unsigned int) bytes_per_second << endl;
  emitSpeed( bytes_per_second );
}

void Job::slotInfoMessage( KIO::Job*, const QString & msg )
{
  emit infoMessage( this, msg );
}

void Job::slotSpeedTimeout()
{
  // send 0 and stop the timer
  // timer will be restarted only when we receive another speed event
  emit speed( this, 0 );
  m_speedTimer->stop();
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
    m_url(url), m_command(command), m_totalSize(0)
{
    if (m_url.isMalformed())
    {
        m_error = ERR_MALFORMED_URL;
        m_errorText = m_url.url();
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
    } else
    {
        Scheduler::doJob(this);
    }
}

void SimpleJob::kill( bool quietly )
{
    Scheduler::cancelJob( this ); // deletes the slave if not 0
    m_slave = 0; // -> set to 0
    Job::kill( quietly );
}

void SimpleJob::putOnHold()
{
    Scheduler::putSlaveOnHold(this, m_url);
    m_slave = 0;
    kill(true);
}

void SimpleJob::removeOnHold()
{
    Scheduler::removeSlaveOnHold();
}

SimpleJob::~SimpleJob()
{
    if (m_slave) // was running
    {
        kdDebug(7007) << "SimpleJob::~SimpleJob: Killing running job in destructor!"  << endl;
#if 0
        m_slave->kill();
        Scheduler::jobFinished( this, m_slave ); // deletes the slave
#endif
	Scheduler::cancelJob( this );
        m_slave = 0; // -> set to 0
    }
}

void SimpleJob::start(Slave *slave)
{
    m_slave = slave;
    connect( m_slave, SIGNAL( error( int , const QString & ) ),
	     SLOT( slotError( int , const QString & ) ) );

    connect( m_slave, SIGNAL( warning( const QString & ) ),
	     SLOT( slotWarning( const QString & ) ) );

    connect( m_slave, SIGNAL( infoMessage( const QString & ) ),
	     SLOT( slotInfoMessage( const QString & ) ) );

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

void SimpleJob::slaveDone()
{
   if (!m_slave) return;
   disconnect(m_slave); // Remove all signals between slave and job
   Scheduler::jobFinished( this, m_slave );
   m_slave = 0;
}

void SimpleJob::slotFinished( )
{
    // Return slave to the scheduler
    slaveDone();

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

void SimpleJob::slotWarning( const QString & errorText )
{
    KMessageBox::information( 0L, errorText );
}

void SimpleJob::slotInfoMessage( const QString & msg )
{
    emit infoMessage( this, msg );
}

void SimpleJob::slotTotalSize( unsigned long size )
{
    m_totalSize = size;
    emit totalSize( this, size );
}

void SimpleJob::slotProcessedSize( unsigned long size )
{
    kdDebug(7007) << "SimpleJob::slotProcessedSize " << size << endl;
    emit processedSize( this, size );

    emitPercent( size, m_totalSize );
}

void SimpleJob::slotSpeed( unsigned long bytes_per_second )
{
    emitSpeed( bytes_per_second );
}

SimpleJob *KIO::mkdir( const KURL& url, int permissions )
{
    kdDebug(7007) << "mkdir " << url.url() << endl;
    KIO_ARGS << url << permissions;
    SimpleJob * job = new SimpleJob(url, CMD_MKDIR, packedArgs, false);
    return job;
}

SimpleJob *KIO::rmdir( const KURL& url )
{
    kdDebug(7007) << "rmdir " << url.url() << endl;
    KIO_ARGS << url << Q_INT8(false); // isFile is false
    return new SimpleJob(url, CMD_DEL, packedArgs, false);
}

SimpleJob *KIO::chmod( const KURL& url, int permissions )
{
    kdDebug(7007) << "chmod " << url.url() << endl;
    KIO_ARGS << url << permissions;
    SimpleJob * job = new SimpleJob(url, CMD_CHMOD, packedArgs, false);
    return job;
}

SimpleJob *KIO::special(const KURL& url, const QByteArray & data, bool showProgressInfo)
{
    kdDebug(7007) << "special " << url.url() << endl;
    SimpleJob * job = new SimpleJob(url, CMD_SPECIAL, data, showProgressInfo);
    return job;
}

SimpleJob *KIO::mount( bool ro, const char *fstype, const QString& dev, const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(1) << Q_INT8( ro ? 1 : 0 )
             << fstype << dev << point;
    return special( KURL("file:/"), packedArgs, showProgressInfo );
}

SimpleJob *KIO::unmount( const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(2) << point;
    return special( KURL("file:/"), packedArgs, showProgressInfo );
}

//////////

StatJob::StatJob( const KURL& url, int command,
                  const QByteArray &packedArgs )
    : SimpleJob(url, command, packedArgs, false)
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

StatJob *KIO::stat(const KURL& url)
{
    kdDebug(7007) << "stat " << url.url() << endl;
    KIO_ARGS << url;
    StatJob * job = new StatJob(url, CMD_STAT, packedArgs );
    return job;
}

//////////

TransferJob::TransferJob( const KURL& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData,
                          bool showProgressInfo)
    : SimpleJob(url, command, packedArgs, showProgressInfo), staticData( _staticData)
{
    m_suspended = false;
}

// Slave sends data
void TransferJob::slotData( const QByteArray &_data)
{
    if(m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error)
      emit data( this, _data);
}

// Slave got a redirection request
void TransferJob::slotRedirection( const KURL &url)
{
    kdDebug(7007) << "TransferJob::slotRedirection(" << url.url() << ")" << endl;

    if (m_redirectionList.contains(url))
    {
       kdDebug(7007) << "TransferJob::slotRedirection: CYCLCIC REDIRECTION!" << endl;
       m_error = ERR_CYCLIC_LINK;
       m_errorText = m_url.url();
    }
    else
    {
       m_redirectionURL = url; // We'll remember that when the job finishes
       m_redirectionList.append(url);
       // Tell the user that we haven't finished yet
       emit redirection(this, m_redirectionURL);
    }
}

void TransferJob::slotFinished()
{
    if (m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error )
        SimpleJob::slotFinished();
    else {
        kdDebug(7007) << "TransferJob: Redirection to " << m_redirectionURL.url() << endl;
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
        KURL dummyUrl;
        QDataStream istream( m_packedArgs, IO_ReadOnly );
        switch( m_command ) {
            case CMD_GET: {
                Q_INT8 iReload;
                istream >> dummyUrl >> iReload;
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << m_url << iReload;
                break;
            }
            case CMD_PUT: {
                int permissions;
                Q_INT8 iOverwrite, iResume;
                istream >> dummyUrl >> iOverwrite >> iResume >> permissions;
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << m_url << iOverwrite << iResume << permissions;
                break;
            }
            case CMD_SPECIAL: {
                int specialcmd;
                istream >> specialcmd;
                assert(specialcmd == 1); // you have to switch() here if other cmds are added
                if (specialcmd == 1) // Assume HTTP POST
                {
                   istream >> dummyUrl;
                   Q_INT8 iReload = 1;
                   m_packedArgs.truncate(0);
                   QDataStream stream( m_packedArgs, IO_WriteOnly );
                   stream << m_url << iReload;
                   m_command = CMD_GET;
                }
                break;
            }
        }

        // Return slave to the scheduler
        slaveDone();
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

void TransferJob::slotMimetype( const QString& type )
{
    kdDebug(7007) << "TransferJob::slotMimetype(" << type << ")" << endl;
    m_mimetype = type;
    emit mimetype( this, m_mimetype);
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

    connect( slave, SIGNAL(mimeType( const QString& ) ),
             SLOT( slotMimetype( const QString& ) ) );

    if (slave->suspended())
    {
       m_mimetype = "unknown";
       // WABA: The slave was put on hold. Resume operation.
       slave->resume();
    }

    SimpleJob::start(slave);
    if (m_suspended)
       slave->connection()->suspend();
}

TransferJob *KIO::get( const KURL& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url << Q_INT8( reload ? 1 : 0);
    TransferJob * job = new TransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    return job;
}

TransferJob *KIO::http_post( const KURL& url, const QByteArray &postData, bool showProgressInfo )
{
    assert( url.protocol() == "http" );
    // Send http post command (1), decoded path and encoded query
    KIO_ARGS << (int)1 << url;
    TransferJob * job = new TransferJob( url, CMD_SPECIAL,
                                         packedArgs, postData, showProgressInfo );
    return job;
}

TransferJob *KIO::put( const KURL& url, int permissions,
                  bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << Q_INT8( overwrite ? 1 : 0 ) << Q_INT8( resume ? 1 : 0 ) << permissions;
    TransferJob * job = new TransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    return job;
}

//////////

MimetypeJob::MimetypeJob( const KURL& url, int command,
                  const QByteArray &packedArgs )
    : TransferJob(url, command, packedArgs, QByteArray(), false)
{
}

void MimetypeJob::start(Slave *slave)
{
    TransferJob::start(slave);

}


void MimetypeJob::slotFinished( )
{
    kdDebug(7007) << "MimetypeJob::slotFinished()" << endl;
    if ( m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error )
    {
        // Return slave to the scheduler
        TransferJob::slotFinished();
    } else {
        kdDebug(7007) << "MimetypeJob: Redirection to " << m_redirectionURL.url() << endl;
        staticData.truncate(0);
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        m_packedArgs.truncate(0);
        QDataStream stream( m_packedArgs, IO_WriteOnly );
        stream << m_url.path() << m_url.query();

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

MimetypeJob *KIO::mimetype(const KURL& url )
{
    kdDebug(7007) << "mimetype " << url.url() << endl;
    KIO_ARGS << url;
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
                          bool move, bool overwrite, bool resume, bool showProgressInfo)
    : Job(showProgressInfo), m_src(src), m_dest(dest),
      m_permissions(permissions), m_move(move), m_overwrite(overwrite), m_resume(resume),
      m_totalSize(0)
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
          KIO_ARGS << src << dest << (Q_INT8) m_overwrite;
          m_moveJob = new SimpleJob(src, CMD_RENAME, packedArgs, false);
          addSubjob( m_moveJob );
          connectSubjob( m_moveJob );
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
    KIO_ARGS << m_src << m_dest << m_permissions << (Q_INT8) m_overwrite;
    m_copyJob = new SimpleJob(m_src, CMD_COPY, packedArgs, false);
    addSubjob( m_copyJob );
    connectSubjob( m_copyJob );
}

void FileCopyJob::connectSubjob( SimpleJob * job )
{
    connect( job, SIGNAL(totalSize( KIO::Job*, unsigned long )),
             this, SLOT( slotTotalSize(KIO::Job*, unsigned long)) );

    connect( job, SIGNAL(processedSize( KIO::Job*, unsigned long )),
             this, SLOT( slotProcessedSize(KIO::Job*, unsigned long)) );

    connect( job, SIGNAL(percent( KIO::Job*, unsigned long )),
             this, SLOT( slotPercent(KIO::Job*, unsigned long)) );

}

void FileCopyJob::slotProcessedSize( KIO::Job *, unsigned long size )
{
    emit processedSize( this, size );
    emitPercent( size, m_totalSize );
}

void FileCopyJob::slotTotalSize( KIO::Job*, unsigned long size )
{
    m_totalSize = size;
    emit totalSize( this, m_totalSize );
}

void FileCopyJob::slotPercent( KIO::Job*, unsigned long pct )
{
    if ( pct > m_percent )
    {
        m_percent = pct;
        emit percent( this, m_percent );
    }
}

void FileCopyJob::startDataPump()
{
    kdDebug(7007) << "FileCopyJob::startDataPump()" << endl;
    m_getJob = get( m_src, false, false /* no GUI */ );
    kdDebug(7007) << "FileCopyJob: m_getJob = " << m_getJob << endl;
    m_putJob = put( m_dest, m_permissions, m_overwrite, m_resume, false /* no GUI */);
    kdDebug(7007) << "FileCopyJob: m_putJob = " << m_putJob << endl;
    m_putJob->suspend();
    addSubjob( m_getJob );
    connectSubjob( m_getJob ); // Progress info depends on get
    addSubjob( m_putJob );
    m_getJob->resume(); // Order a beer

    connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
             SLOT( slotData(KIO::Job *, const QByteArray&)));
    connect( m_putJob, SIGNAL(dataReq(KIO::Job *, QByteArray&)),
             SLOT( slotDataReq(KIO::Job *, QByteArray&)));
}

void FileCopyJob::slotData( KIO::Job * , const QByteArray &data)
{
   //kdDebug(7007) << "FileCopyJob::slotData(" << job << ")" << endl;
   //kdDebug(7007) << " data size : " << data.size() << endl;
   assert(m_putJob);
   m_getJob->suspend();
   m_putJob->resume(); // Drink the beer
   m_buffer = data;
}

void FileCopyJob::slotDataReq( KIO::Job * , QByteArray &data)
{
   //kdDebug(7007) << "FileCopyJob::slotDataReq(" << job << ")" << endl;
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
         m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
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
         m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
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
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, false, overwrite, resume, showProgressInfo );
}

FileCopyJob *KIO::file_move( const KURL& src, const KURL& dest, int permissions,
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, true, overwrite, resume, showProgressInfo );
}

SimpleJob *KIO::file_delete( const KURL& src, bool showProgressInfo)
{
    KIO_ARGS << src << Q_INT8(true); // isFile
    return new SimpleJob(src, CMD_DEL, packedArgs, showProgressInfo );
}

//////////

bool KIO::link( const QString & linkDest, const KURL & destUrl, bool overwriteExistingFiles,
                bool & overwriteAll, bool & autoSkip, bool & cancelAll )
{
    kdDebug(7007) << "linking " << destUrl.path() << " -> " << linkDest << endl;
    if ( !destUrl.isLocalFile() )
    {
        KMessageBox::error( 0L, i18n( "Can't create a remote symlink!" ) );
        return false;
    }
    if ( symlink( linkDest.local8Bit(), destUrl.path().local8Bit() ) == -1 )
    {
        // An error occured
        if ( autoSkip )
            return true; // eat error
        // Does the destination already exist ?
        if ( errno == EEXIST )
        {
            if ( !overwriteExistingFiles && !overwriteAll )
            {
                // Ask the user what to do
                RenameDlg_Mode mode = (RenameDlg_Mode) M_OVERWRITE;
                // TODO if ( files.count() > 0 ) // Not last one
                mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );
                // else
                //  mode = (RenameDlg_Mode) ( mode | M_SINGLE );
                QString newPath;
                RenameDlg_Result res = Observer::self()->open_RenameDlg( 0, i18n("File already exists"),
                                                                         linkDest, destUrl.path(), mode, newPath );
                switch ( res ) {
                    case R_CANCEL:
                        cancelAll = true;
                        return false;
                    case R_RENAME:
                        // Try again, with new name for the link
                        return link( linkDest, newPath, overwriteExistingFiles, overwriteAll, autoSkip, cancelAll );
                    case R_AUTO_SKIP:
                        autoSkip = true;
                        // fall through
                    case R_SKIP:
                        // Move on to next file
                        return true;
                    case R_OVERWRITE_ALL:
                        overwriteAll = true;
                        break;
                    case R_OVERWRITE:
                        overwriteExistingFiles = true;
                        break;
                    default:
                        assert( 0 );
                }
            }
            // Are we allowed to overwrite the files ?
            if ( overwriteExistingFiles || overwriteAll )
            {
                // Try to delete the destination
                if ( unlink( destUrl.path().local8Bit() ) != 0 )
                {
                    KMessageBox::sorry( 0L, i18n( "Could not overwrite\n%1"), destUrl.path() );
                    return false;
                }
                // Try again - this won't loop forever since unlink succeeded
                return link( linkDest, destUrl, overwriteExistingFiles, overwriteAll, autoSkip, cancelAll );
            }
            else
                return false;
        }
        else
        {
            // Some error occured while we tried to symlink
            KMessageBox::sorry( 0L, i18n( "Failed to make symlink from \n%1\nto\n%2\n" ).
                                arg(linkDest).arg(destUrl.url()) );
            return false;
        }
    }
    return true;
}

bool KIO::link( const KURL::List &srcUrls, const KURL & destDir )
{
    kdDebug(1202) << "destDir = " << destDir.url() << endl;
    bool overwriteAll = false;
    bool autoSkip = false;
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
	KURL destUrl( destDir.url(1) + srcUrl.fileName() );
	kdDebug(1202) << "destUrl = " << destUrl.url() << endl;

	// Do we link a file on the local disk?
	if ( srcUrl.isLocalFile() )
	{
            bool cancelAll= false;
	    // Make a symlink
            (void)KIO::link( srcUrl.path(), destUrl, false, overwriteAll, autoSkip, cancelAll );
            if (cancelAll)
                return false;
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
    SimpleJob(u, CMD_LISTDIR, QByteArray(), showProgressInfo),
    recursive(_recursive), prefix(_prefix), m_processedEntries(0)
{
    // We couldn't set the args when calling the parent constructor,
    // so do it now.
    QDataStream stream( m_packedArgs, IO_WriteOnly );
    stream << u;
}

void ListJob::slotListEntries( const KIO::UDSEntryList& list )
{
    // Emit progress info (takes care of emit processedSize and percent)
    m_processedEntries += list.count();
    slotProcessedSize( m_processedEntries );

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
		    ListJob *job = new ListJob(newone, m_progressId!=0, true, prefix + filename + "/");
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

void ListJob::slotRedirection( const KURL & url )
{
    emit redirection( this, url );
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
    connect( slave, SIGNAL( totalSize( unsigned long ) ),
             SLOT( slotTotalSize( unsigned long ) ) );
    connect( slave, SIGNAL( redirection(const KURL &) ),
             SLOT( slotRedirection(const KURL &) ) );
    SimpleJob::start(slave);
}


CopyJob::CopyJob( const KURL::List& src, const KURL& dest, bool move, bool showProgressInfo )
  : Job(showProgressInfo), m_move(move),
    destinationState(DEST_NOT_STATED), state(STATE_STATING),
      m_totalSize(0), m_processedSize(0), m_fileProcessedSize(0), m_srcList(src), m_dest(dest),
      m_bAutoSkip( false ), m_bOverwriteAll( false )
{
  if ( showProgressInfo ) {
    connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

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
        info.permissions = (mode_t) -1;
        info.mtime = (time_t) -1;
        info.ctime = (time_t) -1;
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
            // Append filename or dirname to destination URL
            if ( destinationState == DEST_IS_DIR )
                info.uDest.addPath( relName );
            if ( info.linkDest.isEmpty() && (S_ISDIR(info.type)) ) // Dir
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
            m_currentDest.addPath( srcurl.fileName() );
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            m_error = ERR_IS_FILE;
            emit result(this);
            return;
        }
        else // (case 3)
        {
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;
        }

        // If moving,
        // Before going for the full list+copy[+del] thing, try to rename
        if ( m_move )
        {
            if ((srcurl.protocol() == m_currentDest.protocol()) &&
                (srcurl.host() == m_currentDest.host()) &&
                (srcurl.port() == m_currentDest.port()) &&
                (srcurl.user() == m_currentDest.user()) &&
                (srcurl.pass() == m_currentDest.pass()))
            {
                kdDebug(7007) << "This seems to be a suitable case for trying to rename the dir before copy+del" << endl;
                KIO_ARGS << srcurl << m_currentDest << (Q_INT8) false /*m_overwrite*/;
                state = STATE_RENAMING;
                Job * newJob = new SimpleJob(srcurl, CMD_RENAME, packedArgs, false);
                addSubjob( newJob );
                return;
            }
        }

        startListing( srcurl );
    }
    else
    {
        kdDebug(7007) << " Source is a file (or a symlink) " << endl;

	kdDebug() << "totalSize: " << (unsigned int) m_totalSize << endl;
	// emit all signals for total numbers
	emit totalSize( this, m_totalSize );
	emit totalFiles( this, 1 );
	emit totalDirs( this, 0 );

        // Skip the "listing" stage and go directly copying the file
        state = STATE_COPYING_FILES;
        copyNextFile();
    }
}

void CopyJob::startListing( const KURL & src )
{
    state = STATE_LISTING;
    ListJob * newjob = listRecursive( src, false );
    connect(newjob, SIGNAL(entries( KIO::Job *,
                                    const KIO::UDSEntryList& )),
            SLOT( slotEntries( KIO::Job*,
                               const KIO::UDSEntryList& )));
    addSubjob( newjob );
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
    RenameDlg_Result r = Observer::self()->open_RenameDlg( this, i18n("Directory already exists"),
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
            emit renamed( this, (*it).uDest, newUrl );

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
        res = Observer::self()->open_RenameDlg( this, m_conflictError == ERR_FILE_ALREADY_EXIST ?
                                i18n("File already exists") : i18n("Already exists as a directory"),
                              (*it).uSource.url(), (*it).uDest.path(), mode, newPath,
                              (*it).size, destsize,
                              (*it).ctime, destctime,
                              (*it).mtime, destmtime );
    }
    else
    {
        SkipDlg_Result skipResult = Observer::self()->open_SkipDlg( this, files.count() > 0,
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
            emit renamed( this, (*it).uDest, newUrl );
            (*it).uDest = newUrl;
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

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

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
            bool bCancelAll = false;
            // The "source" is in fact what the existing link points to
            if ( KIO::link( (*it).linkDest, (*it).uDest, bOverwrite, m_bOverwriteAll, m_bAutoSkip, bCancelAll ) )
            {
                if (m_move)
                {
                    newjob = KIO::del( (*it).uSource, false /*shred*/, false /*no GUI*/ );
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
                if ( bCancelAll )
                {
                    m_error = ERR_USER_CANCELED;
                    emit result(this);
                    delete this;
                    return;
                }
                // Error - move to next file
                files.remove( it );
                copyNextFile();
                return;
            }
        } else if (m_move) // Moving a file
        {
            newjob = KIO::file_move( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false, false/*no GUI*/ );
            kdDebug() << "CopyJob::copyNextFile : Moving " << (*it).uSource.url() << " to " << (*it).uDest.url() << endl;
	    emit moving( this, (*it).uSource, (*it).uDest );
        }
        else // Copying a file
        {
            newjob = KIO::file_copy( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false, false/*no GUI*/ );
            kdDebug() << "CopyJob::copyNextFile : Copying " << (*it).uSource.url() << " to " << (*it).uDest.url() << endl;
	    emit copying( this, (*it).uSource, (*it).uDest );
        }
        addSubjob(newjob);
	connect( newjob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
		 this, SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
	connect( newjob, SIGNAL( speed( KIO::Job*, unsigned long ) ),
		 this, SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
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
  m_fileProcessedSize = data_size;

  kdDebug(7007) << "CopyJob::slotProcessedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;
  emit processedSize( this, m_processedSize + m_fileProcessedSize );

  emitPercent( m_processedSize + m_fileProcessedSize, m_totalSize );
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
    kdDebug() << "CopyJob::slotResult() state=" << (int) state << endl;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - subjobs.remove(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to rename a directory
        {
            bool err = job->error() != 0;
            subjobs.remove( job );
            assert ( subjobs.isEmpty() ); // We should have only one job at a time ...
            if ( err )
            {
                kdDebug(7007) << "Couldn't rename, starting listing, for copy and del" << endl;
                startListing( *(m_srcList.begin()) );
            }
            else
            {
                kdDebug(7007) << "Renaming succeeded, move on" << endl;
                m_srcList.remove(m_srcList.begin()); // done with this url
                startNextJob(); // done
            }
        }
        break;
        case STATE_LISTING: // recursive listing finished
	    kdDebug() << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count() << endl;
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
    : Job(showProgressInfo), m_totalSize(0), m_processedSize(0), m_fileProcessedSize(0), m_srcList(src), m_shred(shred)
{
  if ( showProgressInfo ) {
    connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

    connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
    connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	     Observer::self(), SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

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
    //kdDebug(7007) << "startNextJob" << endl;
    files.clear();
    symlinks.clear();
    dirs.clear();
    KURL::List::Iterator it = m_srcList.begin();
    if (it != m_srcList.end())
    {
        // Stat first
        KIO::Job * job = KIO::stat( *it );
        //kdDebug(7007) << "KIO::stat (DeleteJob) " << (*it).url() << endl;
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
    //kdDebug(7007) << "deleteNextFile" << endl;
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
            job = KIO::special(KURL("file:/"), packedArgs, false /*no GUI*/);
 	    emit deleting( this, *it );
        } else
        {
            // Normal deletion
            job = KIO::file_delete( *it, false /*no GUI*/);
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
  // Note: this is the same implementation as CopyJob::slotProcessedSize but
  // it's different from FileCopyJob::slotProcessedSize - which is why this
  // is not in Job.

  m_fileProcessedSize = data_size;

  kdDebug(7007) << "DeleteJob::slotProcessedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;
  emit processedSize( this, m_processedSize + m_fileProcessedSize );

  // calculate percents
  unsigned long ipercent = m_percent;

  if ( m_totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (unsigned long)(( (float)(m_processedSize + m_fileProcessedSize) / (float)m_totalSize ) * 100.0);

  if ( m_percent > ipercent ) {
    emit percent( this, m_percent );
    kdDebug(7007) << "DeleteJob::slotProcessedSize - percent =  " << (unsigned int) m_percent << endl;
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
            unsigned long size = 0L;
            UDSEntry::ConstIterator it2 = entry.begin();
            for( ; it2 != entry.end(); it2++ ) {
                if ( ((*it2).m_uds) == UDS_FILE_TYPE )
                    bDir = S_ISDIR( (mode_t)(*it2).m_long );
                else if ( ((*it2).m_uds) == UDS_LINK_DEST )
                    bLink = !((*it2).m_str.isEmpty());
                else if ( ((*it2).m_uds) == UDS_SIZE )
                    size = (*it2).m_long;
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
                ListJob *newjob = listRecursive( url, false );
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

                emit deleting( this, url );
                m_totalSize = size;
                emit totalSize( this, size );
                state = STATE_DELETING_FILES;
                SimpleJob *newjob;
                if ( m_shred && url.isLocalFile() && !bLink )
                {
                    // KShred your KTie
                    KIO_ARGS << int(3) << url.path();
                    newjob = KIO::special(KURL("file:/"), packedArgs, false);
                    addSubjob(newjob);
                }
                else
                {
                   // Normal deletion
                   newjob = KIO::file_delete(url, false/*no GUI*/);
                   addSubjob( newjob );
                }
                connect( newjob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
                         this, SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
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
            kdDebug(7007) << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count() << endl;

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
