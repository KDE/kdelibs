#include "kmimetypes.h"
#include "kmimemagic.h"
#include "kservices.h"
#include "krun.h"
#include "kautomount.h"
#include "kio_job.h"
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <qstring.h>
#include <qmessagebox.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

QDict<KMimeType>* KMimeType::s_mapMimeTypes = 0L;
KMimeType* KMimeType::s_pDefaultType = 0L;
bool KMimeType::s_bChecked = false;

void KMimeType::initStatic()
{
  if ( s_mapMimeTypes != 0L )
    return;
  
  s_mapMimeTypes = new QDict<KMimeType>;
}

void KMimeType::check()
{
  if ( s_bChecked )
    return;
  initStatic();

  kdebug( KDEBUG_INFO, 7009, "================== %d MTs ==========", s_mapMimeTypes->count() );

  s_bChecked = true; // must be done before building mimetypes
  
  // Try to find the default type
  if ( ( s_pDefaultType = KMimeType::mimeType( "application/octet-stream" ) ) == 0L )
    errorMissingMimeType( "application/octet-stream" );

  // No default type ?
  if ( s_pDefaultType == 0L )
  {
    QStringList tmp;
    s_pDefaultType = new KMimeType( "application/octet-stream", "unknown.xpm", "", tmp );
  }
  
  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( s_mapMimeTypes->count() <= 1 )
    QMessageBox::critical( 0, i18n( "KFM Error" ), i18n( "No mime types installed!" ), i18n( "OK" ) );
	
  if ( KMimeType::mimeType( "inode/directory" ) == s_pDefaultType )
    errorMissingMimeType( "inode/directory" );
  if ( KMimeType::mimeType( "inode/directory-locked" ) == s_pDefaultType )
    errorMissingMimeType( "inode/directory-locked" );
  if ( KMimeType::mimeType( "inode/blockdevice" ) == s_pDefaultType )
    errorMissingMimeType( "inode/blockdevice" );
  if ( KMimeType::mimeType( "inode/chardevice" ) == s_pDefaultType )
    errorMissingMimeType( "inode/chardevice" );
  if ( KMimeType::mimeType( "inode/socket" ) == s_pDefaultType )
    errorMissingMimeType( "inode/socket" );
  if ( KMimeType::mimeType( "inode/fifo" ) == s_pDefaultType )
    errorMissingMimeType( "inode/fifo" );
  if ( KMimeType::mimeType( "application/x-shellscript" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-shellscript" );
  if ( KMimeType::mimeType( "application/x-executable" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-executable" );
  if ( KMimeType::mimeType( "application/x-desktop" ) == s_pDefaultType )
    errorMissingMimeType( "application/x-desktop" );
}

void KMimeType::errorMissingMimeType( const QString& _type )
{
  QString tmp = i18n( "Could not find mime type\n%1" ).arg( _type );
    
  QMessageBox::critical( 0, i18n( "KFM Error" ), tmp, i18n("OK" ) );

  QStringList dummy;
  
  KMimeType *e;
  if ( _type == "inode/directory" )
    e = new KFolderType( _type, "unknown.xpm", "", dummy );
  else if ( _type == "application/x-desktop" )
    e = new KDEDesktopMimeType( _type, "unknown.xpm", "", dummy);
  else if ( _type == "application/x-executable" || _type == "application/x-shellscript" )
    e = new KExecMimeType( _type, "unknown.xpm", "", dummy );
  else
    e = new KMimeType( _type, "unknown.xpm", "", dummy );

  s_mapMimeTypes->insert( _type, e );
}

KMimeType* KMimeType::mimeType( const QString& _name )
{
  check();

  assert( s_mapMimeTypes );

  KMimeType* mime = (*s_mapMimeTypes)[ _name ];
  if ( !mime )
    return s_pDefaultType;

  return mime;
}

KMimeType* KMimeType::findByURL( const KURL& _url, mode_t _mode,
				 bool _is_local_file, bool _fast_mode )
{
  check();

  if ( !_fast_mode && !_is_local_file && _url.isLocalFile() )
    _is_local_file = true;
  
  if ( !_fast_mode && _is_local_file && _mode == 0 )
  {
    struct stat buff;
    if ( stat( _url.path().ascii(), &buff ) != -1 )
      _mode = buff.st_mode;
  }

  // Look at mode_t first
  if ( S_ISDIR( _mode ) )
  {
    // Special hack for local files. We want to see whether we
    // are allowed to enter the directory
    if ( _is_local_file )
    {
      QString path ( _url.path( 0 ) );
      if ( access( path.data(), R_OK ) == -1 )
	return find( "inode/directory-locked" );
    }
    return find( "inode/directory" );
  }
  if ( S_ISCHR( _mode ) )
    return find( "inode/chardevice" );
  if ( S_ISBLK( _mode ) )
    return find( "inode/blockdevice" );
  if ( S_ISFIFO( _mode ) )
    return find( "inode/fifo" );
  if ( S_ISSOCK( _mode ) )
    return find( "inode/socket" );
  // KMimeMagic can do that better for local files
  if ( !_is_local_file && S_ISREG( _mode ) && ( _mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
    return find( "application/x-executable" );
      
  QString path ( _url.path( 0 ) );
  
  // Try to find it out by looking at the filename
  assert( s_mapMimeTypes );
  QDictIterator<KMimeType> it( *s_mapMimeTypes );
  for( ; it.current() != 0L; ++it )
    if ( it.current()->matchFilename( path.data() ) )
      return it.current();
  
  // Another filename binding, hardcoded, is .desktop:
  if ( path.right(8) == ".desktop" )
    return find( "application/x-desktop" );
  // Another filename binding, hardcoded, is .kdelnk;
  // this is preserved for backwards compatibility
  if ( path.right(7) == ".kdelnk" )
    return find( "application/x-desktop" );

  if ( !_is_local_file || _fast_mode )
  {
    QString path = _url.path();
    if ( path.right(1) == "/" || path.isEmpty() )
      return find( "inode/directory" );
  }
  
  // No more chances for non local URLs
  if ( !_is_local_file || _fast_mode )
    return find( "application/octet-stream" );
  
  // Do some magic for local files
  kdebug( KDEBUG_INFO, 7009, "Mime Type finding for '%s'", path.data() );
  KMimeMagicResult* result = KMimeMagic::self()->findFileType( path.ascii() );
  /** DEBUG code **/
  /* assert( result );
  assert( result->mimeType() );
  assert( result->mimeType()[0] != 0 ); */
  /** End Debug code **/

  // If we still did not find it, we must assume the default mime type
  if ( !result || !result->isValid() )  /* !result->mimeType() || result->mimeType()[0] == 0 ) */
    return find( "application/octet-stream" );

  // The mimemagic stuff was successful
  return find( result->mimeType() );
}

KMimeType::KMimeType( const QString& _type, const QString& _icon, const QString& _comment,
		      const QStringList& _patterns )
  : KServiceType( _type, _icon, _comment )
{
  initStatic();
  
  assert( s_mapMimeTypes );
  
  s_mapMimeTypes->insert( _type, this );
  m_lstPatterns = _patterns;
}

KMimeType::KMimeType( KSimpleConfig& _cfg ) : KServiceType( _cfg )
{
  initStatic();
 
  _cfg.setDesktopGroup(); 
  m_lstPatterns = _cfg.readListEntry( "Patterns", ';' );

  if ( isValid() )
  {
    kdebug( KDEBUG_INFO, 7009, "inserting mimetype in map for m_strName = '%s'", m_strName.ascii());
    s_mapMimeTypes->insert( m_strName, this );
  } else
    kdebug( KDEBUG_WARN, 7009, "mimetype not valid '%s' (missing entry in the file ?)", m_strName.ascii());
}

KMimeType::KMimeType( QDataStream& _str ) : KServiceType( _str )
{
  initStatic();
  load( _str );
}

KMimeType::KMimeType() : KServiceType()
{
  initStatic();
}

void KMimeType::load( QDataStream& _str )
{
  if ( !m_strName.isEmpty() )
    s_mapMimeTypes->remove( m_strName );

  KServiceType::load( _str );
  _str >> m_lstPatterns;

  if ( !m_strName.isEmpty() )
    s_mapMimeTypes->insert( m_strName, this );
}

void KMimeType::save( QDataStream& _str ) const
{
  KServiceType::save( _str );
  _str << m_lstPatterns;
}

KServiceType::PropertyPtr KMimeType::property( const QString& _name ) const
{
  QProperty* p = 0;
  
  if ( _name == "Patterns" )
    p = new QProperty( m_lstPatterns );
  
  if ( p )
  {    
    // We are not interested in these
    p->deref();
    return p;
  }

  return KServiceType::property( _name );
}

QStringList KMimeType::propertyNames() const
{
  QStringList res = KServiceType::propertyNames();
  res.append( "Patterns" );
  
  return res;
}

KMimeType::~KMimeType()
{
  s_mapMimeTypes->remove( m_strName );
}

bool KMimeType::matchFilename( const QString& _filename ) const
{
  int len = _filename.length();
  
  QStringList::ConstIterator it = m_lstPatterns.begin();
  for( ; it != m_lstPatterns.end(); ++it )
  {
    const char* s = it->ascii();
    int pattern_len = it->length();
    if (!pattern_len)
      continue;

    if ( s[ pattern_len - 1 ] == '*' && len + 1 >= pattern_len )
      if ( strncasecmp( _filename.ascii(), s, pattern_len - 1 ) == 0 )
	return true;
    
    if ( s[ 0 ] == '*' && len + 1 >= pattern_len )
      if ( strncasecmp( _filename.ascii() + len - pattern_len + 1, s + 1, pattern_len - 1 ) == 0 )
	return true;
    if ( strcasecmp( _filename.ascii(), s ) == 0 )
      return true;
  }

  return false;
}

/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

KFolderType::KFolderType( const QString& _type, const QString& _icon, const QString& _comment,
			  const QStringList& _patterns )
  : KMimeType( _type, _icon, _comment, _patterns )
{
}

QString KFolderType::icon( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return KFolderType::icon( KURL(_url), _is_local );
}

QString KFolderType::icon( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::icon( _url, _is_local );

  KURL u( _url );
  u.addPath( ".directory" );
  
  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  QString icon = cfg.readEntry( "Icon" );
  QString empty_icon = cfg.readEntry( "EmptyIcon" );

  if ( !empty_icon.isEmpty() )
  {
    bool isempty = false;
    DIR *dp = 0L;
    struct dirent *ep;
    dp = opendir( _url.path().ascii() );
    if ( dp )
    {
      ep=readdir( dp );
      ep=readdir( dp );      // ignore '.' and '..' dirent
      if ( (ep=readdir( dp )) == 0L ) // third file is NULL entry -> empty directory
	isempty = true;
      // if third file is .directory and no fourth file -> empty directory
      if (!isempty && !strcmp(ep->d_name, ".directory"))
        isempty = (readdir(dp) == 0L);
      closedir( dp );
    }
    
    if ( isempty )
      return empty_icon;
  }

  if ( icon.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return icon;
}

QString KFolderType::comment( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return KFolderType::comment( KURL(_url), _is_local );
}

QString KFolderType::comment( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::comment( _url, _is_local );

  KURL u( _url );
  u.addPath( ".directory" );

  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  QString comment = cfg.readEntry( "Comment" );
  if ( comment.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return comment;
}

/*******************************************************
 *
 * KDEDesktopMimeType
 *
 ******************************************************/

KDEDesktopMimeType::KDEDesktopMimeType( const QString& _type, const QString& _icon, const QString& _comment,
				const QStringList& _patterns )
  : KMimeType( _type, _icon, _comment, _patterns )
{
}

QString KDEDesktopMimeType::icon( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  KURL u( _url );
  return icon( u, _is_local );
}

QString KDEDesktopMimeType::icon( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::icon( _url, _is_local );

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString icon = cfg.readEntry( "Icon" );
  QString type = cfg.readEntry( "Type" );

  if ( type == "FSDevice" )
  {
    QString unmount_icon = cfg.readEntry( "UnmountIcon" );
    QString dev = cfg.readEntry( "Dev" );
    if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
    {
      QString mp = KIOJob::findDeviceMountPoint( dev.ascii() );
      // Is the device not mounted ?
      if ( mp.isNull() )
	return unmount_icon;
    }
  }
  
  if ( icon.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return icon;
}

QString KDEDesktopMimeType::comment( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  KURL u( _url );
  return comment( u, _is_local );
}

QString KDEDesktopMimeType::comment( const KURL& _url, bool _is_local ) const
{
  if ( !_is_local )
    return KMimeType::comment( _url, _is_local );

  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString comment = cfg.readEntry( "Comment" );
  if ( comment.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return comment;
}

bool KDEDesktopMimeType::run( const QString& _url, bool _is_local )
{
  // It might be a security problem to run external untrusted desktop
  // entry files
  if ( !_is_local )
    return false;

  KURL u( _url );
  
  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );
  if ( type.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\n"
		       "has no Type=... entry").arg(_url );
    QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
    return false;
  }

  kdebug( KDEBUG_INFO, 7009, "TYPE = %s", type.data() );
  
  if ( type == "FSDevice" )
    return runFSDevice( _url, cfg );
  else if ( type == "Application" )
    return runApplication( _url, cfg );
  else if ( type == "Link" )
    return runLink( _url, cfg );
  else if ( type == "MimeType" )
    return runMimeType( _url, cfg );
  
  
  QString tmp = i18n("The desktop entry of type\n%1\nis unknown").arg( type );
  QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
  
  return false;    
}

bool KDEDesktopMimeType::runFSDevice( const QString& _url, KSimpleConfig &cfg )
{
  QString point = cfg.readEntry( "MountPoint" );
  QString dev = cfg.readEntry( "Dev" );

  if ( dev.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( _url );
    QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
    return false;
  }

  QString mp = KIOJob::findDeviceMountPoint( dev.ascii() );
  // Is the device already mounted ?
  if ( !mp.isNull() )
  {
    QString mp2 = "file:";
    mp2 += mp;
    // Open a new window
    KFileManager::getFileManager()->openFileManagerWindow( mp2.ascii() );
  }
  else
  {    
    QString readonly = cfg.readEntry( "ReadOnly" );
    bool ro = FALSE;
    if ( !readonly.isNull() )
      if ( readonly == "1" )
	ro = true;
		    
    (void) new KAutoMount( ro, 0L, dev.ascii(), 0L );
  }

  return true;
}

bool KDEDesktopMimeType::runApplication( const QString& , KSimpleConfig &cfg )
{
  KService s( cfg, false );
  if ( !s.isValid() )
    // The error message was already displayed, so we can just quit here
    return false;
  
  QStringList empty;
  bool res = KRun::run( s, empty );

  return res;
}

bool KDEDesktopMimeType::runLink( const QString& _url, KSimpleConfig &cfg )
{
  QString url = cfg.readEntry( "URL" );
  if ( url.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type Link but has no URL=... entry").arg( _url );
    QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
    return false;
  }

  (void)new KRun( url );

  return true;
}

bool KDEDesktopMimeType::runMimeType( const QString& , KSimpleConfig & )
{
  // HACK: TODO
  return false;
}

QValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::builtinServices( const KURL& _url )
{
  QValueList<Service> result;
  
  if ( !_url.isLocalFile() )
    return result;
  
  KSimpleConfig cfg( _url.path(), true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );
  if ( type.isEmpty() )
    return result;
  
  if ( type == "FSDevice" )
  {
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( _url.path() );
      QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
    }
    else
    {
      QString mp = KIOJob::findDeviceMountPoint( dev.ascii() );
      // not mounted ?
      if ( mp.isEmpty() )
      {
	Service mount;
	mount.m_strName = i18n("Mount");
	mount.m_type = ST_MOUNT;
	result.append( mount );  
      }
      else
      {
	Service unmount;
	unmount.m_strName = i18n("Unmount");
	unmount.m_type = ST_UNMOUNT;
	result.append( unmount );  
      }
    }
  }

  /* Service props;
  props.m_strName = i18n("Properties");
  props.m_type = ST_PROPERTIES;
  _lst.push_back( props );   */
  
  return result;
}

QValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const KURL& _url )
{
  QValueList<Service> result;
  
  if ( !_url.isLocalFile() )
    return result;

  KSimpleConfig cfg( _url.path(), true );
  cfg.setGroup( "Menu" );
  
  QStrList keys;
  if ( cfg.readListEntry( "Menus", keys ) <= 0 )
    return result;
  
  const char *k;
  for( k = keys.first(); k != 0L; k = keys.next() )
  {
    kdebug( KDEBUG_INFO, 7009, "CURRENT KEY = %s", k );
   
    QStrList lst;
    if ( cfg.readListEntry( k, lst ) == 3 )
    {
      Service s;
      s.m_strName = lst.at(0);
      s.m_strIcon = lst.at(1);
      s.m_strExec = lst.at(2);
      s.m_type = ST_USER_DEFINED;
      result.append( s ); 
    }
    else
    {
      QString tmp = i18n("The desktop entry file\n%1\n has an invalid menu entry\n%2").arg( _url.path()).arg( k );
      QMessageBox::critical( 0L, i18n("Error"), tmp, i18n("OK" ) );
    }
  }

  return result;
}

void KDEDesktopMimeType::executeService( const QString& _url, KDEDesktopMimeType::Service& _service )
{
  kdebug( KDEBUG_INFO, 7009, "EXECUTING Service %s", _service.m_strName.data() );
  
  KURL u( _url );
  
  if ( _service.m_type == ST_USER_DEFINED )
  {
    QStringList lst;
    KRun::run( _service.m_strExec, lst, _service.m_strName, _service.m_strIcon,
	       _service.m_strIcon, u.path() );
    return;
  }
  /* else if ( _service.m_type == ST_PROPERTIES )
  {
  } */
  else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT )
  {
    kdebug( KDEBUG_INFO, 7009, "MOUNT&UNMOUNT" );

    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( u.path() );
      QMessageBox::critical( 0L, i18n("Error"), tmp, i18n( "OK" ) );
      return;
    }
    QString mp = KIOJob::findDeviceMountPoint( dev.ascii() );

    if ( _service.m_type == ST_MOUNT )
    {
      // Already mounted? Strange, but who knows ...
      if ( !mp.isEmpty() )
      {
	kdebug( KDEBUG_INFO, 7009, "ALREADY Mounted" );
	return;
      }
      
      QString readonly = cfg.readEntry( "ReadOnly" );
      bool ro = false;
      if ( !readonly.isNull() )
	if ( readonly == "1" )
	  ro = true;

      (void)new KAutoMount( ro, 0L, dev.ascii(), 0L, false );
    }
    else if ( _service.m_type == ST_UNMOUNT )
    {
      // Not mounted? Strange, but who knows ...
      if ( mp.isEmpty() )
	return;

      (void)new KAutoUnmount( mp.ascii() );
    }
  }
  else
    assert( 0 );
}

/*******************************************************
 *
 * KExecMimeType
 *
 ******************************************************/

KExecMimeType::KExecMimeType( const QString& _type, const QString& _icon, const QString& _comment,
			      const QStringList& _patterns )
  : KMimeType( _type, _icon, _comment, _patterns )
{
}

