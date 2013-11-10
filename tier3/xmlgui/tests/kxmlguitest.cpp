/*
 *  Copyright 2001 Simon Hausmann <hausmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#include "kxmlguitest.h"

#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QtCore/QDir>
#include <QTest>

#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kactioncollection.h>

void Client::slotSec()
{
    qDebug() << "Client::slotSec()";
}

int main( int argc, char **argv )
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app(argc, argv);
    QAction *a;

    KMainWindow *mainwindow = new KMainWindow;

    QLineEdit* line = new QLineEdit( mainwindow );
    mainwindow->setCentralWidget( line );

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder( mainwindow );

    KXMLGUIFactory *factory = new KXMLGUIFactory( builder );

    Client *shell = new Client;
    shell->setComponentName(QStringLiteral("konqueror"), QLatin1String("Konqueror"));

    a = new QAction( QIcon::fromTheme( QStringLiteral("view-split-left-right") ), QLatin1String("Split"), shell );
    shell->actionCollection()->addAction( QStringLiteral("splitviewh"), a );

    shell->setXMLFile( QFINDTESTDATA("kxmlguitest_shell.rc") );

    factory->addClient( shell );

    Client *part = new Client;

    a = new QAction( QIcon::fromTheme( QStringLiteral("zoom-out") ), QLatin1String("decfont"), part );
    part->actionCollection()->addAction( QStringLiteral("decFontSizes"), a );
    a = new QAction( QIcon::fromTheme( QStringLiteral("security-low") ), QLatin1String("sec"), part );
    part->actionCollection()->addAction( QStringLiteral("security"), a );
    part->actionCollection()->setDefaultShortcuts(a, QList<QKeySequence>() << Qt::ALT + Qt::Key_1);
    a->connect( a, SIGNAL(triggered(bool)), part, SLOT(slotSec()) );

    part->setXMLFile( QFINDTESTDATA("kxmlguitest_part.rc") );

    factory->addClient( part );
    for ( int i = 0; i < 10; ++i )
    {
        factory->removeClient( part );
        factory->addClient( part );
    }

    return app.exec();
}
