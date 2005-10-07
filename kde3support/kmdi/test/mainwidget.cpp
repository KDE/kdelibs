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
#include <q3toolbar.h>
#include <q3multilineedit.h>
#include <q3listview.h>
#include <qfile.h>
#include <k3mdimainfrm.h>
#include <k3mditoolviewaccessor.h>
#include <q3popupmenu.h>
#include "mainwidget.h"

MainWidget::MainWidget(QDomElement& dockConfig,K3Mdi::MdiMode mode)
: K3MdiMainFrm(0L, "theMDIMainFrm",mode)
 ,m_dockConfig(dockConfig)
{
   setIDEAlModeStyle(1); // KDEV3

   dockManager->setReadDockConfigMode(K3DockManager::RestoreAllDockwidgets);
   initMenu();

   if (m_dockConfig.hasChildNodes()) {
        readDockConfig(m_dockConfig);
   }

   Q3MultiLineEdit* mle = new Q3MultiLineEdit(0L,"theMultiLineEditWidget");
   mle->setText("This is a QMultiLineEdit widget.");
   addToolWindow( mle, K3DockWidget::DockBottom, m_pMdi, 70);

   Q3MultiLineEdit* mle2 = new Q3MultiLineEdit(0L,"theMultiLineEditWidget2");
   addToolWindow( mle2, K3DockWidget::DockCenter, mle, 70);

   Q3MultiLineEdit* mle3 = new Q3MultiLineEdit(0L,"theMultiLineEditWidget3");
   addToolWindow( mle3, K3DockWidget::DockCenter, mle, 70);

   Q3MultiLineEdit* mle4 = new Q3MultiLineEdit(0L,"theMultiLineEditWidget4");
   addToolWindow( mle4, K3DockWidget::DockCenter, mle, 70);

   K3MdiToolViewAccessor *tva=createToolWindow();
   tva->setWidgetToWrap(new Q3MultiLineEdit(tva->wrapperWidget(),"theMultiLineEditWidget5"));
   tva->placeAndShow(K3DockWidget::DockCenter,mle,70);   

   Q3ListView* lv = new Q3ListView(0L,"theListViewWidget");
#include "../res/filenew.xpm"
   lv->setIcon(filenew);
   lv->addColumn("Test", 50);
   lv->addColumn("K3MDI", 70);
   new Q3ListViewItem(lv,QString("test"),QString("test"));
   addToolWindow( lv, K3DockWidget::DockLeft, m_pMdi, 35, "1");

   Q3ListView* lv2 = new Q3ListView(0L,"theListViewWidget2");
   lv2->setIcon(filenew);
   lv2->addColumn("Test2", 50);
   lv2->addColumn("K3MDI2", 70);
   new Q3ListViewItem(lv,QString("test2"),QString("test2"));
   addToolWindow( lv2, K3DockWidget::DockCenter, lv, 35, "2");
   
   Q3ListView* lv3 = new Q3ListView(0L,"theListViewWidget3");
   lv3->setIcon(filenew);
   lv3->addColumn("Test3", 50);
   lv3->addColumn("K3MDI3", 70);
   new Q3ListViewItem(lv,QString("test3"),QString("test3"));
   addToolWindow( lv3, K3DockWidget::DockCenter, lv, 35, "3");

   dockManager->finishReadDockConfig();

   setMenuForSDIModeSysButtons( menuBar());
}

MainWidget::~MainWidget()
{
    writeDockConfig(m_dockConfig);
    QDomDocument doc = m_dockConfig.ownerDocument();
    QString s = doc.toString();
    QFile f("/tmp/dc.txt");
    f.open(QIODevice::ReadWrite);
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
   K3MdiMainFrm::resizeEvent( pRSE);
   setSysButtonsAtMenuPosition();
}

RestartWidget::RestartWidget():KMainWindow()
{
    mdimode=K3Mdi::ChildframeMode;
    QWidget *central=new QWidget(this);
    QVBoxLayout* bl = new QVBoxLayout(central);
    QLabel* l = new QLabel("This is for the testing of\nK3MdiMainFrm::read/writeDockConfig().\n", central);
    QCheckBox* b1 = new QCheckBox("K3MdiMainFrm close/restart", central);
    b1->toggle();
    QObject::connect(b1, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    bl->add(l);
    bl->add(b1);
    bl->setMargin(10);
    //bl->activate();
    setCentralWidget(central);
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
