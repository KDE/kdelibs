#include "kio_interface.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

/**********************************************************
 *
 * ConnectionSignals
 *
 **********************************************************/

ConnectionSignals::ConnectionSignals( Connection *_conn )
{
  m_pConnection = _conn;
}

void ConnectionSignals::setConnection( Connection *_conn )
{
  m_pConnection = _conn;
}

bool ConnectionSignals::testDir( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( CMD_TESTDIR, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool ConnectionSignals::get( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( CMD_GET, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool ConnectionSignals::put( const char *_url, int _mode, bool _overwrite, bool _resume )
{
  assert( m_pConnection );

  int l = strlen( _url );
  if ( l >= 0xFFF0 )
    return false;
  
  sprintf( m_pConnection->buffer(), "%c%c%8x_", '0' + (char)_overwrite,
	   '0' + (char)_resume, _mode );
  strcpy( m_pConnection->buffer() + 11, _url );
  
  m_pConnection->send( CMD_PUT, m_pConnection->buffer(), 11 + l + 1 );
  return true;
}

bool ConnectionSignals::copy( list<string>& _source, const char *_dest )
{
  assert( m_pConnection );
  
  int l = strlen( _dest );
  if ( l >= 0xFFFF )
    return false;
  
  list<string>::iterator it = _source.begin();
  for( ; it != _source.end(); ++it )
    if ( !source( it->c_str() ) )
      return false;
  
  m_pConnection->send( CMD_COPY, _dest, l + 1 );
  return true;
}

bool ConnectionSignals::copy( const char *_from, const char *_to )
{
  assert( m_pConnection );

  int l1 = strlen( _from );
  int l2 = strlen( _to );
  if ( l1 + 1 + l2 + 1 >= 0xFFFF )
    return false;
  
  memcpy( m_pConnection->buffer(), _from, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 1, _to, l2 + 1 );
  
  m_pConnection->send( CMD_COPY_SINGLE, m_pConnection->buffer(), l1 + 1 + l2 + 1 );
  return true;
}

bool ConnectionSignals::source( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( CMD_SOURCE, _url, l + 1 );
  return true;
}

bool ConnectionSignals::move( const char *_source, const char *_dest )
{
  assert( m_pConnection );
  return true;
}

bool ConnectionSignals::move( list<string>& _source, const char *_dest )
{
  assert( m_pConnection );
  return true;
}

bool ConnectionSignals::del( const char *_url )
{
  assert( m_pConnection );
  return true;
}

bool ConnectionSignals::del( list<string>& _source )
{
  assert( m_pConnection );
  return true;
}

bool ConnectionSignals::listDir( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( CMD_LIST, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool ConnectionSignals::mkdir( const char *_url, int _mode )
{
  assert( m_pConnection );

  int l = strlen( _url );
  if ( l >= 0xFFF0 )
    return false;
  
  sprintf( m_pConnection->buffer(), "%8x_", _mode );
  strcpy( m_pConnection->buffer() + 9, _url );
  
  m_pConnection->send( CMD_MKDIR, m_pConnection->buffer(), 9 + l + 1 );
  return true;
}

bool ConnectionSignals::data( void *_buffer, int _len )
{
  assert( m_pConnection );

  m_pConnection->send( MSG_DATA, _buffer, _len );
  return true;
}

bool ConnectionSignals::dataEnd()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_DATA_END, 0L, 0 );
  return true;
}

bool ConnectionSignals::error( int _errid, const char *_text )
{
  assert( m_pConnection );
  int l = strlen( _text );
  sprintf( m_pConnection->buffer(), "%4x_", _errid );
  strcpy( m_pConnection->buffer() + 5, _text );
  m_pConnection->send( MSG_ERROR, m_pConnection->buffer(), 5 + l + 1 );
  return true;
}

bool ConnectionSignals::ready()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_READY, 0L, 0 );
  return true;
}

bool ConnectionSignals::finished()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_FINISHED, 0L, 0 );
  return true;
}

bool ConnectionSignals::renamed( const char *_new )
{
  assert( m_pConnection );
  
  int l = strlen( _new );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( MSG_RENAMED, _new, l + 1 );
  return true;
}

bool ConnectionSignals::totalSize( unsigned long _bytes )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes );
  m_pConnection->send( INF_TOTAL_SIZE, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::totalFiles( unsigned long _files )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_files );
  m_pConnection->send( INF_TOTAL_COUNT_OF_FILES, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::totalDirs( unsigned long _dirs )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_dirs );
  m_pConnection->send( INF_TOTAL_COUNT_OF_DIRS, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::processedSize( unsigned long _bytes )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes );
  m_pConnection->send( INF_PROCESSED_SIZE, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::processedFiles( unsigned long _files )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_files );
  m_pConnection->send( INF_PROCESSED_COUNT_OF_FILES, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::processedDirs( unsigned long _dirs )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_dirs );
  m_pConnection->send( INF_PROCESSED_COUNT_OF_DIRS, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::scanningDir( const char *_dir )
{
  assert( m_pConnection );
  
  int l = strlen( _dir );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( INF_SCANNING_DIR, _dir, l + 1 );
  return true;
}

bool ConnectionSignals::speed( unsigned long _bytes_per_second )
{
  assert( m_pConnection );
  
  sprintf( m_pConnection->buffer(), "%8x", (int)_bytes_per_second );
  m_pConnection->send( INF_SPEED, m_pConnection->buffer(), 9 );
  return true;
}

bool ConnectionSignals::copyingFile( const char *_from, const char *_to )
{
  assert( m_pConnection );

  int l1 = strlen( _from );
  int l2 = strlen( _to );
  if ( l1 + 1 + l2 + 1 >= 0xFFFF )
    return false;
  
  memcpy( m_pConnection->buffer(), _from, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 1, _to, l2 + 1 );
  
  m_pConnection->send( INF_COPYING_FILE, m_pConnection->buffer(), l1 + 1 + l2 + 1 );
  return true;
}

bool ConnectionSignals::makingDir( const char *_dir )
{
  assert( m_pConnection );
  
  int l = strlen( _dir );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( INF_MAKING_DIR, _dir, l + 1 );
  return true;
}

bool ConnectionSignals::gettingFile( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( INF_GETTING_FILE, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool ConnectionSignals::redirection( const char *_url )
{
  assert( m_pConnection );
  
  int l = strlen( _url );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( INF_REDIRECTION, static_cast<const char*>(_url), l + 1 );
  return true;
}

bool ConnectionSignals::errorPage()
{
  assert( m_pConnection );
  m_pConnection->send( INF_ERROR_PAGE, 0L, 0 );
  return true;
}

bool ConnectionSignals::mimeType( const char *_type )
{
  assert( m_pConnection );
  
  int l = strlen( _type );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( INF_MIME_TYPE, static_cast<const char*>(_type), l + 1 );
  return true;
}

bool ConnectionSignals::unmount( const char *_point )
{
  assert( m_pConnection );
  
  int l = strlen( _point );
  if ( l >= 0xFFFF )
    return false;
  
  m_pConnection->send( CMD_UNMOUNT, _point, l + 1 );
  return true;
}

bool ConnectionSignals::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  assert( m_pConnection );

  int l1 = 0, l2 = 0, l3 = 0;
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
  if ( l1 + 1 + l2 + 1 + l3 + 1 >= 0xFFFF )
    return false;
  
  m_pConnection->buffer()[0] = '0' + (char)_ro;

  memcpy( m_pConnection->buffer() + 1, _fstype, l1 + 1 );
  memcpy( m_pConnection->buffer() + l1 + 2, _dev, l2 + 1 );
  memcpy( m_pConnection->buffer() + l1 + l2 + 3, _dev, l3 + 1 );
  
  m_pConnection->send( CMD_MOUNT, m_pConnection->buffer(), 1 + l1 + l2 + l3 + 3 );
  return true;
}

bool ConnectionSignals::listEntry( UDSEntry& _entry )
{
  int size = 0;
  char *p = m_pConnection->buffer();
  sprintf( p, "%8x_", _entry.size() );
  size += 9;
  UDSEntry::iterator it = _entry.begin();
  for( ; it != _entry.end(); it++ )
  {
    // char *x = p + size;
    
    sprintf( p + size, "%8x_", (int)it->m_uds );
    size += 9;
    if ( it->m_uds & UDS_LONG )
    {
      sprintf( p + size, "%8x_", (int)it->m_long );
      size += 9;
    }
    else if ( it->m_uds & UDS_STRING )
    {
      strcpy( p + size, it->m_str.c_str() );
      size += it->m_str.size();
      p[ size++ ] = 0;
    }
    else
      assert( 0 );

    // cerr << "Packed " << (const char*)x << endl;
  }

  m_pConnection->send( MSG_LIST_ENTRY, m_pConnection->buffer(), size + 1 );
  return true;  
}

bool ConnectionSignals::isDirectory()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_IS_DIRECTORY, 0L, 0 );
  return true;
}

bool ConnectionSignals::isFile()
{
  assert( m_pConnection );

  m_pConnection->send( MSG_IS_FILE, 0L, 0 );
  return true;
}

/**********************************************************
 *
 * ConnectionSlots
 *
 **********************************************************/

ConnectionSlots::ConnectionSlots( Connection *_conn )
{
  m_pConnection = _conn;
}

void ConnectionSlots::setConnection( Connection *_conn )
{
  m_pConnection = _conn;
}

void ConnectionSlots::dispatchLoop()
{
  while( dispatch() );
}

bool ConnectionSlots::dispatch()
{
  assert( m_pConnection );

  int cmd = 0, len = 0;
  char *p = static_cast<char*>( m_pConnection->read( &cmd, &len ) );
  if ( !p )
    return false;
  
  dispatch( cmd, p, len );
  return true;
}

void ConnectionSlots::dispatch( int _cmd, void *_p, int _len )
{
  switch( _cmd )
    {
    case CMD_SOURCE:
      m_lstSource.push_back( (const char*)(_p) );
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
	cerr << "!!!!!!!!! MOUNTING " << fstype << " " << dev << " " << point << endl;
	slotMount( ro, fstype, dev, point );
      }
      break;
    case CMD_COPY:
      {	
	const char* arg = (const char*)_p;
	slotCopy( m_lstSource, arg );
	m_lstSource.clear();
      }
      break;
    case CMD_COPY_SINGLE:
      {	
	const char* arg1 = (const char*)_p;
	const char* arg2 = (const char*)_p + strlen( arg1 ) + 1;
	slotCopy( arg1, arg2 );
      }
      break;
    case CMD_PUT:
      {
	char *p = (char*)_p + 2;
	while( *p == ' ' ) p++;
	int mode = strtol( p, 0L, 16 );
	bool overwrite = false;
	if ( *((char*)_p) == '1' )
	  overwrite = true;
	bool resume = false;
	if ( *((char*)(_p + 1)) == '1' )
	  resume = true;
	slotPut( (const char*)(_p) + 11, mode, overwrite, resume );
      }
      break;
    case CMD_GET:
      slotGet( (const char*)(_p) );
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
	UDSEntry entry;
	int size = 9;
	char *p = (char*)_p;
	while( *p == ' ' ) p++;
	int count = strtol( p, 0L, 16 );
	for( int i = 0; i < count; i++ )
	{
	  UDSAtom atom;
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
	    size += atom.m_str.size() + 1;
	  }
	  else
	    assert( 0 );

	  entry.push_back( atom );
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

/**********************************************************
 *
 * IOProtocol
 *
 **********************************************************/

IOProtocol::IOProtocol( Connection *_conn ) : ConnectionSignals( _conn ), ConnectionSlots( _conn )
{
}

void IOProtocol::setConnection( Connection* _conn )
{
  ConnectionSignals::setConnection( _conn );
  ConnectionSlots::setConnection( _conn );
}
