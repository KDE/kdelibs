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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <qtextstream.h>

#include <kbookmarkimporter.h>
#include <kpopupmenu.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

#include "kfiledialog.h"
#include "kfilebookmarkhandler.h"


KFileBookmarkHandler::KFileBookmarkHandler( KFileDialog *dialog )
    : QObject( dialog, "KFileBookmarkHandler" ),
      KBookmarkOwner(),
      m_dialog( dialog ),
      m_importStream( 0L )
{
    m_menu = new KPopupMenu( dialog, "bookmark menu" );

    QString file = locate( "data", "kfile/bookmarks.xml" );
    if ( file.isEmpty() )
        file = locateLocal( "data", "kfile/bookmarks.xml" );

    // import old bookmarks
    if ( !KStandardDirs::exists( file ) ) {
        QString oldFile = locate( "data", "kfile/bookmarks.html" );
        if ( !oldFile.isEmpty() )
            importOldBookmarks( oldFile, file );
    }

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);
    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu,
                                        dialog->actionCollection(), true );
}

KFileBookmarkHandler::~KFileBookmarkHandler()
{
    //     delete m_bookmarkMenu; ###
}

QString KFileBookmarkHandler::currentURL() const
{
    return m_dialog->baseURL().url();
}

void KFileBookmarkHandler::importOldBookmarks( const QString& path,
                                               const QString& destinationPath )
{
    KSaveFile file( destinationPath );
    if ( file.status() != 0 )
        return;

    m_importStream = file.textStream();
    *m_importStream << "<!DOCTYPE xbel>\n<xbel>\n";

    KNSBookmarkImporter importer( path );
    connect( &importer,
             SIGNAL( newBookmark( const QString&, const QCString&, const QString& )),
             SLOT( slotNewBookmark( const QString&, const QCString&, const QString& )));
    connect( &importer,
             SIGNAL( newFolder( const QString&, bool, const QString& )),
             SLOT( slotNewFolder( const QString&, bool, const QString& )));
    connect( &importer, SIGNAL( newSeparator() ), SLOT( newSeparator() ));
    connect( &importer, SIGNAL( endMenu() ), SLOT( endMenu() ));

    importer.parseNSBookmarks( false );

    *m_importStream << "</xbel>";

    file.close();
    m_importStream = 0L;
}

void KFileBookmarkHandler::slotNewBookmark( const QString& text,
                                            const QCString& url,
                                            const QString& additionalInfo )
{
    *m_importStream << "<bookmark icon=\"" << KMimeType::iconForURL( url );
    *m_importStream << "\" href=\"" << QString::fromUtf8(url) << "\">\n";
    *m_importStream << "<title>" << (additionalInfo.isEmpty() ? QString::fromUtf8(url) : additionalInfo) << "</title>\n</bookmark>\n";
}

void KFileBookmarkHandler::slotNewFolder( const QString& text, bool /*open*/,
                                          const QString& /*additionalInfo*/ )
{
    *m_importStream << "<folder icon=\"bookmark_folder\">\n<title=\"";
    *m_importStream << text << "\">\n";
}

void KFileBookmarkHandler::newSeparator()
{
    *m_importStream << "<separator/>\n";
}

void KFileBookmarkHandler::endFolder()
{
    *m_importStream << "</folder>\n";
}

void KFileBookmarkHandler::virtual_hook( int id, void* data )
{ KBookmarkOwner::virtual_hook( id, data ); }

#include "kfilebookmarkhandler.moc"
