//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter_kde1.h"
#include <kfiledialog.h>
#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <qtextcodec.h>

#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>

////////////////////

void KBookmarkImporter::import( const QString & path )
{
    QDomElement elem = m_pDoc->documentElement();
    Q_ASSERT(!elem.isNull());
    scanIntern( elem, path );
}

void KBookmarkImporter::scanIntern( QDomElement & parentElem, const QString & _path )
{
    kDebug(7043) << "KBookmarkImporter::scanIntern " << _path << endl;
    // Substitute all symbolic links in the path
    QDir dir( _path );
    QString canonical = dir.canonicalPath();

    if ( m_lstParsedDirs.contains(canonical) )
    {
        kWarning() << "Directory " << canonical << " already parsed" << endl;
        return;
    }

    m_lstParsedDirs.append( canonical );

    DIR *dp;
    struct dirent *ep;
    dp = opendir( QFile::encodeName(_path) );
    if ( dp == 0L )
        return;

    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
    {
        if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
        {
            KUrl file;
            file.setPath( QString( _path ) + '/' + QFile::decodeName(ep->d_name) );

            KMimeType::Ptr res = KMimeType::findByURL( file, 0, true );
            //kDebug(7043) << " - " << file.url() << "  ->  " << res->name() << endl;

            if ( res->name() == "inode/directory" )
            {
                // We could use KBookmarkGroup::createNewFolder, but then it
                // would notify about the change, so we'd need a flag, etc.
                QDomElement groupElem = m_pDoc->createElement( "folder" );
                parentElem.appendChild( groupElem );
                QDomElement textElem = m_pDoc->createElement( "title" );
                groupElem.appendChild( textElem );
                textElem.appendChild( m_pDoc->createTextNode( KIO::decodeFileName( ep->d_name ) ) );
                if ( KIO::decodeFileName( ep->d_name ) == "Toolbar" )
                    groupElem.setAttribute("toolbar","yes");
                scanIntern( groupElem, file.path() );
            }
            else if ( res->name() == "application/x-desktop" )
            {
                KSimpleConfig cfg( file.path(), true );
                cfg.setDesktopGroup();
                QString type = cfg.readEntry( "Type" );
                // Is it really a bookmark file ?
                if ( type == "Link" )
                    parseBookmark( parentElem, ep->d_name, cfg, 0 /* desktop group */ );
                else
                    kWarning(7043) << "  Not a link ? Type=" << type << endl;
            }
            else if ( res->name() == "text/plain")
            {
                // maybe its an IE Favourite..
                KSimpleConfig cfg( file.path(), true );
                QStringList grp = cfg.groupList().filter( "internetshortcut", Qt::CaseInsensitive );
                if ( grp.count() == 0 )
                    continue;
                cfg.setGroup( *grp.begin() );

                QString url = cfg.readPathEntry("URL");
                if (!url.isEmpty() )
                    parseBookmark( parentElem, ep->d_name, cfg, *grp.begin() );
            } else
                kWarning(7043) << "Invalid bookmark : found mimetype='" << res->name() << "' for file='" << file.path() << "'!" << endl;
        }
    }

    closedir( dp );
}

void KBookmarkImporter::parseBookmark( QDomElement & parentElem, const QByteArray& _text,
                                       KSimpleConfig& _cfg, const QString &_group )
{
    if ( !_group.isEmpty() )
        _cfg.setGroup( _group );
    else
        _cfg.setDesktopGroup();

    QString url = _cfg.readPathEntry( "URL" );
    QString icon = _cfg.readEntry( "Icon" );
    if (icon.endsWith( ".xpm" ) ) // prevent warnings
        icon.truncate( icon.length() - 4 );

    QString text = KIO::decodeFileName( QString::fromLocal8Bit(_text) );
    if ( text.length() > 8 && text.endsWith( ".desktop" ) )
        text.truncate( text.length() - 8 );
    if ( text.length() > 7 && text.endsWith( ".kdelnk" ) )
        text.truncate( text.length() - 7 );

    QDomElement elem = m_pDoc->createElement( "bookmark" );
    parentElem.appendChild( elem );
    elem.setAttribute( "href", url );
    //if ( icon != "www" ) // No need to save the default
    // Hmm, after all, it makes KBookmark::pixmapFile faster,
    // and it shows a nice feature to those reading the file
    elem.setAttribute( "icon", icon );
    QDomElement textElem = m_pDoc->createElement( "title" );
    elem.appendChild( textElem );
    textElem.appendChild( m_pDoc->createTextNode( text ) );
    kDebug(7043) << "KBookmarkImporter::parseBookmark text=" << text << endl;
}
