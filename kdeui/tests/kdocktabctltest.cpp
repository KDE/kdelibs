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

#include "kdocktabctltest.h"

#include <kapplication.h>
#include <ktoolbar.h>
#include <kmenubar.h>

#include <<qobjectlist.h>
#include <qobjectdict.h>>
#include <qmultilineedit.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qcolor.h>

static const char* class_xpm[]={
"16 16 8 1",
"b c #000000",
"f c #ffff00",
"c c #800000",
". c None",
"d c #ff0000",
"# c #808080",
"a c #c0c0c0",
"e c #808000",
"...##aa.........",
"..#.a##aaa......",
".#.aaaa##aaaa...",
"..b#aaaaa#####..",
"...ccb#aa#####b.",
"...dccbbb####bb.",
"...ddce##bbbbb..",
".##dddefeeebb...",
"#.##ddeeeebb.b..",
"#a..##b##b..#b..",
"#aaa..##..###b..",
"#aaaaa..#####b..",
"bbaaaaaa#####b#.",
"..bbaaaa####bb##",
"....bbaa##bb###.",
"......bbbb###..."};

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

application::application( const char* name )
: KMainWindow( 0, name )
{
  initMenuBar();

  lastTabDisabled = -1;

  tab = new KDockTabCtl(this);
  setCentralWidget(tab);

  edit  = new QMultiLineEdit(tab);
  edit->setText("QMultiLineEdit 1");
  tab->insertPage( edit, "Tab 1", 1 );
  tab->setPixmap(1,  class_xpm );

  edit  = new QMultiLineEdit(tab);
  edit->setText("QMultiLineEdit 2");
  tab->insertPage( edit, "Tab 2", 2 );
  tab->setPixmap(2,  folder );

  edit  = new QMultiLineEdit(tab);
  edit->setText("QMultiLineEdit 3");
  tab->insertPage( edit, "Tab 3", 3 );
  tab->setPixmap(3,  class_xpm );

  edit  = new QMultiLineEdit(tab);
  edit->setMinimumSize(300,100);
  edit->setText("QMultiLineEdit 4");
  tab->insertPage( edit, "Tab 4", 4 );
  tab->setPixmap(4,  class_xpm );
}

application::~application()
{
}

void application::initMenuBar()
{
  QPopupMenu *file_menu = new QPopupMenu();

  file_menu->insertItem( "Add Tab", this, SLOT(slotAddTab()) );
  file_menu->insertItem( "Delete current Tab", this, SLOT(slotDelTab()) );
  file_menu->insertSeparator();
  file_menu->insertItem( "Disable current Tab", this, SLOT(slotDisTab())  );
  file_menu->insertItem( "Enable last disabled Tab", this, SLOT(slotEnableLastTab())  );
  file_menu->insertSeparator();
  file_menu->insertItem( "Show Tab icon", this, SLOT(slotShowIcon())  );
  file_menu->insertItem( "Hide Tab icon", this, SLOT(slotHideIcon())  );
  file_menu->insertItem( "Set TabPos to TAB_RIGHT", this, SLOT(slotTAB_RIGHT())  );
  file_menu->insertItem( "Set TabPos to TAB_TOP", this, SLOT(slotTAB_TOP())  );
  file_menu->insertSeparator();
  file_menu->insertItem( "Quit", qApp, SLOT(quit()) );

  KMenuBar* menu_bar = new KMenuBar(this);
  menu_bar->insertItem("KDockTabCtl", file_menu);
}

void application::slotAddTab()
{
  QString tname = QString().setNum( tab->children()->count() );
  tname.prepend("Tab ");

  QMultiLineEdit* edit  = new QMultiLineEdit(tab);
  edit->setText(tname);
  int id = tab->insertPage( edit, tname );
  tab->setPixmap( id,  folder );
}

void application::slotDelTab()
{
  int id = tab->visiblePageId();
  if ( id != -1 ){
    tab->removePage( id );
  }
}

void application::slotEnableLastTab()
{
  if ( lastTabDisabled != -1 ){
    tab->setPageEnabled( lastTabDisabled, true );
    lastTabDisabled = -1;
  }
}

void application::slotDisTab()
{
  int id = tab->visiblePageId();
  if ( id != -1 ){
    tab->setPageEnabled( id, false );
    lastTabDisabled = id;
  }
}

void application::slotShowIcon()
{
  tab->showTabIcon( true );
}

void application::slotHideIcon()
{
  tab->showTabIcon( false );
}

void application::slotTAB_RIGHT()
{
  tab->setTabPos( KDockTabBar::TAB_RIGHT );
}

void application::slotTAB_TOP()
{
//  edit->setMinimumSize(700,700);
  tab->setTabPos( KDockTabBar::TAB_TOP );
}

int main(int argc, char* argv[]) { 
  KApplication a(argc,argv,"kdocktabctltest");
  
  application* stabctl = new application("KDockTabCtl test");
  a.setMainWidget(stabctl);
  stabctl->setCaption("KDockTabCtl test");
  stabctl->show();

  return a.exec();
}

#include "kdocktabctltest.moc"

