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
#include <qtl.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <klocale.h>
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

KService* KService::service( const QString& _name )
{
  initStatic();
  
  assert( s_lstServices );

  KService *s;
  for( s = s_lstServices->first(); s != 0L; s = s_lstServices->next() )
    if ( strcmp( s->name(), _name ) == 0 )
      return s;
  
  return 0L;
}

void KService::initServices( const QString&  _path )
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
    if ( strcmp( ep->d_name, "." ) != 0 &&
	 strcmp( ep->d_name, ".." ) != 0 && ep->d_name[0] != '.' )
    {    
      QString file = _path;
      file += "/";
      file += ep->d_name;

      struct stat buff;
      stat( file, &buff );
      if ( S_ISDIR( buff.st_mode ) )
	initServices( file );
      else if ( S_ISREG( buff.st_mode ) )
      {
	// Do we have read access ?
	if ( access( file, R_OK ) == 0 )
	{
	  KSimpleConfig config( file, true );

	  parseService( file, config );	  
	}
      }
    }
  }

  (void) closedir( dp );
}

KService* KService::parseService( const QString& _file, KSimpleConfig &config,
				  bool _put_in_list )
{
  initStatic();
  
  config.setGroup( "KDE Desktop Entry" );
  QString exec = config.readEntry( "Exec" );
  QString name = config.readEntry( "Name" );
  if ( name.isEmpty() )
  {
    QString tmp = i18n( "The application config file\n"
			"%1\n"
			"does not contain a Name=... entry").arg(_file);
    QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp, i18n( "OK" ) );
    return 0L;
  }
  if ( exec.isEmpty() )
  {
    QString tmp = i18n( "The application config file\n"
			"%1\n"
			"does not contain an Exec=... entry").arg(_file);
    QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp, i18n( "OK" ) );
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
  QStringList types;
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
		       path, terminal, _file, _put_in_list );
}

KService::KService( const QString& _name, const QString& _exec, const QString& _icon,
		    const QStringList& _lstServiceTypes, const QString& _comment,
		    bool _allow_as_default, const QString& _path,
		    const QString& _terminal, const QString& _file,
		    bool _put_in_list )
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
  if ( _file )
    m_strFile = _file;
  else
    m_strFile = "";    
  m_bAllowAsDefault = _allow_as_default;
}

KService::~KService()
{
  s_lstServices->removeRef( this );
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  debug("Testing %s", m_strName.ascii());
  
  QStringList::ConstIterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
    debug("    has %s", it->ascii() );
  
  return ( m_lstServiceTypes.find( _servicetype ) != m_lstServiceTypes.end() );
}

