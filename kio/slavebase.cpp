#include "kio/slavebase.h"
#include "kio/slaveinterface.h"
#include "kio/connection.h"
#include <assert.h>
#include <kdebug.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <kdebug2.h>

using namespace KIO;

template class QList<QValueList<UDSAtom> >;

#define KIO_DATA QByteArray data; QDataStream stream( data, IO_WriteOnly ); stream

//////////////

SlaveBase::SlaveBase( Connection * connection )
{
    m_pConnection = connection;

    pendingListEntries.setAutoDelete(true);
    listEntryCurrentSize = 0;
    struct timeval tp;
    gettimeofday(&tp, 0);
    listEntry_sec = tp.tv_sec;
    listEntry_usec = tp.tv_usec;
}

void SlaveBase::data( const QByteArray &data )
{
    m_pConnection->send( MSG_DATA, data );
}

void SlaveBase::dataReq( )
{
    m_pConnection->send( MSG_DATA_REQ );
}

void SlaveBase::error( int _errid, const QString &_text )
{
    KIO_DATA << _errid << _text;

    m_pConnection->send( MSG_ERROR, data );
}

void SlaveBase::ready()
{
    m_pConnection->send( MSG_READY );
}

void SlaveBase::connected()
{
    m_pConnection->send( MSG_CONNECTED );
}

void SlaveBase::finished()
{
    m_pConnection->send( MSG_FINISHED );
}

void SlaveBase::renamed( const QString &_new )
{
    KIO_DATA << _new;
    m_pConnection->send( MSG_RENAMED, data );
}

void SlaveBase::canResume( bool _resume )
{
    KIO_DATA << (int)_resume;
    m_pConnection->send( MSG_RESUME, data );
}

void SlaveBase::totalSize( unsigned long _bytes )
{
    KIO_DATA << _bytes;
    m_pConnection->send( INF_TOTAL_SIZE, data );
}

void SlaveBase::totalFiles( unsigned int _files )
{
    KIO_DATA << _files;
    m_pConnection->send( INF_TOTAL_COUNT_OF_FILES, data );
}

void SlaveBase::totalDirs( unsigned int _dirs )
{
    KIO_DATA << _dirs;
    m_pConnection->send( INF_TOTAL_COUNT_OF_DIRS, data );
}

void SlaveBase::processedSize( unsigned long _bytes )
{
    KIO_DATA << _bytes;
    m_pConnection->send( INF_PROCESSED_SIZE, data );
}

void SlaveBase::scanningDir( const KURL&_dir )
{
    KIO_DATA << _dir;
    m_pConnection->send( INF_SCANNING_DIR, data );
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

void SlaveBase::mimeType( const QString &_type)
{
    KIO_DATA << _type;
    m_pConnection->send( INF_MIME_TYPE, data );
}

void SlaveBase::gettingFile( const QString &file)
{
    KIO_DATA << file;
    m_pConnection->send( INF_GETTING_FILE, data );
}

void SlaveBase::warning( const QString &_msg)
{
    KIO_DATA << _msg;
    m_pConnection->send( INF_WARNING, data );
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
	pendingListEntries.append(new UDSEntry(entry));
	
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
    kDebugInfo( 7007, "listEntries %ld", list.count() );

    KIO_DATA << list.count();
    UDSEntryListIterator it(list);
    for (; it.current(); ++it)
	stream << *it.current();
    m_pConnection->send( MSG_LIST_ENTRIES, data);
}

/*
void SlaveBase::isDirectory()
{
    m_pConnection->send( MSG_IS_DIRECTORY, QByteArray());
}

void SlaveBase::isFile()
{
    m_pConnection->send( MSG_IS_FILE, QByteArray());
}
*/

void SlaveBase::sigsegv_handler (int)
{
    // Debug and printf should be avoided because they might
    // call malloc.. and get in a nice recursive malloc loop
    write(2, "kioslave : ###############SEG FAULT#############\n", 49);
    exit(1);
}

void SlaveBase::openConnection(QString const &host, int, QString const &, QString const &)
{
   kDebugInfo( 7007, "openConnection( host = %s)", host.ascii() );
   connected();
}

void SlaveBase::closeConnection(void)
{
   ready();
}



void SlaveBase::stat(QString const &)
{ error(  ERR_UNSUPPORTED_ACTION, "stat" ); }
void SlaveBase::put(QString const &, int, bool, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "put" ); }
void SlaveBase::special(QArray<char> const &)
{ error(  ERR_UNSUPPORTED_ACTION, "special" ); }
void SlaveBase::listDir(QString const &)
{ error(  ERR_UNSUPPORTED_ACTION, "listDir" ); }
void SlaveBase::get(QString const &, QString const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "get" ); }
void SlaveBase::mimetype(QString const &path)
{ get(path, QString::null, false); }
void SlaveBase::rename(QString const &, QString const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "rename" ); }
void SlaveBase::copy(const QString &, const QString &, int, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "copy" ); }
void SlaveBase::del(QString const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, "del" ); }
void SlaveBase::mkdir(QString const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, "mkdir" ); }
void SlaveBase::chmod(QString const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, "chmod" ); }

void SlaveBase::dispatchLoop()
{
    while( dispatch() );
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

bool SlaveBase::openPassDlg( const QString& head, QString& user, QString& pass )
{
    kdDebug(7007) << "openPassDlg " << head << endl;
    KIO_DATA << head << user << pass;
    m_pConnection->send( INF_NEED_PASSWD, data );
    int cmd;
    if ( m_pConnection->read( &cmd, data ) == -1 ) {
	return false;
    }
    kDebugInfo(7007, "reading %d", cmd);
    if (cmd != CMD_USERPASS) {
	if (cmd != CMD_NONE)
	    dispatch( cmd, data );
      return false;
    } else {
      QDataStream stream( data, IO_ReadOnly );
      stream >> user >> pass;
      kDebugInfo(7007, "got %d %s %s", cmd, user.ascii(), pass.ascii());
      return true;
    }
}

int SlaveBase::readData( QByteArray &buffer)
{
   int cmd;
   int result;
   result = m_pConnection->read( &cmd, buffer );

   if (result == -1)
      return -1;

   kDebugInfo( 7007, "readData: cmd = %d, length = %d ", cmd, result );

   if (cmd != MSG_DATA)
      return -1;

   return result;
}


void SlaveBase::dispatch( int command, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );

    QString str1, str2;
    int i;

    switch( command ) {
    case CMD_CONNECT: {
        QString passwd;
	stream >> str1 >> i >> str2 >> passwd;
	openConnection( str1, i, str2, passwd );
    }
    break;
    case CMD_DISCONNECT:
	closeConnection( );
	break;
    case CMD_GET: {
        Q_INT8 iReload;

        stream >> str1 >> str2 >> iReload;

        bool reload = (iReload != 0);	

	get( str1, str2, reload );
    }
    break;
    case CMD_PUT: {
	int permissions;
	Q_INT8 iOverwrite, iResume;
	
	stream >> iOverwrite >> iResume >> permissions >> str1;
	
	bool overwrite = ( iOverwrite != 0 );
	bool resume = ( iResume != 0 );
	
	put( str1, permissions, overwrite, resume);
    }
    break;
    case CMD_STAT:
	stream >> str1;
	stat( str1 );
	break;
    case CMD_MIMETYPE:
	stream >> str1;
	mimetype( str1 );
	break;
    case CMD_LISTDIR:
	stream >> str1;
	listDir( str1 );
	break;
    case CMD_MKDIR:
	stream >> str1 >> i;
	mkdir( str1, i );
	break;
    case CMD_RENAME:{
	Q_INT8 iOverwrite;
        stream >> str1 >> str2 >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        rename( str1, str2, overwrite );
    }
    break;
    case CMD_COPY: {
        int permissions;
        Q_INT8 iOverwrite;
        stream >> str1 >> str2 >> permissions >> iOverwrite;
        bool overwrite = (iOverwrite != 0);
        copy( str1, str2, permissions, overwrite );
    }
    break;
    case CMD_DEL:
        Q_INT8 isFile;
        stream >> str1 >> isFile;
	del( str1, isFile != 0);
	break;
    case CMD_CHMOD:
        stream >> str1 >> i;
	chmod( str1, i);
	break;
    case CMD_SPECIAL:
	special( data );
	break;
    default:
	assert( 0 );
    }
}

