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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kio/slaveinterface.h"
#include "kio/slavebase.h"
#include "kio/connection.h"
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <kio/observer.h>
#include <kapplication.h>
#include <dcopclient.h>

using namespace KIO;

//////////////

SlaveInterface::SlaveInterface( Connection * connection )
{
    m_pConnection = connection;
    m_progressId = 0;
    signal( SIGPIPE, sigpipe_handler );
}

SlaveInterface::~SlaveInterface()
{
    // Note: no kdDebug() here (scheduler is deleted very late)
    m_pConnection = 0; // a bit like the "wasDeleted" of QObject...
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

bool SlaveInterface::dispatch( int _cmd, const QByteArray &rawdata )
{
    //kdDebug(7007) << "dispatch " << _cmd << endl;

    QDataStream stream( rawdata, IO_ReadOnly );

    QString str1;
    int i;
    Q_INT8 b;
    unsigned long ul;

    switch( _cmd ) {
    case MSG_DATA:
	emit data( rawdata );
	break;
    case MSG_DATA_REQ:
        emit dataReq();
	break;
    case MSG_FINISHED:
	//kdDebug(7007) << "Finished [this = " << this << "]" << endl;
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
	    uint count;
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
	stream >> ul;
	emit canResume( ul );
	break;
     case MSG_CANRESUME: // From the get job
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
           QCString protocol;
           stream >> pid >> protocol >> str1 >> b;
           emit slaveStatus(pid, protocol, str1, (b != 0));
        }
        break;
    case MSG_CONNECTED:
	emit connected();
	break;

    case INF_TOTAL_SIZE:
	stream >> ul;
	emit totalSize( ul );
	break;
    case INF_PROCESSED_SIZE:
	stream >> ul;

	emit processedSize( ul );
	break;
    case INF_SPEED:
	stream >> ul;

	emit speed( ul );
	break;
    case INF_GETTING_FILE:
	stream >> str1;

	emit gettingFile( str1 );
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
	QString text, caption, buttonYes, buttonNo;
        int type;
	stream >> type >> text >> caption >> buttonYes >> buttonNo;
	messageBox(type, text, caption, buttonYes, buttonNo);
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
        QCString key, group;
        stream >> key >> group >> keep;
        kdDebug(7007) << "Got auth-key:      " << key << endl
                      << "    group-key:     " << group << endl
                      << "    keep password: " << keep << endl;
        emit authenticationKey( key, group );
        emit authorizationKey( key, group, keep );
        break;
    }
    case MSG_DEL_AUTH_KEY: {
        QCString key;
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

void SlaveInterface::requestNetwork(const QString &host, const QString &slaveid)
{
    kdDebug(7007) << "requestNetwork " << host << slaveid << endl;
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly );
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
    return;
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
        QDataStream stream( data, IO_WriteOnly );
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
    kdDebug(7007) << "messageBox " << type << " " << text << " - " << _caption << endl;
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly );

    QString caption( _caption );
    if ( type == KIO::SlaveBase::SSLMessageBox )
        caption = QString::fromUtf8(kapp->dcopClient()->appId()); // hack, see uiserver.cpp

    emit needProgressId();
    kdDebug(7007) << "SlaveInterface::messageBox m_progressId=" << m_progressId << endl;
    int result = Observer::self()->messageBox( m_progressId, type, text, caption, buttonYes, buttonNo );
    if ( m_pConnection ) // Don't do anything if deleted meanwhile
    {
        kdDebug(7007) << this << " SlaveInterface result=" << result << endl;
        stream << result;
        m_pConnection->sendnow( CMD_MESSAGEBOXANSWER, packedArgs );
    }
}

void SlaveInterface::sigpipe_handler(int)
{
    kdDebug(7007) << "*** SIGPIPE ***" << endl;
    // Do nothing.
    // dispatch will return false and that will trigger ERR_SLAVE_DIED in slave.cpp
}

QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e )
{
    // On 32-bit platforms we send UDS_SIZE with UDS_SIZE_LARGE in front
    // of it to carry the 32 msb. We can't send a 64 bit UDS_SIZE because
    // that would break the compatibility of the wire-protocol with KDE 2.
    // On 64-bit platforms nothing has changed.
    if (sizeof(long) == 8)
    {
        s << (Q_UINT32)e.size();
        KIO::UDSEntry::ConstIterator it = e.begin();
        for( ; it != e.end(); ++it )
           s << *it;
        return s;
    }

    Q_UINT32 size = 0;
    KIO::UDSEntry::ConstIterator it = e.begin();
    for( ; it != e.end(); ++it )
    {
       size++;
       if ((*it).m_uds == KIO::UDS_SIZE)
          size++;
    }
    s << size;
    it = e.begin();
    for( ; it != e.end(); ++it )
    {
       if ((*it).m_uds == KIO::UDS_SIZE)
       {
          KIO::UDSAtom a;
          a.m_uds = KIO::UDS_SIZE_LARGE;
          a.m_long = (*it).m_long >> 32;
          s << a;
       }
       s << *it;
    }
    return s;
}

QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e )
{
    e.clear();
    Q_UINT32 size;
    s >> size;

    // On 32-bit platforms we send UDS_SIZE with UDS_SIZE_LARGE in front
    // of it to carry the 32 msb. We can't send a 64 bit UDS_SIZE because
    // that would break the compatibility of the wire-protocol with KDE 2.
    // On 64-bit platforms nothing has changed.
    if (sizeof(long) == 8)
    {
       for(Q_UINT32 i = 0; i < size; i++)
       {
          KIO::UDSAtom a;
          s >> a;
          e.append(a);
       }
    }
    else
    {
       long long msb = 0;
       for(Q_UINT32 i = 0; i < size; i++)
       {
          KIO::UDSAtom a;
          s >> a;
          if (a.m_uds == KIO::UDS_SIZE_LARGE)
          {
             msb = a.m_long;
          }
          else
          {
             if (a.m_uds == KIO::UDS_SIZE)
             {
                a.m_long += msb << 32;
             }
             e.append(a);
             msb = 0;
          }
       }
    }
    return s;
}


#include "slaveinterface.moc"

