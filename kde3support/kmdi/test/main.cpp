//----------------------------------------------------------------------------
//    filename             : main.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           02/2000       by Massimo Morin (mmorin@schedsys.com)
//
//    copyright            : (C) 1999-2000 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef NO_KDE
# include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
 KApplication* theApp;
#else
# include <qapplication.h>
 QApplication* theApp;
#endif

//#include <iostream.h>
#include "mainwidget.h"
#include "hello.h"

#define SHOW(v) cout << #v << " -> " << v << endl;

int main( int argc, char **argv )
{
#ifndef NO_KDE
  KAboutData about("KFourChildren", "KFourChildren", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication a(argc,argv);
#else
   QApplication a(argc,argv);
#endif
   theApp = &a;

   RestartWidget* restartWidget = new RestartWidget;
   MainWidget* mainWdg = new MainWidget(restartWidget->dockConfig,restartWidget->mdimode);
   restartWidget->setWindow(mainWdg);

   Hello* h1 = new Hello( "Hello1", "Click the right mouse button on the mainframe!", mainWdg);
   h1->setTabCaption("Tab changed");
   h1->setBackgroundColor( Qt::white );
   mainWdg->addWindow( h1);
//   SHOW(h1->caption());
//   SHOW(h1->tabCaption());

   QWidget* w = new QWidget(mainWdg);
   K3MdiChildView* h2 = mainWdg->createWrapper(w, "I'm a common but wrapped QWidget!", "Hello2");
   mainWdg->addWindow( h2 );
//   SHOW(h2->caption());
//   SHOW(h2->tabCaption());

   Hello* h3 = new Hello( "Hello3", "Dock me using the taskbar button context menu!", 0);
   h3->setBackgroundColor( Qt::white );
   h3->setFont( QFont("times",20,QFont::Bold) );
   mainWdg->addWindow( h3, K3Mdi::Detach );  // undock this!
   h3->setGeometry( 20, 20, 400, 100);

   Hello* h4 = new Hello( "Hello4", "Hello world!", mainWdg);
   h4->setMDICaption("Hello4 both changed");
   h4->setFont( QFont("times",32,QFont::Bold) );	
   mainWdg->addWindow( h4);
//   SHOW(h4->caption());
//   SHOW(h4->tabCaption());

   Hello* h5 = new Hello( "Hello5", "I'm not a MDI widget :-(");
   h5->setGeometry(40, 40, 400, 100);
   h5->setFont( QFont("times",20,QFont::Bold) );
   h5->setCaption("MDI Test Application");
   h5->show();

#if 0
   K3MdiIterator<K3MdiChildView*>  *it = mainWdg->createIterator();
   for ( it->first(); !it->isDone(); it->next()) {
      //cout << "--> " << it->currentItem()->caption() << endl;

   }
   //delete it;
#endif

   mainWdg->resize(500,500);
   restartWidget->show();
   mainWdg->show();

   mainWdg->cascadeWindows();
   return a.exec();
}
