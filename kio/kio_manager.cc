#include "kio_manager.h"

#include <k2config.h>

#include <string.h>
#include <assert.h>

#include <kapp.h>
#include <kdebug.h>

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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
    return false;
  }
  
  return g->readStringList( "listing", _listing );
}

bool ProtocolManager::supportsReading( const char *_protocol )
{
  K2Config *g = findIntern( _protocol );
  if ( !g )
  {
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol );
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
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  int read_timeout = config.readNumEntry( "ReadTimeout", 15 ); // 15 seconds
  
  return read_timeout;
}


bool
ProtocolManager::getMarkPartial() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  bool mark_partial = config.readBoolEntry( "MarkPartial", true );

  return mark_partial;
}


int
ProtocolManager::getMinimumKeepSize() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  int minimum_keep_size = config.readNumEntry( "MinimumKeepSize", 5000 ); // 5000 bytes

  return minimum_keep_size;
}


bool
ProtocolManager::getAutoResume() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  bool automatic_resume = config.readBoolEntry( "AutoResume", false );

  return automatic_resume;
}


bool
ProtocolManager::getPersistent() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  bool persistent = config.readBoolEntry( "PersistentConnections", true );

  return persistent;
}


bool
ProtocolManager::getUseProxy() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  bool useproxy = config.readBoolEntry( "UseProxy", false );

  return useproxy;
}


QString
ProtocolManager::getFtpProxy() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  QString ftpproxy = config.readEntry( "FtpProxy" );

  return ftpproxy;
}


QString
ProtocolManager::getHttpProxy() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  QString httpproxy = config.readEntry( "HttpProxy" );

  return httpproxy;
}


QString
ProtocolManager::getNoProxyFor() {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  QString noproxy = config.readEntry( "NoProxyFor" );

  return noproxy;
}


void ProtocolManager::setReadTimeout( int _timeout ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "ReadTimeout", _timeout );

  config.sync();
}


void ProtocolManager::setMarkPartial( bool _mode ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "MarkPartial", _mode );

  config.sync();
}


void ProtocolManager::setMinimumKeepSize( int _size ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "MinimumKeepSize", _size );

  config.sync();
}


void ProtocolManager::setAutoResume( bool _mode ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "AutoResume", _mode );

  config.sync();
}


void ProtocolManager::setPersistent( bool _mode ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "PersistentConnections", _mode );

  config.sync();
}


void ProtocolManager::setUseProxy( bool _mode ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "UseProxy", _mode );

  config.sync();
}


void ProtocolManager::setFtpProxy( const char* _proxy ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "FtpProxy", _proxy );

  config.sync();
}


void ProtocolManager::setHttpProxy( const char* _proxy ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "HttpProxy", _proxy );

  config.sync();
}


void ProtocolManager::setNoProxyFor( const char* _noproxy ) {
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "NoProxyFor", _noproxy );

  config.sync();
}
