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
#include <signal.h>

#include <qfile.h>

#include <ksock.h>
#include <kcrash.h>
#include <kdesu/client.h>
#include <klocale.h>

#include <ksocks.h>

#include "slavebase.h"

#include "kio/slavebase.h"
#include "kio/connection.h"
#include "kio/ioslave_defaults.h"
#include "kio/slaveinterface.h"

using namespace KIO;

template class QPtrList<QValueList<UDSAtom> >;
typedef QValueList<QCString> AuthKeysList;
typedef QMap<QString,QCString> AuthKeysMap;
#define KIO_DATA QByteArray data; QDataStream stream( data, IO_WriteOnly ); stream
#define KIO_FILESIZE_T(x) (unsigned long)(x & 0xffffffff) << (unsigned long)(x >> 32)

namespace KIO {

class SlaveBaseConfig : public KConfigBase
{
public:
   SlaveBaseConfig(SlaveBase *_slave)
	: slave(_slave) { }

   bool internalHasGroup(const QCString &) const { qWarning("hasGroup(const QCString &)");
return false; }

   QStringList groupList() const { return QStringList(); }

   QMap<QString,QString> entryMap(const QString &) const
      { return QMap<QString,QString>(); }

   void reparseConfiguration() { }

   KEntryMap internalEntryMap( const QString &) const { return KEntryMap(); }

   KEntryMap internalEntryMap() const { return KEntryMap(); }

   void putData(const KEntryKey &, const KEntry&, bool) { }

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
    KURL onHoldUrl;
    bool onHold;

    struct timeval last_tv;
    KIO::filesize_t processed_size;
};

};

SlaveBase *globalSlave=0;

void genericsig_handler(int sigNumber)
{
   signal(sigNumber,SIG_IGN);
   //I don't think we can have the same problem here as in the sigsegv handler
   kdDebug()<<"kioslave : exiting due to signal "<<sigNumber<<endl;
   //call the dtor of the slave and exit
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
    listEntryCurrentSize = 100;
    struct timeval tp;
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;
    mConnectedToApp = true;

    d = new SlaveBasePrivate;
    // by kahl for netmgr (need a way to identify slaves)
    d->slaveid = protocol;
    d->slaveid += QString::number(getpid());
    d->resume = false;
    d->needSendCanResume = false;
    d->multipleAuthCaching = false;
    d->config = new SlaveBaseConfig(this);
    d->onHold = false;
    d->last_tv.tv_sec = 0;
    d->last_tv.tv_usec = 0;
    d->processed_size = 0;

    connectSlave(mAppSocket);
}

SlaveBase::~SlaveBase()
{
    delete d;
}

void SlaveBase::dispatchLoop()
{
    fd_set rfds;
    int retval;

    while (true) {
    FD_ZERO(&rfds);

    assert(appconn->inited());
    FD_SET(appconn->fd_from(), &rfds);

    retval = select(appconn->fd_from()+ 1, &rfds, NULL, NULL, 0);
    //kdDebug(7019) << "dispatchLoop(): select returned " << retval << endl;
    if (retval && FD_ISSET(appconn->fd_from(), &rfds))
    { // dispatch application messages
        int cmd;
        QByteArray data;
        if ( appconn->read(&cmd, data) != -1 )
        {
          dispatch(cmd, data);
        }
        else // some error occured, perhaps no more application
        {
          // When the app exits, should the slave be put back in the pool ?
          if (mConnectedToApp && !mPoolSocket.isEmpty())
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
    else
    {
       kdDebug(7019) << "dispatchLoop(): select returned " << retval << " "
                     << (errno==EBADF?"EBADF":errno==EINTR?"EINTR":errno==EINVAL?"EINVAL":errno==ENOMEM?"ENOMEM":"unknown")
                     << " (" << errno << ")" << endl;
       return;
    }
  }
}

void SlaveBase::connectSlave(const QString& path)
{
    appconn->init(new KSocket(QFile::encodeName(path)));
    if (!appconn->inited())
    {
        kdDebug(7019) << "SlaveBase: failed to connect to " << path << endl;
        this->~SlaveBase();
        ::exit(255);
    }

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

    // reset
    listEntryCurrentSize = 100;
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
    if (d->onHold)
       stream << d->onHoldUrl;
    m_pConnection->send( MSG_SLAVE_STATUS, data );
}

void SlaveBase::canResume()
{
    m_pConnection->send( MSG_CANRESUME );
}

void SlaveBase::totalSize( KIO::filesize_t _bytes )
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    m_pConnection->send( INF_TOTAL_SIZE, data );
}

void SlaveBase::processedSize( KIO::filesize_t _bytes )
{
    struct timeval tv;
    if ( gettimeofday( &tv, 0L ) == 0 ) {
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
	if ( msecdiff >= 100 ) { // emit size 10 times a second
	    KIO_DATA << KIO_FILESIZE_T(_bytes);
	    m_pConnection->send( INF_PROCESSED_SIZE, data );
	    d->last_tv.tv_sec = tv.tv_sec;
	    d->last_tv.tv_usec = tv.tv_usec;
	}
    }
    d->processed_size = _bytes;
}

void SlaveBase::processedPercent( float /* percent */ )
{
  kdDebug(7019) << "SlaveBase::processedPercent: STUB" << endl;
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
            (cmd == CMD_SLAVE_STATUS) ||
            (cmd == CMD_SLAVE_CONNECT) ||
            (cmd == CMD_SLAVE_HOLD) ||
            (cmd == CMD_MULTI_GET));
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
      // kdDebug(7019) << "(" << getpid() << ") mimeType: emitting meta data" << endl;
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
       // kdDebug(7019) << "(" << getpid() << ") Slavebase: mimetype got " << cmd << endl;
       if ( cmd == CMD_HOST) // Ignore.
          continue;
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
    static const int maximum_updatetime = 300;
    static const int minimum_updatetime = 100;

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
    KIO_DATA << (uint)list.count();
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
    exit(0);
    // signal(SIGPIPE,&sigpipe_handler);
}

void SlaveBase::setHost(QString const &, int, QString const &, QString const &)
{
}

void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, opening connections is not supported with the protocol %1" ).arg(mProtocol)); }
void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KURL const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, accessing files is not supported with the protocol %1").arg(mProtocol) ); }
void SlaveBase::put(KURL const &, int, bool, bool)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, writing to %1 is not supported").arg(mProtocol) ); }
void SlaveBase::special(QMemArray<char> const &)
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
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, using suburls with %1 is not supported").arg(mProtocol) ); }
void SlaveBase::multiGet(QMemArray<char> const &)
{ error(  ERR_UNSUPPORTED_ACTION, i18n("Sorry, multiple get is not supported with protocol %1").arg(mProtocol) ); }


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
    {
    kdDebug(7019) << "SlaveBase::dispatch() has read error." << endl;
        return false;
    }

    dispatch( cmd, data );
    return true;
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

int SlaveBase::messageBox( MessageBoxType type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo )
{
    kdDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo << endl;
    KIO_DATA << (int)type << text << caption << buttonYes << buttonNo;
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

bool SlaveBase::canResume( KIO::filesize_t offset )
{
    kdDebug(7019) << "SlaveBase::canResume offset=" << KIO::number(offset) << endl;
    d->needSendCanResume = false;
    KIO_DATA << KIO_FILESIZE_T(offset);
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
        {
    kdDebug(7019) << "SlaveBase::waitForAnswer has read error." << endl;
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
    case CMD_SLAVE_CONNECT:
    {
        d->onHold = false;
        QString app_socket;
        QDataStream stream( data, IO_ReadOnly);
        stream >> app_socket;
        appconn->send( MSG_SLAVE_ACK );
        disconnectSlave();
        mConnectedToApp = true;
        connectSlave(app_socket);
    } break;
    case CMD_SLAVE_HOLD:
    {
        KURL url;
        QDataStream stream( data, IO_ReadOnly);
        stream >> url;
        d->onHoldUrl = url;
        d->onHold = true;
        disconnectSlave();
        mConnectedToApp = false;
        // Do not close connection!
        connectSlave(mPoolSocket);
    } break;
    case CMD_REPARSECONFIGURATION:
        reparseConfiguration();
        break;
    case CMD_CONFIG:
        stream >> d->configData;
        KSocks::setConfig(d->config);
        break;
    case CMD_GET:
    {
        stream >> url;
        get( url );
    } break;
    case CMD_PUT:
    {
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
    } break;
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
    case CMD_RENAME:
    {
        Q_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        rename( url, url2, overwrite );
    } break;
    case CMD_SYMLINK:
    {
        Q_INT8 iOverwrite;
        QString target;
        stream >> target >> url >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        symlink( target, url, overwrite );
    } break;
    case CMD_COPY:
    {
        int permissions;
        Q_INT8 iOverwrite;
        KURL url2;
        stream >> url >> url2 >> permissions >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        copy( url, url2, permissions, overwrite );
    } break;
    case CMD_DEL:
    {
        Q_INT8 isFile;
        stream >> url >> isFile;
        del( url, isFile != 0);
    } break;
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
    case CMD_MULTI_GET:
        multiGet( data );
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
    if (auth_key.isEmpty() || info.username.isNull() || info.password.isNull())
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

void SlaveBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

