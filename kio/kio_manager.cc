#include "kio_manager.h"

#include <k2config.h>

#include <string.h>
#include <assert.h>

#include <kapp.h>

ProtocolManager* ProtocolManager::s_pManager = 0L;

ProtocolManager::ProtocolManager()
{
  assert( !s_pManager );
  
  s_pManager = this;
  m_pConfig = 0L;
}

void ProtocolManager::init()
{
  if ( m_pConfig )
    return;
  
  string file = KApplication::kde_datadir().data();
  file += "/kio/kio.kfg";
  
  m_pConfig = new K2Config( file.c_str() );
}

bool ProtocolManager::supportsListing( const char *_protocol )
{
  list<string> lst;
  return ( listing( _protocol, lst ) );
}

bool ProtocolManager::listing( const char *_protocol, list<string>& _listing )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }
  
  return g->readStringList( "listing", _listing );
}

bool ProtocolManager::supportsReading( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  bool b;

  if ( g->readBool( "reading" , b ) )
    return b;

  return false;
}

bool ProtocolManager::supportsWriting( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }
  
  bool b;

  if ( g->readBool( "writing" , b ) )
    return b;

  return false;
}

bool ProtocolManager::supportsMakeDir( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  bool b;
  if ( g->readBool( "makedir" , b ) )
    return b;

  return false;
}

bool ProtocolManager::supportsDeleting( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }
  
  bool b;

  if ( g->readBool( "deleting" , b ) )
    return b;

  return false;
}

bool ProtocolManager::supportsLinking( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }
  
  bool b;

  if ( g->readBool( "linking" , b ) )
    return b;

  return false;
}

bool ProtocolManager::supportsMoving( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    cerr << "Protocol " << _protocol << " not found" << endl;
    return false;
  }
  
  bool b;

  if ( g->readBool( "moving" , b ) )
    return b;

  return false;
}

string ProtocolManager::find( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );

  string exec;
  if ( g != 0L && g->readString( "exec", exec ) )
  {
    // We dont have KApplication here
    string file = KApplication::kde_bindir().data();
    file += "/";
    file += exec;
    return file;
  }
  
  return exec;
}

K2Config* ProtocolManager::findIntern( const char *_protocol )
{
  assert( s_pManager );
  
  init();

  K2Config::iterator it = m_pConfig->begin();
  for( ; it != m_pConfig->end(); it++ )
  {
    K2Config *g;
    if ( ( g = it.group() ) && strcmp( g->name(), _protocol ) == 0L )
      return g;
  }
  
  return 0L;
}

ProtocolManager::Type ProtocolManager::inputType( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
    return T_ERROR;
  
  string exec;
  if ( g != 0L && g->readString( "input", exec ) )
  {
    if ( exec == "stream" )
      return T_STREAM;
    else if ( exec == "filesystem" )
      return T_FILESYSTEM;
    else
      return T_NONE;
  }
  
  return T_NONE;
}

ProtocolManager::Type ProtocolManager::outputType( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
    return T_ERROR;
  
  string exec;
  if ( g != 0L && g->readString( "output", exec ) )
  {
    if ( exec == "stream" )
      return T_STREAM;
    else if ( exec == "filesystem" )
      return T_FILESYSTEM;
    else
      return T_NONE;
  }
  
  return T_NONE;
}


int
ProtocolManager::getReadTimeout() {
  K2Config *g = findIntern( "common" );

  int read_timeout = 5; // 5 minutes

  if ( g )
    g->readLong( "readTimeout", read_timeout );


  return read_timeout;
}


int
ProtocolManager::getReadTimeoutNoResume() {
  K2Config *g = findIntern( "common" );

  int read_timeout_no_resume = 15;  // 15 minutes

  if ( g )
    g->readLong( "readTimeoutNoResume", read_timeout_no_resume );

  return read_timeout_no_resume;
}


bool
ProtocolManager::getMarkPartial() {
  K2Config *g = findIntern( "common" );

  bool mark_partial = true;

  if ( g )
    g->readBool( "markPartial", mark_partial );

  return mark_partial;
}


int
ProtocolManager::getMinimumKeepSize() {
  K2Config *g = findIntern( "common" );

  int minimum_keep_size = 5000;  // 5000 bytes

  if ( g )
    g->readLong( "minimumKeepSize", minimum_keep_size );

  return minimum_keep_size;
}


bool
ProtocolManager::getAutoResume() {
  K2Config *g = findIntern( "common" );

  bool automatic_resume = false;

  if ( g )
    g->readBool( "autoResume", automatic_resume );

  return automatic_resume;
}


void ProtocolManager::setReadTimeout( int _timeout ) {
  K2Config *g = findIntern( "common" );

  if ( g )
    g->writeLong( "readTimeout", _timeout );

  m_pConfig->save();
}


void ProtocolManager::setReadTimeoutNoResume( int _timeout ) {
  K2Config *g = findIntern( "common" );

  if ( g )
    g->writeLong( "readTimeoutNoResume", _timeout );

  m_pConfig->save();
}


void ProtocolManager::setMarkPartial( bool _mode ) {
  K2Config *g = findIntern( "common" );

  if ( g )
    g->writeBool( "markPartial", _mode );

  m_pConfig->save();
}


void ProtocolManager::setMinimumKeepSize( int _size ) {
  K2Config *g = findIntern( "common" );

  if ( g )
    g->writeLong( "minimumKeepSize", _size );

  m_pConfig->save();
}


void ProtocolManager::setAutoResume( bool _mode ) {
  K2Config *g = findIntern( "common" );

  if ( g )
    g->writeBool( "autoResume", _mode );

  m_pConfig->save();
}
