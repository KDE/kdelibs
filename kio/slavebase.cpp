/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (c) 2000 David Faure <faure@kde.org>
 *  Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 **/

#include <config.h>

#include <sys/time.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <qfile.h>

#include <ksock.h>
#include <kcrash.h>
#include <kdesu/client.h>
#include <kprotocolmanager.h>

#include "slavebase.h"

#include "kio/slavebase.h"
#include "kio/slaveinterface.h"
#include "kio/connection.h"

using namespace KIO;

template class QList<QValueList<UDSAtom> >;
typedef QValueList<QCString> AuthKeysList;
typedef QMap<QString,QCString> AuthKeysMap;
#define KIO_DATA QByteArray data; QDataStream stream( data, IO_WriteOnly ); stream

namespace KIO {

class SlaveBaseConfig : public KConfigBase
{
public:
   SlaveBaseConfig(SlaveBase *_slave)
	: slave(_slave) { }

   bool hasGroup(const QString &) const { qWarning("hasGroup(const QString &)");
return false; }
   bool hasGroup(const QCString &) const { qWarning("hasGroup(const QCString &)");
return false; }
   bool hasGroup(const char *) const { qWarning("hasGroup(const char *)");
return false; }

   QStringList groupList() const { return QStringList(); }

   bool hasKey(const QString &pKey) const
   {
      return slave->hasMetaData(pKey);
   }
   bool hasKey(const char *pKey) const { return hasKey(QString::fromLatin1(pKey)); }

   QMap<QString,QString> entryMap(const QString &) const
      { return QMap<QString,QString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const QString &) const { return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &, const KEntry&) { }

   KEntry lookupData(const KEntryKey &key) const
   {
     KEntry entry;
     QString value = slave->metaData(key.c_key);
     if (!value.isNull())
        entry.mValue = value.utf8();
     return entry;
   }
protected:
   SlaveBase *slave;
};


class SlaveBasePrivate {
public:
    QString slaveid;
    bool resume:1;
    bool needSendCanResume:1;
    bool multipleAuthCaching:1;
    MetaData configData;
    SlaveBaseConfig *config;
};

};

SlaveBase *globalSlave=0;

void genericsig_handler(int sigNumber)
{
   signal(sigNumber,SIG_IGN);
   //I don't think we can have the same problem here as in the sigsegv handler
   kdDebug()<<"kioslave : exiting due to signal "<<sigNumber<<endl;
   if (globalSlave!=0)
      globalSlave->~SlaveBase();
   exit(2);
};

//////////////

SlaveBase::SlaveBase( const QCString &protocol,
                      const QCString &pool_socket,
                      const QCString &app_socket )
    : mProtocol(protocol), m_pConnection(0),
      mPoolSocket( QFile::decodeName(pool_socket)),
      mAppSocket( QFile::decodeName(app_socket))
{
    if (!getenv("KDE_DEBUG"))
        KCrash::setCrashHandler( sigsegv_handler );
    signal( SIGPIPE, sigpipe_handler );

   signal(SIGINT,&genericsig_handler);
	signal(SIGQUIT,&genericsig_handler);
	signal(SIGILL,&genericsig_handler);
	signal(SIGTRAP,&genericsig_handler);
	signal(SIGABRT,&genericsig_handler);
	signal(SIGBUS,&genericsig_handler);
	signal(SIGALRM,&genericsig_handler);
	signal(SIGTERM,&genericsig_handler);
	signal(SIGFPE,&genericsig_handler);
#ifdef SIGPOLL
   signal(SIGPOLL, &genericsig_handler);
#endif
#ifdef SIGSYS
   signal(SIGSYS, &genericsig_handler);
#endif
#ifdef SIGVTALRM
   signal(SIGVTALRM, &genericsig_handler);
#endif
#ifdef SIGXCPU
   signal(SIGXCPU, &genericsig_handler);
#endif
#ifdef SIGXFSZ
   signal(SIGXFSZ, &genericsig_handler);
#endif
   globalSlave=this;

    appconn = new Connection();
    listEntryCurrentSize = 0;
    struct timeval tp;
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;
    mConnectedToApp = true;
    connectSlave(mAppSocket);

    d = new SlaveBasePrivate;
    // by kahl for netmgr (need a way to identify slaves)
    d->slaveid = protocol;
    d->slaveid += QString::number(getpid());
    d->resume = false;
    d->needSendCanResume = false;
    d->multipleAuthCaching = false;
    d->config = new SlaveBaseConfig(this);
}

SlaveBase::~SlaveBase()
{
    delete d;
}

void SlaveBase::dispatchLoop()
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    while (true) {
    FD_ZERO(&rfds);

    if (appconn->inited())
      FD_SET(appconn->fd_from(), &rfds);

    /* Wait up to 30 seconds. */
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    retval = select(appconn->fd_from()+ 1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */
    if (retval > 0)
    {
      if (FD_ISSET(appconn->fd_from(), &rfds))
      { // dispatch application messages
        int cmd;
        QByteArray data;
        if ( appconn->read(&cmd, data) != -1 )
        {
          if (cmd == CMD_SLAVE_CONNECT)
          {
            QString app_socket;
            QDataStream stream( data, IO_ReadOnly);
            stream >> app_socket;
            appconn->send( MSG_SLAVE_ACK );
            disconnectSlave();
            mConnectedToApp = true;
            connectSlave(app_socket);
          }
          else
          {
            dispatch(cmd, data);
          }
        }
        else // some error occured, perhaps no more application
        {
          // When the app exits, should the slave be put back in the pool ?
          if (mConnectedToApp)
          {
            disconnectSlave();
            mConnectedToApp = false;
            closeConnection();
            connectSlave(mPoolSocket);
          }
          else
          {
            return;
          }
        }
      }
    }
    else if (retval == -1) // error
    {
      kdDebug(7019) << "dispatchLoop(): select returned error "
                    << (errno==EBADF?"EBADF":errno==EINTR?"EINTR":errno==EINVAL?"EINVAL":errno==ENOMEM?"ENOMEM":"unknown")
                    << " (" << errno << ")" << endl;
       return;
    }
  }
}

void SlaveBase::connectSlave(const QString& path)
{
    appconn->init(new KSocket(QFile::encodeName(path)));
    setConnection(appconn);
}

void SlaveBase::disconnectSlave()
{
    appconn->close();
}

void SlaveBase::setMetaData(const QString &key, const QString &value)
{
   mOutgoingMetaData.replace(key, value);
}

QString SlaveBase::metaData(const QString &key)
{
   if (mIncomingMetaData.contains(key))
      return mIncomingMetaData[key];
   if (d->configData.contains(key))
      return d->configData[key];
   return QString::null;
}

bool SlaveBase::hasMetaData(const QString &key)
{
   if (mIncomingMetaData.contains(key))
      return true;
   if (d->configData.contains(key))
      return true;
   return false;
}

KConfigBase *SlaveBase::config()
{
   return d->config;
}

void SlaveBase::sendMetaData()
{
   KIO_DATA << mOutgoingMetaData;

   m_pConnection->send( INF_META_DATA, data );
   mOutgoingMetaData.clear(); // Clear
}


void SlaveBase::data( const QByteArray &data )
{
    if (!mOutgoingMetaData.isEmpty())
       sendMetaData();
    m_pConnection->send( MSG_DATA, data );
}

void SlaveBase::dataReq( )
{
/*
    if (!mOutgoingMetaData.isEmpty())
       sendMetaData();
*/
    if (d->needSendCanResume)
        canResume(0);
    m_pConnection->send( MSG_DATA_REQ );
}

void SlaveBase::error( int _errid, const QString &_text )
{
    mIncomingMetaData.clear(); // Clear meta data
    mOutgoingMetaData.clear();
    KIO_DATA << _errid << _text;

    m_pConnection->send( MSG_ERROR, data );
}

void SlaveBase::connected()
{
    m_pConnection->send( MSG_CONNECTED );
}

void SlaveBase::finished()
{
    mIncomingMetaData.clear(); // Clear meta data
    if (!mOutgoingMetaData.isEmpty())
       sendMetaData();
    m_pConnection->send( MSG_FINISHED );
}

void SlaveBase::needSubURLData()
{
    m_pConnection->send( MSG_NEED_SUBURL_DATA );
}

void SlaveBase::slaveStatus( const QString &host, bool connected )
{
    pid_t pid = getpid();
    Q_INT8 b = connected ? 1 : 0;
    KIO_DATA << pid << mProtocol << host << b;
    m_pConnection->send( MSG_SLAVE_STATUS, data );
}

void SlaveBase::canResume()
{
    m_pConnection->send( MSG_CANRESUME );
}

void SlaveBase::totalSize( unsigned long _bytes )
{
    KIO_DATA << _bytes;
    m_pConnection->send( INF_TOTAL_SIZE, data );
}

void SlaveBase::processedSize( unsigned long _bytes )
{
    KIO_DATA << _bytes;
    m_pConnection->send( INF_PROCESSED_SIZE, data );
}

void SlaveBase::speed( unsigned long _bytes_per_second )
{
    KIO_DATA << _bytes_per_second;
    m_pConnection->send( INF_SPEED, data );
}

void SlaveBase::redirection( const KURL& _url )
{
    KIO_DATA << _url;
    m_pConnection->send( INF_REDIRECTION, data );
}

void SlaveBase::errorPage()
{
    m_pConnection->send( INF_ERROR_PAGE );
}

static bool isSubCommand(int cmd)
{
   return ( (cmd == CMD_REPARSECONFIGURATION) ||
            (cmd == CMD_META_DATA) ||
            (cmd == CMD_CONFIG) ||
            (cmd == CMD_SUBURL) ||
            (cmd == CMD_SLAVE_STATUS) );
}

void SlaveBase::mimeType( const QString &_type)
{
  kdDebug(7019) << "(" << getpid() << ") SlaveBase::mimeType '" << _type << "'" << endl;
  int cmd;
  do
  {
    // Send the meta-data each time we send the mime-type.
    if (!mOutgoingMetaData.isEmpty())
    {
      kdDebug(7019) << "(" << getpid() << ") mimeType: emitting meta data"
                    << endl;
      KIO_DATA << mOutgoingMetaData;
      m_pConnection->send( INF_META_DATA, data );
    }
    KIO_DATA << _type;
    m_pConnection->send( INF_MIME_TYPE, data );
    while(true)
    {
       cmd = 0;
       if ( m_pConnection->read( &cmd, data ) == -1 ) {
           kdDebug(7019) << "SlaveBase: mimetype: read error" << endl;
           this->~SlaveBase();
           ::exit(255);
       }
       if ( isSubCommand(cmd) )
       {
          dispatch( cmd, data );
          continue; // Disguised goto
       };
       break;
    }
  }
  while (cmd != CMD_NONE);
  mOutgoingMetaData.clear();
  // WABA: cmd can be "CMD_NONE" or "CMD_GET" (in which
  // case the slave had been put on hold.) [or special,
  // for http posts]. Something else is basically an error
  ASSERT( (cmd == CMD_NONE) || (cmd == CMD_GET) || (cmd == CMD_SPECIAL) );
}

// remove in KDE 3.0
void SlaveBase::gettingFile( const QString & )
{
}

void SlaveBase::warning( const QString &_msg)
{
    KIO_DATA << _msg;
    m_pConnection->send( INF_WARNING, data );
}

void SlaveBase::infoMessage( const QString &_msg)
{
    KIO_DATA << _msg;
    m_pConnection->send( INF_INFOMESSAGE, data );
}

bool SlaveBase::requestNetwork(const QString& host)
{
    KIO_DATA << host << d->slaveid;
    m_pConnection->send( MSG_NET_REQUEST, data );

    if ( waitForAnswer( INF_NETWORK_STATUS, 0, data ) != -1 )
    {
        bool status;
        QDataStream stream( data, IO_ReadOnly );
        stream >> status;
        return status;
    } else
        return false;
}

void SlaveBase::dropNetwork(const QString& host)
{
    KIO_DATA << host << d->slaveid;
    m_pConnection->send( MSG_NET_DROP, data );
}

void SlaveBase::statEntry( const UDSEntry& entry )
{
    KIO_DATA << entry;
    m_pConnection->send( MSG_STAT_ENTRY, data );
}

void SlaveBase::listEntry( const UDSEntry& entry, bool _ready )
{
    static struct timeval tp;
    static const int maximum_updatetime = 800;
    static const int minimum_updatetime = (maximum_updatetime * 3) / 4;

    if (!_ready) {
    pendingListEntries.append(entry);

    if (pendingListEntries.count() > listEntryCurrentSize) {

            gettimeofday(&tp, 0);

            long diff = ((tp.tv_sec - listEntry_sec) * 1000000 +
                         tp.tv_usec - listEntry_usec) / 1000;

            if (diff > maximum_updatetime) {
                listEntryCurrentSize = listEntryCurrentSize * 3 / 4;
                _ready = true;
            } else if (diff < minimum_updatetime) {
                listEntryCurrentSize = listEntryCurrentSize * 5 / 4;
            } else {
                _ready = true;
            }
        }
    }

    if (_ready) { // may happen when we started with !ready
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;

    listEntries( pendingListEntries );
    pendingListEntries.clear();
    }
}

void SlaveBase::listEntries( const UDSEntryList& list )
{
    kdDebug(7019) << "listEntries " << list.count() << endl;

    KIO_DATA << list.count();
    UDSEntryListConstIterator it = list.begin();
    UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it)
      stream << *it;
    m_pConnection->send( MSG_LIST_ENTRIES, data);
}

void SlaveBase::sendAuthenticationKey( const QCString& key,
                                       const QCString& group,
                                       bool keepPass )
{
    KIO_DATA << key << group << keepPass;
    m_pConnection->send( MSG_AUTH_KEY, data );
}

void SlaveBase::delCachedAuthentication( const QString& key )
{
    KIO_DATA << key.utf8() ;
    m_pConnection->send( MSG_DEL_AUTH_KEY, data );
}

void SlaveBase::sigsegv_handler (int)
{
    signal(SIGSEGV,SIG_IGN);
    // Debug and printf should be avoided because they might
    // call malloc.. and get in a nice recursive malloc loop
    write(2, "kioslave : ###############SEG FAULT#############\n", 49);
    exit(1);
}

void SlaveBase::sigpipe_handler (int)
{
    signal(SIGPIPE,SIG_IGN);
    // We ignore a SIGPIPE in slaves.
    // A SIGPIPE can happen in two cases:
    // 1) Communication error with application.
    // 2) Communication error with network.

    kdDebug(7019) << "SIGPIPE" << endl;
    signal(SIGPIPE,&sigpipe_handler);
}

void SlaveBase::setHost(QString const &, int, QString const &, QString const &)
{
}

void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, "open" ); }
void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, "stat" ); }
void SlaveBase::put(KURL const &, int, bool, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "put" ); }
void SlaveBase::special(QArray<char> const &)
{ error(  ERR_UNSUPPORTED_ACTION, "special" ); }
void SlaveBase::listDir(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, "listDir" ); }
void SlaveBase::get(KURL const & )
{ error(  ERR_UNSUPPORTED_ACTION, "get" ); }
void SlaveBase::mimetype(KURL const &url)
{ get(url); }
void SlaveBase::rename(KURL const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "rename" ); }
void SlaveBase::symlink(QString const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "symlink" ); }
void SlaveBase::copy(KURL const &, KURL const &, int, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "copy" ); }
void SlaveBase::del(KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "del" ); }
void SlaveBase::mkdir(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, "mkdir" ); }
void SlaveBase::chmod(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, "chmod" ); }
void SlaveBase::setSubURL(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, "suburl" ); }

/*void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, opening connections is not supported with the protocol %1" ).arg(mProtocol)); }

void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, accessing files is not supported with the protocol %1").arg(mProtocol) ); }
void SlaveBase::put(KURL const &, int, bool, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, writing to %1 is not supported").arg(mProtocol) ); }
void SlaveBase::special(QArray<char> const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("There are no special actions available for protocol %1").arg(mProtocol) ); }
void SlaveBase::listDir(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, listing directories is not supported for protocol %1").arg(mProtocol) ); }
void SlaveBase::get(KURL const & )
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, retrieving data from %1 is not supported").arg(mProtocol) ); }
void SlaveBase::mimetype(KURL const &url)
{ get(url); }
void SlaveBase::rename(KURL const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, renaming or moving files within %1 is not supported").arg(mProtocol) ); }
void SlaveBase::symlink(QString const &, KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, creating symlinks is not supported with protocol %1").arg(mProtocol) ); }
void SlaveBase::copy(KURL const &, KURL const &, int, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, copying files within %1 is not supported").arg(mProtocol) ); }
void SlaveBase::del(KURL const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, deleting files from %1 is not supported").arg(mProtocol) ); }
void SlaveBase::mkdir(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, creating directories is not supported with protocol %1").arg(mProtocol) ); }
void SlaveBase::chmod(KURL const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, changing the attributes of files is not supported with protocol %1").arg(mProtocol) ); }
void SlaveBase::setSubURL(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, using suburls with %1 is not supported").arg(mProtocol) ); }*/

void SlaveBase::slave_status()
{ slaveStatus( QString::null, false ); }

void SlaveBase::reparseConfiguration()
{
}

bool SlaveBase::dispatch()
{
    assert( m_pConnection );

    int cmd;
    QByteArray data;
    if ( m_pConnection->read( &cmd, data ) == -1 )
        return false;

    dispatch( cmd, data );
    return true;
}

bool SlaveBase::openPassDlg( const QString& msg, QString& user, QString& passwd, bool lock )
{
    AuthInfo info;
    info.prompt = msg;
    info.username = user;
    info.readOnly = lock;
    bool result = openPassDlg( info );
    if ( result )
    {
        user = info.username;
        passwd = info.password;
    }
    return result;
}

bool SlaveBase::openPassDlg( AuthInfo& info )
{
    kdDebug(7019) << "SlaveBase::OpenPassDlg User= " << info.username << endl;
    int cmd;
    KIO_DATA << info;
    m_pConnection->send( INF_NEED_PASSWD, data );
    if ( waitForAnswer( CMD_USERPASS, CMD_NONE, data, &cmd ) != -1 && cmd == CMD_USERPASS )
    {
       AuthInfo res_info;
        QDataStream stream( data, IO_ReadOnly );
        stream >> res_info;
       info.username = res_info.username;
       info.password = res_info.password;
       info.keepPassword = res_info.keepPassword;
        kdDebug(7019) << "SlaveBase::openPassDlg got:" << endl
                      << " User= " << info.username << endl
                      << " Password= [hidden]" << endl
                      << " KeepPassword= " << info.keepPassword << endl;
        return true;
    }
    return false;
}

int SlaveBase::messageBox( int type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo )
{
    kdDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo << endl;
    KIO_DATA << type << text << caption << buttonYes << buttonNo;
    m_pConnection->send( INF_MESSAGEBOX, data );
    if ( waitForAnswer( CMD_MESSAGEBOXANSWER, 0, data ) != -1 )
    {
        QDataStream stream( data, IO_ReadOnly );
        int answer;
        stream >> answer;
        kdDebug(7019) << "got messagebox answer" << answer << endl;
        return answer;
    } else
        return 0; // communication failure
}

bool SlaveBase::canResume( unsigned long offset )
{
    kdDebug(7019) << "SlaveBase::canResume offset=" << offset << endl;
    d->needSendCanResume = false;
    KIO_DATA << offset;
    m_pConnection->send( MSG_RESUME, data );
    if ( offset )
    {
        int cmd;
        if ( waitForAnswer( CMD_RESUMEANSWER, CMD_NONE, data, &cmd ) != -1 )
        {
            kdDebug(7019) << "SlaveBase::canResume returning " << (cmd == CMD_RESUMEANSWER) << endl;
            return cmd == CMD_RESUMEANSWER;
        } else
            return false;
    }
    else // No resuming possible -> no answer to wait for
        return true;
}



int SlaveBase::waitForAnswer( int expected1, int expected2, QByteArray & data, int *pCmd )
{
    int cmd, result;
    for (;;)
    {
        result = m_pConnection->read( &cmd, data );
        if ( result == -1 )
            return -1;
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
            kdWarning() << "Got cmd " << cmd << " while waiting for an answer!" << endl;
        }
    }
}


int SlaveBase::readData( QByteArray &buffer)
{
   int result = waitForAnswer( MSG_DATA, 0, buffer );
   //kdDebug(7019) << "readData: length = " << result << " " << endl;
   return result;
}


void SlaveBase::dispatch( int command, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );

    KURL url;
    int i;

    switch( command ) {
    case CMD_HOST: {
        QString passwd;
        QString host, user;
        stream >> host >> i >> user >> passwd;
        setHost( host, i, user, passwd );
    }
    break;
    case CMD_CONNECT:
        openConnection( );
        break;
    case CMD_DISCONNECT:
        closeConnection( );
        break;
    case CMD_SLAVE_STATUS:
        slave_status();
        break;
    case CMD_REPARSECONFIGURATION:
        KProtocolManager::reparseConfiguration();
        reparseConfiguration();
        break;
    case CMD_CONFIG:
        stream >> d->configData;
        break;
    case CMD_GET: {
        stream >> url;
        get( url );
    }
    break;
    case CMD_PUT: {
        int permissions;
        Q_INT8 iOverwrite, iResume;
        stream >> url >> iOverwrite >> iResume >> permissions;
        bool overwrite = ( iOverwrite != 0 );
        bool resume = ( iResume != 0 );

        // Remember that we need to send canResume(), TransferJob is expecting
        // it. Well, in theory this shouldn't be done if resume is true.
        //   (the resume bool is currently unused)
        d->needSendCanResume = true   /* !resume */;

        put( url, permissions, overwrite, resume);
    }
    break;
    case CMD_STAT:
        stream >> url;
        stat( url );
        break;
    case CMD_MIMETYPE:
        stream >> url;
        mimetype( url );
        break;
    case CMD_LISTDIR:
        stream >> url;
        listDir( url );
        break;
    case CMD_MKDIR:
        stream >> url >> i;
        mkdir( url, i );
        break;
    case CMD_RENAME: {
        Q_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        rename( url, url2, overwrite );
    }
    break;
    case CMD_SYMLINK: {
        Q_INT8 iOverwrite;
        QString target;
        stream >> target >> url >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        symlink( target, url, overwrite );
    }
    break;
    case CMD_COPY: {
        int permissions;
        Q_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> permissions >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        copy( url, url2, permissions, overwrite );
    }
    break;
    case CMD_DEL: {
        Q_INT8 isFile;
        stream >> url >> isFile;
        del( url, isFile != 0);
    }
    break;
    case CMD_CHMOD:
        stream >> url >> i;
        chmod( url, i);
        break;
    case CMD_SPECIAL:
        special( data );
        break;
    case CMD_META_DATA:
        //kdDebug(7019) << "(" << getpid() << ") Incoming meta-data..." << endl;
        stream >> mIncomingMetaData;
        break;
    case CMD_SUBURL:
        stream >> url;
        setSubURL(url);
        break;
    case CMD_NONE:
        fprintf(stderr, "Got unexpected CMD_NONE!\n");
        break;
    default:
        // Some command we don't understand.
        // Just ignore it, it may come from some future version of KDE.
        break;
    }
}

QString SlaveBase::createAuthCacheKey( const KURL& url )
{
    if( url.isMalformed() )
        return QString::null;

    // Generate the basic key sequence.
    QString key = url.protocol();
    key += '-';
    key += url.host();
    int port = url.port();
    if( port )
    {
      key += ':';
      key += QString::number(port);
    }

    return key;
}

bool SlaveBase::pingCacheDaemon() const
{
    KDEsuClient client;
    int sucess = client.ping();
    if( sucess == -1 )
    {
        sucess = client.startServer();
        if( sucess == -1 )
        {
            kdDebug(7019) << "Cannot start a new deamon!!" << endl;
            return false;
        }
        kdDebug(7019) << "Sucessfully started new cache deamon!!" << endl;
    }
    return true;
}

bool SlaveBase::checkCachedAuthentication( const KURL& url, QString& user,
                                           QString& passwd )
{
    AuthInfo info;
    info.url = url;
    info.username = user;
    info.password = passwd;
    return checkCachedAuthentication( info );
}

bool SlaveBase::checkCachedAuthentication( const KURL& url,
                                           QString& user,
                                           QString& passwd,
                                           QString& realm,
                                           QString& extra,
                                           bool verify )
{
    AuthInfo info;
    info.url = url;
    info.username = user;
    info.password = passwd;
    info.realmValue = realm;
    info.digestInfo = extra;
    info.verifyPath = verify;
    return checkCachedAuthentication( info );
}

bool SlaveBase::checkCachedAuthentication( AuthInfo& info )
{
    if ( !pingCacheDaemon() )
        return false;

    QCString auth_key = createAuthCacheKey(info.url).utf8();
    if ( auth_key.isEmpty() )
        return false;

    KDEsuClient client;
    bool found = false;
    QCString grp_key = auth_key.copy();

    // Always ask for the keys that belong in a single group.  This
    // single check will determine whether we need to do further tests
    // to find a matching stored authentication key and hence reduce
    // the number of unnecessary calls to kdesud.
    if ( client.findGroup(grp_key) )
    {
        AuthKeysList list = client.getKeys(grp_key);
        int count = list.count();
        if ( count > 0 )
        {
            // Deal with protection space based authentications, namely HTTP.
            // It has by far the most complex scheme in terms of password
            // caching requirements. (DA)
            if ( info.verifyPath )
            {

                AuthKeysMap path, keys;
                AuthKeysList::Iterator lit = list.begin();
                for( ; lit != list.end(); lit++ )
                {
                    kdDebug(7019) << "key: " << *lit << endl;
                    path.insert(QString::fromUtf8( client.getVar(((*lit) +
                                                   "-path"))), (*lit));

                    if( !info.realmValue.isEmpty() )
                        keys.insert(QString::fromUtf8( client.getVar(((*lit) +
                                                       "-realm"))), (*lit));

                    if( d->multipleAuthCaching && !info.username.isEmpty() )
                        keys.insert(QString::fromUtf8( client.getVar(((*lit) +
                                                       "-user"))), (*lit));
                }

                QString new_path = info.url.path();
                if( new_path.isEmpty() )
                    new_path = '/';

                // Look for an exact match...
                if ( path.contains(new_path) )
                {
                    kdDebug(7019) << "STRICT TEST: Absolute Path match for: "
                                  << auth_key << endl;

                    if ( !info.realmValue.isEmpty() )
                    {
                        if ( keys.contains(info.realmValue) &&
                             (!d->multipleAuthCaching ||
                              info.username.isEmpty() ||
                              keys.find(info.username).data() ==
                              keys.find(info.realmValue).data()) )
                        {
                            auth_key = keys[info.realmValue];
                            found = true;
                            kdDebug(7019) << "Realm match: " << auth_key
                                          << endl;
                        }
                    }
                    else
                    {
                        if ( !d->multipleAuthCaching ||
                             info.username.isEmpty() ||
                             keys.find(info.username).data() ==
                             path.find(new_path).data() )
                        {
                            auth_key = path[new_path];
                            found = true;
                            kdDebug(7019) << "Path match: " << auth_key
                                          << endl;
                        }
                    }
                }

                if ( !found )
                {
                    // Now we can attempt the directory prefix match, i.e.
                    // testing whether the directiory of the new URL contains
                    // the stored_one.  If it does, then we have a match...
                    int last_slash = new_path.findRev( '/' );
                    if ( last_slash >= 0 && new_path.length() > 1 )
                        new_path.truncate(last_slash);

                    uint slen;
                    QString str;
                    AuthKeysMap::Iterator mit = path.begin();
                    kdDebug(7019) << "LOOSE TEST: Path only "
                                     "match for: " << auth_key << endl;
                    for ( ; mit != path.end(); ++mit )
                    {
                        str = mit.key();

                        kdDebug(7019) << "Stored path: " << str << endl;
                        kdDebug(7019) << "Stored Value: " << mit.data()
                                      << endl;

                        last_slash = str.findRev( '/' );
                        slen = str.length();

                        if ( last_slash >= 0 && slen > 1 )
                        {
                            str.truncate(last_slash);
                            slen = str.length();
                        }

                        if ( new_path.startsWith(str) &&
                             (new_path.length() == slen ||
                             slen == 1 || new_path[slen] == '/') )
                        {
                            if ( !info.realmValue.isEmpty() )
                            {
                                if ( keys.contains(info.realmValue) &&
                                     (!d->multipleAuthCaching ||
                                      info.username.isEmpty() ||
                                      keys.find(info.username).data() ==
                                      keys.find(info.realmValue).data() ) )
                                {
                                    auth_key = keys[info.realmValue];
                                    found = true;
                                    kdDebug(7019) << "Realm match: " << auth_key
                                                  << endl;
                                }
                            }
                            else
                            {
                                if ( !d->multipleAuthCaching ||
                                     info.username.isEmpty() ||
                                     keys.find(info.username).data() ==
                                     mit.data() )
                                {
                                    auth_key = mit.data();
                                    found = true;
                                    kdDebug(7019) << "Path match: " << auth_key
                                                  << endl;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // For non-realm based systems simply continue to the user
                // name matching section.  Otherwise perform a realm-only
                // based verification...
                if ( info.realmValue.isEmpty() )
                {
                    if ( d->multipleAuthCaching && !info.username.isEmpty() )
                    {
                        QCString key = auth_key;
                        key += ':';
                        key += info.username.utf8();
                        kdDebug(7019) << "Looking for: " << key << endl;
                        if ( list.findIndex(key) != -1 )
                        {
                            kdDebug(7019) << "Found matching key!" << endl;
                            auth_key = key;
                        }
                    }
                    found = true;
                }
                else
                {
                    kdDebug(7019) << "Realm based match for: " << info.realmValue << endl;
                    AuthKeysList::Iterator it = list.begin();
                    for( ; it != list.end(); ++it )
                    {
                        if ( info.realmValue == QString::fromUtf8(client.getVar((*it) + "-realm")) )
                        {
                            kdDebug(7019) << "Found a realm match!" << endl;
                            auth_key = (*it);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }

        // If we have no match then return false!!
        if ( !found )
        {
            kdDebug(7019) << "NO cached Authorization found!" << endl;
            return false;
        }

        // Now we obtain the cached Authentication if the user
        // name is empty or there is a match b/n the stored
        // and the supplied one.
        QString stored_user = QString::fromUtf8(client.getVar( auth_key + "-user") );
        bool emptyUser = info.username.isEmpty();
        kdDebug(7019) << "Stored username: " << stored_user << endl;
        kdDebug(7019) << "Current username: " << info.username << endl;
        if ( emptyUser || info.username == stored_user )
        {
            if ( emptyUser )
                info.username = stored_user;
            info.password = QString::fromUtf8(client.getVar( auth_key + "-pass" ) );
            if ( info.realmValue.isEmpty() )
                info.realmValue = QString::fromUtf8( client.getVar(auth_key + "-realm") );
            info.digestInfo = QString::fromUtf8( client.getVar( auth_key + "-extra") );
            kdDebug(7019) << "Found cached authorization for: " << auth_key << endl
                          << "  User= " << info.username << endl
                          << "  Password= [hidden]" << endl
                          << "  Realm= " << info.realmValue << endl
                          << "  Extra= " << info.digestInfo << endl;
            sendAuthenticationKey( auth_key, grp_key, info.keepPassword );
            return true;
        }
    }
    return false;
}

bool SlaveBase::cacheAuthentication( const KURL& url,
                                     const QString& user,
                                     const QString& passwd,
                                     const QString& realm,
                                     const QString& extra )
{
    AuthInfo info;
    info.url = url;
    info.username = user;
    info.password = passwd;
    info.realmValue = realm;
    info.digestInfo = extra;
    return cacheAuthentication( info );
}

bool SlaveBase::storeAuthInfo( const QCString& key, const QCString& group,
                               const AuthInfo& info )
{
    if ( !pingCacheDaemon() )
        return false;

    KDEsuClient client;

    // Add the new Authentication entry...
    client.setVar( (key+"-user"), info.username.utf8(), 0, group );
    client.setVar( (key+"-pass"), info.password.utf8(), 0, group );
    if ( !info.realmValue.isEmpty() )
    {
      client.setVar( (key+"-realm"), info.realmValue.utf8(), 0, group );
      QString new_path = info.url.path();
      if( new_path.isEmpty() )
        new_path = '/';
      client.setVar( (key+"-path"), new_path.utf8(), 0, group );
    }
    if ( !info.digestInfo.isEmpty() )
      client.setVar( (key+"-extra"), info.digestInfo.utf8(), 0, group );

    kdDebug(7019) << "Cached new authentication for: " << key << endl
                  << "  User= " << info.username << endl
                  << "  Password= [hidden]" << endl
                  << "  Realm= " << info.realmValue << endl
                  << "  Extra= " << info.digestInfo << endl;

    sendAuthenticationKey (key, group, info.keepPassword );
    return true;
}

bool SlaveBase::cacheAuthentication( const AuthInfo& info )
{
    QCString auth_key = createAuthCacheKey( info.url ).utf8();
    QCString grp_key = auth_key.copy();

    // Do not allow caching if: URL is malformed or user name is
    // empty or the password is null!!  Empty password is acceptable
    // but not a NULL one. This is mainly intended as a partial defense
    // against incorrect use (read:abuse) of calling this function.
    if ( auth_key.isEmpty() ||
        info.username.isEmpty() || info.password.isNull() )
        return false;

    if ( !info.realmValue.isEmpty() )
    {
      auth_key += ':';
      auth_key += info.realmValue.utf8();
    }

    bool isCached = storeAuthInfo(auth_key, grp_key, info);
    if ( d->multipleAuthCaching )
    {
      auth_key += ':';
      auth_key += info.username.utf8();
      isCached &= storeAuthInfo(auth_key, grp_key, info);
    }
    return isCached;
}

void SlaveBase::setMultipleAuthCaching( bool enable )
{
    d->multipleAuthCaching = enable;
}

bool SlaveBase::multipleAuthCaching() const
{
    return d->multipleAuthCaching;
}

int SlaveBase::connectTimeout()
{
    bool ok;
    QString tmp = metaData("ConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return KProtocolManager::defaultConnectTimeout();
}

int SlaveBase::proxyConnectTimeout()
{
    bool ok;
    QString tmp = metaData("ProxyConnectTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return KProtocolManager::defaultProxyConnectTimeout();
}


int SlaveBase::responseTimeout()
{
    bool ok;
    QString tmp = metaData("ResponseTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return KProtocolManager::defaultResponseTimeout();
}


int SlaveBase::readTimeout()
{
    bool ok;
    QString tmp = metaData("ReadTimeout");
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return KProtocolManager::defaultReadTimeout();
}
