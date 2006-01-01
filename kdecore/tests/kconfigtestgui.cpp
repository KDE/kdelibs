/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include "kconfigtestgui.h"
#include "kconfigtestgui.moc"

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of KConfig class
//
// adapted from Qt widgets demo

#include <unistd.h>
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <qdialog.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <config.h>

// Standard Qt widgets

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

// KDE includes
#include <kconfig.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

//
// Construct the KConfigTestView with buttons
//

KConfigTestView::KConfigTestView( QWidget *parent )
    : QDialog( parent ),
      pConfig( 0L ),
      pFile( 0L ),
      pStream( 0L )
{
  // Set the window caption/title

  setCaption( "KConfig test" );

  // Label and edit for the app config file
  pAppFileLabel = new QLabel( this, "appconfiglabel" );
  pAppFileLabel->setText( "Application config file:" );
  pAppFileLabel->setGeometry( 20, 20, 200, 20 );

  pAppFileEdit = new QLineEdit( this, "appconfigedit" );
  pAppFileEdit->setGeometry( 240, 20, 160, 20 );
  connect( pAppFileEdit, SIGNAL(returnPressed()),
	   SLOT(appConfigEditReturnPressed()));

  // Label and edit for the group
  pGroupLabel = new QLabel( this, "grouplabel" );
  pGroupLabel->setText( "Group:" );
  pGroupLabel->setGeometry( 20, 60, 80, 20 );

  pGroupEdit = new QLineEdit( this, "groupedit" );
  pGroupEdit->setGeometry( 120, 60, 100, 20 );
  connect( pGroupEdit, SIGNAL(returnPressed()),
	   SLOT(groupEditReturnPressed()));

  // Edit and label for the key/value pair
  pKeyEdit = new QLineEdit( this, "keyedit" );
  pKeyEdit->setGeometry( 20, 100, 80, 20 );
  connect( pKeyEdit, SIGNAL( returnPressed()),
	   SLOT(keyEditReturnPressed()));

  pEqualsLabel = new QLabel( this, "equalslabel" );
  pEqualsLabel->setGeometry( 105, 100, 20, 20 );
  pEqualsLabel->setText( "=" );

  pValueEdit = new QLineEdit( this, "valueedit" );
  pValueEdit->setGeometry( 120, 100, 100, 20 );
  pValueEdit->setText( "---" );

  pWriteButton = new QPushButton( this, "writebutton" );
  pWriteButton->setGeometry( 20,140, 80, 20 );
  pWriteButton->setText( "Write entry" );
  connect( pWriteButton, SIGNAL(clicked()), SLOT( writeButtonClicked() ) );

  // Labels for the info line
  pInfoLabel1 = new QLabel( this, "infolabel1" );
  pInfoLabel1->setGeometry( 20, 200, 60, 20 );
  pInfoLabel1->setText( "Info:" );

  pInfoLabel2 = new QLabel( this, "infolabel2" );
  pInfoLabel2->setGeometry( 100, 200, 300, 20 );
  pInfoLabel2->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  // Quit button
  pQuitButton = new QPushButton( this, "quitbutton" );
  pQuitButton->setText( "Quit" );
  pQuitButton->setGeometry( 340, 60, 60, 60 );
  connect( pQuitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );

  // create a default KConfig object in order to be able to start right away
  pConfig = new KConfig( QString() );
}

KConfigTestView::~KConfigTestView()
{
    delete pConfig;
    delete pFile;
    delete pStream;
}

void KConfigTestView::appConfigEditReturnPressed()
{
    // if there already was a config object, delete it and its associated data
    delete pConfig;
    pConfig = 0L;
    delete pFile;
    pFile = 0L;
    delete pStream;
    pStream = 0L;

  // create a new config object
  if( !pAppFileEdit->text().isEmpty() )
	  pConfig = new KConfig( pAppFileEdit->text() );

  pInfoLabel2->setText( "New config object created." );
}

void KConfigTestView::groupEditReturnPressed()
{
  pConfig->setGroup( pGroupEdit->text() );
  // according to the Qt doc, this is begging for trouble, but for a
  // test program this will do
  QString aText;
  aText.sprintf( "Group set to %s", QString( pConfig->group() ).isEmpty() ?
		 QString("<default>").ascii() : pConfig->group().ascii() );
  pInfoLabel2->setText( aText );
}

void KConfigTestView::keyEditReturnPressed()
{
  QString aValue = pConfig->readEntry( pKeyEdit->text(), QString() );
  // just checking aValue.isNull() would be easier here, but this is
  // to demonstrate the HasKey()-method. Besides, it is better data
  // encapsulation because we do not make any assumption about coding
  // non-values here.
  if( !pConfig->hasKey( pKeyEdit->text() ) )
    {
      pInfoLabel2->setText( "Key not found!" );
      pValueEdit->setText( "---" );
    }
  else
    {
      pInfoLabel2->setText( "Key found!" );
      pValueEdit->setText( aValue );
    }
}

void KConfigTestView::writeButtonClicked()
{
  pConfig->writeEntry( pKeyEdit->text(), QString( pValueEdit->text() ) );
  pInfoLabel2->setText( "Entry written" );

  kdDebug() << "Entry written: " << 27 << endl;
}


int main( int argc, char **argv )
{
   KAboutData about("kconfigtestgui", "kconfigtestgui", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;

  KConfigTestView *w = new KConfigTestView();
  a.setMainWidget( w );
  return w->exec();
}
