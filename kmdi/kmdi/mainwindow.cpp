//----------------------------------------------------------------------------
//    filename             : kmdimainfrm.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
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


/*
 * ATTENTION: please do you part to try to make this file legible.  It's
 * extremely hard to read already.  Especially follow the indenting rules.
 */
#include "config.h"

#include <assert.h>

#include <qcursor.h>
#include <qclipboard.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qtabwidget.h>
#include <klocale.h>

#include <kiconloader.h>
#include "dockcontainer.h"
#include "mainwindow.moc"


#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "mainwindow.h"
#include "dockcontainer.h"
#include "toolviewaccessor_p.h"
#include "tabwidget.h"
#include "guiclient.h"

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#ifndef NO_KDE
#include <X11/X.h> // schroder
#include <X11/Xlib.h> // schroder
#endif

#ifdef KeyRelease
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyRelease
#endif

#ifdef KeyPress
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyPress
#endif
#endif // Q_WS_X11 && ! K_WS_QTONLY

using namespace KParts;

KMdi::FrameDecor KMdiMainFrm::m_frameDecoration = KMdi::KDELook;

//KMdi::MdiMode KMdiMainFrm::m_mdiMode = KMdi::ChildframeMode;

class KMdiMainFrmPrivate {
public:
  KMdiMainFrmPrivate() {
    for (int i=0;i<4;i++) {
      activeDockPriority[i]=0;
      m_styleIDEAlMode = 0;
      m_toolviewStyle = 0;
    }
  }
  ~KMdiMainFrmPrivate() {}
  KMdiDockContainer* activeDockPriority[4];
  int m_styleIDEAlMode;
  int m_toolviewStyle;
};

//============ constructor ============//
KMdiMainFrm::KMdiMainFrm(QWidget* parentWidget, const char* name, KMdi::MdiMode mdiMode,WFlags flags)
: KParts::DockMainWindow( parentWidget, name, flags)
   ,m_mdiMode(KMdi::UndefinedMode)
   ,m_pWindowPopup(0L)
   ,m_pTaskBarPopup(0L)
   ,m_pWindowMenu(0L)
   ,m_pDockMenu(0L)
   ,m_pMdiModeMenu(0L)
   ,m_pPlacingMenu(0L)
   ,m_pMainMenuBar(0L)
   ,m_pUndockButtonPixmap(0L)
   ,m_pMinButtonPixmap(0L)
   ,m_pRestoreButtonPixmap(0L)
   ,m_pCloseButtonPixmap(0L)
   ,m_pUndock(0L)
   ,m_pMinimize(0L)
   ,m_pRestore(0L)
   ,m_pClose(0L)
   ,m_bMaximizedChildFrmMode(false)
   ,m_oldMainFrmHeight(0)
   ,m_oldMainFrmMinHeight(0)
   ,m_oldMainFrmMaxHeight(0)
   ,m_bSDIApplication(false)
   ,m_pDockbaseAreaOfDocumentViews(0L)
   ,m_pTempDockSession(0L)
   ,m_bClearingOfWindowMenuBlocked(false)
   ,m_pDragEndTimer(0L)
   ,m_bSwitching(false)
   ,m_leftContainer(0)
   ,m_rightContainer(0)
   ,m_topContainer(0)
   ,m_bottomContainer(0)
   ,d(new KMdiMainFrmPrivate())
   ,m_mdiGUIClient(0)
   ,m_managedDockPositionMode(false)
   ,m_documentTabWidget(0)
{
  kdDebug()<<"=== KMdiMainFrm() ==="<<endl;
  // Create the local lists of windows
  m_pToolViews = new QMap<QWidget*,KMdiToolViewAccessor*>;

  // This seems to be needed (re-check it after Qt2.0 comed out)
  setFocusPolicy(ClickFocus);

  // create the central widget
  createMdiManager();

  // cover KMdi's childarea by a dockwidget
  m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
  m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);
  m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
  m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
//  m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
  // set this dock to main view
  setView(m_pDockbaseAreaOfDocumentViews);
  setMainDockWidget(m_pDockbaseAreaOfDocumentViews);

  // Apply options for the MDI manager
  applyOptions();

  m_pTaskBarPopup=new QPopupMenu( this, "taskbar_popup_menu");
  m_pWindowPopup=new QPopupMenu( this, "window_popup_menu");

  m_pWindowMenu = new QPopupMenu( this, "window_menu");
  m_pWindowMenu->setCheckable( true);
  QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );

  m_pDockMenu = new QPopupMenu( this, "dock_menu");
  m_pDockMenu->setCheckable( true);

  m_pMdiModeMenu = new QPopupMenu( this, "mdimode_menu");
  m_pMdiModeMenu->setCheckable( true);

  m_pPlacingMenu = new QPopupMenu( this, "placing_menu");

  // the MDI view taskbar
  createTaskBar();

  // this is only a hack, but prevents us from crash because the buttons are otherwise
  // not created before we switch the modes where we need them !!!
  setMenuForSDIModeSysButtons(menuBar());


      switchToIDEAlMode();


  // drag end timer
  m_pDragEndTimer = new QTimer();
  connect(m_pDragEndTimer, SIGNAL(timeout()), this, SLOT(dragEndTimeOut()));
}

void KMdiMainFrm::setStandardMDIMenuEnabled(bool showModeMenu) {
  m_mdiGUIClient=new KMDIPrivate::KMDIGUIClient(this,showModeMenu);
  connect(m_mdiGUIClient,SIGNAL(toggleTop()),this,SIGNAL(toggleTop()));
  connect(m_mdiGUIClient,SIGNAL(toggleLeft()),this,SIGNAL(toggleLeft()));
  connect(m_mdiGUIClient,SIGNAL(toggleRight()),this,SIGNAL(toggleRight()));
  connect(m_mdiGUIClient,SIGNAL(toggleBottom()),this,SIGNAL(toggleBottom()));

  if (m_mdiMode==KMdi::IDEAlMode) {
    if (m_topContainer)
      connect(this,SIGNAL(toggleTop()),m_topContainer->getWidget(),SLOT(toggle()));
    if (m_leftContainer)
      connect(this,SIGNAL(toggleLeft()),m_leftContainer->getWidget(),SLOT(toggle()));
    if (m_rightContainer)
      connect(this,SIGNAL(toggleRight()),m_rightContainer->getWidget(),SLOT(toggle()));
    if (m_bottomContainer)
      connect(this,SIGNAL(toggleBottom()),m_bottomContainer->getWidget(),SLOT(toggle()));
  }

  emit mdiModeHasBeenChangedTo(m_mdiMode);
}

//============ ~KMdiMainFrm ============//
KMdiMainFrm::~KMdiMainFrm()
{
  emit lastChildViewClosed();
  delete m_pToolViews;
  m_pToolViews=0;
  delete m_pDragEndTimer;

  delete m_pUndockButtonPixmap;
  delete m_pMinButtonPixmap;
  delete m_pRestoreButtonPixmap;
  delete m_pCloseButtonPixmap;

  //deletes added for Release-Version-Pop-Up-WinMenu-And-Go-Out-Problem
  delete m_pDockMenu;
  delete m_pMdiModeMenu;
  delete m_pPlacingMenu;
  delete m_pTaskBarPopup;
  delete m_pWindowPopup;
  delete m_pWindowMenu;
  delete m_mdiGUIClient;
  m_mdiGUIClient=0;
  delete d;
  d=0;
}

//============ applyOptions ============//
#ifdef __GNUC__
#warning fixme
#endif
void KMdiMainFrm::applyOptions()
{

}

//============ createMdiManager ============//
void KMdiMainFrm::createMdiManager()
{
  // dummy TODO
  /*KDockWidget *w = createDockWidget("muh", SmallIcon("view_tree"), this);



  m_centralView = new QWidget (w);
  w->setWidget (m_centralView);

  setMainDockWidget(w);

  m_centralView->show();*/
}

//============ createTaskBar ==============//
void KMdiMainFrm::createTaskBar()
{
}

void KMdiMainFrm::slot_toggleTaskBar()
{
}

void KMdiMainFrm::resizeEvent(QResizeEvent *e)
{
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget())
    if( e->oldSize().height() != e->size().height()) {
      return;
    }
  KParts::DockMainWindow::resizeEvent(e);
  if (!m_mdiGUIClient) return;
  setSysButtonsAtMenuPosition();
}

//================ setMinimumSize ===============//

void KMdiMainFrm::setMinimumSize( int minw, int minh)
{
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget())
    return;
  DockMainWindow::setMinimumSize( minw, minh);
}

KMdiToolViewAccessor *KMdiMainFrm::createToolWindow()
{
  return new KMdiToolViewAccessor(this);
}


void KMdiMainFrm::deleteToolWindow( QWidget* pWnd) {
  if (m_pToolViews->contains(pWnd)) {
    deleteToolWindow((*m_pToolViews)[pWnd]);
  }
}

void KMdiMainFrm::deleteToolWindow( KMdiToolViewAccessor *accessor) {
  if (!accessor) return;
  delete accessor;
}

//============ addWindow ============//
KMdiToolViewAccessor *KMdiMainFrm::addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos, QWidget* pTargetWnd, int percent, const QString& tabToolTip, const QString& tabCaption)
{
  QWidget *tvta=pWnd;
  KDockWidget* pDW = dockManager->getDockWidgetFromName(pWnd->name());
  if (pDW) {
    // probably readDockConfig already created the widgetContainer, use that
    pDW->setWidget(pWnd);

    if (pWnd->icon()) {
      pDW->setPixmap(*pWnd->icon());
    }
    pDW->setTabPageLabel((tabCaption==0)?pWnd->caption():tabCaption);
    pDW->setToolTipString(tabToolTip);
    dockManager->removeFromAutoCreateList(pDW);
    pWnd=pDW;
  }

  QRect r=pWnd->geometry();

  KMdiToolViewAccessor *mtva=new KMdiToolViewAccessor(this,pWnd,tabToolTip,(tabCaption==0)?pWnd->caption():tabCaption);
  m_pToolViews->insert(tvta,mtva);

  if (pos == KDockWidget::DockNone) {
    mtva->d->widgetContainer->setEnableDocking(KDockWidget::DockNone);
    mtva->d->widgetContainer->reparent(this, Qt::WType_TopLevel | Qt::WType_Dialog, r.topLeft(), true); //pToolView->isVisible());
  } else {   // add (and dock) the toolview as DockWidget view
    //const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();

    //KDockWidget *pCover=mtva->d->widgetContainer;

    mtva->place(pos, pTargetWnd,percent);

  }

  return mtva;
}

void KMdiMainFrm::slotDocCurrentChanged(QWidget* pWidget)
{

}

bool KMdiMainFrm::eventFilter(QObject * /*obj*/, QEvent *e )
{
#if 0
  if( e->type() == QEvent::FocusIn) {
    QFocusEvent* pFE = (QFocusEvent*) e;
    if (pFE->reason() == QFocusEvent::ActiveWindow) {
      if (m_pCurrentWindow && !m_pCurrentWindow->isHidden() && !m_pCurrentWindow->isAttached() && m_pMdi->topChild()) {
        return true;   // eat the event
      }
    }
    if (m_pMdi) {
      static bool bFocusTCIsPending = false;
      if (!bFocusTCIsPending) {
        bFocusTCIsPending = true;
        m_pMdi->focusTopChild();
        bFocusTCIsPending = false;
      }
    }
  } else if (e->type() == QEvent::KeyRelease) {
    if (switching()) {
      KAction *a = actionCollection()->action( "view_last_window" ) ;
      if (a) {
        const KShortcut cut( a->shortcut() );
        const KKeySequence& seq = cut.seq( 0 );
        const KKey& key = seq.key(0);
        int modFlags = key.modFlags();
        int state = ((QKeyEvent *)e)->state();
        KKey key2( (QKeyEvent *)e );

        /** these are quite some assumptions:
         *   The key combination uses exactly one modifier key
         *   The WIN button in KDE is the meta button in Qt
         **/
        if (state != ((QKeyEvent *)e)->stateAfter() &&
            ((modFlags & KKey::CTRL) > 0) == ((state & Qt::ControlButton) > 0 ) &&
            ((modFlags & KKey::ALT) > 0)  == ((state & Qt::AltButton) > 0)      &&
            ((modFlags & KKey::WIN) > 0)  == ((state & Qt::MetaButton) > 0) )
        {
          activeWindow()->updateTimeStamp();
          setSwitching(false);
        }
        return true;
      } else {
        kdDebug(9000) << "KAction( \"view_last_window\") not found." << endl;
      }
    }
  }

#endif
  return false;  // standard event processing
}

/**
 * close all views
 */
void KMdiMainFrm::closeAllViews()
{
}


/**
 * iconify all views
 */
void KMdiMainFrm::iconifyAllViews()
{
}

/**
 * closes the view of the active (topchild) window
 */
void KMdiMainFrm::closeActiveView()
{
}

/** find the root dockwidgets and store their geometry */
void KMdiMainFrm::findRootDockWidgets(QPtrList<KDockWidget>* pRootDockWidgetList, QValueList<QRect>* pPositionList)
{
  if (!pRootDockWidgetList) return;
  if (!pPositionList) return;

  // since we set some windows to toplevel, we must consider the window manager's window frame
  const int frameBorderWidth  = 7;  // @todo: Can we / do we need to ask the window manager?
  const int windowTitleHeight = 10; // @todo:    -"-

  QObjectList* pObjList = queryList( "KDockWidget");
  if (pObjList->isEmpty()) {
    pObjList = queryList( "KDockWidget_Compat::KDockWidget");
  }
  QObjectListIt it( *pObjList);
  QObject* pObj;
  // for all dockwidgets (which are children of this mainwindow)
  while ((pObj = it.current()) != 0L) {
    ++it;
    KDockWidget* pDockW = (KDockWidget*) pObj;
    KDockWidget* pRootDockW = 0L;
    KDockWidget* pUndockCandidate = 0L;
    QWidget* pW = pDockW;
    // find the oldest ancestor of the current dockwidget that can be undocked
    while (!pW->isTopLevel()) {
      if (pW->inherits("KDockWidget") || pW->inherits("KDockWidget_Compat::KDockWidget")) {
        pUndockCandidate = (KDockWidget*) pW;
        if (pUndockCandidate->enableDocking() != KDockWidget::DockNone)
          pRootDockW = pUndockCandidate;
      }
      pW = pW->parentWidget();
    }
    if (pRootDockW) {
      // if that oldest ancestor is not already in the list, append it
      bool found = false;
      QPtrListIterator<KDockWidget> it2( *pRootDockWidgetList);
      if (!pRootDockWidgetList->isEmpty()) {
        for ( ; it2.current() && !found; ++it2 ) {
          KDockWidget* pDockW = it2.current();
          if (pDockW == pRootDockW)
            found = true;
        }
        if (!found) {
          pRootDockWidgetList->append( (KDockWidget*)pDockW);
          kdDebug(760)<<"pRootDockWidgetList->append("<<pDockW->name()<<");"<<endl;
          QPoint p = pDockW->mapToGlobal( pDockW->pos())-pDockW->pos();
          QRect r( p.x(),
              p.y()+m_undockPositioningOffset.y(),
              pDockW->width()  - windowTitleHeight - frameBorderWidth*2,
              pDockW->height() - windowTitleHeight - frameBorderWidth*2);
          pPositionList->append( r);
        }
      }
      else {
        pRootDockWidgetList->append( (KDockWidget*)pRootDockW);
        kdDebug(760)<<"pRootDockWidgetList->append("<<pDockW->name()<<");"<<endl;
        QPoint p = pRootDockW->mapToGlobal( pRootDockW->pos())-pRootDockW->pos();
        QRect r( p.x(),
            p.y()+m_undockPositioningOffset.y(),
            pRootDockW->width()  - windowTitleHeight - frameBorderWidth*2,
            pRootDockW->height() - windowTitleHeight - frameBorderWidth*2);
        pPositionList->append( r);
      }
    }
  }
  delete pObjList;
}

void KMdiMainFrm::setIDEAlModeStyle(int flags)
{
  d->m_styleIDEAlMode = flags; // see KMultiTabBar for the first 3 bits
  if (m_leftContainer) {
    KMdiDockContainer *tmpL=(KMdiDockContainer*) (m_leftContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpL) tmpL->setStyle(flags);
  }

  if (m_rightContainer) {
    KMdiDockContainer *tmpR=(KMdiDockContainer*) (m_rightContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpR) tmpR->setStyle(flags);
  }

  if (m_topContainer) {
    KMdiDockContainer *tmpT=(KMdiDockContainer*) (m_topContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpT) tmpT->setStyle(flags);
  }

  if (m_bottomContainer) {
    KMdiDockContainer *tmpB=(KMdiDockContainer*) (m_bottomContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpB) tmpB->setStyle(flags);
  }
}

void KMdiMainFrm::setToolviewStyle(int flag)
{
  if (m_mdiMode == KMdi::IDEAlMode) {
    setIDEAlModeStyle(flag);
  }
  d->m_toolviewStyle = flag;
  bool toolviewExists = false;
  QMap<QWidget*,KMdiToolViewAccessor*>::Iterator it;
  for (it = m_pToolViews->begin(); it != m_pToolViews->end(); ++it) {
    KDockWidget *dockWidget = dynamic_cast<KDockWidget*>(it.data()->wrapperWidget());
    if (dockWidget) {
      if (flag == KMdi::IconOnly)
      {
        dockWidget->setTabPageLabel(" ");
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
      } else
      if (flag == KMdi::TextOnly)
      {
        dockWidget->setPixmap(); //FIXME: Does not hide the icon in the IDEAl mode.
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      } else
      if (flag == KMdi::TextAndIcon)
      {
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      }
      toolviewExists = true;
    }
  }
  if (toolviewExists)
  {
    //workaround for the above FIXME to make switching to TextOnly mode work in IDEAl as well. Be sure that this version of switch* is called.
    if (m_mdiMode == KMdi::IDEAlMode && flag == KMdi::TextOnly)
    {
   /*   KMdiMainFrm::switchToTabPageMode();
      KMdiMainFrm::switchToIDEAlMode();*/ //TODO
    } else
    {
      writeDockConfig();
      readDockConfig();
    }
  }
}

/**
 * Docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToIDEAlMode()
{
  kdDebug(760)<<"SWITCHING TO IDEAL"<<endl;

  if (m_mdiMode == KMdi::IDEAlMode) {
    emit mdiModeHasBeenChangedTo(KMdi::IDEAlMode);
    return;  // nothing need to be done
  }

  m_mdiMode = KMdi::IDEAlMode;

  setupToolViewsForIDEALMode();

  emit mdiModeHasBeenChangedTo(KMdi::IDEAlMode);
}


void KMdiMainFrm::dockToolViewsIntoContainers(QPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container) {
  for ( KDockWidget *dw = widgetsToReparent.first(); dw;
      dw=widgetsToReparent.next()){
    dw->manualDock(container,KDockWidget::DockCenter,20);
    dw->loseFormerBrotherDockWidget();
  }
}

void KMdiMainFrm::findToolViewsDockedToMain(QPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw) {
  KDockWidget *mainDock=getMainDockWidget();
  if (mainDock->parentDockTabGroup()) {
    mainDock=dynamic_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
    // FIXME: will likely crash below due to unchecked cast
  }

  KDockWidget* widget=mainDock->findNearestDockWidget(dprtmw);
  if (widget) {
    if (widget->parentDockTabGroup()) {
      widget=static_cast<KDockWidget*>(widget->parentDockTabGroup()->parent());
    }

    if (widget) {
      KDockTabGroup *tg=dynamic_cast<KDockTabGroup*>(widget->
          getWidget());
      if (tg) {
        kdDebug(760)<<"KDockTabGroup found"<<endl;
        for (int i=0;i<tg->count();i++)
          list->append((KDockWidget*)static_cast<KDockWidget*>(
                tg->page(i)));
      } else
        list->append((KDockWidget*)widget);
    } else
      kdDebug(760)<<"setupToolViewsForIDEALMode: no  widget found"<<endl;
  } else
    kdDebug(760)<<"No main dock widget found"<<endl;
}


void KMdiMainFrm::setupToolViewsForIDEALMode()
{
  m_leftContainer = createDockWidget("KMdiDock::leftDock",SmallIcon("misc"),0L,"Left Dock");
  m_rightContainer = createDockWidget("KMdiDock::rightDock",SmallIcon("misc"),0L,"Right Dock");
  m_topContainer = createDockWidget("KMdiDock::topDock",SmallIcon("misc"),0L,"Top Dock");
  m_bottomContainer = createDockWidget("KMdiDock::bottomDock",SmallIcon("misc"),0L,"Bottom Dock");

  KDockWidget *mainDock=getMainDockWidget();
  KDockWidget *w=mainDock;
  if (mainDock->parentDockTabGroup()) {
    w=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  QPtrList<KDockWidget> leftReparentWidgets;
  QPtrList<KDockWidget> rightReparentWidgets;
  QPtrList<KDockWidget> bottomReparentWidgets;
  QPtrList<KDockWidget> topReparentWidgets;

  if (mainDock->parentDockTabGroup()) {
    mainDock=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  findToolViewsDockedToMain(&leftReparentWidgets,KDockWidget::DockLeft);
  findToolViewsDockedToMain(&rightReparentWidgets,KDockWidget::DockRight);
  findToolViewsDockedToMain(&bottomReparentWidgets,KDockWidget::DockBottom);
  findToolViewsDockedToMain(&topReparentWidgets,KDockWidget::DockTop);

  mainDock->setEnableDocking(KDockWidget::DockNone); //::DockCorner);
  mainDock->setDockSite(KDockWidget::DockCorner);


  KMdiDockContainer *tmpDC;
  m_leftContainer->setWidget(tmpDC=new KMdiDockContainer(m_leftContainer, this, KDockWidget::DockLeft, d->m_styleIDEAlMode));
  m_leftContainer->setEnableDocking(KDockWidget::DockLeft);
  m_leftContainer->manualDock(mainDock, KDockWidget::DockLeft,20);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleLeft()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_rightContainer->setWidget(tmpDC=new KMdiDockContainer(m_rightContainer, this, KDockWidget::DockRight, d->m_styleIDEAlMode));
  m_rightContainer->setEnableDocking(KDockWidget::DockRight);
  m_rightContainer->manualDock(mainDock, KDockWidget::DockRight,80);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleRight()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_topContainer->setWidget(tmpDC=new KMdiDockContainer(m_topContainer, this, KDockWidget::DockTop, d->m_styleIDEAlMode));
  m_topContainer->setEnableDocking(KDockWidget::DockTop);
  m_topContainer->manualDock(mainDock, KDockWidget::DockTop,20);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleTop()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_bottomContainer->setWidget(tmpDC=new KMdiDockContainer(m_bottomContainer, this, KDockWidget::DockBottom, d->m_styleIDEAlMode));
  m_bottomContainer->setEnableDocking(KDockWidget::DockBottom);
  m_bottomContainer->manualDock(mainDock, KDockWidget::DockBottom,80);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleBottom()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_leftContainer->setDockSite( KDockWidget::DockCenter );
  m_rightContainer->setDockSite( KDockWidget::DockCenter );
  m_topContainer->setDockSite( KDockWidget::DockCenter );
  m_bottomContainer->setDockSite( KDockWidget::DockCenter );

  dockToolViewsIntoContainers(leftReparentWidgets,m_leftContainer);
  dockToolViewsIntoContainers(rightReparentWidgets,m_rightContainer);
  dockToolViewsIntoContainers(bottomReparentWidgets,m_bottomContainer);
  dockToolViewsIntoContainers(topReparentWidgets,m_topContainer);


  dockManager->setSpecialLeftDockContainer(m_leftContainer);
  dockManager->setSpecialRightDockContainer(m_rightContainer);
  dockManager->setSpecialTopDockContainer(m_topContainer);
  dockManager->setSpecialBottomDockContainer(m_bottomContainer);


  ((KMdiDockContainer*) (m_leftContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_rightContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_topContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_bottomContainer->getWidget()))->hideIfNeeded();

}



void KMdiMainFrm::finishIDEAlMode(bool full)
{
  // if tabified, release all views from their docking covers
  if (m_mdiMode == KMdi::IDEAlMode) {
    assert(m_pClose);
    m_pClose->hide();
    QObject::disconnect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );


    QStringList leftNames;
    leftNames=prepareIdealToTabs(m_leftContainer);
    int leftWidth=m_leftContainer->width();

    QStringList rightNames;
    rightNames=prepareIdealToTabs(m_rightContainer);
    int rightWidth=m_rightContainer->width();

    QStringList topNames;
    topNames=prepareIdealToTabs(m_topContainer);
    int topHeight=m_topContainer->height();

    QStringList bottomNames;
    bottomNames=prepareIdealToTabs(m_bottomContainer);
    int bottomHeight=m_bottomContainer->height();


    kdDebug(760)<<"leftNames"<<leftNames<<endl;
    kdDebug(760)<<"rightNames"<<rightNames<<endl;
    kdDebug(760)<<"topNames"<<topNames<<endl;
    kdDebug(760)<<"bottomNames"<<bottomNames<<endl;

    delete m_leftContainer;
    m_leftContainer=0;
    delete m_rightContainer;
    m_rightContainer=0;
    delete m_bottomContainer;
    m_bottomContainer=0;
    delete m_topContainer;
    m_topContainer=0;


    idealToolViewsToStandardTabs(bottomNames,KDockWidget::DockBottom,bottomHeight);
    idealToolViewsToStandardTabs(leftNames,KDockWidget::DockLeft,leftWidth);
    idealToolViewsToStandardTabs(rightNames,KDockWidget::DockRight,rightWidth);
    idealToolViewsToStandardTabs(topNames,KDockWidget::DockTop,topHeight);

    QApplication::sendPostedEvents();
  }

}

QStringList KMdiMainFrm::prepareIdealToTabs(KDockWidget* container) {
  KDockContainer* pDW = dynamic_cast<KDockContainer*>(container->getWidget());
  QStringList widgetNames=((KMdiDockContainer*)pDW)->containedWidgets();
  for (QStringList::iterator it=widgetNames.begin();it!=widgetNames.end();++it) {
    KDockWidget* dw = (KDockWidget*) manager()->getDockWidgetFromName(*it);
    dw->undock();
    dw->setLatestKDockContainer(0);
    dw->loseFormerBrotherDockWidget();
  }
  return widgetNames;
}

void KMdiMainFrm::idealToolViewsToStandardTabs(QStringList widgetNames,KDockWidget::DockPosition pos,int size) {
  Q_UNUSED(size)

  KDockWidget *mainDock=getMainDockWidget();
  if (mainDock->parentDockTabGroup()) {
    mainDock=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  if(widgetNames.count()>0) {
    QStringList::iterator it=widgetNames.begin();
    KDockWidget *dwpd=manager()->getDockWidgetFromName(*it);
    if (!dwpd) {
      kdDebug(760)<<"Fatal error in finishIDEAlMode"<<endl;
      return;
    }
    dwpd->manualDock(mainDock,pos,20);
    ++it;
    for (;it!=widgetNames.end();++it) {
      KDockWidget *tmpdw=manager()->getDockWidgetFromName(*it);
      if (!tmpdw) {
        kdDebug(760)<<"Fatal error in finishIDEAlMode"<<endl;
        return;
      }
      tmpdw->manualDock(dwpd,KDockWidget::DockCenter,20);
    }

#if 0
    QWidget *wid=dwpd->parentDockTabGroup();
    if (!wid) wid=dwpd;
    wid->setGeometry(0,0,20,20);
    /*  wid->resize(
        ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?size:wid->width(),
        ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?wid->height():size);
     */
#endif
  }

}


/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
void KMdiMainFrm::setMenuForSDIModeSysButtons( KMenuBar* pMenuBar)
{

}

void KMdiMainFrm::setSysButtonsAtMenuPosition()
{
  if( m_pMainMenuBar == 0L)
    return;
  if( m_pMainMenuBar->parentWidget() == 0L)
    return;

  int menuW = m_pMainMenuBar->parentWidget()->width();
  int h;
  int y;
  if (frameDecorOfAttachedViews() == KMdi::Win95Look)
    h = 16;
  else if (frameDecorOfAttachedViews() == KMdi::KDE1Look)
    h = 20;
  else if (frameDecorOfAttachedViews() == KMdi::KDELook)
    h = 16;
  else
    h = 14;
  y = m_pMainMenuBar->height()/2 - h/2;

  if (frameDecorOfAttachedViews() == KMdi::KDELaptopLook) {
    int w = 27;
    m_pUndock->setGeometry( ( menuW - ( w * 3) - 5), y, w, h);
    m_pMinimize->setGeometry( ( menuW - ( w * 2) - 5), y, w, h);
    m_pRestore->setGeometry( ( menuW - w - 5), y, w, h);
  }
  else {
    m_pUndock->setGeometry( ( menuW - ( h * 4) - 5), y, h, h);
    m_pMinimize->setGeometry( ( menuW - ( h * 3) - 5), y, h, h);
    m_pRestore->setGeometry( ( menuW - ( h * 2) - 5), y, h, h);
    m_pClose->setGeometry( ( menuW - h - 5), y, h, h);
  }
}

/** Activates the next open view */
void KMdiMainFrm::activateNextWin()
{

}

/** Activates the previous open view */
void KMdiMainFrm::activatePrevWin()
{

}

/** Activates the view we accessed the most time ago */
void KMdiMainFrm::activateFirstWin()
{

}

/** Activates the previously accessed view before this one was activated */
void KMdiMainFrm::activateLastWin()
{

}

/** Activates the view with a certain index (TabPage mode only) */
void KMdiMainFrm::activateView(int index)
{

}

/** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
void KMdiMainFrm::setEnableMaximizedChildFrmMode(bool bEnable)
{

}

/** Shows the view taskbar. This should be connected with your "View" menu. */
bool KMdiMainFrm::isViewTaskBarOn()
{
  bool bOn = false;
  return bOn;
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::showViewTaskBar()
{
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::hideViewTaskBar()
{
}

//=============== fillWindowMenu ===============//
void KMdiMainFrm::fillWindowMenu()
{
  bool bTabPageMode = false;
  if (m_mdiMode == KMdi::TabPageMode)
    bTabPageMode = true;
  bool bIDEAlMode = FALSE;
  if (m_mdiMode == KMdi::IDEAlMode)
    bIDEAlMode = TRUE;

  bool bNoViewOpened = false;
  bNoViewOpened = true;

  // construct the menu and its submenus
  if (!m_bClearingOfWindowMenuBlocked) {
    m_pWindowMenu->clear();
  }
  int closeId = m_pWindowMenu->insertItem(i18n("&Close"), this, SLOT(closeActiveView()));
  int closeAllId = m_pWindowMenu->insertItem(i18n("Close &All"), this, SLOT(closeAllViews()));
  if (bNoViewOpened) {
    m_pWindowMenu->setItemEnabled(closeId, false);
    m_pWindowMenu->setItemEnabled(closeAllId, false);
  }
  if (!bTabPageMode && !bIDEAlMode) {
    int iconifyId = m_pWindowMenu->insertItem(i18n("&Minimize All"), this, SLOT(iconifyAllViews()));
    if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(iconifyId, false);
    }
  }
  m_pWindowMenu->insertSeparator();
  m_pWindowMenu->insertItem(i18n("&MDI Mode"), m_pMdiModeMenu);
  m_pMdiModeMenu->clear();
  m_pMdiModeMenu->insertItem(i18n("&Toplevel Mode"), this, SLOT(switchToToplevelMode()));
  m_pMdiModeMenu->insertItem(i18n("C&hildframe Mode"), this, SLOT(switchToChildframeMode()));
  m_pMdiModeMenu->insertItem(i18n("Ta&b Page Mode"), this, SLOT(switchToTabPageMode()));
  m_pMdiModeMenu->insertItem(i18n("I&DEAl Mode"), this, SLOT(switchToIDEAlMode()));
  switch (m_mdiMode) {
    case KMdi::ToplevelMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(0), true);
      break;
    case KMdi::ChildframeMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(1), true);
      break;
    case KMdi::TabPageMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(2), true);
      break;
    case KMdi::IDEAlMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(3),true);
      break;
    default:
      break;
  }
  m_pWindowMenu->insertSeparator();
}

//================ windowMenuItemActivated ===============//

void KMdiMainFrm::windowMenuItemActivated(int id)
{

}

//================ dockMenuItemActivated ===============//

void KMdiMainFrm::dockMenuItemActivated(int id)
{
}

//================ popupWindowMenu ===============//

void KMdiMainFrm::popupWindowMenu(QPoint p)
{
  if (!isFakingSDIApplication()) {
    m_pWindowMenu->popup( p);
  }
}

//================ dragEndTimeOut ===============//
void KMdiMainFrm::dragEndTimeOut()
{

}

//================ setFrameDecorOfAttachedViews ===============//

void KMdiMainFrm::setFrameDecorOfAttachedViews( int frameDecor)
{

}

void KMdiMainFrm::fakeSDIApplication()
{
  m_bSDIApplication = true;
}

void KMdiMainFrm::closeViewButtonPressed()
{

}

void KMdiMainFrm::setManagedDockPositionModeEnabled(bool enabled)
{
  m_managedDockPositionMode=enabled;
}

void KMdiMainFrm::setActiveToolDock(KMdiDockContainer* td) {
  if (td==d->activeDockPriority[0]) return;
  if (d->activeDockPriority[0]==0) {
    d->activeDockPriority[0]=td;
    //        d->focusList=new KMdiFocusList(this);
    //        if (m_pMdi)  d->focusList->addWidgetTree(m_pMdi);
    //        if (m_documentTabWidget) d->focusList->addWidgetTree(m_documentTabWidget);
    return;
  }
  for (int dst=3,src=2;src>=0;dst--,src--) {
    if (d->activeDockPriority[src]==td) src--;
    if (src<0) break;
    d->activeDockPriority[dst]=d->activeDockPriority[src];
  }
  d->activeDockPriority[0]=td;
}

void KMdiMainFrm::removeFromActiveDockList(KMdiDockContainer* td) {
  for (int i=0;i<4;i++) {
    if (d->activeDockPriority[i]==td) {
      for (;i<3;i++)
        d->activeDockPriority[i]=d->activeDockPriority[i+1];
      d->activeDockPriority[3]=0;
      break;
    }
  }
  /*
        if (d->activeDockPriority[0]==0) {
        if (d->focusList) d->focusList->restore();
        delete d->focusList;
        d->focusList=0;
        }
   */
}

void KMdiMainFrm::prevToolViewInDock() {
  KMdiDockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->prevToolView();
}

void KMdiMainFrm::nextToolViewInDock() {
  KMdiDockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->nextToolView();
}

KMdi::TabWidgetVisibility KMdiMainFrm::tabWidgetVisibility()
{
  if ( m_documentTabWidget )
    return m_documentTabWidget->tabWidgetVisibility();

  return KMdi::NeverShowTabs;
}

void KMdiMainFrm::setTabWidgetVisibility( KMdi::TabWidgetVisibility visibility )
{
  if ( m_documentTabWidget )
    m_documentTabWidget->setTabWidgetVisibility( visibility );
}

KTabWidget * KMdiMainFrm::tabWidget() const
{
  return m_documentTabWidget;
}


// vim: ts=2 sw=2 et
// kate: space-indent on; indent-width 2; replace-tabs on;
