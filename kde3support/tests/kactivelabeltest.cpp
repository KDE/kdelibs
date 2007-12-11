/*
    Tests the K3ActiveLabel Widget class

    Copyright 2006 by Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QtGui/QLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "k3activelabel.h"

#include "kactivelabeltest.h"

KActiveLabelTest::KActiveLabelTest(QWidget *parent)
    : QWidget(parent)
{
  setWindowTitle( "K3ActiveLabel test application" );

  QVBoxLayout *layout = new QVBoxLayout( this );

  K3ActiveLabel *label = new K3ActiveLabel( this );
  label->setHtml( "That's a small test text ;) <a href=\"whatsthis:click me if you can\">Click me</a>" );

  layout->addWidget( label );
}

int main( int argc, char ** argv )
{
  KAboutData about( "KActiveLabelTest", 0, ki18n("KActiveLabelTest"), "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KActiveLabelTest *toplevel = new KActiveLabelTest( 0 );
  toplevel->show();

  app.exec();
}

#include "kactivelabeltest.moc"

