/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <kbookmarkimporter.h>
#include <kbookmarkdombuilder.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

#include "kfiledialog.h"
#include "kfilebookmarkhandler.h"

KFileBookmarkHandler::KFileBookmarkHandler( KFileDialog *dialog )
    : QObject( dialog ),
      KBookmarkOwner(),
      m_dialog( dialog )
{
    setObjectName( "KFileBookmarkHandler" );
    m_menu = new KPopupMenu( dialog );
    m_menu->setObjectName( "bookmark menu" );

    QString file = locate( "data", "kfile/bookmarks.xml" );
    if ( file.isEmpty() )
        file = locateLocal( "data", "kfile/bookmarks.xml" );

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);

    // import old bookmarks
    if ( !KStandardDirs::exists( file ) ) {
        QString oldFile = locate( "data", "kfile/bookmarks.html" );
        if ( !oldFile.isEmpty() )
            importOldBookmarks( oldFile, manager );
    }

    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu,
                                        dialog->actionCollection(), true );
}

KFileBookmarkHandler::~KFileBookmarkHandler()
{
    delete m_bookmarkMenu;
}

QString KFileBookmarkHandler::currentURL() const
{
    return m_dialog->baseURL().url();
}

void KFileBookmarkHandler::importOldBookmarks( const QString& path,
                                               KBookmarkManager *manager )
{
    KBookmarkDomBuilder *builder = new KBookmarkDomBuilder( manager->root(), manager );
    KNSBookmarkImporter importer( path );
    builder->connectImporter( &importer );
    importer.parseNSBookmarks();
    delete builder;
    manager->save();
}

void KFileBookmarkHandler::virtual_hook( int id, void* data )
{ KBookmarkOwner::virtual_hook( id, data ); }

#include "kfilebookmarkhandler.moc"
