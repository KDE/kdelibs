#include "kservices.h"
#include "kuserprofile.h"

#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include <qstring.h>
#include <qmsgbox.h>
#include <string>

#include <ksimpleconfig.h>
#include <kapp.h>

QList<KService>* KService::s_lstServices = 0L;

void KService::initStatic()
{
  if ( !s_lstServices )
    s_lstServices = new QList<KService>;

  /* // Read the application bindings in the local directories
  QString path = kapp->localkdedir().data();
  path += "/share/applnk";
  KService::initServices( path.data() );
  
  // Read the application bindings in the global directories
  path = kapp->kde_appsdir().copy();
  KService::initServices( path.data() ); */
}

KService* KService::findByName( const char *_name )
{
  initStatic();
  
  assert( s_lstServices );

  KService *s;
  for( s = s_lstServices->first(); s != 0L; s = s_lstServices->next() )
    if ( strcmp( s->name(), _name ) == 0 )
      return s;
  
  return 0L;
}

void KService::findServiceByServiceType( const char* _servicetype, list<Offer>& _result )
{
  initStatic();
  
  assert( s_lstServices );

  KService *s;
  for( s = s_lstServices->first(); s != 0L; s = s_lstServices->next() )
  {
    if ( s->hasServiceType( _servicetype ) )
    {
      Offer offer;
      offer.m_strServiceType = _servicetype;
      offer.m_pServiceTypeProfile = KServiceTypeProfile::find( _servicetype );
      offer.m_pService = s;
      _result.push_back( offer );
    }
  }
  
  _result.sort();
}

void KService::initServices( const char * _path )
{
  initStatic();
  
  DIR *dp;
  struct dirent *ep;
  dp = opendir( _path );
  if ( dp == 0L )
    return;
    
  // Loop thru all directory entries
  while ( ( ep = readdir( dp ) ) != 0L )
  {
    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 && ep->d_name[0] != '.' )
    {    
      string file = _path;
      file += "/";
      file += ep->d_name;

      struct stat buff;
      stat( file.c_str(), &buff );
      if ( S_ISDIR( buff.st_mode ) )
	initServices( file.c_str() );
      else if ( S_ISREG( buff.st_mode ) )
      {
	// Do we have read access ?
	if ( access( file.c_str(), R_OK ) == 0 )
	{
	  KSimpleConfig config( file.c_str(), true );

	  parseService( file.c_str(), config );	  
	}
      }
    }
  }

  (void) closedir( dp );
}

KService* KService::parseService( const char *_file, KSimpleConfig &config, bool _put_in_list )
{
  initStatic();
  
  config.setGroup( "KDE Desktop Entry" );
  QString exec = config.readEntry( "Exec" );
  QString name = config.readEntry( "Name" );
  if ( name.isEmpty() )
  {
    QString tmp;
    tmp.sprintf( i18n( "The application config file\n%s\ndoes not contain a Name=... entry"), _file );
    QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp, i18n( "Ok" ) );
    return 0L;
  }

  QString path = config.readEntry( "Path" );
  QString terminal = config.readEntry( "TerminalOptions" );
  // An icon for the binary
  QString app_icon = config.readEntry( "Icon", "unknown.xpm" );
	  
  // The pattern to identify the binary
  QString app_pattern = config.readEntry( "BinaryPattern" );
  QString comment = config.readEntry( "Comment" );
  // A ';' separated list of mime types
  QString mime = config.readEntry( "MimeType" );
  // Allow this program to be a default application for a mime type?
  // For example gzip should never be a default for any mime type.
  QString str_allowdefault = config.readEntry( "AllowDefault" );
  bool allowdefault = true;
  if ( str_allowdefault == "0" )
    allowdefault = false;
  
  /*
    // Define an icon for the program file perhaps ?
    if ( !app_icon.isEmpty() && !app_pattern.isEmpty() )
    {
    KMimeType *t;
    types->append( t = new KMimeType( name.data(), app_icon.data() ) );
    t->setComment( comment.data() );
    t->setApplicationPattern();
    int pos2 = 0;
    int old_pos2 = 0;
    while ( ( pos2 = app_pattern.find( ";", pos2 ) ) != - 1 )
    {
    QString pat = app_pattern.mid( old_pos2, pos2 - old_pos2 );
    t->addPattern( pat.data() );
    pos2++;
    old_pos2 = pos2;
    }
    } 
    */
	  
  // To which mime types is the application bound ?
  QStrList types;
  int pos2 = 0;
  int old_pos2 = 0;
  while ( ( pos2 = mime.find( ";", pos2 ) ) != - 1 )
  {
    // 'bind' is the name of a mime type
    QString bind = mime.mid( old_pos2, pos2 - old_pos2 );
    types.append( bind );
    
    pos2++;
    old_pos2 = pos2;
  }
  
  return new KService( name, exec, app_icon, types, comment, allowdefault,
		       path, terminal, _put_in_list );
}

KService::KService( const char *_name, const char *_exec, const char *_icon,
		    const QStrList& _lstServiceTypes, const char *_comment = 0L,
		    bool _allow_as_default = true, const char *_path = 0L,
		    const char *_terminal = 0L, bool _put_in_list )
{
  initStatic();
  
  assert( _name && _exec && _icon && s_lstServices );

  if ( _put_in_list )
    s_lstServices->append( this );
  
  m_strName = _name;
  m_strExec = _exec;
  m_strIcon = _icon;
  if ( _comment )
    m_strComment = _comment;
  else
    m_strComment = "";
  m_lstServiceTypes = _lstServiceTypes;
  if ( _path )
    m_strPath = _path;
  else
    m_strPath = "";
  if ( _terminal )
    m_strTerminalOptions = _terminal;
  else
    m_strTerminalOptions = "";
  m_bAllowAsDefault = _allow_as_default;
}

KService::~KService()
{
  s_lstServices->removeRef( this );
}

bool KService::hasServiceType( const char *_service )
{
  return ( m_lstServiceTypes.find( _service ) != -1 );
}

bool KService::Offer::allowAsDefault()
{
  bool allow = m_pService->allowAsDefault();
  if ( m_pServiceTypeProfile )
    allow = m_pServiceTypeProfile->allowAsDefault( m_pService->name() );

  return allow;
}

int KService::Offer::preference()
{
  if ( m_pServiceTypeProfile )
    return m_pServiceTypeProfile->preference( m_pService->name() );

  return 0;
}

bool KService::Offer::operator< ( KService::Offer& _o )
{
  if ( _o.allowAsDefault() && !allowAsDefault() )
    return true;
  if ( _o.preference() > preference() )
    return true;
  return false;
}

QStrList& KService::serviceTypes()
{
  return m_lstServiceTypes;
}
