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
#include <dirent.h>
#include <QtCore/QFile>
#include <QtCore/QSet>

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
    return KMimeTypePrivate::iconName( _url );

  if ( icon.startsWith( "./" ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KUrl v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.path();
  }

  return icon;
}

QString KFolderMimeTypePrivate::comment( const KUrl& _url ) const
{
    if ( _url.isEmpty() || !_url.isLocalFile() )
        return KMimeTypePrivate::comment( _url );

    KUrl u( _url );
    u.addPath( ".directory" );

    const KDesktopFile cfg( u.path() );
    QString comment = cfg.readComment();
    if ( comment.isEmpty() )
        return KMimeTypePrivate::comment( _url );

    return comment;
}
