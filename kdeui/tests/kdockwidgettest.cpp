/***************************************************************************
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdockwidgettest.h"

#include <qpushbutton.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <qvbox.h>

static const char*folder[]={
"16 16 9 1",
"g c #808080",
"b c #ffa858",
"e c #c0c0c0",
"# c #000000",
"c c #ffdca8",
". c None",
"a c #585858",
"f c #a0a0a4",
"d c #ffffff",
"..#a#...........",
".#abc##.........",
".#daabc#####....",
".#ddeaabcccb#...",
".#dedeeabccca...",
".#edeeeeaaaab#..",
".#deeeeeeefe#ba.",
".#eeeeeeefef#ba.",
".#eeeeeefeff#ba.",
".#eeeeefefff#ba.",
".##geefeffff#ba.",
"...##gefffff#ba.",
".....##fffff#ba.",
".......##fff#b##",
".........##f#b##",
"...........####."};


DockApplication::DockApplication( const char* name )
: KDockMainWindow( 0L, name )
{
  QPixmap p(folder);

  initMenuBar();
  initToolBars();
  initStatusBar();

  /*****************************************************/
  dock = createDockWidget( "Green Widget", p );
  dock->setCaption("Green");
  dock->setGeometry(50, 50, 100, 100);
  l = new QWidget(dock);
  l->setBackgroundColor(green);
  l->setMinimumSize(100,100);
  dock->setWidget(l);
  /*****************************************************/
  dock1 = createDockWidget( "Blue Widget", p );
  dock1->setCaption("Blue");
  dock1->setGeometry( 150, 150, 100, 100);
  setView( dock1 );
  setMainDockWidget( dock1 );

  mainW = new QWidget( dock1, "createdOnBlueDock" );
  mainW->setBackgroundColor(blue);
  mainW->setMinimumSize(300,150);
  dock1->setWidget( mainW );
  /*****************************************************/

  KDockWidget* dock2 = createDockWidget( "Yellow Widget", p );
  dock2->setGeometry(300, 300, 100, 100);
  dock2->setCaption("Yellow");

  /* test set new header widget...*/
//  dock2->setHeader( new KDockWidgetHeader(dock2) );

  QWidget* l2 = new QWidget(dock2);
  l2->setBackgroundColor(yellow);
  dock2->setWidget( l2 );
  /*****************************************************/

  /*****************************************************/
  dock5 = createDockWidget( "Container Widget", p );
  dock5->setCaption("Container");
  dock5->setGeometry(50, 50, 100, 100);
  l = new CTW(dock5);
  l->setBackgroundColor(white);
  l->setMinimumSize(100,100);
  dock5->setWidget(l);
  if (l->qt_cast("KDockContainer")) qDebug("KDockContainer created for dock 5");
  /*****************************************************/

  /*****************************************************/
  dock6 = createDockWidget( "Container Widget2", p );
  dock6->setCaption("Container2");
  dock6->setGeometry(50, 50, 100, 100);
  l = new CTW(dock6);
  l->setBackgroundColor(white);
  l->setMinimumSize(100,100);
  dock6->setWidget(l);
  if (l->qt_cast("KDockContainer")) qDebug("KDockContainer created for dock 6");
  /*****************************************************/



  QPushButton* b1 = new QPushButton(mainW);
  b1->setGeometry(10, 10, 250, 25);
  b1->setText("write dock config");
  connect(b1, SIGNAL(clicked()), SLOT(wConfig()));

  QPushButton* b2 = new QPushButton(mainW);
  b2->setGeometry(10, 35, 250, 25);
  b2->setText("read dock config");
  connect(b2, SIGNAL(clicked()), SLOT(rConfig()));

  m_bname = new QPushButton(mainW);
  m_bname->setGeometry(10, 60, 250, 25);
  m_bname->setEnabled( false );

  setGeometry(200, 100, 500, 300);

  qDebug("load config");
  readDockConfig();

  updateButton();
}

DockApplication::~DockApplication()
{
  qDebug("Close & store config");
  writeDockConfig();
}

void DockApplication::rConfig()
{
  readDockConfig();
}

void DockApplication::wConfig()
{
  writeDockConfig();
}

void DockApplication::initMenuBar()
{
  QPixmap p(folder);
  QPopupMenu *file_menu = new QPopupMenu();

  file_menu->insertItem(p, "Change Green Widget Caption", this, SLOT(cap()) );
  file_menu->insertSeparator();
  file_menu->insertItem(p, "Set Green Widget as MainDockWidget", this, SLOT(greenMain()) );
  file_menu->insertItem(p, "Set Blue Widget as MainDockWidget", this, SLOT(blueMain()) );
  file_menu->insertItem(p, "Set NULL as MainDockWidget", this, SLOT(nullMain()) );

  KMenuBar* menu_bar = menuBar();
  menu_bar->insertItem( "&Test", file_menu );
  menu_bar->insertItem( "&Docking Windows", dockHideShowMenu() );
}

void DockApplication::initToolBars()
{
  QPixmap p(folder);
  KToolBar* tool_bar_0 = toolBar(0);
  tool_bar_0->setFullSize(false);
  tool_bar_0->insertButton( p, 1 );
  tool_bar_0->insertButton(p, 2 );
  tool_bar_0->setFullSize( true );
}

void DockApplication::initStatusBar()
{
  KStatusBar* status_bar = statusBar();
  status_bar->insertItem("Welcome to KDockWidget test...", 1);
}

void DockApplication::cap()
{
  if ( dock->caption() != "Test Caption1" )
    dock->setCaption("Test Caption1");
  else
    dock->setCaption("Another Caption");
}

void DockApplication::greenMain()
{
  setMainDockWidget( dock );
  updateButton();
}

void DockApplication::blueMain()
{
  setMainDockWidget( dock1 );
  updateButton();
}

void DockApplication::nullMain()
{
  setMainDockWidget( 0L );
  updateButton();
}

void DockApplication::updateButton()
{
  if ( getMainDockWidget() )
    m_bname->setText(QString("MainDockWidget is %1").arg(getMainDockWidget()->name()));
  else
    m_bname->setText("MainDockWidget is NULL");
}

int main(int argc, char* argv[]) {
  KApplication a(argc,argv, "kdockdemoapp1");
  DockApplication* ap = new DockApplication("DockWidget demo");
  ap->setCaption("DockWidget demo");
  a.setMainWidget(ap);
  ap->show();
  return a.exec();
}

#include "kdockwidgettest.moc"

