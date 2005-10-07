/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kio/slaveinterface.h"
#include "kio/slavebase.h"
#include "kio/connection.h"
#include <errno.h>
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <kio/observer.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <time.h>
#include <qtimer.h>

using namespace KIO;

QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e )
{
    s << e.size();
    KIO::UDSEntry::ConstIterator it = e.begin();
    const KIO::UDSEntry::ConstIterator end = e.end();
    for( ; it != end; ++it )
    {
        const quint32 uds = it.key();
        s << uds;
        if ( uds & KIO::UDS_STRING )
            s << it.value().toString();
        else if ( uds & KIO::UDS_NUMBER )
            s << it.value().toNumber();
        else
            Q_ASSERT( 0 );
    }
    return s;
}

QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e )
{
    e.clear();
    quint32 size;
    s >> size;
    for( quint32 i = 0; i < size; ++i )
    {
        quint32 uds;
        s >> uds;
        if (uds & KIO::UDS_STRING) {
            QString str;
            s >> str;
            e.insert( uds, str );
        } else if (uds & KIO::UDS_NUMBER) {
            long long val;
            s >> val;
            e.insert( uds, val );
        } else
            Q_ASSERT( 0 );
    }
    return s;
}

static const unsigned int max_nums = 8;

class KIO::SlaveInterfacePrivate
{
public:
  SlaveInterfacePrivate() {
    slave_calcs_speed = false;
    start_time.tv_sec = 0;
    start_time.tv_usec = 0;
    last_time = 0;
    nums = 0;
    filesize = 0;
    offset = 0;
  }
  bool slave_calcs_speed;
  struct timeval start_time;
  uint nums;
  long times[max_nums];
  KIO::filesize_t sizes[max_nums];
  size_t last_time;
  KIO::filesize_t filesize, offset;

  QTimer speed_timer;
};

//////////////

SlaveInterface::SlaveInterface( Connection * connection )
{
    m_pConnection = connection;
    m_progressId = 0;

    d = new SlaveInterfacePrivate;
    connect(&d->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kdDebug() here (scheduler is deleted very late)
    m_pConnection = 0; // a bit like the "wasDeleted" of QObject...

    delete d;
}

static KIO::filesize_t readFilesize_t(QDataStream &stream)
{
   KIO::filesize_t result;
   quint32 ul; // TODO: this was a long and possibly it was meant to be
   stream >> ul;
   result = ul;
   if (stream.atEnd())
      return result;
   stream >> ul;
   result += ((KIO::filesize_t)ul) << 32;
   return result;
}


bool SlaveInterface::dispatch()
{
    assert( m_pConnection );

    int cmd;
    QByteArray data;

    if (m_pConnection->read( &cmd, data ) == -1)
      return false;

    return dispatch( cmd, data );
}

void SlaveInterface::calcSpeed()
{
  if (d->slave_calcs_speed) {
    d->speed_timer.stop();
    return;
  }

  struct timeval tv;
  gettimeofday(&tv, 0);

  long diff = ((tv.tv_sec - d->start_time.tv_sec) * 1000000 +
	       tv.tv_usec - d->start_time.tv_usec) / 1000;
  if (diff - d->last_time >= 900) {
    d->last_time = diff;
    if (d->nums == max_nums) {
      // let's hope gcc can optimize that well enough
      // otherwise I'd try memcpy :)
      for (unsigned int i = 1; i < max_nums; ++i) {
	d->times[i-1] = d->times[i];
	d->sizes[i-1] = d->sizes[i];
      }
      d->nums--;
    }
    d->times[d->nums] = diff;
    d->sizes[d->nums++] = d->filesize - d->offset;

    KIO::filesize_t lspeed = 1000 * (d->sizes[d->nums-1] - d->sizes[0]) / (d->times[d->nums-1] - d->times[0]);

//     kdDebug() << "proceeed " << (long)d->filesize << " " << diff << " "
// 	      << long(d->sizes[d->nums-1] - d->sizes[0]) << " "
// 	      <<  d->times[d->nums-1] - d->times[0] << " "
// 	      << long(lspeed) << " " << double(d->filesize) / diff
// 	      << " " << convertSize(lspeed) << " "
// 	      << convertSize(long(double(d->filesize) / diff) * 1000) << " "
// 	      <<  endl ;

    if (!lspeed) {
      d->nums = 1;
      d->times[0] = diff;
      d->sizes[0] = d->filesize - d->offset;
    }
    emit speed(lspeed);
  }
}

bool SlaveInterface::dispatch( int _cmd, const QByteArray &rawdata )
{
    //kdDebug(7007) << "dispatch " << _cmd << endl;

    QDataStream stream( rawdata );

    QString str1;
    qint32 i;
    qint8 b;
    quint32 ul;

    switch( _cmd ) {
    case MSG_DATA:
	emit data( rawdata );
	break;
    case MSG_DATA_REQ:
        emit dataReq();
	break;
    case MSG_FINISHED:
	//kdDebug(7007) << "Finished [this = " << this << "]" << endl;
        d->offset = 0;
        d->speed_timer.stop();
	emit finished();
	break;
    case MSG_STAT_ENTRY:
	{
	    UDSEntry entry;
	    stream >> entry;
	    emit statEntry(entry);
	}
	break;
    case MSG_LIST_ENTRIES:
	{
	    quint32 count;
	    stream >> count;

	    UDSEntryList list;
	    UDSEntry entry;
	    for (uint i = 0; i < count; i++) {
		stream >> entry;
		list.append(entry);
	    }
	    emit listEntries(list);

	}
	break;
    case MSG_RESUME: // From the put job
	{
	    d->offset = readFilesize_t(stream);
	    emit canResume( d->offset );
	}
	break;
    case MSG_CANRESUME: // From the get job
        d->filesize = d->offset;
        emit canResume(0); // the arg doesn't matter
        break;
    case MSG_ERROR:
	stream >> i >> str1;
	kdDebug(7007) << "error " << i << " " << str1 << endl;
	emit error( i, str1 );
	break;
    case MSG_SLAVE_STATUS:
        {
           pid_t pid;
           DCOPCString protocol;
           stream >> pid >> protocol >> str1 >> b;
           emit slaveStatus(pid, protocol, str1, (b != 0));
        }
        break;
    case MSG_CONNECTED:
	emit connected();
	break;

    case INF_TOTAL_SIZE:
	{
	    KIO::filesize_t size = readFilesize_t(stream);
	    gettimeofday(&d->start_time, 0);
	    d->last_time = 0;
	    d->filesize = d->offset;
	    d->sizes[0] = d->filesize - d->offset;
	    d->times[0] = 0;
	    d->nums = 1;
	    d->speed_timer.start(1000);
	    d->slave_calcs_speed = false;
	    emit totalSize( size );
	}
	break;
    case INF_PROCESSED_SIZE:
	{
	    KIO::filesize_t size = readFilesize_t(stream);
	    emit processedSize( size );
	    d->filesize = size;
	}
	break;
    case INF_SPEED:
	stream >> ul;
	d->slave_calcs_speed = true;
	d->speed_timer.stop();

	emit speed( ul );
	break;
    case INF_GETTING_FILE:
	break;
    case INF_ERROR_PAGE:
	emit errorPage();
	break;
    case INF_REDIRECTION:
      {
	KURL url;
	stream >> url;

	emit redirection( url );
      }
      break;
    case INF_MIME_TYPE:
	stream >> str1;

	emit mimeType( str1 );
        if (!m_pConnection->suspended())
            m_pConnection->sendnow( CMD_NONE, QByteArray() );
	break;
    case INF_WARNING:
	stream >> str1;

	emit warning( str1 );
	break;
    case INF_NEED_PASSWD: {
        AuthInfo info;
       	stream >> info;
	openPassDlg( info );
	break;
    }
    case INF_MESSAGEBOX: {
	kdDebug(7007) << "needs a msg box" << endl;
	QString text, caption, buttonYes, buttonNo, dontAskAgainName;
        int type;
	stream >> type >> text >> caption >> buttonYes >> buttonNo;
	if (stream.atEnd())
	messageBox(type, text, caption, buttonYes, buttonNo);
	else {
	    stream >> dontAskAgainName;
	    messageBox(type, text, caption, buttonYes, buttonNo, dontAskAgainName);
	}
	break;
    }
    case INF_INFOMESSAGE: {
        QString msg;
        stream >> msg;
        infoMessage(msg);
        break;
    }
    case INF_META_DATA: {
        MetaData meta_data;
        stream >> meta_data;
        metaData(meta_data);
        break;
    }
    case MSG_NET_REQUEST: {
        QString host;
	QString slaveid;
        stream >> host >> slaveid;
        requestNetwork(host, slaveid);
        break;
    }
    case MSG_NET_DROP: {
        QString host;
	QString slaveid;
        stream >> host >> slaveid;
        dropNetwork(host, slaveid);
        break;
    }
    case MSG_NEED_SUBURL_DATA: {
        emit needSubURLData();
        break;
    }
    case MSG_AUTH_KEY: {
        bool keep;
        Q3CString key, group;
        stream >> key >> group >> keep;
        kdDebug(7007) << "Got auth-key:      " << key << endl
                      << "    group-key:     " << group << endl
                      << "    keep password: " << keep << endl;
        emit authorizationKey( key, group, keep );
        break;
    }
    case MSG_DEL_AUTH_KEY: {
        Q3CString key;
        stream >> key;
        kdDebug(7007) << "Delete auth-key: " << key << endl;
        emit delAuthorization( key );
    }
    default:
        kdWarning(7007) << "Slave sends unknown command (" << _cmd << "), dropping slave" << endl;
	return false;
    }
    return true;
}

void SlaveInterface::setOffset( KIO::filesize_t o)
{
    d->offset = o;
}

KIO::filesize_t SlaveInterface::offset() const { return d->offset; }

void SlaveInterface::requestNetwork(const QString &host, const QString &slaveid)
{
    kdDebug(7007) << "requestNetwork " << host << slaveid << endl;
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );
    stream << true;
    m_pConnection->sendnow( INF_NETWORK_STATUS, packedArgs );
}

void SlaveInterface::dropNetwork(const QString &host, const QString &slaveid)
{
    kdDebug(7007) << "dropNetwork " << host << slaveid << endl;
}

void SlaveInterface::sendResumeAnswer( bool resume )
{
    kdDebug(7007) << "SlaveInterface::sendResumeAnswer ok for resuming :" << resume << endl;
    m_pConnection->sendnow( resume ? CMD_RESUMEANSWER : CMD_NONE, QByteArray() );
}

void SlaveInterface::openPassDlg( const QString& prompt, const QString& user, bool readOnly )
{
    AuthInfo info;
    info.prompt = prompt;
    info.username = user;
    info.readOnly = readOnly;
    openPassDlg( info );
}

void SlaveInterface::openPassDlg( const QString& prompt, const QString& user,
                                  const QString& caption, const QString& comment,
                                  const QString& label, bool readOnly )
{
    AuthInfo info;
    info.prompt = prompt;
    info.username = user;
    info.caption = caption;
    info.comment = comment;
    info.commentLabel = label;
    info.readOnly = readOnly;
    openPassDlg( info );
}

void SlaveInterface::openPassDlg( AuthInfo& info )
{
    kdDebug(7007) << "SlaveInterface::openPassDlg: "
                  << "User= " << info.username
                  << ", Message= " << info.prompt << endl;
    bool result = Observer::self()->openPassDlg( info );
    if ( m_pConnection )
    {
        QByteArray data;
        QDataStream stream( &data, QIODevice::WriteOnly );
        if ( result )
        {
            stream << info;
            kdDebug(7007) << "SlaveInterface:::openPassDlg got: "
                          << "User= " << info.username
                          << ", Password= [hidden]" << endl;
            m_pConnection->sendnow( CMD_USERPASS, data );
        }
        else
            m_pConnection->sendnow( CMD_NONE, data );
    }
}

void SlaveInterface::messageBox( int type, const QString &text, const QString &_caption,
                                 const QString &buttonYes, const QString &buttonNo )
{
    messageBox( type, text, _caption, buttonYes, buttonNo, QString::null );
}

void SlaveInterface::messageBox( int type, const QString &text, const QString &_caption,
                                 const QString &buttonYes, const QString &buttonNo, const QString &dontAskAgainName )
{
    kdDebug(7007) << "messageBox " << type << " " << text << " - " << _caption << " " << dontAskAgainName << endl;
    QByteArray packedArgs;
    QDataStream stream( &packedArgs, QIODevice::WriteOnly );

    QString caption( _caption );
    if ( type == KIO::SlaveBase::SSLMessageBox )
        caption = QString::fromUtf8(KApplication::dcopClient()->appId()); // hack, see observer.cpp

    emit needProgressId();
    kdDebug(7007) << "SlaveInterface::messageBox m_progressId=" << m_progressId << endl;
    QPointer<SlaveInterface> me = this;
    m_pConnection->suspend();
    int result = Observer::/*self()->*/messageBox( m_progressId, type, text, caption, buttonYes, buttonNo, dontAskAgainName );
    if ( me && m_pConnection ) // Don't do anything if deleted meanwhile
    {
        m_pConnection->resume();
        kdDebug(7007) << this << " SlaveInterface result=" << result << endl;
        stream << result;
        m_pConnection->sendnow( CMD_MESSAGEBOXANSWER, packedArgs );
    }
}

void SlaveInterface::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "slaveinterface.moc"
