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
#include <stdlib.h>

#include <kprotocolinfo.h>
#include <kio/global.h>
#include "kmimetype.h"
#include "kservicetypefactory.h"
#include "kmimemagic.h"
#include "kservice.h"
#include "krun.h"
#include "kautomount.h"

#include <qstring.h>
#include <qfile.h>
#include <kmessageboxwrapper.h>

#include <kapp.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kurl.h>

template class KSharedPtr<KMimeType>;
template class QValueList<KMimeType::Ptr>;

KMimeType::Ptr KMimeType::s_pDefaultType = 0L;
bool KMimeType::s_bChecked = false;

void KMimeType::buildDefaultType()
{
  assert ( !s_pDefaultType );
  // Try to find the default type
  KServiceType * mime = KServiceTypeFactory::self()->
	findServiceTypeByName( "application/octet-stream" );

  if (mime && mime->isType( KST_KMimeType ))
  {
      s_pDefaultType = KMimeType::Ptr((KMimeType *) mime);
  }
  else
  {
     errorMissingMimeType( "application/octet-stream" );
     KStandardDirs stdDirs;
     QString sDefaultMimeType = stdDirs.resourceDirs("mime").first()+"application/octet-stream.desktop";
     s_pDefaultType = new KMimeType( sDefaultMimeType, "application/octet-stream",
                                     "unknown", "mime", QStringList() );
  }
}

// Check for essential mimetypes
void KMimeType::checkEssentialMimeTypes()
{
  if ( s_bChecked ) // already done
    return;
  if ( !s_pDefaultType ) // we need a default type first
    buildDefaultType();

  s_bChecked = true; // must be done before building mimetypes

  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( !KServiceTypeFactory::self()->checkMimeTypes() )
  {
    KMessageBoxWrapper::error( 0L, i18n( "No mime types installed!" ) );
    return; // no point in going any further
  }
	
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

  KMessageBoxWrapper::sorry( 0, tmp );
}

KMimeType::Ptr KMimeType::mimeType( const QString& _name )
{
  KServiceType * mime = KServiceTypeFactory::self()->findServiceTypeByName( _name );

  if ( !mime || !mime->isType( KST_KMimeType ) )
  {
    if ( !s_pDefaultType )
      buildDefaultType();
    return s_pDefaultType;
  }

  // We got a mimetype
  return KMimeType::Ptr((KMimeType *) mime);
}

KMimeType::List KMimeType::allMimeTypes()
{
  return KServiceTypeFactory::self()->allMimeTypes();
}

KMimeType::Ptr KMimeType::findByURL( const KURL& _url, mode_t _mode,
				 bool _is_local_file, bool _fast_mode )
{
  checkEssentialMimeTypes();

  if ( !_fast_mode && !_is_local_file && _url.isLocalFile() )
    _is_local_file = true;

  if ( !_fast_mode && _is_local_file && (_mode == 0 || _mode == (mode_t)-1) )
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
      if ( access( QFile::encodeName(path), R_OK ) == -1 )
	return mimeType( "inode/directory-locked" );
    }
    return mimeType( "inode/directory" );
  }
  if ( S_ISCHR( _mode ) )
    return mimeType( "inode/chardevice" );
  if ( S_ISBLK( _mode ) )
    return mimeType( "inode/blockdevice" );
  if ( S_ISFIFO( _mode ) )
    return mimeType( "inode/fifo" );
  if ( S_ISSOCK( _mode ) )
    return mimeType( "inode/socket" );
  // KMimeMagic can do that better for local files
  if ( !_is_local_file && S_ISREG( _mode ) && ( _mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
    return mimeType( "application/x-executable" );

  QString fileName ( _url.fileName() );

  if ( ! fileName.isNull() && _url.path().right(1) != "/" )
    {
      // Try to find it out by looking at the filename
      KMimeType::Ptr mime = KServiceTypeFactory::self()->findFromPattern( fileName );
      if ( mime )
      {
        // Found something - can we trust it ? (e.g. don't trust *.pl over HTTP, could be anything)
        if ( _is_local_file ||
             (KProtocolInfo::mimetypeFastMode( _url.protocol(), mime->name() ) &&
              KProtocolInfo::patternFastMode( _url.protocol(), fileName )) )
          return mime;
      }

      // Another filename binding, hardcoded, is .desktop:
      if ( fileName.right(8) == ".desktop" )
	return mimeType( "application/x-desktop" );
      // Another filename binding, hardcoded, is .kdelnk;
      // this is preserved for backwards compatibility
      if ( fileName.right(7) == ".kdelnk" )
	return mimeType( "application/x-desktop" );
      // .directory files are detected as x-desktop by mimemagic
      // but don't have a Type= entry. Better cheat and say they are text files
      if ( fileName == ".directory" )
	return mimeType( "text/plain" );
    }

  if ( !_is_local_file || _fast_mode )
  {
    QString path = _url.path();
    if ( path.right(1) == "/" || path.isEmpty() )
    {
      // We have no filename at all. Maybe the protocol has a setting for
      // which mimetype this means. For HTTP we set unknown now, because
      // of redirections (e.g. freshmeat downloads).
      // Assume inode/directory otherwise.
      QString def = KProtocolInfo::defaultMimetype( _url.protocol() );
      return mimeType( def.isEmpty() ? QString::fromLatin1("inode/directory") : def );
    }
  }

  // No more chances for non local URLs
  if ( !_is_local_file || _fast_mode )
    return mimeType( "application/octet-stream" );

  // Do some magic for local files
  QString path = _url.path( 0 );
  //kdDebug(7009) << QString("Mime Type finding for '%1'").arg(path) << endl;
  KMimeMagicResult* result = KMimeMagic::self()->findFileType( path );

  // If we still did not find it, we must assume the default mime type
  if ( !result || !result->isValid() )
    return mimeType( "application/octet-stream" );

  // The mimemagic stuff was successful
  return mimeType( result->mimeType() );
}

KMimeType::KMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
                      const QString& _comment, const QStringList& _patterns )
  : KServiceType( _fullpath, _type, _icon, _comment )
{
  m_lstPatterns = _patterns;
}

KMimeType::KMimeType( const QString & _fullpath ) : KServiceType( _fullpath )
{
  KDesktopFile _cfg( _fullpath, true );
  init ( &_cfg );

  if ( !isValid() )
    kdWarning(7009) << "mimetype not valid '" << m_strName << "' (missing entry in the file ?)" << endl;
}

KMimeType::KMimeType( KDesktopFile *config ) : KServiceType( config )
{
  init( config );

  if ( !isValid() )
    kdWarning(7009) << "mimetype not valid '" << m_strName << "' (missing entry in the file ?)" << endl;
}

void KMimeType::init( KDesktopFile * config )
{
  config->setDesktopGroup();
  m_lstPatterns = config->readListEntry( "Patterns", ';' );

  // Read the X-KDE-AutoEmbed setting and store it in the properties map
  QString XKDEAutoEmbed = QString::fromLatin1("X-KDE-AutoEmbed");
  if ( config->hasKey( XKDEAutoEmbed ) )
    m_mapProps.insert( XKDEAutoEmbed, QVariant( config->readBoolEntry( XKDEAutoEmbed ), 0 ) );

}

KMimeType::KMimeType( QDataStream& _str, int offset ) : KServiceType( _str, offset )
{
  loadInternal( _str ); // load our specific stuff
}

void KMimeType::load( QDataStream& _str )
{
  KServiceType::load( _str );
  loadInternal( _str );
}

void KMimeType::loadInternal( QDataStream& _str )
{
  // kdDebug(7009) << "KMimeType::load( QDataStream& ) : loading list of patterns" << endl;
  _str >> m_lstPatterns;
}

void KMimeType::save( QDataStream& _str )
{
  KServiceType::save( _str );
  // Warning adding/removing fields here involves a binary incompatible change - update version
  // number in ksycoca.h
  _str << m_lstPatterns;
}

QVariant KMimeType::property( const QString& _name ) const
{
  if ( _name == "Patterns" )
    return QVariant( m_lstPatterns );

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
}

QPixmap KMimeType::pixmap( int _group, int _force_size, int _state,
                           QString * _path ) const
{
  return KGlobal::iconLoader()->loadIcon( icon( QString::null, false ),
	_group, _force_size, _state, _path, false );
}

QPixmap KMimeType::pixmap( const KURL& _url, int _group, int _force_size,
                           int _state, QString * _path ) const
{
  return KGlobal::iconLoader()->loadIcon( icon( _url, _url.isLocalFile() ),
	_group, _force_size, _state, _path, false );
}

QPixmap KMimeType::pixmapForURL( const KURL & _url, mode_t _mode, int _group,
                                 int _force_size, int _state, QString * _path )
{
    KMimeType::Ptr mt = findByURL( _url, _mode, _url.isLocalFile(),
				   false /*HACK*/);
    static QString unknown = QString::fromLatin1("unknown");
    QString i( mt->icon( _url, _url.isLocalFile() ));

    // if we don't find an icon, maybe we can use the one for the protocol
    if ( mt->icon( _url, _url.isLocalFile() ) == unknown )
	i = KProtocolInfo::icon( _url.protocol() );

    return KGlobal::iconLoader()->loadIcon( i, _group, _force_size, _state,
					    _path, false );
}

/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

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

  QString icon;
  if ( QFile::exists( u.path() ) )
  {
    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    icon = cfg.readEntry( "Icon" );
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

  if ( type == "FSDevice" || type == "FSDev") // need to provide FSDev for
                                              // backwards compatibility
  {
    QString unmount_icon = cfg.readEntry( "UnmountIcon" );
    QString dev = cfg.readEntry( "Dev" );
    if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
    {
      QString mp = KIO::findDeviceMountPoint( dev.ascii() );
      // Is the device not mounted ?
      if ( mp.isNull() )
        return unmount_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return icon;
}

QPixmap KDEDesktopMimeType::pixmap( const KURL& _url, int _group, int _force_size,
	                            int _state, QString * _path ) const
{
  QString _icon = icon( _url, _url.isLocalFile() );
  QPixmap pix = KGlobal::iconLoader()->loadIcon( _icon, _group,
	_force_size, _state, _path, false );
  return pix;
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

bool KDEDesktopMimeType::run( const KURL& u, bool _is_local )
{
  // It might be a security problem to run external untrusted desktop
  // entry files
  if ( !_is_local )
    return false;

  KSimpleConfig cfg( u.path(), true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );
  if ( type.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\n"
		       "has no Type=... entry").arg(u.path() );
    KMessageBoxWrapper::error( 0, tmp);
    return false;
  }

  //kdDebug(7009) << "TYPE = " << type.data() << endl;

  if ( type == "FSDevice" )
    return runFSDevice( u, cfg );
  else if ( type == "Application" )
    return runApplication( u, u.path() );
  else if ( type == "Link" )
  {
    cfg.setDollarExpansion( true ); // for URL=file:$HOME (Simon)
    return runLink( u, cfg );
  }
  else if ( type == "MimeType" )
    return runMimeType( u, cfg );


  QString tmp = i18n("The desktop entry of type\n%1\nis unknown").arg( type );
  KMessageBoxWrapper::error( 0, tmp);

  return false;
}

bool KDEDesktopMimeType::runFSDevice( const KURL& _url, const KSimpleConfig &cfg )
{
  QString dev = cfg.readEntry( "Dev" );

  if ( dev.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( _url.path() );
    KMessageBoxWrapper::error( 0, tmp);
    return false;
  }

  QString mp = KIO::findDeviceMountPoint( dev.ascii() );
  // Is the device already mounted ?
  if ( !mp.isNull() )
  {
    KURL mpURL;
    mpURL.setPath( mp );
    // Open a new window
    KRun::runURL( mpURL, QString::fromLatin1("inode/directory") );
  }
  else
  {
    bool ro = cfg.readBoolEntry( "ReadOnly", false );
    QString fstype = cfg.readEntry( "FSType" );
    QString point = cfg.readEntry( "MountPoint" );
    (void) new KAutoMount( ro, fstype, dev, point, _url.path() );
  }

  return true;
}

bool KDEDesktopMimeType::runApplication( const KURL& , const QString & _serviceFile )
{
  KService s( _serviceFile );
  if ( !s.isValid() )
    // The error message was already displayed, so we can just quit here
    return false;

  QString user = s.username();
  QString opts = s.terminalOptions();
  if (opts.isNull())
    opts = "";

  QString cmd = s.exec();
  if (s.substituteUid() && !user.isEmpty())
  {
    if (s.terminal())
      cmd = QString("konsole %1 -e su -- %2 -c \"%3\"").arg(opts).arg(user).arg(cmd);
    else
      cmd = QString("kdesu -u %1 -- %2").arg(user).arg(cmd);
  } else if (s.terminal())
    cmd = QString("konsole %1 -e /bin/sh -- -c \"%2\"").arg(opts).arg(cmd);
  else
  {
    // No special options, do straight run
    KURL::List lst;
    return KRun::run( s, lst );
  }

  KURL::List empty;
  bool res = KRun::run( cmd, empty, s.name(), s.icon(), s.icon(), _serviceFile );

  return res;
}

bool KDEDesktopMimeType::runLink( const KURL& _url, const KSimpleConfig &cfg )
{
  QString url = cfg.readEntry( "URL" );
  if ( url.isEmpty() )
  {
    QString tmp = i18n("The desktop entry file\n%1\nis of type Link but has no URL=... entry").arg( _url.url() );
    KMessageBoxWrapper::error( 0, tmp );
    return false;
  }

  (void)new KRun( url );

  return true;
}

bool KDEDesktopMimeType::runMimeType( const KURL& url , const KSimpleConfig & )
{
  // Hmm, can't really use keditfiletype since we might be looking
  // at the global file, or at a file not in share/mimelnk...
  QCString cmd = "kfmclient openProperties \"";
  cmd += url.path().local8Bit();
  cmd += "\"";
  system(cmd.data());
  return true;
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
      KMessageBoxWrapper::error( 0, tmp);
    }
    else
    {
      QString mp = KIO::findDeviceMountPoint( dev.ascii() );
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
    //kdDebug(7009) << "CURRENT KEY = " << (*it) << endl;

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
        s.m_display = !cfg.readBoolEntry( "NoDisplay" );
	result.append( s );
      }
    }
    else
      bInvalidMenu = true;

    if ( bInvalidMenu )
    {
      QString tmp = i18n("The desktop entry file\n%1\n has an invalid menu entry\n%2").arg( _url.path()).arg( *it );
      KMessageBoxWrapper::error( 0, tmp );
    }
  }

  return result;
}

void KDEDesktopMimeType::executeService( const QString& _url, KDEDesktopMimeType::Service& _service )
{
  //kdDebug(7009) << "EXECUTING Service " << _service.m_strName.data() << endl;

  KURL u( _url );

  if ( _service.m_type == ST_USER_DEFINED )
  {
    KURL::List lst;
    lst.append( u );
    KRun::run( _service.m_strExec, lst, _service.m_strName, _service.m_strIcon,
	       _service.m_strIcon, _url );
    // The action may update the desktop file. Example: eject unmounts (#5129).
    KDirWatch::self()->setFileDirty( _url );
    return;
  }
  else if ( _service.m_type == ST_MOUNT || _service.m_type == ST_UNMOUNT )
  {
    //kdDebug(7009) << "MOUNT&UNMOUNT" << endl;

    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    QString dev = cfg.readEntry( "Dev" );
    if ( dev.isEmpty() )
    {
      QString tmp = i18n("The desktop entry file\n%1\nis of type FSDevice but has no Dev=... entry").arg( u.path() );
      KMessageBoxWrapper::error( 0, tmp );
      return;
    }
    QString mp = KIO::findDeviceMountPoint( dev.ascii() );

    if ( _service.m_type == ST_MOUNT )
    {
      // Already mounted? Strange, but who knows ...
      if ( !mp.isEmpty() )
      {
	kdDebug(7009) << "ALREADY Mounted" << endl;
	return;
      }

      bool ro = cfg.readBoolEntry( "ReadOnly", false );
      QString fstype = cfg.readEntry( "FSType" );
      QString point = cfg.readEntry( "MountPoint" );
      (void)new KAutoMount( ro, fstype, dev, point, u.path(), false );
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

