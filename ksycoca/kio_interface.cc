// $Id$

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kio_interface.h"

#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>

QString KIO::kioErrorString( int _errid, const char *_errortext )
{
  QString result;
  
  switch( _errid )
    {
    case  ERR_CANNOT_OPEN_FOR_READING:
      result = i18n( "Could not read\n%1" ).arg( _errortext );      
      break;
    case  ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process\n%1" ).arg( _errortext );
      break;
    case  ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bugreport at http://bugs.kde.org\n\n%1" ).arg( _errortext );
      break;
    case  ERR_MALFORMED_URL:
      result = i18n( "Malformed URL\n%1" ).arg( _errortext );
      break;
    case  ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1\n is not supported" ).arg( _errortext );
      break;
    case  ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1\nis only a filter protocol.\n").arg( _errortext );
      break;
    case  ERR_UNSUPPORTED_ACTION:
      result = i18n( "Unsupported action\n%1" ).arg( _errortext );
      break;
    case  ERR_DOES_NOT_EXIST:
      result = i18n( "The file or directory\n%1\ndoes not exist" ).arg( _errortext );
      break;
    case  ERR_IS_DIRECTORY:
      result = i18n( "%1\n is a directory,\nbut a file was expected" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make directory\n%1" ).arg( _errortext );
      break;
    case  ERR_DOES_ALREADY_EXIST:
      result = i18n( "The file or directory\n%1\ndoes already exist" ).arg( _errortext );
      break;
    case  ERR_ACCESS_DENIED:
      result = i18n( "Access denied to\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter directory\n%1" ).arg( _errortext );
      break;
    case  ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1\ndoes not implement a directory service" ).arg( _errortext );
      break;
    case  ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for\n%1" ).arg( _errortext );
      break;
    case  ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied\nCould not write to\n%1" ).arg( _errortext );
      break;
    case  ERR_USER_CANCELED:
      result = i18n( "Action has been\ncanceled by the user\n%1" ).arg( _errortext );
      break;
    case  ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host\n%1" ).arg( _errortext );
      break;
    case  ERR_UNKNOWN_HOST:
      result = i18n( "Unknown host\n%1" ).arg( _errortext );
      break;
    case  ERR_UNKNOWN_PROXY_HOST:
      result = i18n( "Unknown proxy host\n%1" ).arg( _errortext );
      break;
    case  ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host\n%1\nis broken" ).arg( _errortext );
      break;
    case  ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1\nis not a filter protocol" ).arg( _errortext );
      break;
    case  ERR_IS_FILE:
      result = i18n( "%1\nis a file,\nbut a directory was expected" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n\n%1" ).arg( _errortext );
      break;
    case  ERR_SLAVE_DIED:
      result = i18n( "The process for the\n%1 protocol\ndied unexpectedly" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_STAT:
      result = i18n( "Could not access\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_LOGIN:
      result = i18n( "Could not login into %1" ).arg ( _errortext );
      break;
    case  ERR_COULD_NOT_AUTHENTICATE:
      result = i18n( "Athorization failed, %1 authentication not supported" ).arg( _errortext );
      break;
    case  ERR_WARNING:
      result = i18n( "Warning: %1" ).arg( _errortext );
      break;
    case  ERR_CHECKSUM_MISMATCH:
      if (_errortext)
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg(_errortext);
      else
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg("document");
      break;
    default:
      result = i18n( "Unknown error code %1\n%2\n\nPlease send a full bugreport at http://bugs.kde.org" ).arg(_errid ).arg(_errortext);
      break;
    }

  return result;
}

/**********************************************************
 *
 * KIOConnectionSignals
 *
 **********************************************************/

KIOConnectionSignals::KIOConnectionSignals( KIOConnection *_conn )
{
  m_pConnection = _conn;
}

void KIOConnectionSignals::setConnection( KIOConnection *_conn )
{
  m_pConnection = _conn;
}

bool KIOConnectionSignals::testDir( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;

  m_pConnection->send( CMD_TESTDIR, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::get( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( CMD_GET, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::getSize( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;

  m_pConnection->send( CMD_GET_SIZE, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::put( const char *_url, int _mode, bool _overwrite, bool _resume, int _size )
{
  assert( m_pConnection );

  size_t l = strlen( _url );
  if ( l >= (m_pConnection->bufferSize()-15) )
    return false;

  sprintf( m_pConnection->buffer(), "%c%c%8x_%8x_", '0' + (char)_overwrite,
	   '0' + (char)_resume, _mode, _size );
  strcpy( m_pConnection->buffer() + 20, _url );
    
  m_pConnection->send( CMD_PUT, m_pConnection->buffer(), 20 + l + 1 );
  return true;
}

bool KIOConnectionSignals::copy( QStringList& _source, const char *_dest )
{
  assert( m_pConnection );
  
  size_t l = strlen( _dest );
  if ( l >= m_pConnection->bufferSize() )
    return false;

  QStringList::Iterator it = _source.begin();
  for( ; it != _source.end(); ++it )
    if ( !source( (*it) ) )
      return false;
  
  m_pConnection->send( CMD_MCOPY, _dest, l + 1 );
  return true;
}

bool KIOConnectionSignals::copy( const char *_source, const char *_dest )
{
  assert( m_pConnection );

  size_t l1 = strlen( _source );
  size_t l2 = strlen( _dest );
  if ( l1 + 1 + l2 + 1 >= m_pConnection->bufferSize() )
    return false;
  
  memcpy( m_pConnection->buffer(), _source, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 1, _dest, l2 + 1 );
  
  m_pConnection->send( CMD_COPY, m_pConnection->buffer(), l1 + 1 + l2 + 1 );
  return true;
}

bool KIOConnectionSignals::source( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( CMD_SOURCE, _url, l + 1 );
  return true;
}

bool KIOConnectionSignals::move( QStringList& _source, const char *_dest )
{
  assert( m_pConnection );

  size_t l = strlen( _dest );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  QStringList::Iterator it = _source.begin();
  for( ; it != _source.end(); ++it )
    if ( !source( (*it) ) )
      return false;
  
  m_pConnection->send( CMD_MMOVE, _dest, l + 1 );

  return true;
}

bool KIOConnectionSignals::move( const char *_source, const char *_dest )
{
  assert( m_pConnection );

  size_t l1 = strlen( _source );
  size_t l2 = strlen( _dest );
  if ( l1 + 1 + l2 + 1 >= m_pConnection->bufferSize() )
    return false;
  
  memcpy( m_pConnection->buffer(), _source, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 1, _dest, l2 + 1 );
  
  m_pConnection->send( CMD_MOVE, m_pConnection->buffer(), l1 + 1 + l2 + 1 );

  return true;
}

bool KIOConnectionSignals::del( QStringList& _source )
{
  assert( m_pConnection );

  QStringList::Iterator it = _source.begin();
  for( ; it != _source.end(); ++it )
    if ( !source( (*it) ) )
      return false;
  
  m_pConnection->send( CMD_MDEL, "", 0 );

  return true;
}

bool KIOConnectionSignals::del( const char *_url )
{
  assert( m_pConnection );

  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( CMD_DEL, static_cast<const char*>(_url), l + 1 );

  return true;
}

bool KIOConnectionSignals::listDir( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( CMD_LIST, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::mkdir( const char *_url, int _mode )
{
  assert( m_pConnection );

  size_t l = strlen( _url );
  if ( l >= (m_pConnection->bufferSize()-15) )
    return false;
  
  sprintf( m_pConnection->buffer(), "%8x_", _mode );
  strcpy( m_pConnection->buffer() + 9, _url );
  
  m_pConnection->send( CMD_MKDIR, m_pConnection->buffer(), 9 + l + 1 );
  return true;
}

bool KIOConnectionSignals::data( const void *_buffer, int _len )
{
  assert( m_pConnection );

  m_pConnection->send( MSG_DATA, _buffer, _len );
  return true;
}

bool KIOConnectionSignals::dataEnd()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_DATA_END, 0L, 0 );
  return true;
}

bool KIOConnectionSignals::error( int _errid, const char *_text )
{
  assert( m_pConnection );
  int l = strlen( _text );
  sprintf( m_pConnection->buffer(), "%4x_", _errid );
  strcpy( m_pConnection->buffer() + 5, _text );
  m_pConnection->send( MSG_ERROR, m_pConnection->buffer(), 5 + l + 1 );
  return true;
}

bool KIOConnectionSignals::ready()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_READY, 0L, 0 );
  return true;
}

bool KIOConnectionSignals::finished()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_FINISHED, 0L, 0 );
  return true;
}

bool KIOConnectionSignals::renamed( const char *_new )
{
  assert( m_pConnection );
  
  size_t l = strlen( _new );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( MSG_RENAMED, _new, l + 1 );
  return true;
}

bool KIOConnectionSignals::canResume( bool _resume )
{
  assert( m_pConnection );

  m_pConnection->buffer()[0] = '0' + (char)_resume;

  m_pConnection->send( MSG_RESUME, m_pConnection->buffer(), 1 );
  return true;
}

bool KIOConnectionSignals::totalSize( unsigned long _bytes )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes );
  m_pConnection->send( INF_TOTAL_SIZE, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::totalFiles( unsigned long _files )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_files );
  m_pConnection->send( INF_TOTAL_COUNT_OF_FILES, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::totalDirs( unsigned long _dirs )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_dirs );
  m_pConnection->send( INF_TOTAL_COUNT_OF_DIRS, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::processedSize( unsigned long _bytes )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes );
  m_pConnection->send( INF_PROCESSED_SIZE, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::processedFiles( unsigned long _files )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_files );
  m_pConnection->send( INF_PROCESSED_COUNT_OF_FILES, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::processedDirs( unsigned long _dirs )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_dirs );
  m_pConnection->send( INF_PROCESSED_COUNT_OF_DIRS, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::scanningDir( const char *_dir )
{
  assert( m_pConnection );
  
  size_t l = strlen( _dir );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_SCANNING_DIR, _dir, l + 1 );
  return true;
}

bool KIOConnectionSignals::speed( unsigned long _bytes_per_second )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes_per_second );
  m_pConnection->send( INF_SPEED, m_pConnection->buffer(), 9 );
  return true;
}

bool KIOConnectionSignals::copyingFile( const char *_from, const char *_to )
{
  assert( m_pConnection );

  size_t l1 = strlen( _from );
  size_t l2 = strlen( _to );
  if ( l1 + 1 + l2 + 1 >= m_pConnection->bufferSize() )
    return false;
  
  memcpy( m_pConnection->buffer(), _from, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 1, _to, l2 + 1 );
  
  m_pConnection->send( INF_COPYING_FILE, m_pConnection->buffer(), l1 + 1 + l2 + 1 );
  return true;
}

bool KIOConnectionSignals::makingDir( const char *_dir )
{
  assert( m_pConnection );
  
  size_t l = strlen( _dir );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_MAKING_DIR, _dir, l + 1 );
  return true;
}

bool KIOConnectionSignals::gettingFile( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_GETTING_FILE, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::deletingFile( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_DELETING_FILE, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::redirection( const char *_url )
{
  assert( m_pConnection );
  
  size_t l = strlen( _url );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_REDIRECTION, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool KIOConnectionSignals::errorPage()
{
  assert( m_pConnection );
  m_pConnection->send( INF_ERROR_PAGE, 0L, 0 );
  return true;
}

bool KIOConnectionSignals::mimeType( const char *_type )
{
  assert( m_pConnection );
  
  size_t l = strlen( _type );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( INF_MIME_TYPE, static_cast<const char*>(_type), l + 1 );
  return true;
}

bool KIOConnectionSignals::unmount( const char *_point )
{
  assert( m_pConnection );
  
  size_t l = strlen( _point );
  if ( l >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->send( CMD_UNMOUNT, _point, l + 1 );
  return true;
}

bool KIOConnectionSignals::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  assert( m_pConnection );

  size_t l1 = 0, l2 = 0, l3 = 0;
  if ( _fstype )
    l1 = strlen( _fstype );
  else
    _fstype = "";
  if ( _dev )
    l2 = strlen( _dev );
  else
    _dev = "";
  if ( _point )
    l3 = strlen( _point );
  else
    _point = "";
  if ( l1 + 1 + l2 + 1 + l3 + 1 >= m_pConnection->bufferSize() )
    return false;
  
  m_pConnection->buffer()[0] = '0' + (char)_ro;

  memcpy( m_pConnection->buffer() + 1, _fstype, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 2, _dev, l2 + 1 );
  memcpy( m_pConnection->buffer() + l1 + l2 + 3, _dev, l3 + 1 );
  
  m_pConnection->send( CMD_MOUNT, m_pConnection->buffer(), 1 + l1 + l2 + l3 + 3 );
  return true;
}

bool KIOConnectionSignals::listEntry( KUDSEntry& _entry )
{
  int size = 0;
  char *p = m_pConnection->buffer();
  sprintf( p, "%8x_", _entry.count() );
  size += 9;
  KUDSEntry::Iterator it = _entry.begin();
  for( ; it != _entry.end(); it++ ) {
    // char *x = p + size;

    sprintf( p + size, "%8x_", (int)((*it).m_uds));
    size += 9;
    if ( (*it).m_uds & UDS_LONG ) {
      sprintf( p + size, "%8x_", (int)((*it).m_long) );
      size += 9;
    } else if ( (*it).m_uds & UDS_STRING )  {
      strcpy( p + size, (*it).m_str.ascii() );
      size += (*it).m_str.length();
      p[ size++ ] = 0;
    }
    else
      assert( 0 );

    // debug( "Packed %s", (const char*)x );
  }

  m_pConnection->send( MSG_LIST_ENTRY, m_pConnection->buffer(), size + 1 );
  return true;  
}

bool KIOConnectionSignals::isDirectory()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_IS_DIRECTORY, 0L, 0 );
  return true;
}

bool KIOConnectionSignals::isFile()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_IS_FILE, 0L, 0 );
  return true;
}

/**********************************************************
 *
 * KIOConnectionSlots
 *
 **********************************************************/

KIOConnectionSlots::KIOConnectionSlots( KIOConnection *_conn )
{
  m_pConnection = _conn;
}

void KIOConnectionSlots::setConnection( KIOConnection *_conn )
{
  m_pConnection = _conn;
}

void KIOConnectionSlots::dispatchLoop()
{
  while( dispatch() );
}

bool KIOConnectionSlots::dispatch()
{
  assert( m_pConnection );

  int cmd;
  int len = 0;
  char *p = static_cast<char*>( m_pConnection->read( &cmd, &len ) );
  if ( !p )
    return false;
  
  dispatch( (KIOCommand) cmd, p, len );
  return true;
}

void KIOConnectionSlots::dispatch( KIOCommand _cmd, void *_p, int _len )
{
  switch( _cmd )
    {
    case CMD_SOURCE:
      m_lstSource.append( (const char*)(_p) );
      break;
    case CMD_UNMOUNT:
      slotUnmount( (const char*)(_p) );
      break;
    case CMD_MOUNT:
      {
	bool ro = false;
	if ( *((char*)_p) == '1' )
	  ro = true;
	const char* fstype = (const char*)_p + 1;
	const char* dev = (const char*)_p + 1 + strlen( fstype ) + 1;
	const char* point = (const char*)_p + 1 + strlen( fstype ) + 1 + strlen( dev ) + 1;
	kdebug( KDEBUG_INFO, 7006 ,"!!!!!!!!! MOUNTING %s %s %s", fstype, dev, point);
	slotMount( ro, fstype, dev, point );
      }
      break;
    case CMD_MCOPY:
      {	
	const char* arg = (const char*)_p;
	slotCopy( m_lstSource, arg );
	m_lstSource.clear();
      }
      break;
    case CMD_COPY:
      {	
	const char* arg1 = (const char*)_p;
	const char* arg2 = (const char*)_p + strlen( arg1 ) + 1;
	slotCopy( arg1, arg2 );
      }
      break;
    case CMD_MMOVE:
      {	
	const char* arg = (const char*)_p;
	slotMove( m_lstSource, arg );
	m_lstSource.clear();
      }
      break;
    case CMD_MOVE:
      {	
	const char* arg1 = (const char*)_p;
	const char* arg2 = (const char*)_p + strlen( arg1 ) + 1;
	slotMove( arg1, arg2 );
      }
      break;
    case CMD_MDEL:
      {	
	slotDel( m_lstSource );
	m_lstSource.clear();
      }
      break;
    case CMD_DEL:
      {	
	slotDel( (const char*)(_p) );
      }
      break;
    case CMD_PUT:
      {
	char *p = (char*)_p + 2;
	while( *p == ' ' ) p++;
 	int mode = strtol( p, 0L, 16 );

  	p = (char*)_p + 11;
  	int size = strtol( p, 0L, 16 );

	bool overwrite = false;
	if ( *((char*)_p) == '1' )
	  overwrite = true;

	bool resume = false;
	p = (char*)_p + 1;
	if ( *p == '1' )
	  resume = true;

	slotPut( (const char*)(_p) + 20, mode, overwrite, resume, size);
      }
      break;
    case CMD_GET:
      slotGet( (const char*)(_p) );
      break;
    case CMD_GET_SIZE:
      slotGetSize( (const char*)(_p) );
      break;
    case CMD_TESTDIR:
      slotTestDir( (const char*)(_p) );
      break;
    case CMD_LIST:
      slotListDir( (const char*)(_p) );
      break;
    case CMD_MKDIR:
      {
	char *p = (char*)_p;
	while( *p == ' ' ) p++;
	int mode = strtol( p, 0L, 16 );
	slotMkdir( (const char*)(_p) + 9, mode );
      }
      break;
    case MSG_IS_DIRECTORY:
      slotIsDirectory();
      break;
    case MSG_IS_FILE:
      slotIsFile();
      break;
    case MSG_DATA:
      slotData( _p, _len );
      break;
    case MSG_DATA_END:
      slotDataEnd();
      break;
    case MSG_READY:
      slotReady();
      break;
    case MSG_FINISHED:
      slotFinished();
      break;
    case MSG_RENAMED:
      slotRenamed( ( const char*)_p );
      break;
    case MSG_RESUME:
      {
	bool resume = false;
	if ( *((char*)_p) == '1' )
	  resume = true;
	slotCanResume( resume );
      }
      break;
    case MSG_ERROR:
      {	
	char *p = (char*)_p;
	while( *p == ' ' ) p++;
	int id = strtol( p, 0L, 16 );
	slotError( id, (const char*)_p + 5 );
      }
      break;
    case MSG_LIST_ENTRY:
      {
	KUDSEntry entry;
	int size = 9;
	char *p = (char*)_p;
	while( *p == ' ' ) p++;
	int count = strtol( p, 0L, 16 );
	for( int i = 0; i < count; i++ )
	{
	  KUDSAtom atom;
	  char *p = (char*)_p + size;
	  while( *p == ' ' ) p++;
	  atom.m_uds = strtoul( p, 0L, 16 );
	  size += 9;
	  
	  if ( atom.m_uds & UDS_LONG )
	  {
	    char *p = (char*)_p + size;
	    while( *p == ' ' ) p++;
	    atom.m_long = strtol( p, 0L, 16 );
	    size += 9;
	  }
	  else if ( atom.m_uds & UDS_STRING )
	  {
	    char *p = (char*)_p + size;
	    atom.m_str = p;
	    size += atom.m_str.length() + 1;
	  }
	  else
	    assert( 0 );

	  entry.append( atom );
	}
	slotListEntry( entry );
      }
      break;
    case INF_TOTAL_SIZE:
      slotTotalSize( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_TOTAL_COUNT_OF_FILES:
      slotTotalFiles( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_TOTAL_COUNT_OF_DIRS:
      slotTotalDirs( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_PROCESSED_SIZE:
      slotProcessedSize( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_PROCESSED_COUNT_OF_FILES:
      slotProcessedFiles( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_PROCESSED_COUNT_OF_DIRS:
      slotProcessedDirs( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_SCANNING_DIR:
      slotScanningDir( (const char*)_p );
      break;
    case INF_SPEED:
      slotSpeed( strtoul( (char*)_p, 0L, 16 ) );
      break;
    case INF_COPYING_FILE:
      {	
	const char* arg1 = (const char*)_p;
	const char* arg2 = (const char*)_p + strlen( arg1 ) + 1;
	slotCopyingFile( arg1, arg2 );
      }
      break;
    case INF_MAKING_DIR:
      slotMakingDir( ( const char*)_p );
      break;
    case INF_GETTING_FILE:
      slotGettingFile( ( const char*)_p );
      break;
    case INF_DELETING_FILE:
      slotDeletingFile( ( const char*)_p );
      break;
    case INF_ERROR_PAGE:
      slotErrorPage();
      break;
    case INF_REDIRECTION:
      slotRedirection( ( const char*)_p );
      break;
    case INF_MIME_TYPE:
      slotMimeType( ( const char*)_p );
      break;
    default:
      assert( 0 );
    }
}

void KIOConnectionSlots::slotDel (const char *_url)
{
  QStringList lst;
  lst.append( _url );

  slotDel( lst );
}


/**********************************************************
 *
 * KIOProtocol
 *
 **********************************************************/

KIOProtocol::KIOProtocol( KIOConnection *_conn ) : KIOConnectionSignals( _conn ), KIOConnectionSlots( _conn )
{
}

void KIOProtocol::setConnection( KIOConnection* _conn )
{
  KIOConnectionSignals::setConnection( _conn );
  KIOConnectionSlots::setConnection( _conn );
}

void KIOProtocol::sigsegv_handler (int)
{
  // Debug and printf should be avoided because they might
  // call malloc.. and get in a nice recursive malloc loop
  write(2, "kioslave : ###############SEG FAULT#############\n", 49);
  exit(1);
}

void KIOProtocol::sigchld_handler (int)
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
