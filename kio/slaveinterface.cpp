#include "kio/slaveinterface.h"
#include "kio/connection.h"
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <kio/passdlg.h>

using namespace KIO;

//////////////

SlaveInterface::SlaveInterface( Connection * connection )
{
    m_pConnection = connection;
}

void SlaveInterface::sigchld_handler (int)
{
    int pid, status;

    while(true) {
        pid = waitpid(-1, &status, WNOHANG);
        if ( pid <= 0 ) {
            // Reinstall signal handler, since Linux resets to default after
            // the signal occured ( BSD handles it different, but it should do
            // no harm ).
            signal(SIGCHLD, sigchld_handler);
            return;
        }
    }
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
    case MSG_READY:
	emit ready();
	break;
    case MSG_FINISHED:
	kDebugInfo(7007, "Finished [this = %p]", this);
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
    case MSG_RENAMED:
	stream >> str1;
	
	emit renamed( str1 );
	break;
    case MSG_RESUME:
	stream >> b;
	emit canResume( (bool)b );
	break;
    case MSG_ERROR:
	stream >> i >> str1;
	kDebugInfo(7007, "error %d %s", i, debugString(str1));
	emit error( i, str1 );
	break;
    case INF_TOTAL_SIZE:
	stream >> ul;
	emit totalSize( ul );
	break;
    case INF_TOTAL_COUNT_OF_FILES:
	stream >> ul;
	
	emit totalFiles( ul );
	break;
    case INF_TOTAL_COUNT_OF_DIRS:
	stream >> ul;
	
	emit totalDirs( ul );
	break;
    case INF_PROCESSED_SIZE:
	stream >> ul;
	
	emit processedSize( ul );
	break;
    case INF_PROCESSED_COUNT_OF_FILES:
	stream >> ul;
	
	emit processedFiles( ul );
	break;
    case INF_PROCESSED_COUNT_OF_DIRS:
	stream >> ul;
	
	emit processedDirs( ul );
	break;
    case MSG_CONNECTED:
	emit connected();
	break;
    case INF_SCANNING_DIR:
	stream >> str1;
	emit scanningDir( str1 );
	break;
    case INF_SPEED:
	stream >> ul;
	
	emit speed( ul );
	break;
    case INF_MAKING_DIR:
	stream >> str1;
	
	emit makingDir( str1 );
	break;
    case INF_GETTING_FILE:
	stream >> str1;
	
	emit gettingFile( str1 );
	break;
    case INF_DELETING_FILE:
	stream >> str1;
	
	emit deletingFile( str1 );
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
	break;
    case INF_WARNING:
	stream >> str1;
	
	emit warning( str1 );
	break;
    case INF_NEED_PASSWD: {
      QString user, pass;
      stream >> str1 >> user >> pass;
      openPassDlg(str1, user, pass);
    }
    break;
    default:
	assert( 0 );
    }
};

void SlaveInterface::openPassDlg( const QString& head, const QString& user, const QString& pass )
{
    PassDlg dlg( 0L, 0L, true, 0, head, user, pass );
    QByteArray packedArgs;

    if ( dlg.exec() ) {
	QDataStream stream( packedArgs, IO_WriteOnly );
	stream <<  dlg.user()<< dlg.password();
	m_pConnection->sendnow( CMD_USERPASS, packedArgs );
    }
    else
	m_pConnection->sendnow( CMD_NONE, packedArgs );
}

#include "slaveinterface.moc"

