/***************************************************************************
                          mainwidget.cpp  -  description
                             -------------------
    begin                : Mon Nov 8 1999
    copyright            : (C) 1999 by Falk Brettschneider
    email                : falkbr@kdevelop.org
 ***************************************************************************/

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
#include <qfile.h>
#include <kmdimainfrm.h>
#include <kmditoolviewaccessor.h>

#include "mainwidget.h"

MainWidget::MainWidget(QDomElement& dockConfig,KMdi::MdiMode mode)
: KMdiMainFrm(0L, "theMDIMainFrm",mode)
 ,m_dockConfig(dockConfig)
{
   setIDEAlModeStyle(1); // KDEV3

   dockManager->setReadDockConfigMode(KDockManager::RestoreAllDockwidgets);
   initMenu();

   if (m_dockConfig.hasChildNodes()) {
        readDockConfig(m_dockConfig);
   }

   QMultiLineEdit* mle = new QMultiLineEdit(0L,"theMultiLineEditWidget");
   mle->setText("This is a QMultiLineEdit widget.");
   addToolWindow( mle, KDockWidget::DockBottom, m_pMdi, 70);

   QMultiLineEdit* mle2 = new QMultiLineEdit(0L,"theMultiLineEditWidget2");
   addToolWindow( mle2, KDockWidget::DockCenter, mle, 70);

   QMultiLineEdit* mle3 = new QMultiLineEdit(0L,"theMultiLineEditWidget3");
   addToolWindow( mle3, KDockWidget::DockCenter, mle, 70);

   QMultiLineEdit* mle4 = new QMultiLineEdit(0L,"theMultiLineEditWidget4");
   addToolWindow( mle4, KDockWidget::DockCenter, mle, 70);

   KMdiToolViewAccessor *tva=createToolWindow();
   tva->setWidgetToWrap(new QMultiLineEdit(tva->wrapperWidget(),"theMultiLineEditWidget5"));
   tva->placeAndShow(KDockWidget::DockCenter,mle,70);   

   QListView* lv = new QListView(0L,"theListViewWidget");
#include "../res/filenew.xpm"
   lv->setIcon(filenew);
   lv->addColumn("Test", 50);
   lv->addColumn("KMDI", 70);
   new QListViewItem(lv,QString("test"),QString("test"));
   addToolWindow( lv, KDockWidget::DockLeft, m_pMdi, 35, "1");

   QListView* lv2 = new QListView(0L,"theListViewWidget2");
   lv2->setIcon(filenew);
   lv2->addColumn("Test2", 50);
   lv2->addColumn("KMDI2", 70);
   new QListViewItem(lv,QString("test2"),QString("test2"));
   addToolWindow( lv2, KDockWidget::DockCenter, lv, 35, "2");
   
   QListView* lv3 = new QListView(0L,"theListViewWidget3");
   lv3->setIcon(filenew);
   lv3->addColumn("Test3", 50);
   lv3->addColumn("KMDI3", 70);
   new QListViewItem(lv,QString("test3"),QString("test3"));
   addToolWindow( lv3, KDockWidget::DockCenter, lv, 35, "3");

   dockManager->finishReadDockConfig();

   setMenuForSDIModeSysButtons( menuBar());
}

MainWidget::~MainWidget()
{
    writeDockConfig(m_dockConfig);
    QDomDocument doc = m_dockConfig.ownerDocument();
    QString s = doc.toString();
    QFile f("/tmp/dc.txt");
    f.open(IO_ReadWrite);
    f.writeBlock(s.latin1(), s.length());
    f.close();
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

RestartWidget::RestartWidget():KMainWindow()
{
    mdimode=KMdi::ChildframeMode;
    QVBoxLayout* bl = new QVBoxLayout(this);
    QLabel* l = new QLabel("This is for the testing of\nKMdiMainFrm::read/writeDockConfig().\n", this);
    QCheckBox* b1 = new QCheckBox("KMdiMainFrm close/restart", this);
    b1->toggle();
    QObject::connect(b1, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    bl->add(l);
    bl->add(b1);
    bl->setMargin(10);
    bl->activate();
    show();

    dockConfig = domDoc.createElement("dockConfig");
    domDoc.appendChild(dockConfig);
}

void RestartWidget::onStateChanged(int on)
{
    if (on) {
        m_w = new MainWidget(dockConfig,mdimode);
        m_w->resize(500,500);
        m_w->show();
    }
    else {
        mdimode=m_w->mdiMode();
        m_w->close();
        delete m_w;
    }

}

void RestartWidget::setWindow(MainWidget *w) {
        m_w=w;
}

#include "mainwidget.moc"
