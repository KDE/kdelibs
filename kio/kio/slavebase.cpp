/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (c) 2000 David Faure <faure@kde.org>
 *  Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
 *  Copyright (c) 2007 Thiago Macieira <thiago@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 **/

#include "slavebase.h"

#include <config.h>

#include <sys/time.h>

#include <kdebug.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QDateTime>

#include <kapplication.h>
#include <kcrash.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kde_file.h>
#include <kdesu/client.h>
#include <klocale.h>

#include "kremoteencoding.h"

#include "connection.h"
#include "ioslave_defaults.h"
#include "slaveinterface.h"
#include "kpasswdserver_p.h"

#ifndef NDEBUG
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif
#endif

extern "C" {
    static void sigsegv_handler(int sig);
    static void sigpipe_handler(int sig);
}

using namespace KIO;

typedef QList<QByteArray> AuthKeysList;
typedef QMap<QString,QByteArray> AuthKeysMap;
#define KIO_DATA QByteArray data; QDataStream stream( &data, QIODevice::WriteOnly ); stream
#define KIO_FILESIZE_T(x) quint64(x)

namespace KIO {

class SlaveBasePrivate {
public:
    SlaveBase* q;
    SlaveBasePrivate(SlaveBase* owner): q(owner) {}

    UDSEntryList pendingListEntries;
    int listEntryCurrentSize;
    long listEntry_sec, listEntry_usec;
    Connection appConnection;
    QString poolSocket;
    bool isConnectedToApp;
    static qlonglong s_seqNr;

    QString slaveid;
    bool resume:1;
    bool needSendCanResume:1;
    bool onHold:1;
    bool wasKilled:1;
    bool inOpenLoop:1;
    bool exit_loop:1;
    MetaData configData;
    KConfig *config;
    KConfigGroup *configGroup;
    KUrl onHoldUrl;

    struct timeval last_tv;
    KIO::filesize_t totalSize;
    KIO::filesize_t sentListEntries;
    KRemoteEncoding *remotefile;
    time_t timeout;
    enum { Idle, InsideMethod, FinishedCalled, ErrorCalled } m_state;
    QByteArray timeoutData;

    // Reconstructs configGroup from configData and mIncomingMetaData
    void rebuildConfig()
    {
        configGroup->deleteGroup(KConfigGroup::WriteConfigFlags());

        // mIncomingMetaData cascades over config, so we write config first,
        // to let it be overwritten
        MetaData::ConstIterator end = configData.constEnd();
        for (MetaData::ConstIterator it = configData.constBegin(); it != end; ++it)
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());

        end = q->mIncomingMetaData.constEnd();
        for (MetaData::ConstIterator it = q->mIncomingMetaData.constBegin(); it != end; ++it)
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());
    }

    void verifyState(const char* cmdName)
    {
        if ((m_state != FinishedCalled) && (m_state != ErrorCalled)){
            kWarning(7019) << cmdName << "did not call finished() or error()! Please fix the KIO slave.";
        }
    }
    void verifyErrorFinishedNotCalled(const char* cmdName)
    {
        if (m_state == FinishedCalled || m_state == ErrorCalled) {
            kWarning(7019) << cmdName << "called finished() or error(), but it's not supposed to! Please fix the KIO slave.";
        }
    }
};

}

static SlaveBase *globalSlave;
qlonglong SlaveBasePrivate::s_seqNr;

static volatile bool slaveWriteError = false;

static const char *s_protocol;

#ifdef Q_OS_UNIX
extern "C" {
static void genericsig_handler(int sigNumber)
{
   KDE_signal(sigNumber,SIG_IGN);
   //WABA: Don't do anything that requires malloc, we can deadlock on it since
   //a SIGTERM signal can come in while we are in malloc/free.
   //kDebug()<<"kioslave : exiting due to signal "<<sigNumber;
   //set the flag which will be checked in dispatchLoop() and which *should* be checked
   //in lengthy operations in the various slaves
   if (globalSlave!=0)
      globalSlave->setKillFlag();
   KDE_signal(SIGALRM,SIG_DFL);
   alarm(5);  //generate an alarm signal in 5 seconds, in this time the slave has to exit
}
}
#endif

//////////////

SlaveBase::SlaveBase( const QByteArray &protocol,
                      const QByteArray &pool_socket,
                      const QByteArray &app_socket )
    : mProtocol(protocol),
      d(new SlaveBasePrivate(this))

{
    d->poolSocket = QFile::decodeName(pool_socket);
    s_protocol = protocol.data();
#ifdef Q_OS_UNIX
    if (qgetenv("KDE_DEBUG").isEmpty())
    {
        KCrash::setCrashHandler( sigsegv_handler );
        KDE_signal(SIGILL,&sigsegv_handler);
        KDE_signal(SIGTRAP,&sigsegv_handler);
        KDE_signal(SIGABRT,&sigsegv_handler);
        KDE_signal(SIGBUS,&sigsegv_handler);
        KDE_signal(SIGALRM,&sigsegv_handler);
        KDE_signal(SIGFPE,&sigsegv_handler);
#ifdef SIGPOLL
        KDE_signal(SIGPOLL, &sigsegv_handler);
#endif
#ifdef SIGSYS
        KDE_signal(SIGSYS, &sigsegv_handler);
#endif
#ifdef SIGVTALRM
        KDE_signal(SIGVTALRM, &sigsegv_handler);
#endif
#ifdef SIGXCPU
        KDE_signal(SIGXCPU, &sigsegv_handler);
#endif
#ifdef SIGXFSZ
        KDE_signal(SIGXFSZ, &sigsegv_handler);
#endif
    }

    struct sigaction act;
    act.sa_handler = sigpipe_handler;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    sigaction( SIGPIPE, &act, 0 );

    KDE_signal(SIGINT,&genericsig_handler);
    KDE_signal(SIGQUIT,&genericsig_handler);
    KDE_signal(SIGTERM,&genericsig_handler);
#endif

    globalSlave=this;

    d->listEntryCurrentSize = 100;
    struct timeval tp;
    gettimeofday(&tp, 0);
    d->listEntry_sec = tp.tv_sec;
    d->listEntry_usec = tp.tv_usec;
    d->isConnectedToApp = true;

    // by kahl for netmgr (need a way to identify slaves)
    d->slaveid = protocol;
    d->slaveid += QString::number(getpid());
    d->resume = false;
    d->needSendCanResume = false;
    d->config = new KConfig(QString(), KConfig::SimpleConfig);
    // The KConfigGroup needs the KConfig to exist during its whole lifetime.
    d->configGroup = new KConfigGroup(d->config, QString());
    d->onHold = false;
    d->wasKilled=false;
    d->last_tv.tv_sec = 0;
    d->last_tv.tv_usec = 0;
//    d->processed_size = 0;
    d->totalSize=0;
    d->sentListEntries=0;
    d->timeout = 0;
    connectSlave(QFile::decodeName(app_socket));

    d->remotefile = 0;
    d->inOpenLoop = false;
    d->exit_loop = false;
}

SlaveBase::~SlaveBase()
{
    delete d->configGroup;
    delete d->config;
    delete d;
    s_protocol = "";
}

void SlaveBase::dispatchLoop()
{
    while (!d->exit_loop) {
        if (d->timeout && (d->timeout < time(0))) {
            QByteArray data = d->timeoutData;
            d->timeout = 0;
            d->timeoutData = QByteArray();
            special(data);
        }

        Q_ASSERT(d->appConnection.inited());

        int ms = -1;
        if (d->timeout)
            ms = 1000 * qMax<time_t>(d->timeout - time(0), 1);

        int ret = -1;
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(ms)) {
            // dispatch application messages
            int cmd;
            QByteArray data;
            ret = d->appConnection.read(&cmd, data);

            if (ret != -1) {
                if (d->inOpenLoop)
                    dispatchOpenCommand(cmd, data);
                else
                    dispatch(cmd, data);
            }
        } else {
            ret = d->appConnection.isConnected() ? 0 : -1;
        }

        if (ret == -1) { // some error occurred, perhaps no more application
            // When the app exits, should the slave be put back in the pool ?
            if (!d->exit_loop && d->isConnectedToApp && !d->poolSocket.isEmpty()) {
                disconnectSlave();
                d->isConnectedToApp = false;
                closeConnection();
                connectSlave(d->poolSocket);
            } else {
                return;
            }
        }

        //I think we get here when we were killed in dispatch() and not in select()
        if (wasKilled()) {
            kDebug(7019)<<" dispatchLoop() slave was killed, returning";
            return;
        }
    }
}

void SlaveBase::connectSlave(const QString &address)
{
    d->appConnection.connectToRemote(address);

    if (!d->appConnection.inited())
    {
        kDebug(7019) << "SlaveBase: failed to connect to" << address << endl
		      << "Reason:" << d->appConnection.errorString();
        exit();
        return;
    }

    d->inOpenLoop = false;
}

void SlaveBase::disconnectSlave()
{
    d->appConnection.close();
}

void SlaveBase::setMetaData(const QString &key, const QString &value)
{
    mOutgoingMetaData.insert(key, value); // replaces existing key if already there
}

QString SlaveBase::metaData(const QString &key) const
{
   if (mIncomingMetaData.contains(key))
      return mIncomingMetaData[key];
   if (d->configData.contains(key))
      return d->configData[key];
   return QString();
}

MetaData SlaveBase::allMetaData() const
{
    return mIncomingMetaData;
}

bool SlaveBase::hasMetaData(const QString &key) const
{
   if (mIncomingMetaData.contains(key))
      return true;
   if (d->configData.contains(key))
      return true;
   return false;
}

KConfigGroup *SlaveBase::config()
{
   return d->configGroup;
}

void SlaveBase::sendMetaData()
{
    sendAndKeepMetaData();
    mOutgoingMetaData.clear();
}

void SlaveBase::sendAndKeepMetaData()
{
    if (!mOutgoingMetaData.isEmpty()) {
        KIO_DATA << mOutgoingMetaData;

        send(INF_META_DATA, data);
    }
}

KRemoteEncoding *SlaveBase::remoteEncoding()
{
   if (d->remotefile != 0)
      return d->remotefile;

   QByteArray charset = metaData("Charset").toLatin1();
   return d->remotefile = new KRemoteEncoding( charset );
}

void SlaveBase::data( const QByteArray &data )
{
   sendMetaData();
   send( MSG_DATA, data );
}

void SlaveBase::dataReq( )
{
   //sendMetaData();
   if (d->needSendCanResume)
      canResume(0);
   send( MSG_DATA_REQ );
}

void SlaveBase::opened()
{
   sendMetaData();
   send( MSG_OPENED );
   d->inOpenLoop = true;
}

void SlaveBase::error( int _errid, const QString &_text )
{
    if (d->m_state == d->ErrorCalled) {
        kWarning(7019) << "error() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == d->FinishedCalled) {
        kWarning(7019) << "error() called after finished()! Please fix the KIO slave.";
        return;
    }

    d->m_state = d->ErrorCalled;
    mIncomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    mOutgoingMetaData.clear();
    KIO_DATA << (qint32) _errid << _text;

    send( MSG_ERROR, data );
    //reset
    d->listEntryCurrentSize = 100;
    d->sentListEntries=0;
    d->totalSize=0;
    d->inOpenLoop=false;
}

void SlaveBase::connected()
{
    send( MSG_CONNECTED );
}

void SlaveBase::finished()
{
    if (d->m_state == d->FinishedCalled) {
        kWarning(7019) << "finished() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == d->ErrorCalled) {
        kWarning(7019) << "finished() called after error()! Please fix the KIO slave.";
        return;
    }

    d->m_state = d->FinishedCalled;
    mIncomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    sendMetaData();
    send( MSG_FINISHED );

    // reset
    d->listEntryCurrentSize = 100;
    d->sentListEntries=0;
    d->totalSize=0;
    d->inOpenLoop=false;
}

void SlaveBase::needSubUrlData()
{
    send( MSG_NEED_SUBURL_DATA );
}

/*
 * Map pid_t to a signed integer type that makes sense for QByteArray;
 * only the most common sizes 16 bit and 32 bit are special-cased.
 */
template<int T> struct PIDType { typedef pid_t PID_t; } ;
template<> struct PIDType<2> { typedef qint16 PID_t; } ;
template<> struct PIDType<4> { typedef qint32 PID_t; } ;

void SlaveBase::slaveStatus( const QString &host, bool connected )
{
    pid_t pid = getpid();
    qint8 b = connected ? 1 : 0;
    KIO_DATA << (PIDType<sizeof(pid_t)>::PID_t)pid << mProtocol << host << b;
    if (d->onHold)
       stream << d->onHoldUrl;
    send( MSG_SLAVE_STATUS, data );
}

void SlaveBase::canResume()
{
    send( MSG_CANRESUME );
}

void SlaveBase::totalSize( KIO::filesize_t _bytes )
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    send( INF_TOTAL_SIZE, data );

    //this one is usually called before the first item is listed in listDir()
    struct timeval tp;
    gettimeofday(&tp, 0);
    d->listEntry_sec = tp.tv_sec;
    d->listEntry_usec = tp.tv_usec;
    d->totalSize=_bytes;
    d->sentListEntries=0;
}

void SlaveBase::processedSize( KIO::filesize_t _bytes )
{
    bool           emitSignal=false;
    struct timeval tv;
    int            gettimeofday_res=gettimeofday( &tv, 0L );

    if( _bytes == d->totalSize )
        emitSignal=true;
    else if ( gettimeofday_res == 0 ) {
        time_t msecdiff = 2000;
        if (d->last_tv.tv_sec) {
            // Compute difference, in ms
            msecdiff = 1000 * ( tv.tv_sec - d->last_tv.tv_sec );
            time_t usecdiff = tv.tv_usec - d->last_tv.tv_usec;
            if ( usecdiff < 0 ) {
                msecdiff--;
                msecdiff += 1000;
            }
            msecdiff += usecdiff / 1000;
        }
        emitSignal=msecdiff >= 100; // emit size 10 times a second
    }

    if( emitSignal ) {
        KIO_DATA << KIO_FILESIZE_T(_bytes);
        send( INF_PROCESSED_SIZE, data );
        if ( gettimeofday_res == 0 ) {
            d->last_tv.tv_sec = tv.tv_sec;
            d->last_tv.tv_usec = tv.tv_usec;
        }
    }
//    d->processed_size = _bytes;
}

void SlaveBase::written( KIO::filesize_t _bytes )
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    send( MSG_WRITTEN, data );
}

void SlaveBase::position( KIO::filesize_t _pos )
{
    KIO_DATA << KIO_FILESIZE_T(_pos);
    send( INF_POSITION, data );
}

void SlaveBase::processedPercent( float /* percent */ )
{
  kDebug(7019) << "STUB";
}


void SlaveBase::speed( unsigned long _bytes_per_second )
{
    KIO_DATA << (quint32) _bytes_per_second;
    send( INF_SPEED, data );
}

void SlaveBase::redirection( const KUrl& _url )
{
    KIO_DATA << _url;
    send( INF_REDIRECTION, data );
}

void SlaveBase::errorPage()
{
    send( INF_ERROR_PAGE );
}

static bool isSubCommand(int cmd)
{
   return ( (cmd == CMD_REPARSECONFIGURATION) ||
            (cmd == CMD_META_DATA) ||
            (cmd == CMD_CONFIG) ||
            (cmd == CMD_SUBURL) ||
            (cmd == CMD_SLAVE_STATUS) ||
            (cmd == CMD_SLAVE_CONNECT) ||
            (cmd == CMD_SLAVE_HOLD) ||
            (cmd == CMD_MULTI_GET));
}

void SlaveBase::mimeType( const QString &_type)
{
  kDebug(7019) << _type;
  int cmd;
  do
  {
    // Send the meta-data each time we send the mime-type.
    if (!mOutgoingMetaData.isEmpty())
    {
      // kDebug(7019) << "emitting meta data";
      KIO_DATA << mOutgoingMetaData;
      send( INF_META_DATA, data );
    }
    KIO_DATA << _type;
    send( INF_MIME_TYPE, data );
    while(true)
    {
       cmd = 0;
       int ret = -1;
       if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
           ret = d->appConnection.read( &cmd, data );
       }
       if (ret == -1) {
           kDebug(7019) << "read error";
           exit();
           return;
       }
       // kDebug(7019) << "got" << cmd;
       if ( cmd == CMD_HOST) // Ignore.
          continue;
       if (!isSubCommand(cmd))
          break;

       dispatch( cmd, data );
    }
  }
  while (cmd != CMD_NONE);
  mOutgoingMetaData.clear();
}

void SlaveBase::exit()
{
    d->exit_loop = true;
    // Using ::exit() here is too much (crashes in qdbus's qglobalstatic object),
    // so let's cleanly exit dispatchLoop() instead.
    // Update: we do need to call exit(), otherwise a long download (get()) would
    // keep going until it ends, even though the application exited.
    ::exit(255);
}

void SlaveBase::warning( const QString &_msg)
{
    KIO_DATA << _msg;
    send( INF_WARNING, data );
}

void SlaveBase::infoMessage( const QString &_msg)
{
    KIO_DATA << _msg;
    send( INF_INFOMESSAGE, data );
}

bool SlaveBase::requestNetwork(const QString& host)
{
    KIO_DATA << host << d->slaveid;
    send( MSG_NET_REQUEST, data );

    if ( waitForAnswer( INF_NETWORK_STATUS, 0, data ) != -1 )
    {
        bool status;
        QDataStream stream( data );
        stream >> status;
        return status;
    } else
        return false;
}

void SlaveBase::dropNetwork(const QString& host)
{
    KIO_DATA << host << d->slaveid;
    send( MSG_NET_DROP, data );
}

void SlaveBase::statEntry( const UDSEntry& entry )
{
    KIO_DATA << entry;
    send( MSG_STAT_ENTRY, data );
}

void SlaveBase::listEntry( const UDSEntry& entry, bool _ready )
{
   static struct timeval tp;
   static const int maximum_updatetime = 300;
   static const int minimum_updatetime = 100;

   if (!_ready) {
      d->pendingListEntries.append(entry);

      if (d->pendingListEntries.count() > d->listEntryCurrentSize) {
         gettimeofday(&tp, 0);

         long diff = ((tp.tv_sec - d->listEntry_sec) * 1000000 +
                      tp.tv_usec - d->listEntry_usec) / 1000;
         if (diff==0) diff=1;

         if (diff > maximum_updatetime) {
            d->listEntryCurrentSize = d->listEntryCurrentSize * 3 / 4;
            _ready = true;
         }
//if we can send all list entries of this dir which have not yet been sent
//within maximum_updatetime, then make d->listEntryCurrentSize big enough for all of them
         else if (((d->pendingListEntries.count()*maximum_updatetime)/diff) > static_cast<long>(d->totalSize-d->sentListEntries))
            d->listEntryCurrentSize=d->totalSize-d->sentListEntries+1;
//if we are below minimum_updatetime, estimate how much we will get within
//maximum_updatetime
         else if (diff < minimum_updatetime)
            d->listEntryCurrentSize = (d->pendingListEntries.count() * maximum_updatetime) / diff;
         else
            _ready=true;
      }
   }
   if (_ready) { // may happen when we started with !ready
      listEntries( d->pendingListEntries );
      d->pendingListEntries.clear();

      gettimeofday(&tp, 0);
      d->listEntry_sec = tp.tv_sec;
      d->listEntry_usec = tp.tv_usec;
   }
}

void SlaveBase::listEntries( const UDSEntryList& list )
{
    KIO_DATA << (quint32)list.count();
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
      stream << *it;
    send( MSG_LIST_ENTRIES, data);
    d->sentListEntries+=(uint)list.count();
}

static void sigsegv_handler(int sig)
{
#ifdef Q_OS_UNIX
    KDE_signal(sig,SIG_DFL); // Next one kills

    //Kill us if we deadlock
    KDE_signal(SIGALRM,SIG_DFL);
    alarm(5);  //generate an alarm signal in 5 seconds, in this time the slave has to exit

    // Debug and printf should be avoided because they might
    // call malloc.. and get in a nice recursive malloc loop
    char buffer[120];
    qsnprintf(buffer, sizeof(buffer), "kioslave: ####### CRASH ###### protocol = %s pid = %d signal = %d\n", s_protocol, getpid(), sig);
    write(2, buffer, strlen(buffer));
#ifndef NDEBUG
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    if (n)
      backtrace_symbols_fd(trace, n, 2);
#endif
#endif
    ::exit(1);
#endif
}

static void sigpipe_handler (int)
{
    // We ignore a SIGPIPE in slaves.
    // A SIGPIPE can happen in two cases:
    // 1) Communication error with application.
    // 2) Communication error with network.
    slaveWriteError = true;

    // Don't add anything else here, especially no debug output
}

void SlaveBase::setHost(QString const &, quint16, QString const &, QString const &)
{
}

void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CONNECT)); }
void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_STAT)); }
void SlaveBase::put(KUrl const &, int, JobFlags )
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_PUT)); }
void SlaveBase::special(const QByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SPECIAL)); }
void SlaveBase::listDir(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_LISTDIR)); }
void SlaveBase::get(KUrl const & )
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_GET)); }
void SlaveBase::open(KUrl const &, QIODevice::OpenMode)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_OPEN)); }
void SlaveBase::read(KIO::filesize_t)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_READ)); }
void SlaveBase::write(const QByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_WRITE)); }
void SlaveBase::seek(KIO::filesize_t)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SEEK)); }
void SlaveBase::close()
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CLOSE)); }
void SlaveBase::mimetype(KUrl const &url)
{ get(url); }
void SlaveBase::rename(KUrl const &, KUrl const &, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_RENAME)); }
void SlaveBase::symlink(QString const &, KUrl const &, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SYMLINK)); }
void SlaveBase::copy(KUrl const &, KUrl const &, int, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_COPY)); }
void SlaveBase::del(KUrl const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_DEL)); }
void SlaveBase::setLinkDest(const KUrl &, const QString&)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SETLINKDEST)); }
void SlaveBase::mkdir(KUrl const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_MKDIR)); }
void SlaveBase::chmod(KUrl const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CHMOD)); }
void SlaveBase::setModificationTime(KUrl const &, const QDateTime&)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SETMODIFICATIONTIME)); }
void SlaveBase::chown(KUrl const &, const QString &, const QString &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CHOWN)); }
void SlaveBase::setSubUrl(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SUBURL)); }
void SlaveBase::multiGet(const QByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_MULTI_GET)); }


void SlaveBase::slave_status()
{ slaveStatus( QString(), false ); }

void SlaveBase::reparseConfiguration()
{
}

bool SlaveBase::openPasswordDialog( AuthInfo& info, const QString &errorMsg )
{
    const long windowId = metaData("window-id").toLong();
    const unsigned long userTimestamp = metaData("user-timestamp").toULong();
    QString errorMessage;
    if (metaData("no-auth-prompt").toLower() == "true") {
        errorMessage = QLatin1String("<NoAuthPrompt>");
    } else {
        errorMessage = errorMsg;
    }

    KPasswdServer srv;
    qlonglong seqNr = srv.queryAuthInfo(info, errorMessage, windowId,
                                        SlaveBasePrivate::s_seqNr, userTimestamp);
    if (seqNr > 0) {
        SlaveBasePrivate::s_seqNr = seqNr;
        if (info.isModified()) {
            return true;
        }
    }

    return false;
}

int SlaveBase::messageBox( MessageBoxType type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo )
{
    return messageBox( text, type, caption, buttonYes, buttonNo, QString() );
}

int SlaveBase::messageBox( const QString &text, MessageBoxType type, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo,
                           const QString &dontAskAgainName )
{
    kDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo;
    KIO_DATA << (qint32)type << text << caption << buttonYes << buttonNo << dontAskAgainName;
    send( INF_MESSAGEBOX, data );
    if ( waitForAnswer( CMD_MESSAGEBOXANSWER, 0, data ) != -1 )
    {
        QDataStream stream( data );
        int answer;
        stream >> answer;
        kDebug(7019) << "got messagebox answer" << answer;
        return answer;
    } else
        return 0; // communication failure
}

bool SlaveBase::canResume( KIO::filesize_t offset )
{
    kDebug(7019) << "offset=" << KIO::number(offset);
    d->needSendCanResume = false;
    KIO_DATA << KIO_FILESIZE_T(offset);
    send( MSG_RESUME, data );
    if ( offset )
    {
        int cmd;
        if ( waitForAnswer( CMD_RESUMEANSWER, CMD_NONE, data, &cmd ) != -1 )
        {
            kDebug(7019) << "returning" << (cmd == CMD_RESUMEANSWER);
            return cmd == CMD_RESUMEANSWER;
        } else
            return false;
    }
    else // No resuming possible -> no answer to wait for
        return true;
}



int SlaveBase::waitForAnswer( int expected1, int expected2, QByteArray & data, int *pCmd )
{
    int cmd, result = -1;
    for (;;)
    {
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
            result = d->appConnection.read( &cmd, data );
        }
        if (result == -1) {
            kDebug(7019) << "read error.";
            return -1;
        }

        if ( cmd == expected1 || cmd == expected2 )
        {
            if ( pCmd ) *pCmd = cmd;
            return result;
        }
        if ( isSubCommand(cmd) )
        {
            dispatch( cmd, data );
        }
        else
        {
            kFatal(7019) << "Got cmd " << cmd << " while waiting for an answer!";
        }
    }
}


int SlaveBase::readData( QByteArray &buffer)
{
   int result = waitForAnswer( MSG_DATA, 0, buffer );
   //kDebug(7019) << "readData: length = " << result << " ";
   return result;
}

void SlaveBase::setTimeoutSpecialCommand(int timeout, const QByteArray &data)
{
   if (timeout > 0)
      d->timeout = time(0)+(time_t)timeout;
   else if (timeout == 0)
      d->timeout = 1; // Immediate timeout
   else
      d->timeout = 0; // Canceled

   d->timeoutData = data;
}

void SlaveBase::dispatch( int command, const QByteArray &data )
{
    QDataStream stream( data );

    KUrl url;
    int i;

    switch( command ) {
    case CMD_HOST: {
        // Reset s_seqNr, see kpasswdserver/DESIGN
        SlaveBasePrivate::s_seqNr = 0;
        QString passwd;
        QString host, user;
        quint16 port;
        stream >> host >> port >> user >> passwd;
        d->m_state = d->InsideMethod;
        setHost( host, port, user, passwd );
        d->verifyErrorFinishedNotCalled("setHost()");
        d->m_state = d->Idle;
    } break;
    case CMD_CONNECT: {
        openConnection( );
    } break;
    case CMD_DISCONNECT: {
        closeConnection( );
    } break;
    case CMD_SLAVE_STATUS: {
        d->m_state = d->InsideMethod;
        slave_status();
        // TODO verify that the slave has called slaveStatus()?
        d->verifyErrorFinishedNotCalled("slave_status()");
        d->m_state = d->Idle;
    } break;
    case CMD_SLAVE_CONNECT: {
        d->onHold = false;
        QString app_socket;
        QDataStream stream( data );
        stream >> app_socket;
        d->appConnection.send( MSG_SLAVE_ACK );
        disconnectSlave();
        d->isConnectedToApp = true;
        connectSlave(app_socket);
    } break;
    case CMD_SLAVE_HOLD: {
        KUrl url;
        QDataStream stream( data );
        stream >> url;
        d->onHoldUrl = url;
        d->onHold = true;
        disconnectSlave();
        d->isConnectedToApp = false;
        // Do not close connection!
        connectSlave(d->poolSocket);
    } break;
    case CMD_REPARSECONFIGURATION: {
        d->m_state = d->InsideMethod;
        reparseConfiguration();
        d->verifyErrorFinishedNotCalled("reparseConfiguration()");
        d->m_state = d->Idle;
    } break;
    case CMD_CONFIG: {
        stream >> d->configData;
        d->rebuildConfig();
#if 0 //TODO: decide what to do in KDE 4.1
        KSocks::setConfig(d->configGroup);
#endif
        delete d->remotefile;
        d->remotefile = 0;
    } break;
    case CMD_GET: {
        stream >> url;
        d->m_state = d->InsideMethod;
        get( url );
        d->verifyState("get()");
        d->m_state = d->Idle;
    } break;
    case CMD_OPEN: {
        stream >> url >> i;
        QIODevice::OpenMode mode = QFlag(i);
        d->m_state = d->InsideMethod;
        open(url, mode); //krazy:exclude=syscalls
        d->m_state = d->Idle;
    } break;
    case CMD_PUT: {
        int permissions;
        qint8 iOverwrite, iResume;
        stream >> url >> iOverwrite >> iResume >> permissions;
        JobFlags flags;
        if ( iOverwrite != 0 ) flags |= Overwrite;
        if ( iResume != 0 ) flags |= Resume;

        // Remember that we need to send canResume(), TransferJob is expecting
        // it. Well, in theory this shouldn't be done if resume is true.
        //   (the resume bool is currently unused)
        d->needSendCanResume = true   /* !resume */;

        d->m_state = d->InsideMethod;
        put( url, permissions, flags);
        d->verifyState("put()");
        d->m_state = d->Idle;
    } break;
    case CMD_STAT: {
        stream >> url;
        d->m_state = d->InsideMethod;
        stat( url ); //krazy:exclude=syscalls
        d->verifyState("stat()");
        d->m_state = d->Idle;
    } break;
    case CMD_MIMETYPE: {
        stream >> url;
        d->m_state = d->InsideMethod;
        mimetype( url );
        d->verifyState("mimetype()");
        d->m_state = d->Idle;
    } break;
    case CMD_LISTDIR: {
        stream >> url;
        d->m_state = d->InsideMethod;
        listDir( url );
        d->verifyState("listDir()");
        d->m_state = d->Idle;
    } break;
    case CMD_MKDIR: {
        stream >> url >> i;
        d->m_state = d->InsideMethod;
        mkdir( url, i ); //krazy:exclude=syscalls
        d->verifyState("mkdir()");
        d->m_state = d->Idle;
    } break;
    case CMD_RENAME: {
        qint8 iOverwrite;
        KUrl url2;
        stream >> url >> url2 >> iOverwrite;
        JobFlags flags;
        if ( iOverwrite != 0 ) flags |= Overwrite;
        d->m_state = d->InsideMethod;
        rename( url, url2, flags ); //krazy:exclude=syscalls
        d->verifyState("rename()");
        d->m_state = d->Idle;
    } break;
    case CMD_SYMLINK: {
        qint8 iOverwrite;
        QString target;
        stream >> target >> url >> iOverwrite;
        JobFlags flags;
        if ( iOverwrite != 0 ) flags |= Overwrite;
        d->m_state = d->InsideMethod;
        symlink( target, url, flags );
        d->verifyState("symlink()");
        d->m_state = d->Idle;
    } break;
    case CMD_COPY: {
        int permissions;
        qint8 iOverwrite;
        KUrl url2;
        stream >> url >> url2 >> permissions >> iOverwrite;
        JobFlags flags;
        if ( iOverwrite != 0 ) flags |= Overwrite;
        d->m_state = d->InsideMethod;
        copy( url, url2, permissions, flags );
        d->verifyState("copy()");
        d->m_state = d->Idle;
    } break;
    case CMD_DEL: {
        qint8 isFile;
        stream >> url >> isFile;
        d->m_state = d->InsideMethod;
        del( url, isFile != 0);
        d->verifyState("del()");
        d->m_state = d->Idle;
    } break;
    case CMD_CHMOD: {
        stream >> url >> i;
        d->m_state = d->InsideMethod;
        chmod( url, i);
        d->verifyState("chmod()");
        d->m_state = d->Idle;
    } break;
    case CMD_CHOWN: {
        QString owner, group;
        stream >> url >> owner >> group;
        d->m_state = d->InsideMethod;
        chown(url, owner, group);
        d->verifyState("chown()");
        d->m_state = d->Idle;
    } break;
    case CMD_SETMODIFICATIONTIME: {
        QDateTime dt;
        stream >> url >> dt;
        d->m_state = d->InsideMethod;
        setModificationTime(url, dt);
        d->verifyState("setModificationTime()");
        d->m_state = d->Idle;
    } break;
    case CMD_SPECIAL: {
        d->m_state = d->InsideMethod;
        special( data );
        d->verifyState("special()");
        d->m_state = d->Idle;
    } break;
    case CMD_META_DATA: {
        //kDebug(7019) << "(" << getpid() << ") Incoming meta-data...";
        stream >> mIncomingMetaData;
        d->rebuildConfig();
    } break;
    case CMD_SUBURL: {
        stream >> url;
        d->m_state = d->InsideMethod;
        setSubUrl(url);
        d->verifyErrorFinishedNotCalled("setSubUrl()");
        d->m_state = d->Idle;
    } break;
    case CMD_NONE: {
        kWarning(7019) << "Got unexpected CMD_NONE!";
    } break;
    case CMD_MULTI_GET: {
        d->m_state = d->InsideMethod;
        multiGet( data );
        d->verifyState("multiGet()");
        d->m_state = d->Idle;
    } break;
    default: {
        // Some command we don't understand.
        // Just ignore it, it may come from some future version of KDE.
    } break;
    }
}

bool SlaveBase::checkCachedAuthentication( AuthInfo& info )
{
    return KPasswdServer().checkAuthInfo(info, metaData("window-id").toLong(),
                                         metaData("user-timestamp").toULong());
}

void SlaveBase::dispatchOpenCommand( int command, const QByteArray &data )
{
    QDataStream stream( data );

    switch( command ) {
    case CMD_READ: {
        KIO::filesize_t bytes;
        stream >> bytes;
        read(bytes);
        break;
    }
    case CMD_WRITE: {
        write(data);
        break;
    }
    case CMD_SEEK: {
        KIO::filesize_t offset;
        stream >> offset;
        seek(offset);
    }
    case CMD_NONE:
        break;
    case CMD_CLOSE:
        close();                // must call finish(), which will set d->inOpenLoop=false
        break;
    default:
        // Some command we don't understand.
        // Just ignore it, it may come from some future version of KDE.
        break;
    }
}

bool SlaveBase::cacheAuthentication( const AuthInfo& info )
{
    KPasswdServer().addAuthInfo(info, metaData("window-id").toLongLong());
    return true;
}

int SlaveBase::connectTimeout()
{
    bool ok;
    QString tmp = metaData("ConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_CONNECT_TIMEOUT;
}

int SlaveBase::proxyConnectTimeout()
{
    bool ok;
    QString tmp = metaData("ProxyConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_PROXY_CONNECT_TIMEOUT;
}


int SlaveBase::responseTimeout()
{
    bool ok;
    QString tmp = metaData("ResponseTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_RESPONSE_TIMEOUT;
}


int SlaveBase::readTimeout()
{
    bool ok;
    QString tmp = metaData("ReadTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_READ_TIMEOUT;
}

bool SlaveBase::wasKilled() const
{
   return d->wasKilled;
}

void SlaveBase::setKillFlag()
{
   d->wasKilled=true;
}

void SlaveBase::send(int cmd, const QByteArray& arr )
{
   slaveWriteError = false;
   if (!d->appConnection.send(cmd, arr))
       // Note that slaveWriteError can also be set by sigpipe_handler
       slaveWriteError = true;
   if (slaveWriteError) exit();
}

void SlaveBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void SlaveBase::lookupHost(const QString& host)
{
    KIO_DATA << host;
    send(MSG_HOST_INFO_REQ, data);
}

int SlaveBase::waitForHostInfo(QHostInfo& info)
{
    QByteArray data;
    int result = waitForAnswer(CMD_HOST_INFO, 0, data);

    if (result  == -1) {
        info.setError(QHostInfo::UnknownError);
        info.setErrorString(i18n("Unknown Error"));
        return result;
    }

    QDataStream stream(data);
    QString hostName;
    QList<QHostAddress> addresses;
    int error;
    QString errorString;

    stream >> hostName >> addresses >> error >> errorString;

    info.setHostName(hostName);
    info.setAddresses(addresses);
    info.setError(QHostInfo::HostInfoError(error));
    info.setErrorString(errorString);

    return result;
}
