/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                2000, 2007 David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdedesktopmimetype.h"
#include "kmimetype_p.h"
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kmountpoint.h>
#include <dirent.h>
#include <QtCore/QFile>

class KDEDesktopMimeTypePrivate: public KMimeTypePrivate
{
public:
  Q_DECLARE_PUBLIC(KDEDesktopMimeType)
  
  KDEDesktopMimeTypePrivate(KDEDesktopMimeType *q): KMimeTypePrivate(q) {}
};

KDEDesktopMimeType::KDEDesktopMimeType( const QString & fullpath, const QString& type,
                    const QString& comment )
  : KMimeType( *new KDEDesktopMimeTypePrivate(this), fullpath, type, comment )
{
}

KDEDesktopMimeType::KDEDesktopMimeType( QDataStream& str, int offset )
    : KMimeType( *new KDEDesktopMimeTypePrivate(this), str, offset )
{
}

KDEDesktopMimeType::~KDEDesktopMimeType()
{
}

QString KDEDesktopMimeType::icon( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::iconName( _url );

    KDesktopFile cfg( _url.path() );
    const KConfigGroup group = cfg.desktopGroup();
    const QString icon = group.readEntry( "Icon" );
    const QString type = group.readEntry( "Type" );

    // need to check FSDev for backwards compatibility
    if ( type == "FSDevice" || type == "FSDev")
    {
        const QString unmount_icon = group.readEntry( "UnmountIcon" );
        const QString dev = group.readEntry( "Dev" );
        if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
        {
            KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByDevice(dev);
            if (!mountPoint) // not mounted?
                return unmount_icon;
        }
    } else if ( type == "Link" ) {
        const QString emptyIcon = group.readEntry( "EmptyIcon" );
        if ( !emptyIcon.isEmpty() ) {
            const QString u = group.readPathEntry( "URL" );
            const KUrl url( u );
            if ( url.protocol() == "trash" ) {
                // We need to find if the trash is empty, preferrably without using a KIO job.
                // So instead kio_trash leaves an entry in its config file for us.
                KConfig trashConfig( "trashrc", KConfig::OnlyLocal );
                if ( trashConfig.group("Status").readEntry( "Empty", true ) ) {
                    return emptyIcon;
                }
            }
        }
    }

    if ( icon.isEmpty() )
        return KMimeType::iconName( _url );

    return icon;
}

QString KDEDesktopMimeType::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::comment( _url );

    KDesktopFile cfg( _url.path() );
    QString comment = cfg.desktopGroup().readEntry( "Comment" );
    if ( comment.isEmpty() )
        return KMimeType::comment( _url );

    return comment;
}

void KDEDesktopMimeType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }

/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

class KFolderTypePrivate: public KMimeTypePrivate
{
public:
  Q_DECLARE_PUBLIC(KFolderType)
  
  KFolderTypePrivate(KFolderType *q): KMimeTypePrivate(q) {}
};

KFolderType::KFolderType( const QString& fullpath, const QString& name, const QString& comment )
    : KMimeType( *new KFolderTypePrivate(this), fullpath, name, comment )
{
}

KFolderType::KFolderType( QDataStream& str, int offset )
    : KMimeType( *new KFolderTypePrivate(this), str, offset )
{
}

KFolderType::~KFolderType()
{
}

QString KFolderType::icon( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::iconName( _url );

  KUrl u( _url );
  u.addPath( ".directory" );

  QString icon;
  // using KStandardDirs as this one checks for path being
  // a file instead of a directory
  if ( KStandardDirs::exists( u.path() ) )
  {
    KDesktopFile cfg( u.path() );
    KConfigGroup group = cfg.desktopGroup();
    icon = group.readEntry( "Icon" );
    QString empty_icon = group.readEntry( "EmptyIcon" );

    if ( !empty_icon.isEmpty() )
    {
      bool isempty = false;
      DIR *dp = 0L;
      struct dirent *ep;
      dp = opendir( QFile::encodeName(_url.path()) );
      if ( dp )
      {
        QSet<QByteArray> entries;
        // Note that readdir isn't guaranteed to return "." and ".." first (#79826)
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        if ( (ep=readdir( dp )) == 0L ) // third file is NULL entry -> empty directory
          isempty = true;
        else {
          entries.insert( ep->d_name );
          if ( readdir( dp ) == 0 ) { // only three
            // check if we got "." ".." and ".directory"
            isempty = entries.contains( "." ) &&
                      entries.contains( ".." ) &&
                      entries.contains( ".directory" );
          }
        }
        if (!isempty && !strcmp(ep->d_name, ".directory"))
          isempty = (readdir(dp) == 0L);
        closedir( dp );
      }

      if ( isempty )
        return empty_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeType::iconName( _url );

  if ( icon.startsWith( "./" ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KUrl v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.path();
  }

  return icon;
}

QString KFolderType::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeType::comment( _url );

    KUrl u( _url );
    u.addPath( ".directory" );

    const KDesktopFile cfg( u.path() );
    QString comment = cfg.readComment();
    if ( comment.isEmpty() )
        return KMimeType::comment( _url );

    return comment;
}

void KFolderType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }
