#include <stdio.h>
#include <stdlib.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kbookmarkimporter.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

#include "kfiledialog.h"
#include "kfilebookmarkhandler.h"


KFileBookmarkHandler::KFileBookmarkHandler( KFileDialog *dialog )
    : QObject( dialog, "KFileBookmarkHandler" ),
      KBookmarkOwner(),
      m_dialog( dialog )
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

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false );
    manager->setUpdate( true );


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
    QFile file( destinationPath );
    if ( !file.open( IO_WriteOnly | IO_Truncate ) )
        return;

    m_importStream = new QTextStream( &file );
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
    delete m_importStream;
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

void KFileBookmarkHandler::endMenu()
{
    *m_importStream << "</folder>\n";
}


#include "kfilebookmarkhandler.moc"
