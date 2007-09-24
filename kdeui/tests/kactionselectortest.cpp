/*
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include <kactionselector.h>

#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <QtGui/QApplication>
#include <QtGui/QListWidget>

int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );
  KAboutData aboutData( "kactionselectortest", 0, ki18n("kactionselectortest"), "1.0" );
  KComponentData i( &aboutData );

  KActionSelector actionSelector(0);
  actionSelector.availableListWidget()->addItems(QStringList() << "A" << "B" << "C" << "D" << "E");
  actionSelector.selectedListWidget()->addItems(QStringList() << "1" << "2");
  actionSelector.show();

  return app.exec();
}
