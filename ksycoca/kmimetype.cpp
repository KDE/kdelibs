/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/
// $Id$

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>

#include "kio_job.h"
#include "kmimetype.h"
#include "kservicetypefactory.h"
#include "kmimemagic.h"
// #include "kservice.h"
//#include "krun.h"
// #include "kautomount.h"

#include <qstring.h>
#include <qmessagebox.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>

// Should we keep that ? probably not
QDict<KMimeType>* KMimeType::s_mapMimeTypes = 0L; 
KMimeType* KMimeType::s_pDefaultType = 0L;
bool KMimeType::s_bChecked = false;

void KMimeType::check()
{
  if ( s_bChecked )
    return;

  kdebug( KDEBUG_INFO, 7009, "==== %d MimeTypes =======", s_mapMimeTypes->count() );

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
    // No KMessageBox here - we don't want kdeui
    QMessageBox::critical( 0, i18n( "No mime types installed!" ), i18n( "Error" ) );
	
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

  // No KMessageBox here - we don't want kdeui
  QMessageBox::critical( 0, tmp, i18n( "Missing mimetype" ) );

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
  KServiceType * mime = KServiceTypeFactory::findServiceTypeByName( _name );
    
  /*
  check();
  assert( s_mapMimeTypes );
  KMimeType* mime = (*s_mapMimeTypes)[ _name ];
  */

  if ( !mime || !mime->isType( KST_KMimeType ) )
    return s_pDefaultType;

  // We got a mimetype
  return (KMimeType *) mime;
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

  if ( ! path.isNull() )
    {
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
    }

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
  assert( s_mapMimeTypes );

  s_mapMimeTypes->insert( _type, this );
  m_lstPatterns = _patterns;
}

KMimeType::KMimeType( KSimpleConfig& _cfg ) : KServiceType( _cfg )
{
  _cfg.setDesktopGroup();
  m_lstPatterns = _cfg.readListEntry( "Patterns", ';' );

  if ( isValid() )
  {
    // kdebug( KDEBUG_INFO, 7009, "inserting mimetype in map for m_strName = '%s'", m_strName.ascii());
    s_mapMimeTypes->insert( m_strName, this );
  } else
    kdebug( KDEBUG_WARN, 7009, "mimetype not valid '%s' (missing entry in the file ?)", m_strName.ascii());
}

KMimeType::KMimeType( QDataStream& _str ) : KServiceType( _str )
{
  load( _str, true ); // load our specific stuff
}

void KMimeType::load( QDataStream& _str, bool _parentLoaded )
{
  /*
  if ( !m_strName.isEmpty() )
    s_mapMimeTypes->remove( m_strName );
  */

  if ( !_parentLoaded )
    KServiceType::load( _str );

  // kdebug(KDEBUG_INFO, 7009, "KMimeType::load( QDataStream& ) : loading list of patterns");
  _str >> m_lstPatterns;

  /*
  if ( !m_strName.isEmpty() )
    s_mapMimeTypes->insert( m_strName, this );
  */
  // kdebug(KDEBUG_INFO, 7009, "KMimeType::load( QDataStream& ) : done");
}

void KMimeType::save( QDataStream& _str ) const
{
  KServiceType::save( _str );
  _str << m_lstPatterns;
}

KServiceType::PropertyPtr KMimeType::property( const QString& _name ) const
{
  QVariant* p = 0;

  if ( _name == "Patterns" )
    p = new QVariant( m_lstPatterns );

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
    const char* s = (*it).ascii();
    int pattern_len = (*it).length();
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
      QString mp = findDeviceMountPoint( dev.ascii() );
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

/*
  This should be left in KIO somehow
  So that ksycoca doesn't require libkio
  This is a configuration cache
  The actual running should be in kio
  (David)
  
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
    KMessageBox::error( 0, tmp);
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
  KMessageBox::error( 0, tmp);

  return false;
}

bool KDEDesktopMimeType::runFSDevice( const QString& _url, KSimpleConfig &cfg )
{
  QString point = cfg.readEntry( "MountPoint" );
  QString dev = cfg.readEntry( "Dev" );

  if ( dev.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( _url );
    KMessageBox::error( 0, tmp);
    return false;
  }

  QString mp = findDeviceMountPoint( dev.ascii() );
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
		
    KURL u( _url );
    (void) new KAutoMount( ro, 0L, dev.ascii(), 0L, u.path() );
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
    KMessageBox::error( 0, tmp );
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
*/

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
      // No KMessageBox here - we don't want kdeui
      QMessageBox::critical( 0, tmp, i18n( "Mimetype Error" ) );
    }
    else
    {
      QString mp = findDeviceMountPoint( dev.ascii() );
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

  return result;
}

QValueList<KDEDesktopMimeType::Service> KDEDesktopMimeType::userDefinedServices( const KURL& _url )
{
  QValueList<Service> result;

  if ( !_url.isLocalFile() )
    return result;

  KSimpleConfig cfg( _url.path(), true );
  
  cfg.setDesktopGroup();

  if ( !cfg.hasKey( "Actions" ) )
    return result;

  QStringList keys = cfg.readListEntry( "Actions", ';' ); //the desktop standard defines ";" as separator!
 
  if ( keys.count() == 0 )
    return result;

  QStringList::ConstIterator it = keys.begin();
  QStringList::ConstIterator end = keys.end();
  for ( ; it != end; ++it )
  {
    kdebug( KDEBUG_INFO, 7009, "CURRENT KEY = %s", (*it).ascii() );

    QString group = *it;
    group.prepend( "Desktop Action " );

    bool bInvalidMenu = false;

    if ( cfg.hasGroup( group ) )
    {
      cfg.setGroup( group );
  
      if ( !cfg.hasKey( "Name" ) || !cfg.hasKey( "Exec" ) )
        bInvalidMenu = true;
      else
      {
        Service s;
        s.m_strName = cfg.readEntry( "Name" );
        s.m_strIcon = cfg.readEntry( "Icon" );
        s.m_strExec = cfg.readEntry( "Exec" );
	s.m_type = ST_USER_DEFINED;
	result.append( s );
      }
    }
    else
      bInvalidMenu = true;

    if ( bInvalidMenu ) 
    {
      QString tmp = i18n("The desktop entry file\n%1\n has an invalid menu entry\n%2").arg( _url.path()).arg( *it );
      // No KMessageBox here - we don't want kdeui
      QMessageBox::critical( 0, tmp, i18n( "Mimetype Error" ) );
    }
  }

  return result;
}

/*
void KDEDesktopMimeType::executeService( const QString& _url, KDEDesktopMimeType::Service& _service )
{
  kdebug( KDEBUG_INFO, 7009, "EXECUTING Service %s", _service.m_strName.data() );

  KURL u( _url );

  if ( _service.m_type == ST_USER_DEFINED )
  {
    QStringList lst;
    lst.append( _url );
    KRun::run( _service.m_strExec, lst, _service.m_strName, _service.m_strIcon,
	       _service.m_strIcon );
    return;
  }
  else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT )
  {
    kdebug( KDEBUG_INFO, 7009, "MOUNT&UNMOUNT" );

    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( u.path() );
      // No KMessageBox here - we don't want kdeui
      QMessageBox::critical( 0, tmp, i18n( "Mimetype Error" ) );
      return;
    }
    QString mp = findDeviceMountPoint( dev.ascii() );

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

      (void)new KAutoMount( ro, 0L, dev.ascii(), 0L, u.path(), false );
    }
    else if ( _service.m_type == ST_UNMOUNT )
    {
      // Not mounted? Strange, but who knows ...
      if ( mp.isEmpty() )
	return;

      (void)new KAutoUnmount( mp, u.path() );
    }
  }
  else
    assert( 0 );
}
*/

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


/*******************************************************
 *
 * Utility function, used by KMimeType and KAutoMount (kio)
 *
 ******************************************************/

QString KMimeType::findDeviceMountPoint( const char *_device, const char *_file )
{
#ifdef __FreeBSD__
  if( !strcmp( "/etc/mtab", _file ) )
  {
    struct statfs *buf;
    long fsno;
    int flags = MNT_WAIT;
	
    fsno = getfsstat( NULL, 0, flags );
    buf = (struct statfs *)malloc(fsno * sizeof( struct statfs ) );
    if( getfsstat(buf, fsno*sizeof( struct statfs ), flags) == -1 )
    {
      free(buf);
      return QString::null;
    }
    else
    {
      int i;
      for( i = 0; i < fsno; i++ )
	if( !strcmp(buf[i].f_mntfromname, _device ) )
	{
	  QString tmpstr((const char *)buf[i].f_mntonname);
	  free(buf);
	  return tmpstr;
	}
    }
  }
    
#endif /* __FreeBSD__ */             

  // Get the real device name, not some link.
  char buffer[1024];
  QString tmp;
    
  struct stat lbuff;
  lstat( _device, &lbuff );

  // Perhaps '_device' is just a link ?
  const char *device2 = _device;
    
  if ( S_ISLNK( lbuff.st_mode ) )
  {
    int n = readlink( _device, buffer, 1022 );
    if ( n > 0 )
    {
      buffer[ n ] = 0;
      if ( buffer[0] == '/' )
	device2 = buffer;
      else
      {
	tmp = "/dev/";
	tmp += buffer;
	device2 = tmp.data();
      }
    }
  }
    
  int len = strlen( _device );
  int len2 = strlen( device2 );
      
  FILE *f;
  f = fopen( _file, "rb" );
  if ( f != 0L )
  {
    char buff[ 1024 ];
    
    while ( !feof( f ) )
    {
      buff[ 0 ] = 0;
      // Read a line
      fgets( buff, 1023, f );
      // Is it the device we are searching for ?
      if ( strncmp( buff, _device, len ) == 0 && ( buff[len] == ' ' || buff[len] == '\t' ) )
      {
	// Skip all spaces
	while( buff[ len ] == ' ' || buff[ len ] == '\t' )
	  len++;
		    
	char *p = strchr( buff + len, ' ' );
	if ( p != 0L )
	{
	  *p = 0;
	  fclose( f );
	  return QString( buff + len );
	}
      }
      else if ( strncmp( buff, device2, len2 ) == 0 && ( buff[len2] == ' ' || buff[len2] == '\t' ) )
      {
	// Skip all spaces
	while( buff[ len2 ] == ' ' || buff[ len2 ] == '\t' )
	  len2++;
	
	char *p = strchr( buff + len2, ' ' );
	if ( p != 0L )
	{
	  *p = 0;
	  fclose( f );
	  return QString( buff + len2 );
	}	      
      }
    }
    
    fclose( f );
  }
  
  return QString();
}
