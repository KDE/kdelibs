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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/* $Id$

 * $Log$
 * Revision 1.22  1999/05/23 00:53:59  kulow
 * CVS_SILENT moving some header files that STL comes before Qt
 *
 * Revision 1.21  1999/05/20 22:21:21  kulow
 * misc fixes (not related to MICO :)
 *
 * Revision 1.20  1999/05/19 20:41:20  kulow
 * make it compile
 *
 * Revision 1.19  1999/03/01 23:33:59  kulow
 * CVS_SILENT ported to Qt 2.0
 *
 * Revision 1.18  1999/02/26 12:56:59  dfaure
 * '/' after PATH_TMP for SCO
 *
 * Revision 1.17.4.1  1999/02/26 12:24:17  dfaure
 * *** empty log message ***
 *
 * Revision 1.17  1999/01/27 21:04:36  kulow
 * fixing bug 395
 *
 * Revision 1.16  1999/01/19 13:36:15  kulow
 * removed explicit settings of DISTCLEANFILES, where the only sense was
 * to remove the MOC files
 *
 * Revision 1.15  1999/01/15 08:59:16  kulow
 * use seperate moc.cpp files - did you know that most header files herein are
 * complete hacks? ;)
 *
 * Revision 1.14  1998/09/21 18:34:46  bieker
 * Use paths.h if we can.
 *
 * Revision 1.13  1998/05/26 14:17:34  kalle
 * tests for KLed
 *
 * Revision 1.12  1998/01/16 21:29:33  kalle
 * mention the new bug site in the about dialog
 *
 * Revision 1.11  1997/10/21 20:44:56  kulow
 * removed all NULLs and replaced it with 0L or "".
 * There are some left in mediatool, but this is not C++
 *
 * Revision 1.10  1997/10/16 11:15:08  torben
 * Kalle: Copyright headers
 * kdoctoolbar removed
 *
 * Revision 1.9  1997/10/12 14:35:30  kalle
 * Compiles again, test for tempSaveName(), checkRecoverFile()
 *
 * Revision 1.8  1997/10/04 19:42:49  kalle
 * new KConfig
 *
 * Revision 1.7  1997/09/24 19:00:25  kalle
 * Iterators
 *
 * Revision 1.6  1997/09/13 05:51:42  kalle
 * new features in KDebug
 * - asserts
 * - varargs
 * - output to syslog
 * - areas implemented
 *
 * Revision 1.5  1997/09/11 19:44:57  kalle
 * New debugging scheme for KDE (binary incompatible!)
 *
 * Revision 1.4  1997/09/10 13:53:11  kdecvs
 * Kalle: Use KApplication instead of QApplication
 *
 * Revision 1.3  1997/07/08 21:21:06  kulow
 * Coolo:
 *    finally, I found the problem with automake-1.2!
 *
 *    The GNU people haven't the time to help me (I believe in this version :-),
 *    so I took all my time between my exams and fidelt with the Makefiles and
 *    now I've got the problem.
 *    The solution was that near, that I could smell it, but I couldn't find it
 *    till now.
 *    The problem was, that the moc files are created after the depency tracking
 *    and this went wrong with automake-1.2, because make stopped after this
 *    errors. The problem was, that I couldn't find the part, where the depencies
 *    are created, but now I have the solution:
 *    lines like: "ksock.cpp: ksock.h" in the Makefile solved the whole problem.
 *
 *    OK, now I'm my own hero and I can sleep very well now and you can update your
 *    automake to 1.2 (untested).
 *
 *    BTW: Martin, could you do it for me? Please!
 *
 * Revision 1.2  1997/04/15 20:01:57  kalle
 * Kalles changes for 0.8
 *
 * Revision 1.1  1997/04/13 17:49:40  kulow
 * Sources imported
 *
 * Revision 1.7  1997/03/16 22:53:19  kalle
 * KTextStream->QTextStream
 *
 * Revision 1.6  1996/12/14 14:01:53  kalle
 * method names start with small letters
 *
 * Revision 1.5  1996/11/30 19:51:21  kalle
 * open app-specific file read-write
 *
 * Revision 1.4  1996/11/23 21:16:17  kalle
 * KTextStream statt QTextStream
 *
 * Revision 1.3  1996/11/20 20:14:11  kalle
 * ValueEdit instead of ValueLabel, WriteButton added
 *
 * Revision 1.2  1996/11/15 18:22:51  kalle
 * all slots return void
 *
 * Revision 1.1  1996/11/14 20:09:06  kalle
 * Initial revision
 *
 * Revision 1.10  1996/11/03 21:01:25  kalle
 * "found in" line removed (for version 0.2)
 *
 * Revision 1.9  1996/10/26 19:09:47  kalle
 * Show the file the key/value pair was found in.
 *
 * Revision 1.8  1996/10/26 18:55:57  kalle
 * Use HasKey() in KeyEditReturnPressed()
 *
 * Revision 1.7  1996/10/26 18:49:20  kalle
 * Show value "---" if the key was not found
 *
 * Revision 1.6  1996/10/26 18:47:24  kalle
 * info messages about finding a key
 *
 * Revision 1.5  1996/10/26 18:42:44  kalle
 * call default group <default> in group message field
 *
 * Revision 1.4  1996/10/26 18:32:29  kalle
 * check for existence of app config file, open it if necessary
 *
 * Revision 1.3  1996/10/26 18:08:59  kalle
 * KeyEditReturnPressed implemented, works for simple cases
 *
 * Revision 1.2  1996/10/26 09:06:53  kalle
 * AppConfigEditReturnPressed und GroupEditReturnPressed implementiert
 *
 * Revision 1.1  1996/10/25 18:57:35  kalle
 * Initial revision
 *
 *
 */

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of KConfig class
//
// adapted from Qt widgets demo

#include <stdlib.h>
#include <kapp.h>
#include <qdialog.h>
#include <qfile.h>
#include <qfileinfo.h>
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
// KConfigTestView contains lots of Qt widgets.
//

#include "kconfigtest.h"
#include "kconfigtest.h"
//
// Construct the KConfigTestView with buttons
//

KConfigTestView::KConfigTestView( QWidget *parent, const char *name )
    : QDialog( parent, name ),
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
  pConfig = new KConfig( QString::null );
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
  if( strlen( pAppFileEdit->text() ) )
    {
	  pConfig = new KConfig( pAppFileEdit->text() );
    }
  
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
  QString aValue = pConfig->readEntry( pKeyEdit->text() );
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

  KASSERT1( false, KDEBUG_INFO, 1, "Entry written: %d", 27 );
}

//
// Create and display our KConfigTestView.
//

int main( int argc, char **argv )
{
  KApplication  a( argc, argv );

  KConfigTestView   *w = new KConfigTestView();
  a.setMainWidget( w );
  w->show();

  fprintf( stderr, "Autosave name for %s is %s\n", "/home/kalle/text/mytext.txt", kapp->tempSaveName( "/home/kalle/text/mytext.txt" ).ascii() );

  bool bRecoverFile = false;
  QString pRecoverFile = kapp->checkRecoverFile( "/home/kalle/text/mytext.txt", bRecoverFile );
  if( bRecoverFile )
	fprintf( stderr, "Recover file exists and is at %s\n", pRecoverFile.ascii());
  else
	fprintf( stderr, "Recover file does not exist, use %s\n", pRecoverFile.ascii() );

  KSimpleConfig sc( _PATH_TMP"/sc.cfg" );
	/*
    sc.writeEntry( "Bua", "Brumm" );

  sc.writeEntry( "boolEntry1", true );
  sc.writeEntry( "boolEntry2", false );
  sc.writeEntry( "rectEntry", QRect( 10, 23, 5321, 12 ) );
  sc.writeEntry( "pointEntry", QPoint( 4351, 1234 ) );
	*/
  
  bool b1 = sc.readBoolEntry( "boolEntry1" );
  bool b2 = sc.readBoolEntry( "boolEntry2" );
  QRect rect = sc.readRectEntry( "rectEntry" );
  QPoint point = sc.readPointEntry( "pointEntry" );
  fprintf( stderr, "b1 is %s\n", b1 ? "true" : "false" );
  fprintf( stderr, "b2 is %s\n", b2 ? "true" : "false" );
  fprintf( stderr, "rect is (%d,%d,%d,%d)\n", rect.left(), rect.top(), rect.width(), rect.height() );
  fprintf( stderr, "point is (%d,%d)\n", point.x(), point.y() );
  

  a.exec();
  delete w;
}

#include "kconfigtest.moc"

