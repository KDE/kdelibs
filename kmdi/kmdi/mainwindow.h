//----------------------------------------------------------------------------
//    filename             : MainWindow.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           2000-2003     maintained by the KDevelop project
//    patches              : */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
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

#ifndef _KMDI_MAINWINDOW_H_
#define _KMDI_MAINWINDOW_H_

#include <kparts/dockmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>

#include <qptrlist.h>
#include <qrect.h>
#include <qapplication.h>
#include <qguardedptr.h>

#include <kmdi/global.h>

class QTimer;
class QPopupMenu;
class QMenuBar;

#include <kmdi/toolviewaccessor.h>

namespace KMDIPrivate {
  class KMDIGUIClient;
}

class KMdiDockContainer;
class MainWindowPrivate;

namespace KMDI
{

class MainWindow : public KParts::DockMainWindow
{
   Q_OBJECT

   friend class KMDI::ToolViewAccessor;

// attributes
protected:
   QMap<QWidget*,KMDI::ToolViewAccessor*> *m_pToolViews;
   QPopupMenu              *m_pWindowPopup;
   QPopupMenu              *m_pTaskBarPopup;
   QPopupMenu              *m_pDockMenu;
   QPopupMenu              *m_pMdiModeMenu;
   QPopupMenu              *m_pPlacingMenu;

   QPoint                  m_undockPositioningOffset;
   bool                    m_bMaximizedChildFrmMode;
   bool                    m_bSDIApplication;
   KDockWidget*         m_pDockbaseAreaOfDocumentViews;
   bool                    m_bClearingOfWindowMenuBlocked;

   QTimer*                 m_pDragEndTimer;

   KDockWidget*         m_leftContainer;
   KDockWidget*         m_rightContainer;
   KDockWidget*         m_topContainer;
   KDockWidget*         m_bottomContainer;


private:
   MainWindowPrivate*     d;
   KMDIPrivate::KMDIGUIClient*     m_mdiGUIClient;
   bool m_managedDockPositionMode;

// methods
public:
   /**
   * Constructor.
   */
   MainWindow( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel | WDestructiveClose);

   /**
   * Destructor.
   */
   virtual ~MainWindow();

   void setStandardMDIMenuEnabled(bool showModeMenu=true);

   void setManagedDockPositionModeEnabled(bool enabled);

   /**
   * Returns whether this MDI child view is under MDI control (using addWindow() ) or not.
   */
   enum ExistsAs {DocumentView,ToolView,AnyView};

   /**
   * Sets an offset value that is used on detachWindow() . The undocked window
   * is visually moved on the desktop by this offset.
   */
   virtual void setUndockPositioningOffset( QPoint offset) { m_undockPositioningOffset = offset; };

   /**
   * Do nothing when in Toplevel mode
   */
   virtual void setMinimumSize( int minw, int minh);

public slots:
   /**
   * Usually called from addWindow() when adding a tool view window. It reparents the given widget
   * as toplevel and stay-on-top on the application's main widget.
   */
   virtual KMDI::ToolViewAccessor *addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L, int percent = 50, const QString& tabToolTip = 0, const QString& tabCaption = 0);
   virtual void deleteToolWindow( QWidget* pWnd);
   virtual void deleteToolWindow( KMDI::ToolViewAccessor *accessor);
   /**
    * Using this method you have to use the setWidget method of the access object, and it is very recommendet, that you use
    * the widgetContainer() method for the parent of your newly created widget
    */
   KMDI::ToolViewAccessor *createToolWindow();
   /**
    * Sets the appearance of the IDEAl mode. See KMultiTabBar styles for the first 3 bits.
    * @deprecated use setToolviewStyle(int flags) instead
    */
   void setIDEAlModeStyle(int flags);
   //KDE4: Get rid of the above.
   /**
    * Sets the appearance of the toolview tabs.
    * @param flags See KMDI::ToolviewStyle.
    * @since 3.3
    */
   void setToolviewStyle(int flags);

private:
   void setupToolViewsForIDEALMode();
   void setupTabbedDocumentViewSpace();

protected:
   /**
   * prevents fillWindowMenu() from m_pWindowMenu->clear(). You have to care for it by yourself.
   * This is useful if you want to add some actions in your overridden fillWindowMenu() method.
   */
   void blockClearingOfWindowMenu( bool bBlocked) { m_bClearingOfWindowMenuBlocked = bBlocked; };

   void findToolViewsDockedToMain(QPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw);
   void dockToolViewsIntoContainers(QPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container);

protected slots: // Protected slots
   signals:
   /**
   * Signals the last attached KMdiChildView has been closed
   */
   void lastChildFrmClosed();
   /**
   * Signals the last KMdiChildView (that is under MDI control) has been closed
   */
   void lastChildViewClosed();
   /**
   * Signals that the Toplevel mode has been left
   */
   void leftTopLevelMode();
   /**
   * Signals that a child view has been detached (undocked to desktop)
   */
   void childViewIsDetachedNow(QWidget*);

   void collapseOverlapContainers();

public slots:
   void prevToolViewInDock();
   void nextToolViewInDock();

private slots:
   void setActiveToolDock(KMdiDockContainer*);
   void removeFromActiveDockList(KMdiDockContainer*);

#define protected public
signals:
#undef protected
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();
};

}

#endif //_MainWindow_H_

// kate: space-indent on; indent-width 2; replace-tabs on;
