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

#include <config.h>

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
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <kdatastream.h>
#include <kmainwindow.h>

#include <errno.h>

#include "slave.h"
#include "kio/job.h"
#include "scheduler.h"
#include "kdirwatch.h"
#include "kmimemagic.h"
#include "kprotocolinfo.h"
#include "kprotocolmanager.h"

#include "kio/observer.h"

#include <kdirnotify_stub.h>
#include <ktempfile.h>
#include <dcopclient.h>

using namespace KIO;
template class QPtrList<KIO::Job>;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( packedArgs, IO_WriteOnly ); stream

class Job::JobPrivate
{
public:
    JobPrivate() : m_autoErrorHandling( false ), m_parentJob( 0L ) {}

    bool m_autoErrorHandling;
    QGuardedPtr<QWidget> m_errorParentWidget;
    // Maybe we could use the QObject parent/child mechanism instead
    // (requires a new ctor, and moving the ctor code to some init()).
    Job* m_parentJob;
};

Job::Job(bool showProgressInfo) : QObject(0, "job"), m_error(0), m_percent(0)
   , m_progressId(0), m_speedTimer(0), d( new JobPrivate )
{
    // All jobs delete themselves after emiting 'result'.

    // Notify the UI Server and get a progress id
    if ( showProgressInfo )
    {
        m_progressId = Observer::self()->newJob( this, true );
        //kdDebug(7007) << "Created job " << this << " with progress info -- m_progressId=" << m_progressId << endl;
        // Connect global progress info signals
        connect( this, SIGNAL( percent( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotPercent( KIO::Job*, unsigned long ) ) );
        connect( this, SIGNAL( infoMessage( KIO::Job*, const QString & ) ),
                 Observer::self(), SLOT( slotInfoMessage( KIO::Job*, const QString & ) ) );
        connect( this, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
                 Observer::self(), SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
        connect( this, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
                 Observer::self(), SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
        connect( this, SIGNAL( speed( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
    }
    // Don't exit while this job is running
    kapp->ref();
}

Job::~Job()
{
    delete m_speedTimer;
    delete d;
    kapp->deref();
}

void Job::addSubjob(Job *job, bool inheritMetaData)
{
    //kdDebug(7007) << "addSubjob(" << job << ") this = " << this << endl;
    subjobs.append(job);

    connect( job, SIGNAL(result(KIO::Job*)),
             SLOT(slotResult(KIO::Job*)) );

    // Forward information from that subjob.
    connect( job, SIGNAL(speed( KIO::Job*, unsigned long )),
             SLOT(slotSpeed(KIO::Job*, unsigned long)) );

    connect( job, SIGNAL(infoMessage( KIO::Job*, const QString & )),
             SLOT(slotInfoMessage(KIO::Job*, const QString &)) );

    if (inheritMetaData)
       job->mergeMetaData(m_outgoingMetaData);
}

void Job::removeSubjob( Job *job )
{
    //kdDebug(7007) << "removeSubjob(" << job << ") this = " << this << "  subjobs = " << subjobs.count() << endl;
    subjobs.remove(job);
    if (subjobs.isEmpty())
        emitResult();
}

void Job::emitPercent( KIO::filesize_t processedSize, KIO::filesize_t totalSize )
{
  // calculate percents
  unsigned long ipercent = m_percent;

  if ( totalSize == 0 )
    m_percent = 100;
  else
    m_percent = (unsigned long)(( (float)(processedSize) / (float)(totalSize) ) * 100.0);

  if ( m_percent != ipercent || m_percent == 100 /* for those buggy total sizes that grow */ ) {
    emit percent( this, m_percent );
    //kdDebug(7007) << "Job::emitPercent - percent =  " << (unsigned int) m_percent << endl;
  }
}

void Job::emitSpeed( unsigned long bytes_per_second )
{
  //kdDebug(7007) << "Job " << this << " emitSpeed " << bytes_per_second << endl;
  if ( !m_speedTimer )
  {
    m_speedTimer = new QTimer();
    connect( m_speedTimer, SIGNAL( timeout() ), SLOT( slotSpeedTimeout() ) );
  }
  emit speed( this, bytes_per_second );
  m_speedTimer->start( 5000 );   // 5 seconds interval should be enough
}

void Job::emitResult()
{
  // If we are displaying a progress dialog, remove it first.
  if ( m_progressId ) // Did we get an ID from the observer ?
    Observer::self()->jobFinished( m_progressId );
  if ( m_error && d->m_autoErrorHandling )
    showErrorDialog( d->m_errorParentWidget );
  emit result(this);
  delete this;
}

void Job::kill( bool quietly )
{
  kdDebug(7007) << "Job::kill this=" << this << " m_progressId=" << m_progressId << " quietly=" << quietly << endl;
  // kill all subjobs, without triggering their result slot
  QPtrListIterator<Job> it( subjobs );
  for ( ; it.current() ; ++it )
     (*it)->kill( true );
  subjobs.clear();

  if ( ! quietly ) {
    m_error = ERR_USER_CANCELED;
    emit canceled( this ); // Not very useful (deprecated)
    emitResult();
  } else
  {
    if ( m_progressId ) // in both cases we want to hide the progress window
      Observer::self()->jobFinished( m_progressId );
    delete this;
  }
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
  //kdDebug(7007) << "Job::slotSpeed " << bytes_per_second << endl;
  emitSpeed( bytes_per_second );
}

void Job::slotInfoMessage( KIO::Job*, const QString & msg )
{
  emit infoMessage( this, msg );
}

void Job::slotSpeedTimeout()
{
  //kdDebug(7007) << "slotSpeedTimeout()" << endl;
  // send 0 and stop the timer
  // timer will be restarted only when we receive another speed event
  emit speed( this, 0 );
  m_speedTimer->stop();
}

//Job::errorString is implemented in global.cpp

void Job::showErrorDialog( QWidget * parent )
{
  //kdDebug(7007) << "Job::showErrorDialog parent=" << parent << endl;
  kapp->enableStyles();
  // Show a message box, except for "user canceled" or "no content"
  if ( (m_error != ERR_USER_CANCELED) && (m_error != ERR_NO_CONTENT) ) {
    //old plain error message
    kdDebug() << "Default language: " << KGlobal::locale()->defaultLanguage() << endl;
    if ( 1 )
      KMessageBox::queuedMessageBox( parent, KMessageBox::Error, errorString() );
#if 0
    } else {
      QStringList errors = detailedErrorStrings();
      QString caption, err, detail;
      QStringList::iterator it = errors.begin();
      if ( it != errors.end() )
        caption = *(it++);
      if ( it != errors.end() )
        err = *(it++);
      if ( it != errors.end() )
        detail = *it;
      KMessageBox::queuedDetailedError( parent, err, detail, caption );
    }
#endif
  }
}

void Job::setAutoErrorHandlingEnabled( bool enable, QWidget *parentWidget )
{
  d->m_autoErrorHandling = enable;
  d->m_errorParentWidget = parentWidget;
}

bool Job::isAutoErrorHandlingEnabled() const
{
  return d->m_autoErrorHandling;
}

void Job::setWindow(QWidget *window)
{
  m_window = window;
  KIO::Scheduler::registerWindow(window);
}

QWidget *Job::window() const
{
  return m_window;
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
       return QString::null;
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
    QMapConstIterator<QString,QString> it = values.begin();
    for(;it != values.end(); ++it)
      m_outgoingMetaData.insert(it.key(), it.data());
}

void Job::mergeMetaData( const QMap<QString,QString> &values)
{
    QMapConstIterator<QString,QString> it = values.begin();
    for(;it != values.end(); ++it)
      m_outgoingMetaData.insert(it.key(), it.data(), false);
}

MetaData Job::outgoingMetaData() const
{
    return m_outgoingMetaData;
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
        return;
    }

    if ((m_command == CMD_LISTDIR) && !kapp->authorizeURLAction("list", m_url, m_url))
    {
        m_error = ERR_ACCESS_DENIED;
        m_errorText = m_url.url();
        QTimer::singleShot(0, this, SLOT(slotFinished()) );
        return;
    }

    if (m_url.hasSubURL())
    {
       KURL::List list = KURL::split(m_url);
       KURL::List::Iterator it = list.fromLast();
       m_url = *it;
       list.remove(it);
       m_subUrl = KURL::join(list);
       //kdDebug(7007) << "New URL = "  << m_url.url() << endl;
       //kdDebug(7007) << "Sub URL = "  << m_subUrl.url() << endl;
    }

    Scheduler::doJob(this);
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

    connect( m_slave, SIGNAL( connected() ),
             SLOT( slotConnected() ) );

    connect( m_slave, SIGNAL( finished() ),
             SLOT( slotFinished() ) );

    connect( m_slave, SIGNAL( totalSize( KIO::filesize_t ) ),
             SLOT( slotTotalSize( KIO::filesize_t ) ) );

    connect( m_slave, SIGNAL( processedSize( KIO::filesize_t ) ),
             SLOT( slotProcessedSize( KIO::filesize_t ) ) );

    connect( m_slave, SIGNAL( speed( unsigned long ) ),
             SLOT( slotSpeed( unsigned long ) ) );

    connect( slave, SIGNAL( needProgressId() ),
             SLOT( slotNeedProgressId() ) );

    connect( slave, SIGNAL(metaData( const KIO::MetaData& ) ),
             SLOT( slotMetaData( const KIO::MetaData& ) ) );

    if (m_window)
    {
       QString id;
       addMetaData("window-id", id.setNum(m_window->winId()));
    }

    if (!m_outgoingMetaData.isEmpty())
    {
       KIO_ARGS << m_outgoingMetaData;
       slave->connection()->send( CMD_META_DATA, packedArgs );
    }

    if (!m_subUrl.isEmpty())
    {
       KIO_ARGS << m_subUrl;
       m_slave->connection()->send( CMD_SUBURL, packedArgs );
    }

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
        if ( !m_error )
        {
            KDirNotify_stub allDirNotify( "*", "KDirNotify*" );
            if ( m_command == CMD_MKDIR )
            {
                KURL urlDir( url() );
                urlDir.setPath( urlDir.directory() );
                allDirNotify.FilesAdded( urlDir );
            }
            else if ( m_command == CMD_RENAME )
            {
                KURL src, dst;
                QDataStream str( m_packedArgs, IO_ReadOnly );
                str >> src >> dst;
                if ( src.directory() == dst.directory() ) // For the user, moving isn't renaming. Only renaming is.
                    allDirNotify.FileRenamed( src, dst );
            }
        }
        emitResult();
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
    static uint msgBoxDisplayed = 0;
    if ( msgBoxDisplayed == 0 ) // don't bomb the user with message boxes, only one at a time
    {
        msgBoxDisplayed++;
        KMessageBox::information( 0L, errorText );
        msgBoxDisplayed--;
    }
    // otherwise just discard it.
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
    if ( !m_progressId )
        m_progressId = Observer::self()->newJob( this, false );
    m_slave->setProgressId( m_progressId );
}

void SimpleJob::slotTotalSize( KIO::filesize_t size )
{
    m_totalSize = size;
    emit totalSize( this, size );
}

void SimpleJob::slotProcessedSize( KIO::filesize_t size )
{
    //kdDebug(7007) << "SimpleJob::slotProcessedSize " << KIO::number(size) << endl;
    emit processedSize( this, size );
    if ( size > m_totalSize )
      slotTotalSize(size); // safety

    emitPercent( size, m_totalSize );
}

void SimpleJob::slotSpeed( unsigned long bytes_per_second )
{
    //kdDebug(7007) << "SimpleJob::slotSpeed( " << bytes_per_second << " )" << endl;
    emitSpeed( bytes_per_second );
}

void SimpleJob::slotMetaData( const KIO::MetaData &_metaData)
{
    m_incomingMetaData += _metaData;
}

SimpleJob *KIO::mkdir( const KURL& url, int permissions )
{
    //kdDebug(7007) << "mkdir " << url.prettyURL() << endl;
    KIO_ARGS << url << permissions;
    return new SimpleJob(url, CMD_MKDIR, packedArgs, false);
}

SimpleJob *KIO::rmdir( const KURL& url )
{
    //kdDebug(7007) << "rmdir " << url.prettyURL() << endl;
    KIO_ARGS << url << Q_INT8(false); // isFile is false
    return new SimpleJob(url, CMD_DEL, packedArgs, false);
}

SimpleJob *KIO::chmod( const KURL& url, int permissions )
{
    //kdDebug(7007) << "chmod " << url.prettyURL() << endl;
    KIO_ARGS << url << permissions;
    return new SimpleJob(url, CMD_CHMOD, packedArgs, false);
}

SimpleJob *KIO::rename( const KURL& src, const KURL & dest, bool overwrite )
{
    //kdDebug(7007) << "rename " << src.prettyURL() << " " << dest.prettyURL() << endl;
    KIO_ARGS << src << dest << (Q_INT8) overwrite;
    return new SimpleJob(src, CMD_RENAME, packedArgs, false);
}

SimpleJob *KIO::symlink( const QString& target, const KURL & dest, bool overwrite, bool showProgressInfo )
{
    //kdDebug(7007) << "symlink target=" << target << " " << dest.prettyURL() << endl;
    KIO_ARGS << target << dest << (Q_INT8) overwrite;
    return new SimpleJob(dest, CMD_SYMLINK, packedArgs, showProgressInfo);
}

SimpleJob *KIO::special(const KURL& url, const QByteArray & data, bool showProgressInfo)
{
    //kdDebug(7007) << "special " << url.prettyURL() << endl;
    return new SimpleJob(url, CMD_SPECIAL, data, showProgressInfo);
}

SimpleJob *KIO::mount( bool ro, const char *fstype, const QString& dev, const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(1) << Q_INT8( ro ? 1 : 0 )
             << QString::fromLatin1(fstype) << dev << point;
    SimpleJob *job = special( KURL("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->mounting( job, dev, point );
    return job;
}

SimpleJob *KIO::unmount( const QString& point, bool showProgressInfo )
{
    KIO_ARGS << int(2) << point;
    SimpleJob *job = special( KURL("file:/"), packedArgs, showProgressInfo );
    if ( showProgressInfo )
         Observer::self()->unmounting( job, point );
    return job;
}

//////////

StatJob::StatJob( const KURL& url, int command,
                  const QByteArray &packedArgs, bool showProgressInfo )
    : SimpleJob(url, command, packedArgs, showProgressInfo),
    m_bSource(true), m_details(2)
{
}

void StatJob::start(Slave *slave)
{
    m_outgoingMetaData.replace( "statSide", m_bSource ? "source" : "dest" );
    m_outgoingMetaData.replace( "details", QString::number(m_details) );

    SimpleJob::start(slave);

    connect( m_slave, SIGNAL( statEntry( const KIO::UDSEntry& ) ),
             SLOT( slotStatEntry( const KIO::UDSEntry & ) ) );
    connect( slave, SIGNAL( redirection(const KURL &) ),
             SLOT( slotRedirection(const KURL &) ) );
}

void StatJob::slotStatEntry( const KIO::UDSEntry & entry )
{
    //kdDebug(7007) << "StatJob::slotStatEntry" << endl;
    m_statResult = entry;
}

// Slave got a redirection request
void StatJob::slotRedirection( const KURL &url)
{
     kdDebug(7007) << "StatJob::slotRedirection(" << url.prettyURL() << ")" << endl;
     m_redirectionURL = url; // We'll remember that when the job finishes
     if (m_url.hasUser() && !url.hasUser() && (m_url.host().lower() == url.host().lower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
     // Tell the user that we haven't finished yet
     emit redirection(this, m_redirectionURL);
}

void StatJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed())
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        //kdDebug(7007) << "StatJob: Redirection to " << m_redirectionURL.prettyURL() << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        m_packedArgs.truncate(0);
        QDataStream stream( m_packedArgs, IO_WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

StatJob *KIO::stat(const KURL& url, bool showProgressInfo)
{
    // Assume sideIsSource. Gets are more common than puts.
    return stat( url, true, 2, showProgressInfo );
}

StatJob *KIO::stat(const KURL& url, bool sideIsSource, short int details, bool showProgressInfo)
{
    kdDebug(7007) << "stat " << url.prettyURL() << endl;
    KIO_ARGS << url;
    StatJob * job = new StatJob(url, CMD_STAT, packedArgs, showProgressInfo );
    job->setSide( sideIsSource );
    job->setDetails( details );
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

SimpleJob *KIO::http_update_cache( const KURL& url, bool no_cache, time_t expireDate)
{
    assert( (url.protocol() == "http") || (url.protocol() == "https") );
    // Send http update_cache command (2)
    KIO_ARGS << (int)2 << url << no_cache << expireDate;
    SimpleJob * job = new SimpleJob( url, CMD_SPECIAL, packedArgs, false );
    Scheduler::scheduleJob(job);
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
    m_errorPage = false;
    m_subJob = 0L;
    if ( showProgressInfo )
        Observer::self()->slotTransferring( this, url );
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
    kdDebug(7007) << "TransferJob::slotRedirection(" << url.prettyURL() << ")" << endl;

    // Some websites keep redirecting to themselves where each redirection
    // acts as the stage in a state-machine. We define "endless redirections"
    // as 5 redirections to the same URL.
    if (m_redirectionList.contains(url) > 5)
    {
       kdDebug(7007) << "TransferJob::slotRedirection: CYCLIC REDIRECTION!" << endl;
       m_error = ERR_CYCLIC_LINK;
       m_errorText = m_url.prettyURL();
    }
    else
    {
       m_redirectionURL = url; // We'll remember that when the job finishes
       if (m_url.hasUser() && !url.hasUser() && (m_url.host().lower() == url.host().lower()))
          m_redirectionURL.setUser(m_url.user()); // Preserve user
       m_redirectionList.append(url);
       m_outgoingMetaData["ssl_was_in_use"] = m_incomingMetaData["ssl_in_use"];
       // Tell the user that we haven't finished yet
       emit redirection(this, m_redirectionURL);
    }
}

void TransferJob::slotFinished()
{
   //kdDebug(7007) << "TransferJob::slotFinished(" << this << ", " << m_url.prettyURL() << ")" << endl;
    if (m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed())
        SimpleJob::slotFinished();
    else {
        //kdDebug(7007) << "TransferJob: Redirection to " << m_redirectionURL.prettyURL() << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        // Honour the redirection
        // We take the approach of "redirecting this same job"
        // Another solution would be to create a subjob, but the same problem
        // happens (unpacking+repacking)
        staticData.truncate(0);
        m_incomingMetaData.clear();
        if (queryMetaData("cache") != "reload");
            addMetaData("cache","refresh");
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        // The very tricky part is the packed arguments business
        QString dummyStr;
        KURL dummyUrl;
        QDataStream istream( m_packedArgs, IO_ReadOnly );
        switch( m_command ) {
            case CMD_GET: {
                m_packedArgs.truncate(0);
                QDataStream stream( m_packedArgs, IO_WriteOnly );
                stream << m_url;
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
                   addMetaData("cache","reload");
                   m_packedArgs.truncate(0);
                   QDataStream stream( m_packedArgs, IO_WriteOnly );
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
        emit dataReq( this, dataForSlave);

    static const size_t max_size = 14 * 1024 * 1024;
    if (dataForSlave.size() > max_size) {
        kdDebug() << "send " << dataForSlave.size() / 1024 / 1024 << "MB of data in TransferJob::dataReq. This needs to be splitted, which requires a copy. Fix the application.\n";
        staticData.duplicate(dataForSlave.data() + max_size ,  dataForSlave.size() - max_size);
        dataForSlave.truncate(max_size);
    }

    m_slave->connection()->send( MSG_DATA, dataForSlave );
    if (m_subJob)
    {
       // Bitburger protocol in action
       suspend(); // Wait for more data from subJob.
       m_subJob->resume(); // Ask for more!
    }
}

void TransferJob::slotMimetype( const QString& type )
{
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

    connect( slave, SIGNAL(errorPage() ),
             SLOT( slotErrorPage() ) );

    connect( slave, SIGNAL( needSubURLData() ),
             SLOT( slotNeedSubURLData() ) );

    connect( slave, SIGNAL(canResume( KIO::filesize_t ) ),
             SLOT( slotCanResume( KIO::filesize_t ) ) );

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

void TransferJob::slotNeedSubURLData()
{
    // Job needs data from subURL.
    m_subJob = KIO::get( m_subUrl, false, false);
    suspend(); // Put job on hold until we have some data.
    connect(m_subJob, SIGNAL( data(KIO::Job*,const QByteArray &)),
            SLOT( slotSubURLData(KIO::Job*,const QByteArray &)));
    addSubjob(m_subJob);
}

void TransferJob::slotSubURLData(KIO::Job*, const QByteArray &data)
{
    // The Alternating Bitburg protocol in action again.
    staticData = data;
    m_subJob->suspend(); // Put job on hold until we have delivered the data.
    resume(); // Activate ourselves again.
}

void TransferJob::slotErrorPage()
{
    m_errorPage = true;
}

void TransferJob::slotCanResume( KIO::filesize_t offset )
{
    emit canResume(this, offset);
}

void TransferJob::slotResult( KIO::Job *job)
{
   // This can only be our suburl.
   assert(job == m_subJob);
   // Did job have an error ?
   if ( job->error() )
   {
      m_error = job->error();
      m_errorText = job->errorText();

      emitResult();
      return;
   }

   if (job == m_subJob)
   {
      m_subJob = 0; // No action required
      resume(); // Make sure we get the remaining data.
   }
   subjobs.remove(job); // Remove job, but don't kill this job.
}

TransferJob *KIO::get( const KURL& url, bool reload, bool showProgressInfo )
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    TransferJob * job = new TransferJob( url, CMD_GET, packedArgs, QByteArray(), showProgressInfo );
    if (reload)
       job->addMetaData("cache", "reload");
    return job;
}

class PostErrorJob : public TransferJob
{
public:

  PostErrorJob(QString url, const QByteArray &packedArgs, const QByteArray &postData, bool showProgressInfo) : TransferJob("", CMD_SPECIAL, packedArgs, postData, showProgressInfo)
  {
    m_error = KIO::ERR_POST_DENIED;
    m_errorText = url;
  }

};

TransferJob *KIO::http_post( const KURL& url, const QByteArray &postData, bool showProgressInfo )
{
    bool valid = true;

    // filter out non https? protocols
    if ((url.protocol() != "http") && (url.protocol() != "https" ))
        valid = false;

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
            valid = false;
            break;
        }

    if( !valid )
    {
	static bool override_loaded = false;
	static QValueList< int >* overriden_ports = NULL;
	if( !override_loaded )
	{
	    KConfig cfg( "kio_httprc", true );
	    overriden_ports = new QValueList< int >;
	    *overriden_ports = cfg.readIntListEntry( "OverriddenPorts" );
	    override_loaded = true;
	}
	for( QValueList< int >::ConstIterator it = overriden_ports->begin();
	     it != overriden_ports->end();
	     ++it )
	if( overriden_ports->contains( url.port()))
	    valid = true;
    }

    // if request is not valid, return an invalid transfer job
    if (!valid)
    {
        KIO_ARGS << (int)1 << url;
        TransferJob * job = new PostErrorJob(url.url(), packedArgs, postData, showProgressInfo);
        return job;
    }

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
    kdDebug(7007) << "MimetypeJob::slotFinished()" << endl;
    if ( m_error == KIO::ERR_IS_DIRECTORY )
    {
        // It is in fact a directory. This happens when HTTP redirects to FTP.
        // Due to the "protocol doesn't support listing" code in KRun, we
        // assumed it was a file.
        kdDebug(7007) << "It is in fact a directory!" << endl;
        m_mimetype = QString::fromLatin1("inode/directory");
        emit TransferJob::mimetype( this, m_mimetype );
        m_error = 0;
    }
    if ( m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error )
    {
        // Return slave to the scheduler
        TransferJob::slotFinished();
    } else {
        //kdDebug(7007) << "MimetypeJob: Redirection to " << m_redirectionURL.prettyURL() << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        staticData.truncate(0);
        m_suspended = false;
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        m_packedArgs.truncate(0);
        QDataStream stream( m_packedArgs, IO_WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

MimetypeJob *KIO::mimetype(const KURL& url, bool showProgressInfo )
{
    KIO_ARGS << url;
    MimetypeJob * job = new MimetypeJob(url, CMD_MIMETYPE, packedArgs, showProgressInfo);
    if ( showProgressInfo )
      Observer::self()->stating( job, url );
    return job;
}

//////////////////////////


class FileCopyJob::FileCopyJobPrivate
{
public:
    off_t m_sourceSize;
    SimpleJob *m_delJob;
};

/*
 * The FileCopyJob works according to the famous Bayern
 * 'Alternating Bitburger Protocol': we either drink a beer or we
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
   if (showProgressInfo && !move)
      Observer::self()->slotCopying( this, src, dest );
   else if (showProgressInfo && move)
      Observer::self()->slotMoving( this, src, dest );

    //kdDebug(7007) << "FileCopyJob::FileCopyJob()" << endl;
    m_moveJob = 0;
    m_copyJob = 0;
    m_getJob = 0;
    m_putJob = 0;
    d = new FileCopyJobPrivate;
    d->m_delJob = 0;
    d->m_sourceSize = (off_t) -1;
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void FileCopyJob::slotStart()
{
    if ((m_src.protocol() == m_dest.protocol()) &&
        (m_src.host() == m_dest.host()) &&
        (m_src.port() == m_dest.port()) &&
        (m_src.user() == m_dest.user()) &&
        (m_src.pass() == m_dest.pass()))
    {
       if (m_move)
       {
          m_moveJob = KIO::rename( m_src, m_dest, m_overwrite );
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
       if (!m_move &&
           (m_src.isLocalFile() && KProtocolInfo::canCopyFromFile(m_dest))
          )
       {
          startCopyJob(m_dest);
       }
       else if (!m_move &&
           (m_dest.isLocalFile() && KProtocolInfo::canCopyToFile(m_src))
          )
       {
          startCopyJob(m_src);
       }
       else
       {
          startDataPump();
       }
    }
}

FileCopyJob::~FileCopyJob()
{
    delete d;
}

void FileCopyJob::setSourceSize( off_t size )
{
    d->m_sourceSize = size;
}

void FileCopyJob::startCopyJob()
{
    startCopyJob(m_src);
}

void FileCopyJob::startCopyJob(const KURL &slave_url)
{
    //kdDebug(7007) << "FileCopyJob::startCopyJob()" << endl;
    KIO_ARGS << m_src << m_dest << m_permissions << (Q_INT8) m_overwrite;
    m_copyJob = new SimpleJob(slave_url, CMD_COPY, packedArgs, false);
    addSubjob( m_copyJob );
    connectSubjob( m_copyJob );
}

void FileCopyJob::connectSubjob( SimpleJob * job )
{
    connect( job, SIGNAL(totalSize( KIO::Job*, KIO::filesize_t )),
             this, SLOT( slotTotalSize(KIO::Job*, KIO::filesize_t)) );

    connect( job, SIGNAL(processedSize( KIO::Job*, KIO::filesize_t )),
             this, SLOT( slotProcessedSize(KIO::Job*, KIO::filesize_t)) );

    connect( job, SIGNAL(percent( KIO::Job*, unsigned long )),
             this, SLOT( slotPercent(KIO::Job*, unsigned long)) );

}

void FileCopyJob::slotProcessedSize( KIO::Job *, KIO::filesize_t size )
{
    emit processedSize( this, size );
    if ( size > m_totalSize )
         slotTotalSize( this, size ); // safety
    emitPercent( size, m_totalSize );
}

void FileCopyJob::slotTotalSize( KIO::Job*, KIO::filesize_t size )
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
    //kdDebug(7007) << "FileCopyJob::startDataPump()" << endl;

    m_canResume = false;
    m_resumeAnswerSent = false;
    m_getJob = 0L; // for now
    m_putJob = put( m_dest, m_permissions, m_overwrite, m_resume, false /* no GUI */);
    //kdDebug(7007) << "FileCopyJob: m_putJob = " << m_putJob << " m_dest=" << m_dest.prettyURL() << endl;

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
    if ( job == m_putJob )
    {
        kdDebug(7007) << "FileCopyJob::slotCanResume from PUT job. offset=" << KIO::number(offset) << endl;
        if (offset)
        {
            RenameDlg_Result res = R_RESUME;

            if (!KProtocolManager::autoResume())
            {
                QString newPath;
                KIO::Job* job = ( !m_progressId && parentJob() ) ? parentJob() : this;
                // Ask confirmation about resuming previous transfer
                res = Observer::self()->open_RenameDlg(
                      job, i18n("File Already Exists"),
                      m_src.prettyURL(0, KURL::StripFileProtocol),
                      m_dest.prettyURL(0, KURL::StripFileProtocol),
                      (RenameDlg_Mode) (M_OVERWRITE | M_RESUME | M_NORENAME), newPath,
                      d->m_sourceSize, offset );
            }

            if ( res == R_OVERWRITE )
              offset = 0;
            else if ( res == R_CANCEL )
            {
                m_putJob->kill(true);
                m_error = ERR_USER_CANCELED;
                emitResult();
                return;
            }
        }
        else
            m_resumeAnswerSent = true; // No need for an answer

        m_getJob = get( m_src, false, false /* no GUI */ );
        //kdDebug(7007) << "FileCopyJob: m_getJob = " << m_getJob << endl;
        m_getJob->addMetaData( "errorPage", "false" );
        m_getJob->addMetaData( "AllowCompressedPage", "false" );
        if (offset)
        {
            kdDebug(7007) << "Setting metadata for resume to " << (unsigned long) offset << endl;
            m_getJob->addMetaData( "resume", KIO::number(offset) );

            // Might or might not get emitted
            connect( m_getJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
                     SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
        }
        m_putJob->slave()->setOffset( offset );

        m_putJob->suspend();
        addSubjob( m_getJob );
        connectSubjob( m_getJob ); // Progress info depends on get
        m_getJob->resume(); // Order a beer

        connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                 SLOT( slotData(KIO::Job *, const QByteArray&)));
    }
    else if ( job == m_getJob )
    {
        // Cool, the get job said ok, we can resume
        m_canResume = true;
        kdDebug(7007) << "FileCopyJob::slotCanResume from the GET job -> we can resume" << endl;

        m_getJob->slave()->setOffset( m_putJob->slave()->offset() );
    }
    else
        kdWarning(7007) << "FileCopyJob::slotCanResume from unknown job=" << job
                        << " m_getJob=" << m_getJob << " m_putJob=" << m_putJob << endl;
}

void FileCopyJob::slotData( KIO::Job * , const QByteArray &data)
{
   //kdDebug(7007) << "FileCopyJob::slotData" << endl;
   //kdDebug(7007) << " data size : " << data.size() << endl;
   assert(m_putJob);
   m_getJob->suspend();
   m_putJob->resume(); // Drink the beer
   m_buffer = data;

   // On the first set of data incoming, we tell the "put" slave about our
   // decision about resuming
   if (!m_resumeAnswerSent)
   {
       m_resumeAnswerSent = true;
       kdDebug(7007) << "FileCopyJob::slotData (first time) -> send resume answer " << m_canResume << endl;
       m_putJob->slave()->sendResumeAnswer( m_canResume );
   }
}

void FileCopyJob::slotDataReq( KIO::Job * , QByteArray &data)
{
   //kdDebug(7007) << "FileCopyJob::slotDataReq" << endl;
   if (!m_resumeAnswerSent && !m_getJob)
   {
       // This can't happen (except as a migration bug on 12/10/2000)
       m_error = ERR_INTERNAL;
       m_errorText = "'Put' job didn't send canResume or 'Get' job didn't send data!";
       m_putJob->kill(true);
       emitResult();
       return;
   }
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
   //kdDebug(7007) << "FileCopyJob this=" << this << " ::slotResult(" << job << ")" << endl;
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
          m_putJob->kill(true);
      }
      else if (job == m_putJob)
      {
        m_putJob = 0L;
        if (m_getJob)
          m_getJob->kill(true);
      }
      m_error = job->error();
      m_errorText = job->errorText();
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
         m_putJob->resume();
   }

   if (job == m_putJob)
   {
      //kdDebug(7007) << "FileCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      if (m_getJob)
      {
         kdWarning(7007) << "WARNING ! Get still going on..." << endl;
         m_getJob->resume();
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

ListJob::ListJob(const KURL& u, bool showProgressInfo, bool _recursive, QString _prefix, bool _includeHidden) :
    SimpleJob(u, CMD_LISTDIR, QByteArray(), showProgressInfo),
    recursive(_recursive), includeHidden(_includeHidden), prefix(_prefix), m_processedEntries(0)
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
        UDSEntryListConstIterator it = list.begin();
        UDSEntryListConstIterator end = list.end();

        for (; it != end; ++it) {
            bool isDir = false;
            bool isLink = false;
            QString filename;

            UDSEntry::ConstIterator it2 = (*it).begin();
            UDSEntry::ConstIterator end2 = (*it).end();
            for( ; it2 != end2; it2++ ) {
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
                // skip hidden dirs when listing if requested
                if (filename != ".." && filename != "." && (includeHidden || filename[0] != '.')) {
                    KURL newone = url();
                    newone.addPath(filename);
                    ListJob *job = new ListJob(newone, m_progressId!=0, true, prefix + filename + "/",includeHidden);
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

        UDSEntryListConstIterator it = list.begin();
        UDSEntryListConstIterator end = list.end();
        for (; it != end; ++it) {

            UDSEntry newone = *it;
            UDSEntry::Iterator it2 = newone.begin();
            QString filename;
            for( ; it2 != newone.end(); it2++ ) {
                if ((*it2).m_uds == UDS_NAME) {
                    filename = (*it2).m_str;
                    (*it2).m_str = prefix + filename;
                }
            }
            // Avoid returning entries like subdir/. and subdir/.., but include . and .. for
            // the the toplevel dir, and skip hidden files/dirs if that was requested
            if (  (prefix.isNull() || (filename != ".." && filename != ".") )
               && (includeHidden || (filename[0] != '.') )  )
                newlist.append(newone);
        }

        emit entries(this, newlist);
    }
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
    m_redirectionURL = url; // We'll remember that when the job finishes
    if (m_url.hasUser() && !url.hasUser() && (m_url.host().lower() == url.host().lower()))
        m_redirectionURL.setUser(m_url.user()); // Preserve user
    emit redirection( this, url );
}

void ListJob::slotFinished()
{
    if ( m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error )
    {
        // Return slave to the scheduler
        SimpleJob::slotFinished();
    } else {
        kdDebug(7007) << "ListJob: Redirection to " << m_redirectionURL.prettyURL() << endl;
        if (queryMetaData("permanent-redirect")=="true")
            emit permanentRedirection(this, m_url, m_redirectionURL);
        m_url = m_redirectionURL;
        m_redirectionURL = KURL();
        m_packedArgs.truncate(0);
        QDataStream stream( m_packedArgs, IO_WriteOnly );
        stream << m_url;

        // Return slave to the scheduler
        slaveDone();
        Scheduler::doJob(this);
    }
}

ListJob *KIO::listDir( const KURL& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo,false,QString::null,includeHidden);
    return job;
}

ListJob *KIO::listRecursive( const KURL& url, bool showProgressInfo, bool includeHidden )
{
    ListJob * job = new ListJob(url, showProgressInfo, true,QString::null,includeHidden);
    return job;
}

void ListJob::start(Slave *slave)
{
    connect( slave, SIGNAL( listEntries( const KIO::UDSEntryList& )),
             SLOT( slotListEntries( const KIO::UDSEntryList& )));
    connect( slave, SIGNAL( totalSize( KIO::filesize_t ) ),
             SLOT( slotTotalSize( KIO::filesize_t ) ) );
    connect( slave, SIGNAL( redirection(const KURL &) ),
             SLOT( slotRedirection(const KURL &) ) );

    SimpleJob::start(slave);
}


CopyJob::CopyJob( const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo )
  : Job(showProgressInfo), m_mode(mode), m_asMethod(asMethod),
    destinationState(DEST_NOT_STATED), state(STATE_STATING),
    m_totalSize(0), m_processedSize(0), m_fileProcessedSize(0),
    m_processedFiles(0), m_processedDirs(0),
    m_srcList(src), m_currentStatSrc(m_srcList.begin()),
    m_bCurrentOperationIsLink(false), m_bSingleFileCopy(false), m_bOnlyRenames(mode==Move),
    m_dest(dest), m_bAutoSkip( false ), m_bOverwriteAll( false ),
    m_conflictError(0), m_reportTimer(0)
{
    if ( showProgressInfo ) {
        connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

        connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
                 Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );
    }
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void CopyJob::slotStart()
{
    /**
       We call the functions directly instead of using signals.
       Calling a function via a signal takes approx. 65 times the time
       compared to calling it directly (at least on my machine). aleXXX
    */
    m_reportTimer = new QTimer(this);

    connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
    m_reportTimer->start(REPORT_TIMEOUT,false);

    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest, false, 2, false );
    //kdDebug(7007) << "CopyJob:stating the dest " << m_dest.prettyURL() << endl;
    addSubjob(job);
}

void CopyJob::slotResultStating( Job *job )
{
    //kdDebug(7007) << "CopyJob::slotResultStating" << endl;
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        KURL srcurl = ((SimpleJob*)job)->url();
        if ( !srcurl.isLocalFile() )
        {
            // Probably : src doesn't exist. Well, over some protocols (e.g. FTP)
            // this info isn't really reliable (thanks to MS FTP servers).
            // We'll assume a file, and try to download anyway.
            kdDebug(7007) << "Error while stating source. Activating hack" << endl;
            subjobs.remove( job );
            assert ( subjobs.isEmpty() ); // We should have only one job at a time ...
            struct CopyInfo info;
            info.permissions = (mode_t) -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (off_t)-1;
            info.uSource = srcurl;
            info.uDest = m_dest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
                info.uDest.addPath( srcurl.fileName() );

            files.append( info );
            ++m_currentStatSrc;
            statNextSrc();
            return;
        }
        // Local file. If stat fails, the file definitely doesn't exist.
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
        else {
            // Treat symlinks to dirs as dirs here, so no test on bLink
            destinationState = bDir ? DEST_IS_DIR : DEST_IS_FILE;
            //kdDebug(7007) << "CopyJob::slotResultStating dest is dir:" << bDir << endl;
        }
        subjobs.remove( job );
        assert ( subjobs.isEmpty() );

        // After knowing what the dest is, we can start stat'ing the first src.
        statNextSrc();
        return;
    }
    // We were stating the current source URL
    m_currentDest = m_dest; // used by slotEntries
    // Create a dummy list with it, for slotEntries
    UDSEntryList lst;
    lst.append(entry);

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

    if ( bDir
         && !bLink // treat symlinks as files (no recursion)
         && m_mode != Link ) // No recursion in Link mode either.
    {
        //kdDebug(7007) << " Source is a directory " << endl;

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
            // Use <desturl>/<directory_copied> as destination, from now on
            m_currentDest.addPath( srcurl.fileName() );
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            m_error = ERR_IS_FILE;
            m_errorText = m_dest.prettyURL();
            emitResult();
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

        startListing( srcurl );
    }
    else
    {
        //kdDebug(7007) << " Source is a file (or a symlink), or we are linking -> no recursive listing " << endl;
        ++m_currentStatSrc;
        statNextSrc();
    }
}

void CopyJob::slotReport()
{
    // If showProgressInfo was set, m_progressId is > 0.
    Observer * observer = m_progressId ? Observer::self() : 0L;
    switch (state) {
        case STATE_COPYING_FILES:
            emit processedFiles( this, m_processedFiles );
            if (observer) observer->slotProcessedFiles(this,m_processedFiles);
            if (m_mode==Move)
            {
                if (observer) observer->slotMoving( this, m_currentSrcURL,m_currentDestURL);
                emit moving( this, m_currentSrcURL, m_currentDestURL);
            }
            else if (m_mode==Link)
            {
                if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL ); // we don't have a slotLinking
                emit linking( this, m_currentSrcURL.path(), m_currentDestURL );
            }
            else
            {
                if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
                emit copying( this, m_currentSrcURL, m_currentDestURL );
            };
            break;

        case STATE_CREATING_DIRS:
            if (observer) {
                observer->slotProcessedDirs( this, m_processedDirs );
                observer->slotCreatingDir( this,m_currentDestURL);
            }
            emit processedDirs( this, m_processedDirs );
            emit creatingDir( this, m_currentDestURL );
            break;

        case STATE_STATING:
        case STATE_LISTING:
            if (observer) observer->slotCopying( this, m_currentSrcURL, m_currentDestURL );
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;

        default:
            break;
    }
};

void CopyJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    UDSEntryListConstIterator it = list.begin();
    UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it) {
        UDSEntry::ConstIterator it2 = (*it).begin();
        struct CopyInfo info;
        info.permissions = -1;
        info.mtime = (time_t) -1;
        info.ctime = (time_t) -1;
        info.size = (off_t)-1;
        QString relName;
        bool isDir = false;
        for( ; it2 != (*it).end(); it2++ ) {
            switch ((*it2).m_uds) {
                case UDS_FILE_TYPE:
                    //info.type = (mode_t)((*it2).m_long);
                    isDir = S_ISDIR( (mode_t)((*it2).m_long) );
                    break;
                case UDS_NAME:
                    relName = (*it2).m_str;
                    break;
                case UDS_LINK_DEST:
                    info.linkDest = (*it2).m_str;
                    break;
                case UDS_ACCESS:
                    info.permissions = ((*it2).m_long);
                    break;
                case UDS_SIZE:
                    info.size = (off_t)((*it2).m_long);
                    m_totalSize += info.size;
                    break;
                case UDS_MODIFICATION_TIME:
                    info.mtime = (time_t)((*it2).m_long);
                    break;
                case UDS_CREATION_TIME:
                    info.ctime = (time_t)((*it2).m_long);
                default:
                    break;
            }
        }
        if (relName != ".." && relName != ".")
        {
            //kdDebug(7007) << "CopyJob::slotEntries '" << relName << "'" << endl;
            info.uSource = ((SimpleJob *)job)->url();
            if ( m_bCurrentSrcIsDir ) // Only if src is a directory. Otherwise uSource is fine as is
                info.uSource.addPath( relName );
            info.uDest = m_currentDest;
            //kdDebug(7007) << "uDest(1)=" << info.uDest.prettyURL() << endl;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
            {
                // Here we _really_ have to add some filename to the dest.
                // Otherwise, we end up with e.g. dest=..../Desktop/ itself.
                // (This can happen when dropping a link to a webpage with no path)
                if ( relName.isEmpty() )
                    info.uDest.addPath( KIO::encodeFileName( info.uSource.prettyURL() ) );
                else
                    info.uDest.addPath( relName );
            }
            //kdDebug(7007) << "uDest(2)=" << info.uDest.prettyURL() << endl;
            if ( info.linkDest.isEmpty() && (isDir /*S_ISDIR(info.type)*/) && m_mode != Link ) // Dir
            {
                dirs.append( info ); // Directories
                if (m_mode == Move)
                    dirsToRemove.append( info.uSource );
            }
            else {
                files.append( info ); // Files and any symlinks
            }
        }
    }
}

void CopyJob::statNextSrc()
{
    if ( m_currentStatSrc != m_srcList.end() )
    {
        m_currentSrcURL = (*m_currentStatSrc);
        if ( m_mode == Link )
        {
            // Skip the "stating the source" stage, we don't need it for linking
            m_currentDest = m_dest;
            struct CopyInfo info;
            info.permissions = -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (off_t)-1;
            info.uSource = m_currentSrcURL;
            info.uDest = m_currentDest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
            {
                if (
                    (m_currentSrcURL.protocol() == info.uDest.protocol()) &&
                    (m_currentSrcURL.host() == info.uDest.host()) &&
                    (m_currentSrcURL.port() == info.uDest.port()) &&
                    (m_currentSrcURL.user() == info.uDest.user()) &&
                    (m_currentSrcURL.pass() == info.uDest.pass()) )
                {
                    // This is the case of creating a real symlink
                    info.uDest.addPath( m_currentSrcURL.fileName() );
                }
                else
                {
                    // Different protocols, we'll create a .desktop file
                    // We have to change the extension anyway, so while we're at it,
                    // name the file like the URL
                    info.uDest.addPath( KIO::encodeFileName( m_currentSrcURL.prettyURL() )+".desktop" );
                }
            }
            files.append( info ); // Files and any symlinks
            ++m_currentStatSrc;
            statNextSrc(); // we could use a loop instead of a recursive call :)
        }
        // If moving, before going for the full stat+[list+]copy+del thing, try to rename
        else if ( m_mode == Move &&
                  (m_currentSrcURL.protocol() == m_dest.protocol()) &&
                  (m_currentSrcURL.host() == m_dest.host()) &&
                  (m_currentSrcURL.port() == m_dest.port()) &&
                  (m_currentSrcURL.user() == m_dest.user()) &&
                  (m_currentSrcURL.pass() == m_dest.pass()) )
        {
            KURL dest = m_dest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
                dest.addPath( m_currentSrcURL.fileName() );
            kdDebug(7007) << "This seems to be a suitable case for trying to rename before stat+[list+]copy+del" << endl;
            state = STATE_RENAMING;
            SimpleJob * newJob = KIO::rename( m_currentSrcURL, dest, false /*no overwrite */);
            Scheduler::scheduleJob(newJob);
            addSubjob( newJob );
            if ( m_currentSrcURL.directory() != dest.directory() ) // For the user, moving isn't renaming. Only renaming is.
                m_bOnlyRenames = false;
        }
        else
        {
            // Stat the next src url
            Job * job = KIO::stat( m_currentSrcURL, true, 2, false );
            //kdDebug(7007) << "KIO::stat on " << (*it).prettyURL() << endl;
            state = STATE_STATING;
            addSubjob(job);
            m_currentDestURL=m_dest;
            m_bOnlyRenames = false;
        }
    } else
    {
        // Finished the stat'ing phase
        // First make sure that the totals were correctly emitted
        state = STATE_STATING;
        slotReport();
        // Check if we are copying a single file
        m_bSingleFileCopy = ( files.count() == 1 && dirs.isEmpty() );
        // Then start copying things
        state = STATE_CREATING_DIRS;
        createNextDir();
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

void CopyJob::skip( const KURL & sourceUrl )
{
    // Check if this is one if toplevel sources
    // IF yes, remove it from m_srcList, for a correct FilesRemoved() signal
    //kdDebug(7007) << "CopyJob::skip: looking for " << sourceUrl.prettyURL() << endl;
    KURL::List::Iterator sit = m_srcList.find( sourceUrl );
    if ( sit != m_srcList.end() )
    {
        //kdDebug(7007) << "CopyJob::skip: removing " << sourceUrl.prettyURL() << " from list" << endl;
        m_srcList.remove( sit );
    }
    dirsToRemove.remove( sourceUrl );
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
            KURL oldURL = ((SimpleJob*)job)->url();
            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // We dont want to copy files in this directory, so we put it on the skip list
                m_skipList.append( oldURL.path( 1 ) );
                skip( oldURL );
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
                SimpleJob * newJob = KIO::stat( existingDest, false, 2, false );
    Scheduler::scheduleJob(newJob);
                kdDebug(7007) << "KIO::stat for resolving conflict on " << existingDest.prettyURL() << endl;
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
    {
       //this is required for the undo feature
        emit copyingDone( this, (*it).uSource, (*it).uDest, true, false );
        dirs.remove( it );
    }

    m_processedDirs++;
    //emit processedDirs( this, m_processedDirs );
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
    KIO::filesize_t destsize = 0;
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
    if (m_reportTimer)
        m_reportTimer->stop();
    RenameDlg_Result r = Observer::self()->open_RenameDlg( this, i18n("Directory Already Exists"),
                                         (*it).uSource.prettyURL(0, KURL::StripFileProtocol),
                                         (*it).uDest.prettyURL(0, KURL::StripFileProtocol),
                                         mode, newPath,
                                         (*it).size, destsize,
                                         (*it).ctime, destctime,
                                         (*it).mtime, destmtime );
    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT,false);
    switch ( r ) {
        case R_CANCEL:
            m_error = ERR_USER_CANCELED;
            emitResult();
            return;
        case R_RENAME:
        {
            QString oldPath = (*it).uDest.path( 1 );
            KURL newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg

            // Change the current one and strip the trailing '/'
            (*it).uDest = newUrl.path( -1 );
            newPath = newUrl.path( 1 ); // With trailing slash
            QValueList<CopyInfo>::Iterator renamedirit = it;
            renamedirit++;
            // Change the name of subdirectories inside the directory
            for( ; renamedirit != dirs.end() ; ++renamedirit )
            {
                QString path = (*renamedirit).uDest.path();
                if ( path.left(oldPath.length()) == oldPath )
                    (*renamedirit).uDest.setPath( path.replace( 0, oldPath.length(), newPath ) );
            }
            // Change filenames inside the directory
            QValueList<CopyInfo>::Iterator renamefileit = files.begin();
            for( ; renamefileit != files.end() ; ++renamefileit )
            {
                QString path = (*renamefileit).uDest.path();
                if ( path.left(oldPath.length()) == oldPath )
                    (*renamefileit).uDest.setPath( path.replace( 0, oldPath.length(), newPath ) );
            }
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            skip( (*it).uSource );
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
    m_processedDirs++;
    //emit processedDirs( this, m_processedDirs );
    createNextDir();
}

void CopyJob::createNextDir()
{
    KURL udir;
    if ( !dirs.isEmpty() )
    {
        // Take first dir to create out of list
        QValueList<CopyInfo>::Iterator it = dirs.begin();
        // Is this URL on the skip list or the overwrite list ?
        while( it != dirs.end() && udir.isEmpty() )
        {
            QString dir = (*it).uDest.path();
            bool bCreateDir = true; // we'll create it if it's not in any list

            QStringList::Iterator sit = m_skipList.begin();
            for( ; sit != m_skipList.end() && bCreateDir; sit++ )
                // Is dir a subdirectory of *sit ?
                if ( *sit == dir.left( (*sit).length() ) )
                    bCreateDir = false; // skip this dir

            if ( !bCreateDir ) {
                dirs.remove( it );
                it = dirs.begin();
            } else
                udir = (*it).uDest;
        }
    }
    if ( !udir.isEmpty() ) // any dir to create, finally ?
    {
        // Create the directory - with default permissions so that we can put files into it
        // TODO : change permissions once all is finished
        KIO::SimpleJob *newjob = KIO::mkdir( udir, -1 );
        Scheduler::scheduleJob(newjob);

        m_currentDestURL = udir;

        addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        state = STATE_COPYING_FILES;
        m_processedFiles++; // Ralf wants it to start a 1, not 0
        copyNextFile();
    }
}

void CopyJob::slotResultCopyingFiles( Job * job )
{
    // The file we were trying to copy:
    QValueList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        // Should we skip automatically ?
        if ( m_bAutoSkip )
        {
            skip( (*it).uSource );
            files.remove( it ); // Move on to next file
        }
        else
        {
            m_conflictError = job->error(); // save for later
            // Existing dest ?
            if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
                 || ( m_conflictError == ERR_DIR_ALREADY_EXIST ) )
            {
                subjobs.remove( job );
                assert ( subjobs.isEmpty() );
                // We need to stat the existing file, to get its last-modification time
                KURL existingFile( (*it).uDest );
                SimpleJob * newJob = KIO::stat( existingFile, false, 2, false );
                Scheduler::scheduleJob(newJob);
                kdDebug(7007) << "KIO::stat for resolving conflict on " << existingFile.prettyURL() << endl;
                state = STATE_CONFLICT_COPYING_FILES;
                addSubjob(newJob);
                return; // Don't move to next file yet !
            }
            else
            {
                if ( m_bCurrentOperationIsLink && job->inherits( "KIO::DeleteJob" ) )
                {
                    // Very special case, see a few lines below
                    // We are deleting the source of a symlink we successfully moved... ignore error
                    files.remove( it );
                } else {
                    // Go directly to the conflict resolution, there is nothing to stat
                    slotResultConflictCopyingFiles( job );
                    return;
                }
            }
        }
    } else // no error
    {
        // Special case for moving links. That operation needs two jobs, unlike others.
        if ( m_bCurrentOperationIsLink && m_mode == Move
             && !job->inherits( "KIO::DeleteJob" ) // Deleting source not already done
             )
        {
            subjobs.remove( job );
            assert ( subjobs.isEmpty() );
            // The only problem with this trick is that the error handling for this del operation
            // is not going to be right... see 'Very special case' above.
            KIO::Job * newjob = KIO::del( (*it).uSource, false /*don't shred*/, false /*no GUI*/ );
            addSubjob( newjob );
            return; // Don't move to next file yet !
        }

        if ( m_bCurrentOperationIsLink )
        {
            QString target = ( m_mode == Link ? (*it).uSource.path() : (*it).linkDest );
            //required for the undo feature
            emit copyingLinkDone( this, (*it).uSource, target, (*it).uDest );
        }
        else
            //required for the undo feature
            emit copyingDone( this, (*it).uSource, (*it).uDest, false, false );
        // remove from list, to move on to next file
        files.remove( it );
    }
    m_processedFiles++;

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

    //kdDebug(7007) << files.count() << " files remaining" << endl;
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

    if (m_reportTimer)
        m_reportTimer->stop();

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
      || ( m_conflictError == ERR_DIR_ALREADY_EXIST ) )
    {
        // Its modification time:
        time_t destmtime = (time_t)-1;
        time_t destctime = (time_t)-1;
        KIO::filesize_t destsize = 0;
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
        // If src==dest, use "overwrite-itself"
        RenameDlg_Mode mode = (RenameDlg_Mode)
            ( ( m_conflictError == ERR_DIR_ALREADY_EXIST ? 0 :
             ( (*it).uSource == (*it).uDest ) ? M_OVERWRITE_ITSELF : M_OVERWRITE ) );
        if ( files.count() > 0 ) // Not last one
            mode = (RenameDlg_Mode) ( mode | M_MULTI | M_SKIP );
        else
            mode = (RenameDlg_Mode) ( mode | M_SINGLE );
        res = Observer::self()->open_RenameDlg( this, m_conflictError == ERR_FILE_ALREADY_EXIST ?
                                i18n("File Already Exists") : i18n("Already Exists as a Directory"),
                                (*it).uSource.prettyURL(0, KURL::StripFileProtocol),
                                (*it).uDest.prettyURL(0, KURL::StripFileProtocol),
                                mode, newPath,
                              (*it).size, destsize,
                              (*it).ctime, destctime,
                              (*it).mtime, destmtime );

    }
    else
    {
        if ( job->error() == ERR_USER_CANCELED )
            res = R_CANCEL;
        else
        {
            SkipDlg_Result skipResult = Observer::self()->open_SkipDlg( this, files.count() > 0,
                                                                        job->errorString() );

            // Convert the return code from SkipDlg into a RenameDlg code
            res = ( skipResult == S_SKIP ) ? R_SKIP :
                         ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
                                        R_CANCEL;
        }
    }

    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT,false);

    subjobs.remove( job );
    assert ( subjobs.isEmpty() );
    switch ( res ) {
        case R_CANCEL:
            m_error = ERR_USER_CANCELED;
            emitResult();
            return;
        case R_RENAME:
        {
            KURL newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg
            (*it).uDest = newUrl;
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            skip( (*it).uSource );
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
    m_processedFiles++;
    //emit processedFiles( this, m_processedFiles );
    copyNextFile();
}

void CopyJob::copyNextFile()
{
    bool bCopyFile = false;
    //kdDebug(7007) << "CopyJob::copyNextFile()" << endl;
    // Take the first file in the list
    QValueList<CopyInfo>::Iterator it = files.begin();
    // Is this URL on the skip list ?
    while (it != files.end() && !bCopyFile)
    {
        bCopyFile = true;
        QString destFile = (*it).uDest.path();

        QStringList::Iterator sit = m_skipList.begin();
        for( ; sit != m_skipList.end() && bCopyFile; sit++ )
            // Is destFile in *sit (or a subdirectory of *sit) ?
            if ( *sit == destFile.left( (*sit).length() ) )
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
        QString destFile = (*it).uDest.path();
        if ( (*it).uDest == (*it).uSource )
            bOverwrite = false;
        else
        {
            // or if on the overwrite list
            QStringList::Iterator sit = m_overwriteList.begin();
            for( ; sit != m_overwriteList.end() && !bOverwrite; sit++ )
                if ( *sit == destFile.left( (*sit).length() ) )
                    bOverwrite = true;
        }

        m_bCurrentOperationIsLink = false;
        KIO::Job * newjob = 0L;
        if ( m_mode == Link )
        {
            //kdDebug(7007) << "Linking" << endl;
            if (
                ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                ((*it).uSource.host() == (*it).uDest.host()) &&
                ((*it).uSource.port() == (*it).uDest.port()) &&
                ((*it).uSource.user() == (*it).uDest.user()) &&
                ((*it).uSource.pass() == (*it).uDest.pass()) )
            {
                // This is the case of creating a real symlink
                KIO::SimpleJob *newJob = KIO::symlink( (*it).uSource.path(), (*it).uDest, bOverwrite, false /*no GUI*/ );
                newjob = newJob;
                Scheduler::scheduleJob(newJob);
                kdDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).uSource.path() << " link=" << (*it).uDest.prettyURL() << endl;
                //emit linking( this, (*it).uSource.path(), (*it).uDest );
                m_bCurrentOperationIsLink = true;
                m_currentSrcURL=(*it).uSource;
                m_currentDestURL=(*it).uDest;
                //Observer::self()->slotCopying( this, (*it).uSource, (*it).uDest ); // should be slotLinking perhaps
            } else {
                kdDebug(7007) << "CopyJob::copyNextFile : Linking URL=" << (*it).uSource.prettyURL() << " link=" << (*it).uDest.prettyURL() << endl;
                if ( (*it).uDest.isLocalFile() )
                {
                    bool devicesOk=false;

                    // if the source is a devices url, handle it a littlebit special
                    if ((*it).uSource.protocol()==QString::fromLatin1("devices"))
                    {
                       QByteArray data;
                       QByteArray param;
                       QCString retType;
                       QDataStream streamout(param,IO_WriteOnly);
                       streamout<<(*it).uSource;
                       streamout<<(*it).uDest;
                       if ( kapp->dcopClient()->call( "kded",
                            "mountwatcher", "createLink(KURL, KURL)", param,retType,data,false ) )
                       {
                          QDataStream streamin(data,IO_ReadOnly);
                          streamin>>devicesOk;
                       }
                       if (devicesOk)
                       {
                           files.remove( it );
                           m_processedFiles++;
                           //emit processedFiles( this, m_processedFiles );
                           copyNextFile();
                           return;
                       }
                    }

                    if (!devicesOk)
                    {
                       QString path = (*it).uDest.path();
                       kdDebug(7007) << "CopyJob::copyNextFile path=" << path << endl;
                       QFile f( path );
                       if ( f.open( IO_ReadWrite ) )
                       {
                           f.close();
                           KSimpleConfig config( path );
                           config.setDesktopGroup();
                           config.writeEntry( QString::fromLatin1("URL"), (*it).uSource.url() );
                           config.writeEntry( QString::fromLatin1("Type"), QString::fromLatin1("Link") );
                           QString protocol = (*it).uSource.protocol();
                           if ( protocol == QString::fromLatin1("ftp") )
                               config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("ftp") );
                           else if ( protocol == QString::fromLatin1("http") )
                               config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("www") );
                           else if ( protocol == QString::fromLatin1("info") )
                               config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("info") );
                           else if ( protocol == QString::fromLatin1("mailto") )   // sven:
                               config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("kmail") ); // added mailto: support
                           else
                               config.writeEntry( QString::fromLatin1("Icon"), QString::fromLatin1("unknown") );
                           config.sync();
                           files.remove( it );
                           m_processedFiles++;
                           //emit processedFiles( this, m_processedFiles );
                           copyNextFile();
                           return;
                       }
                       else
                       {
                           kdDebug(7007) << "CopyJob::copyNextFile ERR_CANNOT_OPEN_FOR_WRITING" << endl;
                           m_error = ERR_CANNOT_OPEN_FOR_WRITING;
                           m_errorText = (*it).uDest.path();
                           emitResult();
                           return;
                       }
                    }
                } else {
                    // Todo: not show "link" on remote dirs if the src urls are not from the same protocol+host+...
                    m_error = ERR_CANNOT_SYMLINK;
                    m_errorText = (*it).uDest.prettyURL();
                    emitResult();
                    return;
                }
            }
        }
        else if ( !(*it).linkDest.isEmpty() &&
                  ((*it).uSource.protocol() == (*it).uDest.protocol()) &&
                  ((*it).uSource.host() == (*it).uDest.host()) &&
                  ((*it).uSource.port() == (*it).uDest.port()) &&
                  ((*it).uSource.user() == (*it).uDest.user()) &&
                  ((*it).uSource.pass() == (*it).uDest.pass()))
            // Copying a symlink - only on the same protocol/host/etc. (#5601, downloading an FTP file through its link),
        {
            KIO::SimpleJob *newJob = KIO::symlink( (*it).linkDest, (*it).uDest, bOverwrite, false /*no GUI*/ );
            Scheduler::scheduleJob(newJob);
            newjob = newJob;
            kdDebug(7007) << "CopyJob::copyNextFile : Linking target=" << (*it).linkDest << " link=" << (*it).uDest.prettyURL() << endl;
            //emit linking( this, (*it).linkDest, (*it).uDest );
            m_currentSrcURL=(*it).linkDest;
            m_currentDestURL=(*it).uDest;
            //Observer::self()->slotCopying( this, (*it).linkDest, (*it).uDest ); // should be slotLinking perhaps
            m_bCurrentOperationIsLink = true;
            // NOTE: if we are moving stuff, the deletion of the source will be done in slotResultCopyingFiles
        } else if (m_mode == Move) // Moving a file
        {
            KIO::FileCopyJob * moveJob = KIO::file_move( (*it).uSource, (*it).uDest, (*it).permissions, bOverwrite, false, false/*no GUI*/ );
            moveJob->setSourceSize( (*it).size );
            newjob = moveJob;
            //kdDebug(7007) << "CopyJob::copyNextFile : Moving " << (*it).uSource.prettyURL() << " to " << (*it).uDest.prettyURL() << endl;
            //emit moving( this, (*it).uSource, (*it).uDest );
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
            //Observer::self()->slotMoving( this, (*it).uSource, (*it).uDest );
        }
        else // Copying a file
        {
            // If source isn't local and target is local, we ignore the original permissions
            // Otherwise, files downloaded from HTTP end up with -r--r--r--
            // But for files coming from TAR, we want to preserve permissions -> we use default perms only if from remote
            // The real fix would be KProtocolInfo::inputType(protocol) == T_FILESYSTEM, but we can't access ksycoca from here !
            bool remoteSource = !(*it).uSource.isLocalFile() && ((*it).uSource.protocol() != "tar"); // HACK
            int permissions = ( remoteSource && (*it).uDest.isLocalFile() ) ? -1 : (*it).permissions;
            KIO::FileCopyJob * copyJob = KIO::file_copy( (*it).uSource, (*it).uDest, permissions, bOverwrite, false, false/*no GUI*/ );
            copyJob->setParentJob( this ); // in case of rename dialog
            copyJob->setSourceSize( (*it).size );
            newjob = copyJob;
            //kdDebug(7007) << "CopyJob::copyNextFile : Copying " << (*it).uSource.prettyURL() << " to " << (*it).uDest.prettyURL() << endl;
            m_currentSrcURL=(*it).uSource;
            m_currentDestURL=(*it).uDest;
        }
        addSubjob(newjob);
        connect( newjob, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
                 this, SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
        connect( newjob, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
                 this, SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
    }
    else
    {
        // We're done
        //kdDebug(7007) << "copyNextFile finished" << endl;
        deleteNextDir();
    }
}

void CopyJob::deleteNextDir()
{
    if ( m_mode == Move && !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        state = STATE_DELETING_DIRS;
        // Take first dir to delete out of list - last ones first !
        KURL::List::Iterator it = dirsToRemove.fromLast();
        SimpleJob *job = KIO::rmdir( *it );
        Scheduler::scheduleJob(job);
        dirsToRemove.remove(it);
        addSubjob( job );
    }
    else
    {
        // Finished - tell the world
        if ( !m_bOnlyRenames )
        {
            KDirNotify_stub allDirNotify("*", "KDirNotify*");
            KURL url( m_dest );
            if ( destinationState != DEST_IS_DIR || m_asMethod )
                url.setPath( url.directory() );
            //kdDebug(7007) << "KDirNotify'ing FilesAdded " << url.prettyURL() << endl;
            allDirNotify.FilesAdded( url );

            if ( m_mode == Move && !m_srcList.isEmpty() )
                allDirNotify.FilesRemoved( m_srcList );
        }
        if (m_reportTimer!=0)
            m_reportTimer->stop();
        emitResult();
    }
}

void CopyJob::slotProcessedSize( KIO::Job*, KIO::filesize_t data_size )
{
  //kdDebug(7007) << "CopyJob::slotProcessedSize " << data_size << endl;
  m_fileProcessedSize = data_size;

  if ( m_processedSize + m_fileProcessedSize > m_totalSize )
  {
    m_totalSize = m_processedSize + m_fileProcessedSize;
    //kdDebug(7007) << "Adjusting m_totalSize to " << (unsigned int) m_totalSize << endl;
    emit totalSize( this, m_totalSize ); // safety
  }
  //kdDebug(7007) << "emit processedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;
  emit processedSize( this, m_processedSize + m_fileProcessedSize );
  emitPercent( m_processedSize + m_fileProcessedSize, m_totalSize );
}

void CopyJob::slotTotalSize( KIO::Job*, KIO::filesize_t size )
{
  // Special case for copying a single file
  // This is because some protocols don't implement stat properly
  // (e.g. HTTP), and don't give us a size in some cases (redirection)
  // so we'd rather rely on the size given for the transfer
  if ( m_bSingleFileCopy )
  {
    //kdDebug(7007) << "Single file -> updating totalsize to " << size << endl;
    m_totalSize = size;
    emit totalSize( this, size );
  }
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
    //kdDebug(7007) << "CopyJob::slotResult() state=" << (int) state << endl;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - subjobs.remove(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to do a direct renaming, before even stat'ing
        {
            int err = job->error();
            subjobs.remove( job );
            assert ( subjobs.isEmpty() );
            if ( err )
            {
                // Determine dest again
                KURL dest = m_dest;
                if ( destinationState == DEST_IS_DIR && !m_asMethod )
                    dest.addPath( m_currentSrcURL.fileName() );
                // Direct renaming didn't work. Try renaming to a temp name,
                // this can help e.g. when renaming 'a' to 'A' on a VFAT partition.
                // In that case it's the _same_ dir, we don't want to copy+del (data loss!)
                if ( m_currentSrcURL.isLocalFile() &&
                     m_currentSrcURL.url(-1).lower() == dest.url(-1).lower() &&
                     ( job->error() == ERR_FILE_ALREADY_EXIST || job->error() == ERR_DIR_ALREADY_EXIST ) )
                {
                    kdDebug(7007) << "Couldn't rename directly, dest already exists. Detected special case of lower/uppercase renaming in same dir, try with 2 rename calls" << endl;
                    QCString _src( QFile::encodeName(m_currentSrcURL.path()) );
                    QCString _dest( QFile::encodeName(dest.path()) );
                    KTempFile tmpFile( m_currentSrcURL.directory() );
                    QCString _tmp( QFile::encodeName(tmpFile.name()) );
                    kdDebug() << "CopyJob::slotResult KTempFile status:" << tmpFile.status() << " using " << _tmp << " as intermediary" << endl;
                    tmpFile.unlink();
                    if ( ::rename( _src, _tmp ) == 0 )
                    {
                        if ( ::rename( _tmp, _dest ) == 0 )
                        {
                            kdDebug(7007) << "Success." << endl;
                            err = 0;
                        }
                        else
                        {
                            // Revert back to original name!
                            bool b = ::rename( QFile::encodeName(tmpFile.name()), _src );
                            if (!b) {
                                kdError(7007) << "Couldn't rename " << tmpFile.name() << " back to " << _src << " !" << endl;
                                // Severe error, abort
                                Job::slotResult( job ); // will set the error and emit result(this)
                                return;
                            }
                        }
                    }
                }
            }
            if ( err )
            {
                m_currentSrcURL=*m_currentStatSrc;
                m_currentDestURL=m_dest;
                kdDebug(7007) << "Couldn't rename, reverting to normal way, starting with stat" << endl;
                Job * job = KIO::stat( m_currentSrcURL, true, 2, false );
                //kdDebug(7007) << "KIO::stat on " << (*it).prettyURL() << endl;
                state = STATE_STATING;
                addSubjob(job);
                m_bOnlyRenames = false;
            }
            else
            {
                kdDebug(7007) << "Renaming succeeded, move on" << endl;
                emit copyingDone( this, *m_currentStatSrc, m_currentDest, true, true );
                ++m_currentStatSrc;
                statNextSrc();
            }
        }
        break;
        case STATE_LISTING: // recursive listing finished
            //kdDebug(7007) << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count() << endl;
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            subjobs.remove( job );
            assert ( subjobs.isEmpty() );

            ++m_currentStatSrc;
            statNextSrc();
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
    //kdDebug() << "KIO::copy src=" << src.url() << " dest=" << dest.url() << endl;
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::copyAs(const KURL& src, const KURL& dest, bool showProgressInfo )
{
    //kdDebug() << "KIO::copyAs src=" << src.url() << " dest=" << dest.url() << endl;
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Copy, true, showProgressInfo );
}

CopyJob *KIO::copy( const KURL::List& src, const KURL& dest, bool showProgressInfo )
{
    return new CopyJob( src, dest, CopyJob::Copy, false, showProgressInfo );
}

CopyJob *KIO::move(const KURL& src, const KURL& dest, bool showProgressInfo )
{
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::moveAs(const KURL& src, const KURL& dest, bool showProgressInfo )
{
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, dest, CopyJob::Move, true, showProgressInfo );
}

CopyJob *KIO::move( const KURL::List& src, const KURL& dest, bool showProgressInfo )
{
    return new CopyJob( src, dest, CopyJob::Move, false, showProgressInfo );
}

CopyJob *KIO::link(const KURL& src, const KURL& destDir, bool showProgressInfo )
{
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::link(const KURL::List& srcList, const KURL& destDir, bool showProgressInfo )
{
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

CopyJob *KIO::linkAs(const KURL& src, const KURL& destDir, bool showProgressInfo )
{
    KURL::List srcList;
    srcList.append( src );
    return new CopyJob( srcList, destDir, CopyJob::Link, false, showProgressInfo );
}

//////////

DeleteJob::DeleteJob( const KURL::List& src, bool shred, bool showProgressInfo )
: Job(showProgressInfo), m_totalSize( 0 ), m_processedSize( 0 ), m_fileProcessedSize( 0 ),
  m_processedFiles( 0 ), m_processedDirs( 0 ), m_totalFilesDirs( 0 ),
  m_srcList(src), m_currentStat(m_srcList.begin()), m_shred(shred), m_reportTimer(0)
{
  if ( showProgressInfo ) {

     connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );

     connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
              Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

     // See slotReport
     /*connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
      m_observer, SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

      connect( this, SIGNAL( deleting( KIO::Job*, const KURL& ) ),
      m_observer, SLOT( slotDeleting( KIO::Job*, const KURL& ) ) );*/

     m_reportTimer=new QTimer(this);
     connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
     //this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
     m_reportTimer->start(REPORT_TIMEOUT,false);
  }

  QTimer::singleShot(0, this, SLOT(slotStart()));
}

void DeleteJob::slotStart()
{
  statNextSrc();
}

//this is called often, so calling the functions
//from Observer here directly might improve the performance a little bit
//aleXXX
void DeleteJob::slotReport()
{
   if (m_progressId==0)
      return;

   Observer * observer = Observer::self();

   emit deleting( this, m_currentURL );
   observer->slotDeleting(this,m_currentURL);

   switch( state ) {
        case STATE_STATING:
        case STATE_LISTING:
            emit totalSize( this, m_totalSize );
            emit totalFiles( this, files.count() );
            emit totalDirs( this, dirs.count() );
            break;
        case STATE_DELETING_DIRS:
            emit processedDirs( this, m_processedDirs );
            observer->slotProcessedDirs(this,m_processedDirs);
            emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );
            break;
        case STATE_DELETING_FILES:
            observer->slotProcessedFiles(this,m_processedFiles);
            emit processedFiles( this, m_processedFiles );
            if (!m_shred)
               emitPercent( m_processedFiles, m_totalFilesDirs );
            break;
   }
}


void DeleteJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
   UDSEntryListConstIterator it = list.begin();
   UDSEntryListConstIterator end = list.end();
   for (; it != end; ++it)
   {
      UDSEntry::ConstIterator it2 = (*it).begin();
      bool bDir = false;
      bool bLink = false;
      QString relName;
      int atomsFound(0);
      for( ; it2 != (*it).end(); it2++ )
      {
         switch ((*it2).m_uds)
         {
         case UDS_FILE_TYPE:
            bDir = S_ISDIR((*it2).m_long);
            atomsFound++;
            break;
         case UDS_NAME:
            relName = ((*it2).m_str);
            atomsFound++;
            break;
         case UDS_LINK_DEST:
            bLink = !(*it2).m_str.isEmpty();
            atomsFound++;
            break;
         case UDS_SIZE:
            m_totalSize += (off_t)((*it2).m_long);
            atomsFound++;
            break;
         default:
            break;
         }
         if (atomsFound==4) break;
      }
      assert(!relName.isEmpty());
      if (relName != ".." && relName != ".")
      {
         KURL url = ((SimpleJob *)job)->url(); // assumed to be a dir
         url.addPath( relName );
         //kdDebug(7007) << "DeleteJob::slotEntries " << relName << " (" << url.prettyURL() << ")" << endl;
         if ( bLink )
            symlinks.append( url );
         else if ( bDir )
            dirs.append( url );
         else
            files.append( url );
      }
   }
}


void DeleteJob::statNextSrc()
{
    //kdDebug(7007) << "statNextSrc" << endl;
    if ( m_currentStat != m_srcList.end() )
    {
        m_currentURL = (*m_currentStat);
        // Stat it
        state = STATE_STATING;
        KIO::SimpleJob * job = KIO::stat( m_currentURL, true, 1, false );
        Scheduler::scheduleJob(job);
        //kdDebug(7007) << "KIO::stat (DeleteJob) " << m_currentURL.prettyURL() << endl;
        addSubjob(job);
        //if ( m_progressId ) // Did we get an ID from the observer ?
        //  Observer::self()->slotDeleting( this, *it ); // show asap
    } else
    {
        m_totalFilesDirs = files.count()+symlinks.count() + dirs.count();
        slotReport();
        // Now we know which dirs hold the files we're going to delete.
        // To speed things up and prevent double-notification, we disable KDirWatch
        // on those dirs temporarily (using KDirWatch::self, that's the instanced
        // used by e.g. kdirlister).
        for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
            KDirWatch::self()->stopDirScan( *it );
        state = STATE_DELETING_FILES;
	deleteNextFile();
    }
}

void DeleteJob::deleteNextFile()
{
    //kdDebug(7007) << "deleteNextFile" << endl;
    if ( !files.isEmpty() || !symlinks.isEmpty() )
    {
        SimpleJob *job;
        do {
            // Take first file to delete out of list
            KURL::List::Iterator it = files.begin();
            bool isLink = false;
            if ( it == files.end() ) // No more files
            {
                it = symlinks.begin(); // Pick up a symlink to delete
                isLink = true;
            }
            // Use shredding ?
            if ( m_shred && (*it).isLocalFile() && !isLink )
            {
                // KShred your KTie
                KIO_ARGS << int(3) << (*it).path();
                job = KIO::special(KURL("file:/"), packedArgs, false /*no GUI*/);
                Scheduler::scheduleJob(job);
                m_currentURL=(*it);
                connect( job, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
                         this, SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
            } else
            {
                // Normal deletion
                // If local file, try do it directly
                if ( (*it).isLocalFile() && unlink( QFile::encodeName((*it).path()) ) == 0 ) {
                    job = 0;
                    m_processedFiles++;
                    if ( m_processedFiles % 300 == 0 ) { // update progress info every 300 files
                        m_currentURL = *it;
                        slotReport();
                    }
                } else
                { // if remote - or if unlink() failed (we'll use the job's error handling in that case)
                    job = KIO::file_delete( *it, false /*no GUI*/);
                    Scheduler::scheduleJob(job);
                    m_currentURL=(*it);
                }
            }
            if ( isLink )
                symlinks.remove(it);
            else
                files.remove(it);
            if ( job ) {
                addSubjob(job);
                return;
            }
            // loop only if direct deletion worked (job=0) and there is something else to delete
        } while (!job && (!files.isEmpty() || !symlinks.isEmpty()));
    }
    state = STATE_DELETING_DIRS;
    deleteNextDir();
}

void DeleteJob::deleteNextDir()
{
    if ( !dirs.isEmpty() ) // some dirs to delete ?
    {
        do {
            // Take first dir to delete out of list - last ones first !
            KURL::List::Iterator it = dirs.fromLast();
            // If local dir, try to rmdir it directly
            if ( (*it).isLocalFile() && ::rmdir( QFile::encodeName((*it).path()) ) == 0 ) {

                m_processedDirs++;
                if ( m_processedDirs % 100 == 0 ) { // update progress info every 100 dirs
                    m_currentURL = *it;
                    slotReport();
                }
            } else
            {
                SimpleJob *job = KIO::rmdir( *it );
                Scheduler::scheduleJob(job);
                dirs.remove(it);
                addSubjob( job );
                return;
            }
            dirs.remove(it);
        } while ( !dirs.isEmpty() );
    }

    // Re-enable watching on the dirs that held the deleted files
    for ( QStringList::Iterator it = m_parentDirs.begin() ; it != m_parentDirs.end() ; ++it )
        KDirWatch::self()->restartDirScan( *it );

    // Finished - tell the world
    if ( !m_srcList.isEmpty() )
    {
        KDirNotify_stub allDirNotify("*", "KDirNotify*");
        allDirNotify.FilesRemoved( m_srcList );
    }
    if (m_reportTimer!=0)
       m_reportTimer->stop();
    emitResult();
}

void DeleteJob::slotProcessedSize( KIO::Job*, KIO::filesize_t data_size )
{
   // Note: this is the same implementation as CopyJob::slotProcessedSize but
   // it's different from FileCopyJob::slotProcessedSize - which is why this
   // is not in Job.

   m_fileProcessedSize = data_size;

   //kdDebug(7007) << "DeleteJob::slotProcessedSize " << (unsigned int) (m_processedSize + m_fileProcessedSize) << endl;

   emit processedSize( this, m_processedSize + m_fileProcessedSize );

   // calculate percents
   unsigned long ipercent = m_percent;

   if ( m_totalSize == 0 )
      m_percent = 100;
   else
      m_percent = (unsigned long)(( (float)(m_processedSize + m_fileProcessedSize) / (float)m_totalSize ) * 100.0);

   if ( m_percent > ipercent )
   {
      emit percent( this, m_percent );
      //kdDebug(7007) << "DeleteJob::slotProcessedSize - percent =  " << (unsigned int) m_percent << endl;
   }

}

void DeleteJob::slotResult( Job *job )
{
   switch ( state )
   {
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
         KIO::filesize_t size = (KIO::filesize_t)-1;
         UDSEntry::ConstIterator it2 = entry.begin();
         int atomsFound(0);
         for( ; it2 != entry.end(); it2++ )
         {
            if ( ((*it2).m_uds) == UDS_FILE_TYPE )
            {
               bDir = S_ISDIR( (mode_t)(*it2).m_long );
               atomsFound++;
            }
            else if ( ((*it2).m_uds) == UDS_LINK_DEST )
            {
               bLink = !((*it2).m_str.isEmpty());
               atomsFound++;
            }
            else if ( ((*it2).m_uds) == UDS_SIZE )
            {
               size = (*it2).m_long;
               atomsFound++;
            };
            if (atomsFound==3) break;
         }

         KURL url = ((SimpleJob*)job)->url();

         subjobs.remove( job );
         assert( subjobs.isEmpty() );

         if (bDir && !bLink)
         {
            // Add toplevel dir in list of dirs
            dirs.append( url );
            if ( url.isLocalFile() && !m_parentDirs.contains( url.path(-1) ) )
                m_parentDirs.append( url.path(-1) );

            //kdDebug(7007) << " Target is a directory " << endl;
            // List it
            state = STATE_LISTING;
            ListJob *newjob = listRecursive( url, false );
            Scheduler::scheduleJob(newjob);
            connect(newjob, SIGNAL(entries( KIO::Job *,
                                            const KIO::UDSEntryList& )),
                    SLOT( slotEntries( KIO::Job*,
                                       const KIO::UDSEntryList& )));
            addSubjob(newjob);
         }
         else
         {
            if ( bLink ) {
                //kdDebug(7007) << " Target is a symlink" << endl;
                symlinks.append( url );
            } else {
                //kdDebug(7007) << " Target is a file" << endl;
                files.append( url );
            }
            if ( url.isLocalFile() && !m_parentDirs.contains( url.directory(-1) ) )
                m_parentDirs.append( url.directory(-1) );
            ++m_currentStat;
            statNextSrc();
         }
      }
      break;
   case STATE_LISTING:
      if ( job->error() )
      {
         // Try deleting nonetheless, it may be empty (and non-listable)
      }
      subjobs.remove( job );
      assert( subjobs.isEmpty() );
      ++m_currentStat;
      statNextSrc();
      break;
   case STATE_DELETING_FILES:
      if ( job->error() )
      {
         Job::slotResult( job ); // will set the error and emit result(this)
         return;
      }
      subjobs.remove( job );
      assert( subjobs.isEmpty() );
      m_processedFiles++;

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
      m_processedDirs++;
      //emit processedDirs( this, m_processedDirs );
      //if (!m_shred)
         //emitPercent( m_processedFiles + m_processedDirs, m_totalFilesDirs );

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

MultiGetJob::MultiGetJob(const KURL& url,
                         bool showProgressInfo)
 : TransferJob(url, 0, QByteArray(), QByteArray(), showProgressInfo)
{
   m_waitQueue.setAutoDelete(true);
   m_activeQueue.setAutoDelete(true);
   m_currentEntry = 0;
}

void MultiGetJob::get(long id, const KURL &url, const MetaData &metaData)
{
   GetRequest *entry = new GetRequest(id, url, metaData);
   entry->metaData["request-id"] = QString("%1").arg(id);
   m_waitQueue.append(entry);
}

void MultiGetJob::flushQueue(QPtrList<GetRequest> &queue)
{
   GetRequest *entry;
   // Use multi-get
   // Scan all jobs in m_waitQueue
   for(entry = m_waitQueue.first(); entry; )
   {
      if ((m_url.protocol() == entry->url.protocol()) &&
          (m_url.host() == entry->url.host()) &&
          (m_url.port() == entry->url.port()) &&
          (m_url.user() == entry->url.user()))
      {
         m_waitQueue.take();
         queue.append(entry);
         entry = m_waitQueue.current();
      }
      else
      {
         entry = m_waitQueue.next();
      }
   }
   // Send number of URLs, (URL, metadata)*
   KIO_ARGS << (Q_INT32) queue.count();
   for(entry = queue.first(); entry; entry = queue.next())
   {
      stream << entry->url << entry->metaData;
   }
   m_packedArgs = packedArgs;
   m_command = CMD_MULTI_GET;
   m_outgoingMetaData.clear();
}

void MultiGetJob::start(Slave *slave)
{
   // Add first job from m_waitQueue and add it to m_activeQueue
   GetRequest *entry = m_waitQueue.take(0);
   m_activeQueue.append(entry);

   m_url = entry->url;

   if (!entry->url.protocol().startsWith("http"))
   {
      // Use normal get
      KIO_ARGS << entry->url;
      m_packedArgs = packedArgs;
      m_outgoingMetaData = entry->metaData;
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
      for(GetRequest *entry = m_activeQueue.first(); entry; entry = m_activeQueue.next())
      {
         if (entry->id == id)
         {
            m_currentEntry = entry;
            return true;
         }
      }
      m_currentEntry = 0;
      return false;
   }
   else
   {
      m_currentEntry = m_activeQueue.first();
      return (m_currentEntry != 0);
   }
}

void MultiGetJob::slotRedirection( const KURL &url)
{
  if (!findCurrentEntry()) return; // Error
  m_redirectionURL = url;
  if (m_currentEntry->url.hasUser() && !url.hasUser() && (m_currentEntry->url.host().lower() == url.host().lower()))
      m_redirectionURL.setUser(m_currentEntry->url.user()); // Preserve user
  get(m_currentEntry->id, m_redirectionURL, m_currentEntry->metaData); // Try again
}


void MultiGetJob::slotFinished()
{
  if (!findCurrentEntry()) return;
  if (m_redirectionURL.isEmpty())
  {
     // No redirection, tell the world that we are finished.
     emit result(m_currentEntry->id);
  }
  m_redirectionURL = KURL();
  m_error = 0;
  m_incomingMetaData.clear();
  m_activeQueue.removeRef(m_currentEntry);
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
        GetRequest *entry = m_waitQueue.at(0);
        m_url = entry->url;
        slaveDone();
        Scheduler::doJob(this);
     }
  }
}

void MultiGetJob::slotData( const QByteArray &_data)
{
  if(!m_currentEntry) return;// Error, unknown request!
  if(m_redirectionURL.isEmpty() || m_redirectionURL.isMalformed() || m_error)
     emit data(m_currentEntry->id, _data);
}

void MultiGetJob::slotMimetype( const QString &_mimetype )
{
  if (b_multiGetActive)
  {
     QPtrList<GetRequest> newQueue;
     flushQueue(newQueue);
     if (!newQueue.isEmpty())
     {
        while(!newQueue.isEmpty())
           m_activeQueue.append(newQueue.take(0));
        m_slave->connection()->send( m_command, m_packedArgs );
     }
  }
  if (!findCurrentEntry()) return; // Error, unknown request!
  emit mimetype(m_currentEntry->id, _mimetype);
}

MultiGetJob *KIO::multi_get(long id, const KURL &url, const MetaData &metaData)
{
    MultiGetJob * job = new MultiGetJob( url, false );
    job->get(id, url, metaData);
    return job;
}


#ifdef CACHE_INFO
CacheInfo::CacheInfo(const KURL &url)
{
    m_url = url;
}

QString CacheInfo::cachedFileName()
{
   const QChar seperator = '_';

   QString CEF = m_url.path();

   int p = CEF.find('/');

   while(p != -1)
   {
      CEF[p] = seperator;
      p = CEF.find('/', p);
   }

   QString host = m_url.host().lower();
   CEF = host + CEF + '_';

   QString dir = KProtocolManager::cacheDir();
   if (dir[dir.length()-1] != '/')
      dir += "/";

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
      dir += "0";

   unsigned long hash = 0x00000000;
   QCString u = m_url.url().latin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + "/" + CEF;

   return CEF;
}

QFile *CacheInfo::cachedFile()
{
   const char *mode = (readWrite ? "r+" : "r");

   FILE *fs = fopen( CEF.latin1(), mode); // Open for reading and writing
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
      m_etag = QString(buffer).stripWhiteSpace();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_lastModified = QString(buffer).stripWhiteSpace();
   }

   fclose(fs);

   if (ok)
      return fs;

   unlink( CEF.latin1());
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

void Job::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void SimpleJob::virtual_hook( int id, void* data )
{ KIO::Job::virtual_hook( id, data ); }

void StatJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void TransferJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void MultiGetJob::virtual_hook( int id, void* data )
{ TransferJob::virtual_hook( id, data ); }

void MimetypeJob::virtual_hook( int id, void* data )
{ TransferJob::virtual_hook( id, data ); }

void FileCopyJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }

void ListJob::virtual_hook( int id, void* data )
{ SimpleJob::virtual_hook( id, data ); }

void CopyJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }

void DeleteJob::virtual_hook( int id, void* data )
{ Job::virtual_hook( id, data ); }


#include "jobclasses.moc"
