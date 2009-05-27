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

#include "kfoldermimetype.h"
#include "kmimetype_p.h"
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <kde_file.h>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QDirIterator>

class KFolderMimeTypePrivate : public KMimeTypePrivate
{
public:
    K_SYCOCATYPE( KST_KFolderMimeType, KMimeTypePrivate )

    KFolderMimeTypePrivate(const QString &s)
        : KMimeTypePrivate(s)
    {}

    KFolderMimeTypePrivate(QDataStream& str, int offset)
        : KMimeTypePrivate(str, offset)
    {}

    virtual QString comment(const KUrl &url) const;
    virtual QString iconName(const KUrl &url) const;

};

/*******************************************************
 *
 * KFolderMimeType
 *
 ******************************************************/

KFolderMimeType::KFolderMimeType( const QString& fullpath, const QString& name, const QString& comment )
    : KMimeType(*new KFolderMimeTypePrivate(fullpath), name, comment )
{
}

KFolderMimeType::KFolderMimeType( QDataStream& str, int offset )
    : KMimeType( *new KFolderMimeTypePrivate(str, offset))
{
}

KFolderMimeType::~KFolderMimeType()
{
}

QString KFolderMimeTypePrivate::iconName( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeTypePrivate::iconName( _url );

  // Stating .directory files can cause long freezes when e.g. /home
  // uses autofs for every user's home directory, i.e. opening /home
  // in a file dialog will mount every single home directory.
  // These non-mounted directories can be identified by having 0 size.
  // There are also other directories with 0 size, such as /proc, that may
  // be mounted, but those are unlikely to contain .directory (and checking
  // this would require KMountPoint from kio).
  KDE_struct_stat buff;
  if (KDE_stat( QFile::encodeName( _url.toLocalFile()), &buff ) == 0
      && S_ISDIR( buff.st_mode ) && buff.st_size == 0 ) {
    return KMimeTypePrivate::iconName( _url );
  }

  KUrl u( _url );
  u.addPath( ".directory" );

  QString icon;
  // using KStandardDirs as this one checks for path being
  // a file instead of a directory
  if ( KStandardDirs::exists( u.toLocalFile() ) )
  {
    KDesktopFile cfg( u.toLocalFile() );
    KConfigGroup group = cfg.desktopGroup();
    icon = group.readEntry( "Icon" );
    QString empty_icon = group.readEntry( "EmptyIcon" );

    if ( !empty_icon.isEmpty() )
    {
      bool isempty = true;
      QDirIterator dirIt( _url.toLocalFile(), QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot );
      while ( dirIt.hasNext() ) {
        dirIt.next();
        if ( dirIt.fileName() != QLatin1String( ".directory" ) ) {
          isempty = false;
          break;
        }
      }
      if ( isempty )
        return empty_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeTypePrivate::iconName( _url );

  if ( icon.startsWith( QLatin1String( "./" ) ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KUrl v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.toLocalFile();
  }

  return icon;
}

QString KFolderMimeTypePrivate::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeTypePrivate::comment( _url );

    KUrl u( _url );
    u.addPath( ".directory" );

    const KDesktopFile cfg( u.toLocalFile() );
    QString comment = cfg.readComment();
    if ( comment.isEmpty() )
        return KMimeTypePrivate::comment( _url );

    return comment;
}
