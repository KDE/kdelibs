/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>

#include "kpagedialogtest.h"

KPageDialogTest::KPageDialogTest( QWidget *parent )
  : KPageDialog( parent )
{
  setFaceType( Tabbed );

  QWidget *page = new QWidget( this );
  QHBoxLayout *layout = new QHBoxLayout( page );

  QLabel *label = new QLabel( "first page" );
  layout->addWidget( label );

  addPage( page, "First" );

  page = new QWidget( this );
  layout = new QHBoxLayout( page );

  label = new QLabel( "second page" );
  label->setMinimumSize( 300, 200 );
  layout->addWidget( label );

  addPage( page, "Second" );
}

KPageDialogTest::~KPageDialogTest()
{
}

int main( int argc, char **argv )
{
  KAboutData about("KPageDialogTest", 0, ki18n("KPageDialogTest"), "version");
  KCmdLineArgs::init( argc, argv, &about );

  KApplication app;

  KPageDialogTest testDialog( 0 );
  testDialog.exec();

  return 0;
}

#include "kpagedialogtest.moc"
