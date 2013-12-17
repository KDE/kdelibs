/*
    Copyright 2002 Simon Hausmann <hausmann@kde.org>
    Copyright 2005-2006 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QtCore/QTimer>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>

#include "kmainwindowtest.h"

MainWindow::MainWindow()
{
    QTimer::singleShot( 2*1000, this, SLOT(showMessage()) );

    setCentralWidget( new QLabel( QStringLiteral("foo"), this ) );

    menuBar()->addAction( QStringLiteral("hi") );
}

void MainWindow::showMessage()
{
    statusBar()->show();
    statusBar()->showMessage( QStringLiteral("test") );
}

int main( int argc, char **argv )
{
    QApplication::setApplicationName(QStringLiteral("kmainwindowtest"));
    QApplication app(argc, argv);

    MainWindow* mw = new MainWindow; // deletes itself when closed
    mw->show();

    return app.exec();
}


/* vim: et sw=4 ts=4
 */
