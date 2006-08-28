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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kio/job.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <assert.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
extern "C" {
#include <pwd.h>
#include <grp.h>
}
#include <qtimer.h>
#include <qfile.h>

#include <kapplication.h>
#include <kauthorized.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kde_file.h>

#include <errno.h>

#include "jobuidelegate.h"
#include "kmimetype.h"
#include "slave.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kmimemagic.h"
#include "kprotocolinfo.h"
#include "kprotocolmanager.h"
#include "filejob.h"

#include "kio/observer.h"

#include "kssl/ksslcsessioncache.h"

#include <kdirnotify.h>
#include <ktempfile.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

using namespace KIO;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

class Job::JobPrivate
{
public:
    JobPrivate() : m_interactive( true ), m_suspended( false ), m_parentJob( 0L ), m_extraFlags(0)
                   {}

    bool m_interactive;
    bool m_suspended;
    // Maybe we could use the QObject parent/child mechanism instead
    // (requires a new ctor, and moving the ctor code to some init()).
    Job* m_parentJob;
    int m_extraFlags;
};

Job::Job(bool showProgressInfo) : KCompositeJob(0), m_speedTimer(0), d( new JobPrivate )
{
    setUiDelegate( new JobUiDelegate( showProgressInfo ) );
}

Job::~Job()
{
    delete m_speedTimer;
    delete d;
}

JobUiDelegate *Job::ui() const
{
    return static_cast<JobUiDelegate*>( uiDelegate() );
}

int& Job::extraFlags()
{
    return d->m_extraFlags;
}

void Job::addSubjob(Job *job, bool inheritMetaData)
{
    //kDebug(7007) << "addSubjob(" << job << ") this = " << this << endl;

    KCompositeJob::addSubjob( job );

    // Forward information from that subjob.
    connect( job, SIGNAL(speed( KIO::Job*, unsigned long )),
             SLOT(slotSpeed(KIO::Job*, unsigned long)) );

    if (inheritMetaData)
       job->mergeMetaData(m_outgoingMetaData);

    job->ui()->setWindow( m_window );
}

void Job::removeSubjob( KJob *jobBase, bool mergeMetaData )
{
    KIO::Job *job = dynamic_cast<KIO::Job*>( jobBase );

    if ( job == 0 )
    {
        return;
    }

    //kDebug(7007) << "removeSubjob(" << job << ") this = " << this << "  subjobs = " << subjobs().count() << endl;
    // Merge metadata from subjob
    if ( mergeMetaData )
        m_incomingMetaData += job->metaData();

    KCompositeJob::removeSubjob( job );
}

void Job::emitSpeed( unsigned long bytes_per_second )
{
  //kDebug(7007) << "Job " << this << " emitSpeed " << bytes_per_second << endl;
  if ( !m_speedTimer )
  {
    m_speedTimer = new QTimer(this);
    connect( m_speedTimer, SIGNAL( timeout() ), SLOT( slotSpeedTimeout() ) );
  }
  emit speed( this, bytes_per_second );
  m_speedTimer->start( 5000 );   // 5 seconds interval should be enough
}

bool Job::doKill()
{
  kDebug(7007) << "Job::kill this=" << this << " " << metaObject()->className() << " progressId()=" << progressId() << endl;
  // kill all subjobs, without triggering their result slot
  QList<KJob *>::const_iterator it = subjobs().begin();
  const QList<KJob *>::const_iterator end = subjobs().end();
  for ( ; it != end ; ++it )
    (*it)->kill( KJob::Quietly );
  clearSubjobs();

  return true;
}

void Job::suspend()
{
    if ( !d->m_suspended )
    {
        d->m_suspended = true;
        QList<KJob *>::const_iterator it = subjobs().begin();
        const QList<KJob *>::const_iterator end = subjobs().end();
        for ( ; it != end ; ++it )
            static_cast<KIO::Job*>( *it )->suspend();
    }
}

void Job::resume()
{
    if ( d->m_suspended )
    {
        d->m_suspended = false;
        QList<KJob *>::const_iterator it = subjobs().begin();
        const QList<KJob *>::const_iterator end = subjobs().end();
        for ( ; it != end ; ++it )
            static_cast<KIO::Job*>( *it )->resume();
    }
}

bool Job::isSuspended() const
{
  return d->m_suspended;
}

void Job::slotSpeed( KIO::Job*, unsigned long speed )
{
  //kDebug(7007) << "Job::slotSpeed " << speed << endl;
  emitSpeed( speed );
}

void Job::slotSpeedTimeout()
{
  //kDebug(7007) << "slotSpeedTimeout()" << endl;
  // send 0 and stop the timer
  // timer will be restarted only when we receive another speed event
  emit speed( this, 0 );
  m_speedTimer->stop();
}

//Job::errorString is implemented in global.cpp

void Job::showErrorDialog( QWidget *parent )
{
    if ( ui() )
    {
        ui()->setWindow( parent );
        ui()->showErrorMessage();
    }
    else
    {
        kError() << errorString() << endl;
    }
}

bool Job::isInteractive() const
{
  return uiDelegate() != 0;
}

void Job::setParentJob(Job* job)
{
  Q_ASSERT(d->m_parentJob == 0L);
  Q_ASSERT(job);
  d->m_parentJob = job;
}

Job* Job::parentJob() const
{
  return d->m_parentJob;
}

MetaData Job::metaData() const
{
    return m_incomingMetaData;
}

QString Job::queryMetaData(const QString &key)
{
    if (!m_incomingMetaData.contains(key))
       return QString();
    return m_incomingMetaData[key];
}

void Job::setMetaData( const KIO::MetaData &_metaData)
{
    m_outgoingMetaData = _metaData;
}

void Job::addMetaData( const QString &key, const QString &value)
{
    m_outgoingMetaData.insert(key, value);
}

void Job::addMetaData( const QMap<QString,QString> &values)
{
    QMap<QString,QString>::const_iterator it = values.begin();
    for(;it != values.end(); ++it)
      m_outgoingMetaData.insert(it.key(), it.value());
}

void Job::mergeMetaData( const QMap<QString,QString> &values)
{
    QMap<QString,QString>::const_iterator it = values.begin();
    for(;it != values.end(); ++it)
        // there's probably a faster way
        if ( !m_outgoingMetaData.contains( it.key() ) )
            m_outgoingMetaData.insert( it.key(), it.value() );
}

MetaData Job::outgoingMetaData() const
{
    return m_outgoingMetaData;
}


SimpleJob::SimpleJob(const KUrl& url, int command, const QByteArray &packedArgs,
                     bool showProgressInfo )
  : Job(showProgressInfo), m_slave(0), m_packedArgs(packedArgs),
    m_url(url), m_command(command), m_totalSize(0)
{
    if (!m_url.isValid())
    {
        setError( ERR_MALFORMED_URL );
        setErrorText( m_url.url() );
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
        return;
    }


    if (m_url.hasSubUrl())
    {
       KUrl::List list = KUrl::split(m_url);
       list.removeLast();
       m_subUrl = KUrl::join(list);
       //kDebug(7007) << "New URL = "  << m_url.url() << endl;
       //kDebug(7007) << "Sub URL = "  << m_subUrl.url() << endl;
    }

    Scheduler::doJob(this);
}

void SimpleJob::start()
{
}

bool SimpleJob::doKill()
{
    Scheduler::cancelJob( this ); // deletes the slave if not 0
    m_slave = 0; // -> set to 0
    return true;
}

void SimpleJob::suspend()
{
    Q_ASSERT( m_slave );
    if ( m_slave )
        m_slave->suspend();
    Job::suspend();
}

void SimpleJob::resume()
{
    Q_ASSERT( m_slave );
    if ( m_slave )
        m_slave->resume();
    Job::resume();
}

void SimpleJob::putOnHold()
{
    Q_ASSERT( m_slave );
    if ( m_slave )
    {
        Scheduler::putSlaveOnHold(this, m_url);
        m_slave = 0;
    }
    kill( Quietly );
}

void SimpleJob::removeOnHold()
{
    Scheduler::removeSlaveOnHold();
}

SimpleJob::~SimpleJob()
{
    if (m_slave) // was running
    {
        kDebug(7007) << "SimpleJob::~SimpleJob: Killing running job in destructor!"  << endl;
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

    connect( m_slave, SIGNAL( connected() ),
             SLOT( slotConnected() ) );

    connect( m_slave, SIGNAL( finished() ),
             SLOT( slotFinished() ) );

    if ((extraFlags() & EF_TransferJobDataSent) == 0)
    {
        connect( m_slave, SIGNAL( totalSize( KIO::filesize_t ) ),
                 SLOT( slotTotalSize( KIO::filesize_t ) ) );

        connect( m_slave, SIGNAL( processedSize( KIO::filesize_t ) ),
                 SLOT( slotProcessedSize( KIO::filesize_t ) ) );

        connect( m_slave, SIGNAL( speed( unsigned long ) ),
                 SLOT( slotSpeed( unsigned long ) ) );
    }

    connect( slave, SIGNAL( needProgressId() ),
             SLOT( slotNeedProgressId() ) );

    connect( slave, SIGNAL(metaData( const KIO::MetaData& ) ),
             SLOT( slotMetaData( const KIO::MetaData& ) ) );

    if (ui() && ui()->window())
    {
       QString id;
       addMetaData("window-id", id.setNum((ulong)ui()->window()->winId()));
    }

    QString sslSession = KSSLCSessionCache::getSessionForUrl(m_url);
    if ( !sslSession.isNull() )
    {
        addMetaData("ssl_session_id", sslSession);
    }

    if (!isInteractive())
    {
        addMetaData("no-auth-prompt", "true");
    }

    if (!outgoingMetaData().isEmpty())
    {
       KIO_ARGS << outgoingMetaData();
       slave->send( CMD_META_DATA, packedArgs );
    }

    if (!m_subUrl.isEmpty())
    {
       KIO_ARGS << m_subUrl;
       m_slave->send( CMD_SUBURL, packedArgs );
    }

    m_slave->send( m_command, m_packedArgs );
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

    if (!hasSubjobs())
    {
        if ( !error() && (m_command == CMD_MKDIR || m_command == CMD_RENAME ) )
        {
            if ( m_command == CMD_MKDIR )
            {
                KUrl urlDir( url() );
                urlDir.setPath( urlDir.directory() );
                org::kde::KDirNotify::emitFilesAdded( urlDir.url() );
            }
            else /*if ( m_command == CMD_RENAME )*/
            {
                KUrl src, dst;
                QDataStream str( m_packedArgs );
                str >> src >> dst;
                if ( src.directory() == dst.directory() ) // For the user, moving isn't renaming. Only renaming is.
                    org::kde::KDirNotify::emitFileRenamed( src.url(), dst.url() );
            }
        }
        emitResult();
    }
}

void SimpleJob::slotError( int err, const QString & errorText )
{
    setError( err );
    setErrorText( errorText );
    if ((error() == ERR_UNKNOWN_HOST) && m_url.host().isEmpty())
       setErrorText( QString() );
    // error terminates the job
    slotFinished();
}

void SimpleJob::slotWarning( const QString & errorText )
{
    emit warning( this, errorText );
}

void SimpleJob::slotInfoMessage( const QString & msg )
{
    emit infoMessage( this, msg );
}

void SimpleJob::slotConnected()
{
    emit connected( this );
}

void SimpleJob::slotNeedProgressId()
{
    if ( !progressId() )
        setProgressId( Observer::self()->newJob( this, false ) );
    m_slave->setProgressId( progressId() );
}

void SimpleJob::slotTotalSize( KIO::filesize_t size )
{
    if (size > m_totalSize)
    {
        m_totalSize = size;
        emit totalSize( this, size );
    }
}

void SimpleJob::slotProcessedSize( KIO::filesize_t size )
{
    //kDebug(7007) << "SimpleJob::slotProcessedSize " << KIO::number(size) << endl;
    setProcessedSize(size);
    emit processedSize( this, size );
    if ( size > m_totalSize ) {
        slotTotalSize(size); // safety
    }
    emitPercent( size, m_totalSize );
}

void SimpleJob::slotSpeed( unsigned long speed )
{
    //kDebug(7007) << "SimpleJob::slotSpeed( " << speed << " )" << endl;
    emitSpeed( speed );
}

void SimpleJob::slotMetaData( const KIO::MetaData &_metaData)
{
    m_incomingMetaData += _metaData;
}

void SimpleJob::storeSSLSessionFromJob(const KUrl &m_redirectionURL) {
    QString sslSession = queryMetaData("ssl_session_id");

    if ( !sslSession.isNull() ) {
	    const KUrl &queryURL = m_redirectionURL.isEmpty()?m_url:m_redirectionURL;
	    KSSLCSessionCache::putSessionForUrl(queryURL, sslSession);
    }
}

//////////
MkdirJob::MkdirJob( const KUrl& url, int command,
                    const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo)
{
}

void MkdirJob::start(Slave *slave)
{
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

// Slave got a redirection request
void MkdirJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "MkdirJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeUrlAction("redirect", m_url, url))
     {
       kWarning(7007) << "MkdirJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       setError( ERR_ACCESS_DENIED );
       setErrorText( url.prettyUrl() );
       return;
     }
     m_redirectionURL = url; // We'll remember that when the job finishes
     if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
     // Tell the user that we haven't finished yet
     emit redirection(this, m_redirectionURL);
}

void MkdirJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        //kDebug(7007) << "MkdirJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        KUrl dummyUrl;
        int permissions;
        QDataStream istream( m_packedArgs );
        istream >> dummyUrl >> permissions;

        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url << permissions;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

SimpleJob *KIO::mkdir( const KUrl& url, int permissions )
{
    //kDebug(7007) << "mkdir " << url << endl;
    KIO_ARGS << url << permissions;
    return new MkdirJob(url, CMD_MKDIR, packedArgs, false);
}

SimpleJob *KIO::rmdir( const KUrl& url )
{
    //kDebug(7007) << "rmdir " << url << endl;
    KIO_ARGS << url << qint8(false); // isFile is false
    return new SimpleJob(url, CMD_DEL, packedArgs, false);
}

SimpleJob *KIO::chmod( const KUrl& url, int permissions )
{
    //kDebug(7007) << "chmod " << url << endl;
    KIO_ARGS << url << permissions;
    return new SimpleJob(url, CMD_CHMOD, packedArgs, false);
}

SimpleJob *KIO::rename( const KUrl& src, const KUrl & dest, bool overwrite )
{
    //kDebug(7007) << "rename " << src << " " << dest << endl;
    KIO_ARGS << src << dest << (qint8) overwrite;
    return new SimpleJob(src, CMD_RENAME, packedArgs, false);
}

SimpleJob *KIO::symlink( const QString& target, const KUrl & dest, bool overwrite, bool showProgressInfo )
{
    //kDebug(7007) << "symlink target=" << target << " " << dest << endl;
    KIO_ARGS << target << dest << (qint8) overwrite;
    return new SimpleJob(dest, CMD_SYMLINK, packedArgs, showProgressInfo);
}

SimpleJob *KIO::special(const KUrl& url, const QByteArray & data, bool showProgressInfo)
{
    //kDebug(7007) << "special " << url << endl;
    return new SimpleJob(url, CMD_SPECIAL, data, showProgressInfo);
}

SimpleJob *KIO::mount( bool ro, const QByteArray& fstype, const QString& dev, const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(1) << qint8( ro ? 1 : 0 )
             << QString::fromLatin1(fstype) << dev << point;
    SimpleJob *job = special( KUrl("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->mounting( job, dev, point );
    return job;
}

SimpleJob *KIO::unmount( const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(2) << point;
    SimpleJob *job = special( KUrl("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->unmounting( job, point );
    return job;
}



//////////

StatJob::StatJob( const KUrl& url, int command,
                  const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo),
    m_bSource(true), m_details(2)
{
}

void StatJob::start(Slave *slave)
{
    m_outgoingMetaData.insert( "statSide", m_bSource ? "source" : "dest" );
    m_outgoingMetaData.insert( "details", QString::number(m_details) );

    connect( slave, SIGNAL( statEntry( const KIO::UDSEntry& ) ),
             SLOT( slotStatEntry( const KIO::UDSEntry & ) ) );
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

void StatJob::slotStatEntry( const KIO::UDSEntry & entry )
{
    //kDebug(7007) << "StatJob::slotStatEntry" << endl;
    m_statResult = entry;
}

// Slave got a redirection request
void StatJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "StatJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeUrlAction("redirect", m_url, url))
     {
       kWarning(7007) << "StatJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       setError( ERR_ACCESS_DENIED );
       setErrorText( url.prettyUrl() );
       return;
     }
     m_redirectionURL = url; // We'll remember that when the job finishes
     if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
     // Tell the user that we haven't finished yet
     emit redirection(this, m_redirectionURL);
}

void StatJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        //kDebug(7007) << "StatJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

void StatJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

StatJob *KIO::stat(const KUrl& url, bool showProgressInfo)
{
    // Assume sideIsSource. Gets are more common than puts.
    return stat( url, true, 2, showProgressInfo );
}

StatJob *KIO::stat(const KUrl& url, bool sideIsSource, short int details, bool showProgressInfo)
{
    kDebug(7007) << "stat " << url << endl;
    KIO_ARGS << url;
    StatJob * job = new StatJob(url, CMD_STAT, packedArgs, showProgressInfo );
    job->setSide( sideIsSource );
    job->setDetails( details );
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

SimpleJob *KIO::http_update_cache( const KUrl& url, bool no_cache, time_t expireDate)
{
    assert( (url.protocol() == "http") || (url.protocol() == "https") );
    // Send http update_cache command (2)
    KIO_ARGS << (int)2 << url << no_cache << qlonglong(expireDate);
    SimpleJob * job = new SimpleJob( url, CMD_SPECIAL, packedArgs, false );
    Scheduler::scheduleJob(job);
    return job;
}

//////////

TransferJob::TransferJob( const KUrl& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData,
                          bool showProgressInfo)
    : SimpleJob(url, command, packedArgs, showProgressInfo), staticData( _staticData)
{
    m_internalSuspended = false;
    m_errorPage = false;
    m_subJob = 0L;
    if ( showProgressInfo )
        Observer::self()->slotTransferring( this, url );
}

// Slave sends data
void TransferJob::slotData( const QByteArray &_data)
{
    if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error())
      emit data( this, _data);
}

// Slave got a redirection request
void TransferJob::slotRedirection( const KUrl &url)
{
     kDebug(7007) << "TransferJob::slotRedirection(" << url << ")" << endl;
     if (!KAuthorized::authorizeUrlAction("redirect", m_url, url))
     {
       kWarning(7007) << "TransferJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       return;
     }

    // Some websites keep redirecting to themselves where each redirection
    // acts as the stage in a state-machine. We define "endless redirections"
    // as 5 redirections to the same URL.
    if (m_redirectionList.count(url) > 5)
    {
       kDebug(7007) << "TransferJob::slotRedirection: CYCLIC REDIRECTION!" << endl;
       setError( ERR_CYCLIC_LINK );
       setErrorText( m_url.prettyUrl() );
    }
    else
    {
       m_redirectionURL = url; // We'll remember that when the job finishes
       if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
          m_redirectionURL.setUser(m_url.user()); // Preserve user
       m_redirectionList.append(url);
       m_outgoingMetaData["ssl_was_in_use"] = m_incomingMetaData["ssl_in_use"];
       // Tell the user that we haven't finished yet
       emit redirection(this, m_redirectionURL);
    }
}

void TransferJob::slotFinished()
{
   //kDebug(7007) << "TransferJob::slotFinished(" << this << ", " << m_url << ")" << endl;
    if (m_redirectionURL.isEmpty() || !m_redirectionURL.isValid())
        SimpleJob::slotFinished();
    else {
        //kDebug(7007) << "TransferJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        // Honour the redirection
        // We take the approach of "redirecting this same job"
        // Another solution would be to create a subjob, but the same problem
        // happens (unpacking+repacking)
        staticData.truncate(0);
        m_incomingMetaData.clear();
        if (queryMetaData("cache") != "reload")
            addMetaData("cache","refresh");
        m_internalSuspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        // The very tricky part is the packed arguments business
        QString dummyStr;
        KUrl dummyUrl;
        QDataStream istream( m_packedArgs );
        switch( m_command ) {
            case CMD_GET: {
                m_packedArgs.truncate(0);
                QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                stream << m_url;
                break;
            }
            case CMD_PUT: {
                int permissions;
                qint8 iOverwrite, iResume;
                istream >> dummyUrl >> iOverwrite >> iResume >> permissions;
                m_packedArgs.truncate(0);
                QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                stream << m_url << iOverwrite << iResume << permissions;
                break;
            }
            case CMD_SPECIAL: {
                int specialcmd;
                istream >> specialcmd;
                if (specialcmd == 1) // HTTP POST
                {
                   addMetaData("cache","reload");
                   m_packedArgs.truncate(0);
                   QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
                   stream << m_url;
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

void TransferJob::setAsyncDataEnabled(bool enabled)
{
    if (enabled)
       extraFlags() |= EF_TransferJobAsync;
    else
       extraFlags() &= ~EF_TransferJobAsync;
}

void TransferJob::sendAsyncData(const QByteArray &dataForSlave)
{
    if (extraFlags() & EF_TransferJobNeedData)
    {
       m_slave->send( MSG_DATA, dataForSlave );
       if (extraFlags() & EF_TransferJobDataSent)
       {
           KIO::filesize_t size = processedSize()+dataForSlave.size();
           setProcessedSize(size);
           emit processedSize( this, size );
           if ( size > m_totalSize ) {
               slotTotalSize(size); // safety
           }
           emitPercent( size, m_totalSize );
       }
    }

    extraFlags() &= ~EF_TransferJobNeedData;
}

void TransferJob::setReportDataSent(bool enabled)
{
    if (enabled)
       extraFlags() |= EF_TransferJobDataSent;
    else
       extraFlags() &= ~EF_TransferJobDataSent;
}

bool TransferJob::reportDataSent()
{
    return (extraFlags() & EF_TransferJobDataSent);
}


// Slave requests data
void TransferJob::slotDataReq()
{
    QByteArray dataForSlave;

    extraFlags() |= EF_TransferJobNeedData;

    if (!staticData.isEmpty())
    {
       dataForSlave = staticData;
       staticData = QByteArray();
    }
    else
    {
       emit dataReq( this, dataForSlave);

       if (extraFlags() & EF_TransferJobAsync)
          return;
    }

    static const int max_size = 14 * 1024 * 1024;
    if (dataForSlave.size() > max_size)
    {
       kDebug(7007) << "send " << dataForSlave.size() / 1024 / 1024 << "MB of data in TransferJob::dataReq. This needs to be splitted, which requires a copy. Fix the application.\n";
       staticData = QByteArray(dataForSlave.data() + max_size ,  dataForSlave.size() - max_size);
       dataForSlave.truncate(max_size);
    }

    sendAsyncData(dataForSlave);

    if (m_subJob)
    {
       // Bitburger protocol in action
       internalSuspend(); // Wait for more data from subJob.
       m_subJob->internalResume(); // Ask for more!
    }
}

void TransferJob::slotMimetype( const QString& type )
{
    m_mimetype = type;
    emit mimetype( this, m_mimetype);
}


void TransferJob::internalSuspend()
{
    m_internalSuspended = true;
    if (m_slave)
       m_slave->suspend();
}

void TransferJob::internalResume()
{
    m_internalSuspended = false;
    if ( m_slave && !isSuspended() )
       m_slave->resume();
}

void TransferJob::resume()
{
    SimpleJob::resume();
    if ( m_internalSuspended )
        internalSuspend();
}

void TransferJob::start(Slave *slave)
{
    assert(slave);
    connect( slave, SIGNAL( data( const QByteArray & ) ),
             SLOT( slotData( const QByteArray & ) ) );

    connect( slave, SIGNAL( dataReq() ),
             SLOT( slotDataReq() ) );

    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    connect( slave, SIGNAL(mimeType( const QString& ) ),
             SLOT( slotMimetype( const QString& ) ) );

    connect( slave, SIGNAL(errorPage() ),
             SLOT( slotErrorPage() ) );

    connect( slave, SIGNAL( needSubUrlData() ),
             SLOT( slotNeedSubUrlData() ) );

    connect( slave, SIGNAL(canResume( KIO::filesize_t ) ),
             SLOT( slotCanResume( KIO::filesize_t ) ) );

    if (slave->suspended())
    {
       m_mimetype = "unknown";
       // WABA: The slave was put on hold. Resume operation.
       slave->resume();
    }

    SimpleJob::start(slave);
    if (m_internalSuspended)
       slave->suspend();
}

void TransferJob::slotNeedSubUrlData()
{
    // Job needs data from subURL.
    m_subJob = KIO::get( m_subUrl, false, false);
    internalSuspend(); // Put job on hold until we have some data.
    connect(m_subJob, SIGNAL( data(KIO::Job*,const QByteArray &)),
            SLOT( slotSubUrlData(KIO::Job*,const QByteArray &)));
    addSubjob(m_subJob);
}

void TransferJob::slotSubUrlData(KIO::Job*, const QByteArray &data)
{
    // The Alternating Bitburg protocol in action again.
    staticData = data;
    m_subJob->internalSuspend(); // Put job on hold until we have delivered the data.
    internalResume(); // Activate ourselves again.
}

void TransferJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

void TransferJob::slotErrorPage()
{
    m_errorPage = true;
}

void TransferJob::slotCanResume( KIO::filesize_t offset )
{
    emit canResume(this, offset);
}

void TransferJob::slotResult( KJob *job)
{
   // This can only be our suburl.
   assert(job == m_subJob);

   SimpleJob::slotResult( job );

   if (!error() && job == m_subJob)
   {
      m_subJob = 0; // No action required
      internalResume(); // Make sure we get the remaining data.
   }
}

TransferJob *KIO::get( const KUrl& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    TransferJob * job = new TransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

FileJob *KIO::open( const KUrl& url, int access )
{
    // Send decoded path and encoded query
    KIO_ARGS << url << access;
    FileJob * job = new FileJob( url, packedArgs );
    return job;
}

class PostErrorJob : public TransferJob
{
public:

  PostErrorJob(int _error, const QString& url, const QByteArray &packedArgs, const QByteArray &postData, bool showProgressInfo)
      : TransferJob(KUrl(), CMD_SPECIAL, packedArgs, postData, showProgressInfo)
  {
    setError( _error );
    setErrorText( url );
  }

};

TransferJob *KIO::http_post( const KUrl& url, const QByteArray &postData, bool showProgressInfo )
{
    int _error = 0;

    // filter out some malicious ports
    static const int bad_ports[] = {
        1,   // tcpmux
        7,   // echo
        9,   // discard
        11,   // systat
        13,   // daytime
        15,   // netstat
        17,   // qotd
        19,   // chargen
        20,   // ftp-data
        21,   // ftp-cntl
        22,   // ssh
        23,   // telnet
        25,   // smtp
        37,   // time
        42,   // name
        43,   // nicname
        53,   // domain
        77,   // priv-rjs
        79,   // finger
        87,   // ttylink
        95,   // supdup
        101,  // hostriame
        102,  // iso-tsap
        103,  // gppitnp
        104,  // acr-nema
        109,  // pop2
        110,  // pop3
        111,  // sunrpc
        113,  // auth
        115,  // sftp
        117,  // uucp-path
        119,  // nntp
        123,  // NTP
        135,  // loc-srv / epmap
        139,  // netbios
        143,  // imap2
        179,  // BGP
        389,  // ldap
        512,  // print / exec
        513,  // login
        514,  // shell
        515,  // printer
        526,  // tempo
        530,  // courier
        531,  // Chat
        532,  // netnews
        540,  // uucp
        556,  // remotefs
        587,  // sendmail
        601,  //
        989,  // ftps data
        990,  // ftps
        992,  // telnets
        993,  // imap/SSL
        995,  // pop3/SSL
        1080, // SOCKS
        2049, // nfs
        4045, // lockd
        6000, // x11
        6667, // irc
        0};
    for (int cnt=0; bad_ports[cnt]; ++cnt)
        if (url.port() == bad_ports[cnt])
        {
            _error = KIO::ERR_POST_DENIED;
            break;
        }

    if( _error )
    {
	static bool override_loaded = false;
	static QList< int >* overriden_ports = NULL;
	if( !override_loaded )
	{
	    KConfig cfg( "kio_httprc", true );
	    overriden_ports = new QList< int >;
	    *overriden_ports = cfg.readEntry( "OverriddenPorts", QList<int>() );
	    override_loaded = true;
	}
	for( QList< int >::ConstIterator it = overriden_ports->begin();
	     it != overriden_ports->end();
	     ++it )
	    if( overriden_ports->contains( url.port()))
		_error = 0;
    }

    // filter out non https? protocols
    if ((url.protocol() != "http") && (url.protocol() != "https" ))
        _error = KIO::ERR_POST_DENIED;

    bool redirection = false;
    KUrl _url(url);
    if (_url.path().isEmpty())
    {
      redirection = true;
      _url.setPath("/");
    }

    if (!_error && !KAuthorized::authorizeUrlAction("open", KUrl(), _url))
        _error = KIO::ERR_ACCESS_DENIED;

    // if request is not valid, return an invalid transfer job
    if (_error)
    {
        KIO_ARGS << (int)1 << url;
        TransferJob * job = new PostErrorJob(_error, url.prettyUrl(), packedArgs, postData, showProgressInfo);
        return job;
    }

    // Send http post command (1), decoded path and encoded query
    KIO_ARGS << (int)1 << _url;
    TransferJob * job = new TransferJob( _url, CMD_SPECIAL,
                                         packedArgs, postData, showProgressInfo );

    if (redirection)
      QTimer::singleShot(0, job, SLOT(slotPostRedirection()) );

    return job;
}

// http post got redirected from http://host to http://host/ by TransferJob
// We must do this redirection ourselves because redirections by the
// slave change post jobs into get jobs.
void TransferJob::slotPostRedirection()
{
    kDebug(7007) << "TransferJob::slotPostRedirection(" << m_url << ")" << endl;
    // Tell the user about the new url.
    emit redirection(this, m_url);
}


TransferJob *KIO::put( const KUrl& url, int permissions,
                  bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << qint8( overwrite ? 1 : 0 ) << qint8( resume ? 1 : 0 ) << permissions;
    TransferJob * job = new TransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    return job;
}

//////////

StoredTransferJob::StoredTransferJob(const KUrl& url, int command,
                                     const QByteArray &packedArgs,
                                     const QByteArray &_staticData,
                                     bool showProgressInfo)
    : TransferJob( url, command, packedArgs, _staticData, showProgressInfo ),
      m_uploadOffset( 0 )
{
    connect( this, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotStoredData( KIO::Job *, const QByteArray & ) ) );
    connect( this, SIGNAL( dataReq( KIO::Job *, QByteArray & ) ),
             SLOT( slotStoredDataReq( KIO::Job *, QByteArray & ) ) );
}

void StoredTransferJob::setData( const QByteArray& arr )
{
    Q_ASSERT( m_data.isNull() ); // check that we're only called once
    Q_ASSERT( m_uploadOffset == 0 ); // no upload started yet
    m_data = arr;
}

void StoredTransferJob::slotStoredData( KIO::Job *, const QByteArray &data )
{
  // check for end-of-data marker:
  if ( data.size() == 0 )
    return;
  unsigned int oldSize = m_data.size();
  m_data.resize( oldSize + data.size() );
  memcpy( m_data.data() + oldSize, data.data(), data.size() );
}

void StoredTransferJob::slotStoredDataReq( KIO::Job *, QByteArray &data )
{
  // Inspired from kmail's KMKernel::byteArrayToRemoteFile
  // send the data in 64 KB chunks
  const int MAX_CHUNK_SIZE = 64*1024;
  int remainingBytes = m_data.size() - m_uploadOffset;
  if( remainingBytes > MAX_CHUNK_SIZE ) {
    // send MAX_CHUNK_SIZE bytes to the receiver (deep copy)
    data = QByteArray( m_data.data() + m_uploadOffset, MAX_CHUNK_SIZE );
    m_uploadOffset += MAX_CHUNK_SIZE;
    //kDebug() << "Sending " << MAX_CHUNK_SIZE << " bytes ("
    //                << remainingBytes - MAX_CHUNK_SIZE << " bytes remain)\n";
  } else {
    // send the remaining bytes to the receiver (deep copy)
    data = QByteArray( m_data.data() + m_uploadOffset, remainingBytes );
    m_data = QByteArray();
    m_uploadOffset = 0;
    //kDebug() << "Sending " << remainingBytes << " bytes\n";
  }
}

StoredTransferJob *KIO::storedGet( const KUrl& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

StoredTransferJob *KIO::storedPut( const QByteArray& arr, const KUrl& url, int permissions,
                                   bool overwrite, bool resume, bool showProgressInfo )
{
    KIO_ARGS << url << qint8( overwrite ? 1 : 0 ) << qint8( resume ? 1 : 0 ) << permissions;
    StoredTransferJob * job = new StoredTransferJob( url, CMD_PUT, packedArgs, QByteArray(), showProgressInfo );
    job->setData( arr );
    return job;
}

//////////

MimetypeJob::MimetypeJob( const KUrl& url, int command,
                  const QByteArray &packedArgs, bool showProgressInfo )
    : TransferJob(url, command, packedArgs, QByteArray(), showProgressInfo)
{
}

void MimetypeJob::start(Slave *slave)
{
    TransferJob::start(slave);
}


void MimetypeJob::slotFinished( )
{
    //kDebug(7007) << "MimetypeJob::slotFinished()" << endl;
    if ( error() == KIO::ERR_IS_DIRECTORY )
    {
        // It is in fact a directory. This happens when HTTP redirects to FTP.
        // Due to the "protocol doesn't support listing" code in KRun, we
        // assumed it was a file.
        kDebug(7007) << "It is in fact a directory!" << endl;
        m_mimetype = QString::fromLatin1("inode/directory");
        emit TransferJob::mimetype( this, m_mimetype );
        setError( 0 );
    }
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error() )
    {
        // Return slave to the scheduler
        TransferJob::slotFinished();
    } else {
        //kDebug(7007) << "MimetypeJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        staticData.truncate(0);
        m_internalSuspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

MimetypeJob *KIO::mimetype(const KUrl& url, bool showProgressInfo )
{
    KIO_ARGS << url;
    MimetypeJob * job = new MimetypeJob(url, CMD_MIMETYPE, packedArgs, showProgressInfo);
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

//////////////////////////

DirectCopyJob::DirectCopyJob( const KUrl& url, int command,
                              const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo)
{
}

void DirectCopyJob::start( Slave* slave )
{
    connect( slave, SIGNAL(canResume( KIO::filesize_t ) ),
             SLOT( slotCanResume( KIO::filesize_t ) ) );
    SimpleJob::start(slave);
}

void DirectCopyJob::slotCanResume( KIO::filesize_t offset )
{
    emit canResume(this, offset);
}

//////////////////////////


class FileCopyJob::FileCopyJobPrivate
{
public:
    KIO::filesize_t m_sourceSize;
    time_t m_modificationTime;
    SimpleJob *m_delJob;
};

/*
 * The FileCopyJob works according to the famous Bayern
 * 'Alternating Bitburger Protocol': we either drink a beer or we
 * we order a beer, but never both at the same time.
 * Tranlated to io-slaves: We alternate between receiving a block of data
 * and sending it away.
 */
FileCopyJob::FileCopyJob( const KUrl& src, const KUrl& dest, int permissions,
                          bool move, bool overwrite, bool resume, bool showProgressInfo)
    : Job(showProgressInfo), m_src(src), m_dest(dest),
      m_permissions(permissions), m_move(move), m_overwrite(overwrite), m_resume(resume),
      m_totalSize(0),d(new FileCopyJobPrivate)
{
   if (showProgressInfo && !move)
      Observer::self()->slotCopying( this, src, dest );
   else if (showProgressInfo && move)
      Observer::self()->slotMoving( this, src, dest );

    //kDebug(7007) << "FileCopyJob::FileCopyJob()" << endl;
    m_moveJob = 0;
    m_copyJob = 0;
    m_getJob = 0;
    m_putJob = 0;
    d->m_delJob = 0;
    d->m_sourceSize = (KIO::filesize_t) -1;
    d->m_modificationTime = static_cast<time_t>( -1 );
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void FileCopyJob::slotStart()
{
   if ( m_move )
   {
      // The if() below must be the same as the one in startBestCopyMethod
      if ((m_src.protocol() == m_dest.protocol()) &&
          (m_src.host() == m_dest.host()) &&
          (m_src.port() == m_dest.port()) &&
          (m_src.user() == m_dest.user()) &&
          (m_src.pass() == m_dest.pass()) &&
          !m_src.hasSubUrl() && !m_dest.hasSubUrl())
      {
         startRenameJob(m_src);
         return;
      }
      else if (m_src.isLocalFile() && KProtocolManager::canRenameFromFile(m_dest))
      {
         startRenameJob(m_dest);
         return;
      }
      else if (m_dest.isLocalFile() && KProtocolManager::canRenameToFile(m_src))
      {
         startRenameJob(m_src);
         return;
      }
      // No fast-move available, use copy + del.
   }
   startBestCopyMethod();
}

void FileCopyJob::startBestCopyMethod()
{
   if ((m_src.protocol() == m_dest.protocol()) &&
       (m_src.host() == m_dest.host()) &&
       (m_src.port() == m_dest.port()) &&
       (m_src.user() == m_dest.user()) &&
       (m_src.pass() == m_dest.pass()) &&
       !m_src.hasSubUrl() && !m_dest.hasSubUrl())
   {
      startCopyJob();
   }
   else if (m_src.isLocalFile() && KProtocolManager::canCopyFromFile(m_dest))
   {
      startCopyJob(m_dest);
   }
   else if (m_dest.isLocalFile() && KProtocolManager::canCopyToFile(m_src))
   {
      startCopyJob(m_src);
   }
   else
   {
      startDataPump();
   }
}

FileCopyJob::~FileCopyJob()
{
    delete d;
}


void FileCopyJob::setSourceSize( KIO::filesize_t size )
{
    d->m_sourceSize = size;
    if (size != (KIO::filesize_t) -1)
       m_totalSize = size;
}

void FileCopyJob::setModificationTime( time_t mtime )
{
    d->m_modificationTime = mtime;
}

void FileCopyJob::startCopyJob()
{
    startCopyJob(m_src);
}

void FileCopyJob::startCopyJob(const KUrl &slave_url)
{
    //kDebug(7007) << "FileCopyJob::startCopyJob()" << endl;
    KIO_ARGS << m_src << m_dest << m_permissions << (qint8) m_overwrite;
    m_copyJob = new DirectCopyJob(slave_url, CMD_COPY, packedArgs, false);
    addSubjob( m_copyJob );
    connectSubjob( m_copyJob );
    connect( m_copyJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
             SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
}

void FileCopyJob::startRenameJob(const KUrl &slave_url)
{
    KIO_ARGS << m_src << m_dest << (qint8) m_overwrite;
    m_moveJob = new SimpleJob(slave_url, CMD_RENAME, packedArgs, false);
    addSubjob( m_moveJob );
    connectSubjob( m_moveJob );
}

void FileCopyJob::connectSubjob( SimpleJob * job )
{
    connect( job, SIGNAL(totalSize( KJob*, qulonglong )),
             this, SLOT( slotTotalSize(KJob*, qulonglong)) );

    connect( job, SIGNAL(processedSize( KJob*, qulonglong )),
             this, SLOT( slotProcessedSize(KJob*, qulonglong)) );

    connect( job, SIGNAL(percent( KJob*, unsigned long )),
             this, SLOT( slotPercent(KJob*, unsigned long)) );

}

void FileCopyJob::slotProcessedSize( KJob *, qulonglong size )
{
    setProcessedSize(size);
    emit processedSize( this, size );
    if ( size > m_totalSize ) {
        slotTotalSize( this, size ); // safety
    }
    emitPercent( size, m_totalSize );
}

void FileCopyJob::slotTotalSize( KJob*, qulonglong size )
{
    if (size > m_totalSize)
    {
        m_totalSize = size;
        emit totalSize( this, m_totalSize );
    }
}

void FileCopyJob::slotPercent( KJob*, unsigned long pct )
{
    if ( pct > percent() )
    {
        setPercent( pct );
        emit percent( this, percent() );
    }
}

void FileCopyJob::startDataPump()
{
    //kDebug(7007) << "FileCopyJob::startDataPump()" << endl;

    m_canResume = false;
    m_resumeAnswerSent = false;
    m_getJob = 0L; // for now
    m_putJob = put( m_dest, m_permissions, m_overwrite, m_resume, false /* no GUI */);
    //kDebug(7007) << "FileCopyJob: m_putJob = " << m_putJob << " m_dest=" << m_dest << endl;
    if ( d->m_modificationTime != static_cast<time_t>( -1 ) ) {
        QDateTime dt; dt.setTime_t( d->m_modificationTime );
        m_putJob->addMetaData( "modified", dt.toString( Qt::ISODate ) );
    }

    // The first thing the put job will tell us is whether we can
    // resume or not (this is always emitted)
    connect( m_putJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
             SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
    connect( m_putJob, SIGNAL(dataReq(KIO::Job *, QByteArray&)),
             SLOT( slotDataReq(KIO::Job *, QByteArray&)));
    addSubjob( m_putJob );
}

void FileCopyJob::slotCanResume( KIO::Job* job, KIO::filesize_t offset )
{
    if ( job == m_putJob || job == m_copyJob )
    {
        //kDebug(7007) << "FileCopyJob::slotCanResume from PUT job. offset=" << KIO::number(offset) << endl;
        if (offset)
        {
            RenameDlg_Result res = R_RESUME;

            if (!KProtocolManager::autoResume() && !m_overwrite)
            {
                QString newPath;
                KIO::Job* job = ( !progressId() && parentJob() ) ? parentJob() : this;
                // Ask confirmation about resuming previous transfer
                res = Observer::self()->open_RenameDlg(
                      job, i18n("File Already Exists"),
                      m_src.url(),
                      m_dest.url(),
                      (RenameDlg_Mode) (M_OVERWRITE | M_RESUME | M_NORENAME), newPath,
                      d->m_sourceSize, offset );
            }

            if ( res == R_OVERWRITE || m_overwrite )
              offset = 0;
            else if ( res == R_CANCEL )
            {
                if ( job == m_putJob )
                    m_putJob->kill( Quietly );
                else
                    m_copyJob->kill( Quietly );
                setError( ERR_USER_CANCELED );
                emitResult();
                return;
            }
        }
        else
            m_resumeAnswerSent = true; // No need for an answer

        if ( job == m_putJob )
        {
            m_getJob = get( m_src, false, false /* no GUI */ );
            //kDebug(7007) << "FileCopyJob: m_getJob = " << m_getJob << endl;
            m_getJob->addMetaData( "errorPage", "false" );
            m_getJob->addMetaData( "AllowCompressedPage", "false" );
            // Set size in subjob. This helps if the slave doesn't emit totalSize.
            if ( d->m_sourceSize != (KIO::filesize_t)-1 )
                m_getJob->slotTotalSize( d->m_sourceSize );
            if (offset)
            {
                //kDebug(7007) << "Setting metadata for resume to " << (unsigned long) offset << endl;
                // TODO KDE4: rename to seek or offset and document it
                // This isn't used only for resuming, but potentially also for extracting (#72302).
                m_getJob->addMetaData( "resume", KIO::number(offset) );

                // Might or might not get emitted
                connect( m_getJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
                         SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
            }
            m_putJob->slave()->setOffset( offset );

            m_putJob->internalSuspend();
            addSubjob( m_getJob );
            connectSubjob( m_getJob ); // Progress info depends on get
            m_getJob->internalResume(); // Order a beer

            connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                     SLOT( slotData(KIO::Job *, const QByteArray&)));
        }
        else // copyjob
        {
            m_copyJob->slave()->sendResumeAnswer( offset != 0 );
        }
    }
    else if ( job == m_getJob )
    {
        // Cool, the get job said ok, we can resume
        m_canResume = true;
        //kDebug(7007) << "FileCopyJob::slotCanResume from the GET job -> we can resume" << endl;

        m_getJob->slave()->setOffset( m_putJob->slave()->offset() );
    }
    else
        kWarning(7007) << "FileCopyJob::slotCanResume from unknown job=" << job
                        << " m_getJob=" << m_getJob << " m_putJob=" << m_putJob << endl;
}

void FileCopyJob::slotData( KIO::Job * , const QByteArray &data)
{
   //kDebug(7007) << "FileCopyJob::slotData" << endl;
   //kDebug(7007) << " data size : " << data.size() << endl;
   assert(m_putJob);
   if (!m_putJob) return; // Don't crash
   m_getJob->internalSuspend();
   m_putJob->internalResume(); // Drink the beer
   m_buffer = data;

   // On the first set of data incoming, we tell the "put" slave about our
   // decision about resuming
   if (!m_resumeAnswerSent)
   {
       m_resumeAnswerSent = true;
       //kDebug(7007) << "FileCopyJob::slotData (first time) -> send resume answer " << m_canResume << endl;
       m_putJob->slave()->sendResumeAnswer( m_canResume );
   }
}

void FileCopyJob::slotDataReq( KIO::Job * , QByteArray &data)
{
   //kDebug(7007) << "FileCopyJob::slotDataReq" << endl;
   if (!m_resumeAnswerSent && !m_getJob)
   {
       // This can't happen (except as a migration bug on 12/10/2000)
       setError( ERR_INTERNAL );
       setErrorText( "'Put' job didn't send canResume or 'Get' job didn't send data!" );
       m_putJob->kill( Quietly );
       emitResult();
       return;
   }
   if (m_getJob)
   {
      m_getJob->internalResume(); // Order more beer
      m_putJob->internalSuspend();
   }
   data = m_buffer;
   m_buffer = QByteArray();
}

void FileCopyJob::slotResult( KJob *job)
{
   //kDebug(7007) << "FileCopyJob this=" << this << " ::slotResult(" << job << ")" << endl;
   // Did job have an error ?
   if ( job->error() )
   {
      if ((job == m_moveJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_moveJob = 0;
         startBestCopyMethod();
         removeSubjob(job);
         return;
      }
      else if ((job == m_copyJob) && (job->error() == ERR_UNSUPPORTED_ACTION))
      {
         m_copyJob = 0;
         startDataPump();
         removeSubjob(job);
         return;
      }
      else if (job == m_getJob)
      {
        m_getJob = 0L;
        if (m_putJob)
          m_putJob->kill( Quietly );
      }
      else if (job == m_putJob)
      {
        m_putJob = 0L;
        if (m_getJob)
          m_getJob->kill( Quietly );
      }
      setError( job->error() );
      setErrorText( job->errorText() );
      emitResult();
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
         d->m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
         addSubjob(d->m_delJob);
      }
   }

   if (job == m_getJob)
   {
      m_getJob = 0; // No action required
      if (m_putJob)
         m_putJob->internalResume();
   }

   if (job == m_putJob)
   {
      //kDebug(7007) << "FileCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      if (m_getJob)
      {
         kWarning(7007) << "WARNING ! Get still going on..." << endl;
         m_getJob->internalResume();
      }
      if (m_move)
      {
         d->m_delJob = file_delete( m_src, false/*no GUI*/ ); // Delete source
         addSubjob(d->m_delJob);
      }
   }

   if (job == d->m_delJob)
   {
      d->m_delJob = 0; // Finished
   }
   removeSubjob(job);
   if ( !hasSubjobs() )
       emitResult();
}

FileCopyJob *KIO::file_copy( const KUrl& src, const KUrl& dest, int permissions,
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, false, overwrite, resume, showProgressInfo );
}

FileCopyJob *KIO::file_move( const KUrl& src, const KUrl& dest, int permissions,
                             bool overwrite, bool resume, bool showProgressInfo)
{
   return new FileCopyJob( src, dest, permissions, true, overwrite, resume, showProgressInfo );
}

SimpleJob *KIO::file_delete( const KUrl& src, bool showProgressInfo)
{
    KIO_ARGS << src << qint8(true); // isFile
    return new SimpleJob(src, CMD_DEL, packedArgs, showProgressInfo );
}

//////////

ListJob::ListJob(const KUrl& u, bool showProgressInfo, bool _recursive, const QString &_prefix, bool _includeHidden) :
    SimpleJob(u, CMD_LISTDIR, QByteArray(), showProgressInfo),
    recursive(_recursive), includeHidden(_includeHidden), prefix(_prefix), m_processedEntries(0)
{
    // We couldn't set the args when calling the parent constructor,
    // so do it now.
    QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
    stream << u;
}

void ListJob::slotListEntries( const KIO::UDSEntryList& list )
{
    // Emit progress info (takes care of emit processedSize and percent)
    m_processedEntries += list.count();
    slotProcessedSize( m_processedEntries );

    if (recursive) {
        UDSEntryList::ConstIterator it = list.begin();
        const UDSEntryList::ConstIterator end = list.end();

        for (; it != end; ++it) {

            const UDSEntry& entry = *it;

            KUrl itemURL;
            const UDSEntry::ConstIterator end2 = entry.end();
            UDSEntry::ConstIterator it2 = entry.find( KIO::UDS_URL );
            if ( it2 != end2 )
                itemURL = it2.value().toString();
            else { // no URL, use the name
                itemURL = url();
                itemURL.addPath( entry.stringValue( KIO::UDS_NAME ) );
            }

            if (entry.isDir() && !entry.isLink()) {
                const QString filename = itemURL.fileName();
                // skip hidden dirs when listing if requested
                if (filename != ".." && filename != "." && (includeHidden || filename[0] != '.')) {
                    ListJob *job = new ListJob(itemURL,
                                               false /*no progress info!*/,
                                               true /*recursive*/,
                                               prefix + filename + '/',
                                               includeHidden);
                    Scheduler::scheduleJob(job);
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
    // exclusion of hidden files also requires the full sweep, but the case for full-listing
    // a single dir is probably common enough to justify the shortcut
    if (prefix.isNull() && includeHidden) {
        emit entries(this, list);
    } else {
        // cull the unwanted hidden dirs and/or parent dir references from the listing, then emit that
        UDSEntryList newlist;

        UDSEntryList::const_iterator it = list.begin();
        const UDSEntryList::const_iterator end = list.end();
        for (; it != end; ++it) {

            // Modify the name in the UDSEntry
            UDSEntry newone = *it;
            const QString filename = newone.stringValue( KIO::UDS_NAME );
            // Avoid returning entries like subdir/. and subdir/.., but include . and .. for
            // the toplevel dir, and skip hidden files/dirs if that was requested
            if (  (prefix.isNull() || (filename != ".." && filename != ".") )
                  && (includeHidden || (filename[0] != '.') )  )
            {
                // ## Didn't find a way to use the iterator instead of re-doing a key lookup
                newone.insert( KIO::UDS_NAME, prefix + filename );
                newlist.append(newone);
            }
        }

        emit entries(this, newlist);
    }
}

void ListJob::gotEntries(KIO::Job *, const KIO::UDSEntryList& list )
{
    // Forward entries received by subjob - faking we received them ourselves
    emit entries(this, list);
}

void ListJob::slotResult( KJob * job )
{
    // If we can't list a subdir, the result is still ok
    // This is why we override Job::slotResult() - to skip error checking
    removeSubjob( job );
    if ( !hasSubjobs() )
        emitResult();
}

void ListJob::slotRedirection( const KUrl & url )
{
     if (!KAuthorized::authorizeUrlAction("redirect", m_url, url))
     {
       kWarning(7007) << "ListJob: Redirection from " << m_url << " to " << url << " REJECTED!" << endl;
       return;
     }
    m_redirectionURL = url; // We'll remember that when the job finishes
    if (m_url.hasUser() && !url.hasUser() && (m_url.host().toLower() == url.host().toLower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
    emit redirection( this, m_redirectionURL );
}

void ListJob::slotFinished()
{
    // Support for listing archives as directories
    if ( error() == KIO::ERR_IS_FILE && m_url.isLocalFile() ) {
        KMimeType::Ptr ptr = KMimeType::findByUrl( m_url, 0, true, true );
        if ( ptr ) {
            QString proto = ptr->property("X-KDE-LocalProtocol").toString();
            if ( !proto.isEmpty() && KProtocolInfo::isKnownProtocol( proto) ) {
                m_redirectionURL = m_url;
                m_redirectionURL.setProtocol( proto );
                setError( 0 );
                emit redirection(this,m_redirectionURL);
            }
        }
    }
    if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error() ) {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {

        //kDebug(7007) << "ListJob: Redirection to " << m_redirectionURL << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KUrl();
        m_packedArgs.truncate(0);
        QDataStream stream( &m_packedArgs, QIODevice::WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

void ListJob::slotMetaData( const KIO::MetaData &_metaData) {
    SimpleJob::slotMetaData(_metaData);
    storeSSLSessionFromJob(m_redirectionURL);
}

ListJob *KIO::listDir( const KUrl& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo,false,QString(),includeHidden);
    return job;
}

ListJob *KIO::listRecursive( const KUrl& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo, true,QString(),includeHidden);
    return job;
}

void ListJob::setUnrestricted(bool unrestricted)
{
    if (unrestricted)
       extraFlags() |= EF_ListJobUnrestricted;
    else
       extraFlags() &= ~EF_ListJobUnrestricted;
}

void ListJob::start(Slave *slave)
{
    if (!KAuthorized::authorizeUrlAction("list", m_url, m_url) && !(extraFlags() & EF_ListJobUnrestricted))
    {
        setError( ERR_ACCESS_DENIED );
        setErrorText( m_url.url() );
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
        return;
    }
    connect( slave, SIGNAL( listEntries( const KIO::UDSEntryList& )),
             SLOT( slotListEntries( const KIO::UDSEntryList& )));
    connect( slave, SIGNAL( totalSize( KIO::filesize_t ) ),
             SLOT( slotTotalSize( KIO::filesize_t ) ) );
    connect( slave, SIGNAL( redirection(const KUrl &) ),
             SLOT( slotRedirection(const KUrl &) ) );

    SimpleJob::start(slave);
}

////

MultiGetJob::MultiGetJob(const KUrl& url,
                         bool showProgressInfo)
 : TransferJob(url, 0, QByteArray(), QByteArray(), showProgressInfo),
   m_currentEntry( 0, KUrl(), MetaData() )
{
}

MultiGetJob::~MultiGetJob()
{
}

void MultiGetJob::get(long id, const KUrl &url, const MetaData &metaData)
{
   GetRequest entry(id, url, metaData);
   entry.metaData["request-id"] = QString::number(id);
   m_waitQueue.append(entry);
}

void MultiGetJob::flushQueue(RequestQueue &queue)
{
   // Use multi-get
   // Scan all jobs in m_waitQueue
   RequestQueue::iterator wqit = m_waitQueue.begin();
   const RequestQueue::iterator wqend = m_waitQueue.end();
   while ( wqit != wqend )
   {
       const GetRequest& entry = *wqit;
      if ((m_url.protocol() == entry.url.protocol()) &&
          (m_url.host() == entry.url.host()) &&
          (m_url.port() == entry.url.port()) &&
          (m_url.user() == entry.url.user()))
      {
         queue.append( entry );
         wqit = m_waitQueue.erase( wqit );
      }
      else
      {
         ++wqit;
      }
   }
   // Send number of URLs, (URL, metadata)*
   KIO_ARGS << (qint32) queue.count();
   RequestQueue::const_iterator qit = queue.begin();
   const RequestQueue::const_iterator qend = queue.end();
   for( ; qit != qend; ++qit )
   {
      stream << (*qit).url << (*qit).metaData;
   }
   m_packedArgs = packedArgs;
   m_command = CMD_MULTI_GET;
   m_outgoingMetaData.clear();
}

void MultiGetJob::start(Slave *slave)
{
   // Add first job from m_waitQueue and add it to m_activeQueue
   GetRequest entry = m_waitQueue.takeFirst();
   m_activeQueue.append(entry);

   m_url = entry.url;

   if (!entry.url.protocol().startsWith("http"))
   {
      // Use normal get
      KIO_ARGS << entry.url;
      m_packedArgs = packedArgs;
      m_outgoingMetaData = entry.metaData;
      m_command = CMD_GET;
      b_multiGetActive = false;
   }
   else
   {
      flushQueue(m_activeQueue);
      b_multiGetActive = true;
   }

   TransferJob::start(slave); // Anything else to do??
}

bool MultiGetJob::findCurrentEntry()
{
   if (b_multiGetActive)
   {
      long id = m_incomingMetaData["request-id"].toLong();
      RequestQueue::const_iterator qit = m_activeQueue.begin();
      const RequestQueue::const_iterator qend = m_activeQueue.end();
      for( ; qit != qend; ++qit )
      {
         if ((*qit).id == id)
         {
            m_currentEntry = *qit;
            return true;
         }
      }
      m_currentEntry.id = 0;
      return false;
   }
   else
   {
      if ( m_activeQueue.isEmpty() )
        return false;
      m_currentEntry = m_activeQueue.first();
      return true;
   }
}

void MultiGetJob::slotRedirection( const KUrl &url)
{
  if (!findCurrentEntry()) return; // Error
  if (!KAuthorized::authorizeUrlAction("redirect", m_url, url))
  {
     kWarning(7007) << "MultiGetJob: Redirection from " << m_currentEntry.url << " to " << url << " REJECTED!" << endl;
     return;
  }
  m_redirectionURL = url;
  if (m_currentEntry.url.hasUser() && !url.hasUser() && (m_currentEntry.url.host().toLower() == url.host().toLower()))
      m_redirectionURL.setUser(m_currentEntry.url.user()); // Preserve user
  get(m_currentEntry.id, m_redirectionURL, m_currentEntry.metaData); // Try again
}


void MultiGetJob::slotFinished()
{
  if (!findCurrentEntry()) return;
  if (m_redirectionURL.isEmpty())
  {
     // No redirection, tell the world that we are finished.
     emit result(m_currentEntry.id);
  }
  m_redirectionURL = KUrl();
  setError( 0 );
  m_incomingMetaData.clear();
  m_activeQueue.removeAll(m_currentEntry);
  if (m_activeQueue.count() == 0)
  {
     if (m_waitQueue.count() == 0)
     {
        // All done
        TransferJob::slotFinished();
     }
     else
     {
        // return slave to pool
        // fetch new slave for first entry in m_waitQueue and call start
        // again.
        m_url = m_waitQueue.first().url;
        slaveDone();
        Scheduler::doJob(this);
     }
  }
}

void MultiGetJob::slotData( const QByteArray &_data)
{
  if(m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || error())
     emit data(m_currentEntry.id, _data);
}

void MultiGetJob::slotMimetype( const QString &_mimetype )
{
  if (b_multiGetActive)
  {
     RequestQueue newQueue;
     flushQueue(newQueue);
     if (!newQueue.isEmpty())
     {
        m_activeQueue += newQueue;
        m_slave->send( m_command, m_packedArgs );
     }
  }
  if (!findCurrentEntry()) return; // Error, unknown request!
  emit mimetype(m_currentEntry.id, _mimetype);
}

MultiGetJob *KIO::multi_get(long id, const KUrl &url, const MetaData &metaData)
{
    MultiGetJob * job = new MultiGetJob( url, false );
    job->get(id, url, metaData);
    return job;
}

// Never defined, never used - what's this code about?
#ifdef CACHE_INFO
CacheInfo::CacheInfo(const KUrl &url)
{
    m_url = url;
}

QString CacheInfo::cachedFileName()
{
   const QChar separator = '_';

   QString CEF = m_url.path();

   int p = CEF.find('/');

   while(p != -1)
   {
      CEF[p] = separator;
      p = CEF.find('/', p);
   }

   QString host = m_url.host().toLower();
   CEF = host + CEF + '_';

   QString dir = KProtocolManager::cacheDir();
   if (dir[dir.length()-1] != '/')
      dir += '/';

   int l = m_url.host().length();
   for(int i = 0; i < l; i++)
   {
      if (host[i].isLetter() && (host[i] != 'w'))
      {
         dir += host[i];
         break;
      }
   }
   if (dir[dir.length()-1] == '/')
      dir += '0';

   unsigned long hash = 0x00000000;
   QString u = m_url.url().toLatin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + '/' + CEF;

   return CEF;
}

QFile *CacheInfo::cachedFile()
{
#ifdef Q_WS_WIN
   const char *mode = (readWrite ? "rb+" : "rb");
#else
   const char *mode = (readWrite ? "r+" : "r");
#endif

   FILE *fs = fopen(QFile::encodeName(CEF), mode); // Open for reading and writing
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // URL
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      int l = strlen(buffer);
      if (l>0)
         buffer[l-1] = 0; // Strip newline
      if (m_.url.url() != buffer)
      {
         ok = false; // Hash collision
      }
   }

   // Creation Date
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_bMustRevalidate = true;
         m_expireDate = currentDate;
      }
   }

   // Expiration Date
   m_cacheExpireDateOffset = ftell(fs);
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      if (m_request.cache == CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_bMustRevalidate = true;
         m_expireDate = date;
      }
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_etag = QString(buffer).trimmed();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_lastModified = QString(buffer).trimmed();
   }

   fclose(fs);

   if (ok)
      return fs;

   unlink( QFile::encodeName(CEF) );
   return 0;

}

void CacheInfo::flush()
{
    cachedFile().remove();
}

void CacheInfo::touch()
{

}
void CacheInfo::setExpireDate(int);
void CacheInfo::setExpireTimeout(int);


int CacheInfo::creationDate();
int CacheInfo::expireDate();
int CacheInfo::expireTimeout();
#endif

#include "jobclasses.moc"
