/*
    Copyright (c) 2000 David Faure <faure@kde.org>
    Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kiconloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kfiledialog.h>
#include <kcmdlineargs.h>
#include <qmimedatabase.h>

#include <QApplication>
#include <QWidget>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include "partviewer.h"
#include <kmimetypetrader.h>

PartViewer::PartViewer()
{
    setXMLFile(KDESRCDIR "/partviewer_shell.rc");

    KAction * paOpen = new KAction( KDE::icon("document-open"), "&Open file", this );
    actionCollection()->addAction( "file_open", paOpen );
    connect( paOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()) );

    KAction * paQuit = new KAction( KDE::icon("application-exit"), "&Quit", this );
    actionCollection()->addAction( "file_quit", paQuit );
    connect(paQuit, SIGNAL(triggered()), this, SLOT(close()));

    m_part = 0;

    // Set a reasonable size
    resize( 600, 350 );

    slotFileOpen();
}

PartViewer::~PartViewer()
{
    delete m_part;
}

void PartViewer::openUrl( const QUrl & url )
{
    delete m_part;
    QMimeDatabase db;
    const QString mimeType = db.mimeTypeForUrl(url).name();
    m_part = KMimeTypeTrader::self()->createPartInstanceFromQuery<KParts::ReadOnlyPart>(mimeType,
                                                                                        this,
                                                                                        this);

    if ( m_part )
    {
        setCentralWidget( m_part->widget() );
        // Integrate its GUI
        createGUI( m_part );

        m_part->openUrl( url );
    }
}

void PartViewer::slotFileOpen()
{
    QUrl url = KFileDialog::getOpenUrl();
    if( !url.isEmpty() )
        openUrl( url );
}

int main( int argc, char **argv )
{
    KCmdLineOptions options;
    options.add("+file(s)", qi18n("Files to load"));

    const char version[] = "v0.0.1 2000 (c) David Faure";
    QLocalizedString description = qi18n("This is a test window for showing any part.");

    KCmdLineArgs::init(argc, argv, "partviewer", 0, qi18n("partviewer"), version, description);
    KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.
    QApplication app(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv());
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    PartViewer *shell = new PartViewer;
    if ( args->count() == 1 )
    {
        QUrl url = args->url(0);
        shell->openUrl( url );
    }
    shell->show();
    return app.exec();
}

