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

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <qmimedatabase.h>

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QWidget>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QTest>

#include "partviewer.h"
#include <kmimetypetrader.h>

PartViewer::PartViewer()
{
    setXMLFile(QFINDTESTDATA("partviewer_shell.rc"));

    QAction * paOpen = new QAction( QIcon::fromTheme(QStringLiteral("document-open")), QStringLiteral("&Open file"), this );
    actionCollection()->addAction( QStringLiteral("file_open"), paOpen );
    connect( paOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()) );

    QAction * paQuit = new QAction( QIcon::fromTheme(QStringLiteral("application-exit")), QStringLiteral("&Quit"), this );
    actionCollection()->addAction( QStringLiteral("file_quit"), paQuit );
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
    QUrl url = QFileDialog::getOpenFileUrl();
    if( !url.isEmpty() )
        openUrl( url );
}

int main( int argc, char **argv )
{
    // This is a test window for showing any part

    QApplication app(argc, argv);
    PartViewer *shell = new PartViewer;
    if (argc > 1) {
        QUrl url = QUrl::fromUserInput(QLatin1String(argv[1]));
        shell->openUrl( url );
    }
    shell->show();
    return app.exec();
}

