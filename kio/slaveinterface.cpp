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
#include "kio/connection.h"
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <kio/observer.h>

using namespace KIO;

//////////////

SlaveInterface::SlaveInterface( Connection * connection )
{
    m_pConnection = connection;
    signal( SIGPIPE, sigpipe_handler );
}

bool SlaveInterface::dispatch()
{
    assert( m_pConnection );

    int cmd;
    QByteArray data;

    if (m_pConnection->read( &cmd, data ) == -1)
	return false;

    dispatch( cmd, data );
    return true;
}

void SlaveInterface::dispatch( int _cmd, const QByteArray &rawdata )
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
	kdDebug(7007) << "Finished [this = " << this << "]" << endl;
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
	    list.setAutoDelete(true);
	    UDSEntry entry;
	    for (uint i = 0; i < count; i++) {
		stream >> entry;
		list.append(new UDSEntry(entry));
	    }
	    emit listEntries(list);
	
	}
	break;
    case MSG_RESUME:
	stream >> b;
	emit canResume( (bool)b );
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
	kdDebug(7007) << "needs passwd\n";
	QString key, user, pass;
	stream >> key >> str1 >> user >> pass;
	openPassDlg(str1, user, pass, key);
	break;
    }
    case INF_INFOMESSAGE: {
        QString msg;
        stream >> msg;
        infoMessage(msg);
        break;
    }
    default:
	assert( 0 );
    }
}

void SlaveInterface::openPassDlg( const QString& head, const QString& user, const QString& pass )
{
    openPassDlg( head, user, pass, QString::null );
}

void SlaveInterface::openPassDlg( const QString& head, const QString& user, const QString& pass, const QString& key )
{
    kdDebug(7007) << "openPassDlg " << head << endl;
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly );
    QString u = user, p = pass;
    bool result = Observer::self()->authorize( u, p, head, key );
    if( result )
    {
        stream << u << p;
        m_pConnection->sendnow( CMD_USERPASS, packedArgs );
    }
    else
        m_pConnection->sendnow( CMD_NONE, packedArgs );		

}

void SlaveInterface::sigpipe_handler(int)
{
    kdDebug() << "*** SIGPIPE ***" << endl;
    // Do nothing.
    // dispatch will return false and that will trigger ERR_SLAVE_DIED in slave.cpp
}

#include "slaveinterface.moc"

