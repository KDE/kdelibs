/***************************************************************************
                          mainwidget.cpp  -  description
                             -------------------
    begin                : Mon Nov 8 1999
    copyright            : (C) 1999 by Falk Brettschneider
    email                : falkbr@kdevelop.org
 ***************************************************************  ************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <kmdimainfrm.h>

#include "mainwidget.h"

MainWidget::MainWidget()
: KMdiMainFrm(0L, "theMDIMainFrm")
{
   initMenu();

   QMultiLineEdit* mle = new QMultiLineEdit(0L,"theMultiLineEditWidget");
   QMultiLineEdit* mle2 = new QMultiLineEdit(0L,"theMultiLineEditWidget2");
   QMultiLineEdit* mle3 = new QMultiLineEdit(0L,"theMultiLineEditWidget3");
   mle->setText("This is a QMultiLineEdit widget.");
   addToolWindow( mle, KDockWidget::DockBottom, m_pMdi, 70);
   addToolWindow( mle2, KDockWidget::DockCenter, mle, 70);
   addToolWindow( mle3, KDockWidget::DockCenter, mle, 70);

   QListView* lv = new QListView(0L,"theListViewWidget");
   QListView* lv2 = new QListView(0L,"theListViewWidget2");
   QListView* lv3 = new QListView(0L,"theListViewWidget3");
   lv->addColumn("Test", 50);
   lv->addColumn("KMDI", 70);
   new QListViewItem(lv,QString("test"),QString("test"));
   addToolWindow( lv, KDockWidget::DockLeft, m_pMdi, 35);
   lv2->addColumn("Test2", 50);
   lv2->addColumn("KMDI2", 70);
   new QListViewItem(lv,QString("test2"),QString("test2"));
   addToolWindow( lv2, KDockWidget::DockCenter, lv, 35);
   lv3->addColumn("Test3", 50);
   lv3->addColumn("KMDI3", 70);
   new QListViewItem(lv,QString("test3"),QString("test3"));
   addToolWindow( lv3, KDockWidget::DockCenter, lv, 35);

   setMenuForSDIModeSysButtons( menuBar());
}

void MainWidget::initMenu()
{
   menuBar()->insertItem("&Window", windowMenu());
   menuBar()->insertItem("&Docking", dockHideShowMenu());
}

/** additionally fit the system menu button position to the menu position */
void MainWidget::resizeEvent( QResizeEvent *pRSE)
{
   KMdiMainFrm::resizeEvent( pRSE);
   setSysButtonsAtMenuPosition();
}

//void KDockWidget_Compat::KDockMainWindow::virtual_hook(int,void*) {}
