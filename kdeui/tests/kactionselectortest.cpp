/*
    Copyright (c) 2006 David Faure <faure@kde.org>

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

#include <kactionselector.h>

#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <QApplication>
#include <QListWidget>

int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );
  KAboutData aboutData( "kactionselectortest", "kactionselectortest", "1.0" );
  KInstance i( &aboutData );

  KActionSelector actionSelector(0);
  actionSelector.availableListWidget()->addItems(QStringList() << "A" << "B" << "C" << "D" << "E");
  actionSelector.selectedListWidget()->addItems(QStringList() << "1" << "2");
  actionSelector.show();

  return app.exec();
}
