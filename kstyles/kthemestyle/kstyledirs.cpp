/*
  $Id$
  
  This file is part of the KDE libraries
  Adopted from the KStandardDirs code by
    Maksim Orlovich <mo002j@mail.rochester.edu>,

  KStandardDirs are Copyright:
(C) 1999 Sirtaj Singh Kang <taj@kde.org>
(C) 1999 Stephan Kulow <coolo@kde.org>
(C) 1999 Waldo Bastian <bastian@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>

#include <qdir.h>

#include <sys/param.h>
#include <unistd.h>

#include "kstyledirs.h"

static QString readEnvPath( const char *env )
{
    QCString c_path = getenv( env );
    if ( c_path.isEmpty() )
        return QString::null;
    return QFile::decodeName( c_path );
}

static void fixHomeDir( QString &dir )
{
    if ( dir[ 0 ] == '~' )
    {
        dir = QDir::homeDirPath() + dir.mid( 1 );
    }
}

static int tokenize( QStringList& tokens, const QString& str,
                     const QString& delim )
{
    int len = str.length();
    QString token = "";

    for ( int index = 0; index < len; index++ )
    {
        if ( delim.find( str[ index ] ) >= 0 )
        {
            tokens.append( token );
            token = "";
        }
        else
        {
            token += str[ index ];
        }
    }
    if ( token.length() > 0 )
    {
        tokens.append( token );
    }

    return tokens.count();
}


class KStyleDirsPrivate
{
public:
    QStringList dirs;

    void addPrefix( const QString& _dir )
    {
        if ( _dir.isNull() )
            return ;

        QString dir = _dir;
        if ( dir.at( dir.length() - 1 ) != '/' )
            dir += '/';

        if ( !dirs.contains( dir ) )
        {
            dirs.append( dir );
        }
    }


    KStyleDirsPrivate::KStyleDirsPrivate()
    {
        QString kdedirs = readEnvPath( "KDEDIRS" );
        QStringList kdedirList;

        if ( !kdedirs.isEmpty() )
        {
            tokenize( kdedirList, kdedirs, ":" );
        }
        else
        {
            QString kdedir = readEnvPath( "KDEDIR" );
            if ( !kdedir.isEmpty() )
            {
                fixHomeDir( kdedir );
                kdedirList.append( kdedir );
            }
        }
        kdedirList.append( KDEDIR );

#ifdef __KDE_EXECPREFIX

        QString execPrefix( __KDE_EXECPREFIX );
        if ( execPrefix != "NONE" )
            kdedirList.append( execPrefix );
#endif

        QString localKdeDir;
        if ( getuid() )
        {
            localKdeDir = readEnvPath( "KDEHOME" );
            if ( !localKdeDir.isEmpty() )
            {
                if ( localKdeDir[ localKdeDir.length() - 1 ] != '/' )
                    localKdeDir += '/';
            }
            else
            {
                localKdeDir = QDir::homeDirPath() + "/.kde/";
            }
        }
        else
        {
            // We treat root different to prevent root messing up the
            // file permissions in the users home directory.
            localKdeDir = readEnvPath( "KDEROOTHOME" );
            if ( !localKdeDir.isEmpty() )
            {
                if ( localKdeDir[ localKdeDir.length() - 1 ] != '/' )
                    localKdeDir += '/';
            }
            else
            {
                struct passwd *pw = getpwuid( 0 );
                localKdeDir = QFile::decodeName( ( pw && pw->pw_dir ) ? pw->pw_dir : "/root" ) + "/.kde/";
            }
        }

        fixHomeDir( localKdeDir );
        addPrefix( localKdeDir );

        for ( QStringList::ConstIterator it = kdedirList.begin();
                it != kdedirList.end(); it++ )
        {
            QString dir = *it;
            fixHomeDir( dir );
            addPrefix( dir );
        }
    }
};

KStyleDirs::KStyleDirs()
{
    d = new KStyleDirsPrivate;
}

const QStringList& KStyleDirs::enumerateDirs() const
{
    return d->dirs;
}

KStyleDirs::~KStyleDirs()
{
    delete d;
}

void KStyleDirs::addToSearch( QSettings& s, QString apnd ) const
{
    const QStringList & dirs = enumerateDirs();
    for ( int c = 0; c < dirs.size(); c++ )
    {
        s.insertSearchPath( QSettings::Unix, dirs[ c ] + apnd );
    }
}

QString KStyleDirs::locate( const QString& path, const QString& file ) const
{
    const QStringList & dirs = enumerateDirs();
    for ( int c = 0; c < dirs.size(); c++ )
    {
        if ( QFile::exists( dirs[ c ] + path + file ) )
        {
            return dirs[ c ] + path + file;
        }
    }
    return "";
}
