//----------------------------------------------------------------------------
//    filename             : k3mdimainfrm.h
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

#ifndef _K3MDIMAINFRM_H_
#define _K3MDIMAINFRM_H_

#include <kparts/dockmainwindow3.h>

#include <kmenubar.h>
#include <kpopupmenu.h>

#include <q3ptrlist.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdom.h>
#include <qpointer.h>

#include "k3mditaskbar.h"
#include "k3mdichildarea.h"
#include "k3mdichildview.h"
#include "k3mdiiterator.h"
#include "k3mdilistiterator.h"
#include "k3mdinulliterator.h"
#include "k3mditoolviewaccessor.h"

class QTimer;
class Q3PopupMenu;
class QMenuBar;


class QToolButton;

namespace K3MDIPrivate
{
class K3MDIGUIClient;
}

class K3MdiDockContainer;
class K3MdiMainFrmPrivate;

/**
 * @short Internal class
 *
 * This special event is needed because the view has to inform the main frame that it`s being closed.
 */
class KDE3SUPPORT_EXPORT K3MdiViewCloseEvent : public QCustomEvent
{
public:
	K3MdiViewCloseEvent( K3MdiChildView* pWnd ) : QCustomEvent( QEvent::User, pWnd ) {}
};

/**
 * \short Base class for all your special main frames.
 *
 * It contains the child frame area (QMainWindow's central widget) and a child view taskbar
 * for switching the MDI views. Most methods are virtual functions for later overriding.
 *
 * Basically, this class provides functionality for docking/undocking view windows and
 * manages the taskbar. Usually a developer will only need to know about this class and
 * \ref K3MdiChildView.
 *
 * \par General usage
 *
 * Your program mainwidget should inherit K3MdiMainFrm. Then usually you'll just need
 * addWindow() and removeWindowFromMdi() to control the views.
 * \code
 *		class MyMainWindow : public K3MdiMainFrm
 *		{ .... };
 * \endcode
 *
 * to define your main window class and
 *
 * \code
 *		MyMainWindow mainframe;
 *		qApp->setMainWidget(&mainframe);
 *		mainframe->addWindow(view1); // put it under MDI control
 *		mainframe->addWindow(view2);
 * \endcode
 *
 * when you wish to use your main window class. The above example also adds a few windows
 * to the frame.
 *
 * K3MdiMainFrm will provide the "Window" menu needed in common MDI applications. Just
 * insert it in your main menu:
 *
 * \code
 * 		if ( !isFakingSDIApplication() )
 * 		{
 * 			menuBar()->insertItem( i18n( "&Window" ), windowMenu() );
 * 		}
 * \endcode
 *
 * To synchronize the positions of the MDI control buttons inserted in your mainmenu:
 * \code
 * 		void B_MainModuleWidget::initMenuBar()
 * 		{
 * 			setMenuForSDIModeSysButtons( menuBar() );
 * 		}
 * 		...
 *		void B_MainModuleWidget::resizeEvent ( QResizeEvent *e )
 *		{
 *			K3MdiMainFrm::resizeEvent( e );
 *			setSysButtonsAtMenuPosition();
 *		}
 * \endcode
 *
 * \par Dynamic mode switching
 *
 * Dynamic switching of the MDI mode can be done via the following functions:
 * - switchToChildframeMode()
 * - switchToToplevelMode()
 * - switchToTabPageMode()
 * - switchToIDEAlMode()
 *
 * The MDI mode can be gotten using mdiMode(). If you need to fake the look of an SDI application
 * use fakeSDIApplication() to fake it and isFakingSDIApplication() to query whether or not an SDI
 * interface is being faked.
 *
 * You can dynamically change the shape of the attached MDI views using setFrameDecorOfAttachedViews().
 *
 * Additionally, here's a hint how to restore the mainframe's settings from config file:
 * \code
 *
 *    // restore MDI mode (toplevel, childframe, tabpage)
 *    int mdiMode = config->readIntEntry( "mainmodule session", "MDI mode", K3Mdi::ChildframeMode);
 *    switch (mdiMode) {
 *    case K3Mdi::ToplevelMode:
 *       {
 *          int childFrmModeHt = config->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
 *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
 *          mainframe->switchToToplevelMode();
 *       }
 *       break;
 *    case K3Mdi::ChildframeMode:
 *       break;
 *    case K3Mdi::TabPageMode:
 *       {
 *          int childFrmModeHt = m_pCfgFileManager->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
 *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
 *          mainframe->switchToTabPageMode();
 *       }
 *       break;
 *    default:
 *       break;
 *    }
 *
 *    // restore a possible maximized Childframe mode
 *    bool maxChildFrmMode = config->readBoolEntry( "mainmodule session", "maximized childframes", true);
 *    mainframe->setEnableMaximizedChildFrmMode(maxChildFrmMode);
 * \endcode
 * The maximized-Childframe mode means that currently all views are maximized in Childframe mode's application desktop.
 *
 * \par Managing views
 *
 * This class provides placing algorithms in Childframe mode. The following is a list of the window placement functions
 * - tilePragma() - Tile the windows and allow them to overlap
 * - tileAnodine() - Tile the windows but don't allow them to overlap
 * - tileVertically() - Tile the windows vertically
 * - cascadeWindows() - cascade windows
 * - cascadeMaximized() - cascade windows and maximize their viewing area
 * - expandVertical() - expand all the windows to use the most amount of vertical space
 * - expandHorizontal() - expand all the windows to use the most amount of horizontal space
 *
 * activateView(K3MdiChildView*) and activateView(int index) set the appropriate MDI child view as the active
 * one. It will be raised, will get an active MDI frame and will get the focus. Call activeView() to find out what the
 * current MDI view is.
 *
 * Use detachWindow() and attachWindow() for docking the MDI views to desktop and back.
 *
 * Connect accels of your program with activatePrevWin(), activateNextWin() and activateView(int index).
 *
 * Note: K3MdiChildViews can be added in 2 meanings: Either as a normal child view (usually containing
 * user document views) or as a tool-view (usually containing status, info or control widgets).
 * The tool-views can be added as floating dockwidgets or as stay-on-top desktop windows in tool style.
 *
 * Also, pay attention to the fact that when you click on the close button of MDI views that their
 * close event should be redirected to closeWindow(). Otherwise the mainframe class will
 * not get noticed about the deleted view and a dangling pointer will remain in the MDI control. The
 * closeWindow() or the removeWindowFromMdi() method is for that issue. The difference is closeWindow()
 * deletes the view object. So if your application wants to control that by itself, call removeWindowFromMdi()
 * and call delete by yourself. See also K3MdiChildView::closeEvent() for that issue.
 *
 * Here's an example how you can suggest things for the adding of views to the MDI control via flags:
 * \code
 *		m_mapOfMdiWidgets.insert( pWnd, mh );
 *		unsigned int mdiFlags = K3Mdi::StandardAdd;
 *
 * 		if ( !show )
 *			mdiFlags |= K3Mdi::Hide;
 *
 * 		if ( !attach )
 *			mdiFlags |= K3Mdi::Detach;
 *
 * 		if ( minimize )
 *			mdiFlags |= K3Mdi::Minimize;
 *
 * 		if ( bToolWindow)
 *			mdiFlags |= K3Mdi::ToolWindow;
 *
 *		if ( m_pMdiMainFrm->isFakingSDIApplication() )
 *		{
 *			if ( attach ) //fake an SDI app
 *				mdiFlags |= K3Mdi::Maximize;
 *		}
 *		else
 *		{
 *			m_pMdiMainFrm->addWindow( pWnd, QPoint(20, 20), K3Mdi::AddWindowFlags(mdiFlags));
 *			return;
 *		}
 *		m_pMdiMainFrm->addWindow( pWnd, K3Mdi::AddWindowFlags(mdiFlags));
 * \endcode
 */
class KDE3SUPPORT_EXPORT K3MdiMainFrm : public KParts::DockMainWindow3
{
	friend class K3MdiChildView;
	friend class K3MdiTaskBar;
	Q_OBJECT

	friend class K3MdiToolViewAccessor;
	// attributes
protected:
	K3Mdi::MdiMode m_mdiMode;
	K3MdiChildArea *m_pMdi;
	K3MdiTaskBar *m_pTaskBar;
	Q3PtrList<K3MdiChildView> *m_pDocumentViews;
	QMap<QWidget*, K3MdiToolViewAccessor*> *m_pToolViews;
	K3MdiChildView *m_pCurrentWindow;
	Q3PopupMenu *m_pWindowPopup;
	Q3PopupMenu *m_pTaskBarPopup;
	Q3PopupMenu *m_pWindowMenu;
	Q3PopupMenu *m_pDockMenu;
	Q3PopupMenu *m_pMdiModeMenu;
	Q3PopupMenu *m_pPlacingMenu;
	KMenuBar *m_pMainMenuBar;

	QPixmap *m_pUndockButtonPixmap;
	QPixmap *m_pMinButtonPixmap;
	QPixmap *m_pRestoreButtonPixmap;
	QPixmap *m_pCloseButtonPixmap;

	QToolButton *m_pUndock;
	QToolButton *m_pMinimize;
	QToolButton *m_pRestore;
	QToolButton *m_pClose;
	QPoint m_undockPositioningOffset;
	bool m_bMaximizedChildFrmMode;
	int m_oldMainFrmHeight;
	int m_oldMainFrmMinHeight;
	int m_oldMainFrmMaxHeight;
	static K3Mdi::FrameDecor m_frameDecoration;
	bool m_bSDIApplication;
	K3DockWidget* m_pDockbaseAreaOfDocumentViews;
	QDomDocument* m_pTempDockSession;
	bool m_bClearingOfWindowMenuBlocked;

	QTimer* m_pDragEndTimer;

	bool m_bSwitching;

	K3DockWidget* m_leftContainer;
	K3DockWidget* m_rightContainer;
	K3DockWidget* m_topContainer;
	K3DockWidget* m_bottomContainer;


private:
	K3MdiMainFrmPrivate* d;
	K3MDIPrivate::K3MDIGUIClient* m_mdiGUIClient;
	bool m_managedDockPositionMode;

	// methods
public:
	K3MdiMainFrm( QWidget* parentWidget, const char* name = "", K3Mdi::MdiMode mdiMode = K3Mdi::ChildframeMode, Qt::WFlags flags = Qt::WType_TopLevel | Qt::WDestructiveClose );
	virtual ~K3MdiMainFrm();

	/**
	 * Control whether or not the standard MDI menu is displayed
	 * when a context menu is displayed
	 */
	void setStandardMDIMenuEnabled( bool showModeMenu = true );

	void setManagedDockPositionModeEnabled( bool enabled );

	/**
	 * Returns whether the application's MDI views are in maximized state or not.
	 */
	bool isInMaximizedChildFrmMode() { return m_bMaximizedChildFrmMode; }

	/**
	 * Returns the MDI mode. This can be one of the enumerations K3Mdi::MdiMode.
	 */
	K3Mdi::MdiMode mdiMode() { return m_mdiMode; }

	/**
	 * Returns the focused attached MDI view.
	 */
	K3MdiChildView* activeWindow();

	/**
	 * Returns a popup menu filled according to the MDI view state. You can override this
	 * method to insert additional entries there. The popup menu is usually popuped when the user
	 * clicks with the right mouse button on a taskbar entry. The default entries are:
	 * Undock/Dock, Restore/Maximize/Minimize, Close and an empty sub-popup ( windowPopup() )
	 * menu called Operations.
	 */
	virtual Q3PopupMenu * taskBarPopup( K3MdiChildView *pWnd, bool bIncludeWindowPopup = false );

	/**
	 * Returns a popup menu with only a title "Window". You can fill it with own operations entries
	 * on the MDI view. This popup menu is inserted as last menu item in taskBarPopup() .
	 */
	virtual Q3PopupMenu * windowPopup( K3MdiChildView *pWnd, bool bIncludeTaskbarPopup = true );

	/**
	 * Called in the constructor (forces a resize of all MDI views)
	 */
	virtual void applyOptions();

	/**
	 * Returns the K3MdiChildView belonging to the given caption string.
	 */
	K3MdiChildView * findWindow( const QString& caption );

	enum ExistsAs {DocumentView, ToolView, AnyView};
	/**
	 * Returns whether this MDI child view is under MDI control (using addWindow() ) or not.
	 */
	bool windowExists( K3MdiChildView *pWnd, ExistsAs as );

	/**
	 * Catches certain Qt events and processes it here.
	 * Currently, here this catches only the K3MdiViewCloseEvent (a K3Mdi user event) which is sent
	 * from itself in childWindowCloseRequest() right after a K3MdiChildView::closeEvent() .
	 * The reason for this event to itself is simple: It just wants to break the function call stack.
	 * It continues the processing with calling closeWindow() .
	 * You see, a close() is translated to a closeWindow() .
	 * It is necessary that the main frame has to start an MDI view close action because it must
	 * remove the MDI view from MDI control, additionally.
	 *
	 * This method calls QMainWindow::event , additionally.
	 */
	virtual bool event( QEvent* e );

	/**
	 * If there's a main menubar given, it will create the 4 maximize mode buttons there (undock, minimize, restore, close).
	 */
	virtual void setSysButtonsAtMenuPosition();

	/**
	 * Returns the height of the taskbar.
	 */
	virtual int taskBarHeight() { return m_pTaskBar ? m_pTaskBar->height() : 0; }

	/**
	 * Sets an offset value that is used on detachWindow() . The undocked window
	 * is visually moved on the desktop by this offset.
	 */
	virtual void setUndockPositioningOffset( QPoint offset ) { m_undockPositioningOffset = offset; }

	/**
	 * If you don't want to know about the inner structure of the K3Mdi system, you can use
	 * this iterator to handle with the MDI view list in a more abstract way.
	 * The iterator hides what special data structure is used in K3Mdi.
	 */
	// FIXME And what exactly are we supposed to fix? -mattr
	K3MdiIterator<K3MdiChildView*>* createIterator()
	{
		if ( m_pDocumentViews == 0L )
		{
			return new K3MdiNullIterator<K3MdiChildView*>();
		}
		else
		{
			return new K3MdiListIterator<K3MdiChildView>( *m_pDocumentViews );
		}
	}

	/**
	 * Deletes an K3MdiIterator created in the K3Mdi library (needed for the windows dll problem).
	 */
	void deleteIterator( K3MdiIterator<K3MdiChildView*>* pIt )
	{
		delete pIt;
	}

	/**
	 * Returns a popup menu that contains the MDI controlled view list.
	 * Additionally, this menu provides some placing actions for these views.
	 * Usually, you insert this popup menu in your main menubar as "Window" menu.
	 */
	Q3PopupMenu* windowMenu() const  { return m_pWindowMenu; };

	/**
	 * Sets a background color for the MDI view area widget.
	 */
	virtual void setBackgroundColor( const QColor &c ) { m_pMdi->setBackgroundColor( c ); }

	/**
	 * Sets a background pixmap for the MDI view area widget.
	 */
	virtual void setBackgroundPixmap( const QPixmap &pm ) { m_pMdi->setBackgroundPixmap( pm ); }

	/**
	 * Sets a size that is used as the default size for a newly to the MDI system added K3MdiChildView .
	 *  By default this size is 600x400. So all non-resized added MDI views appear in that size.
	 */
	void setDefaultChildFrmSize( const QSize& sz ) { m_pMdi->m_defaultChildFrmSize = sz; }

	/**
	 * Returns the default size for a newly added K3MdiChildView. See setDefaultChildFrmSize() .
	 */
	QSize defaultChildFrmSize() { return m_pMdi->m_defaultChildFrmSize; }

	/**
	 * Do nothing when in Toplevel mode
	 */
	virtual void setMinimumSize( int minw, int minh );

	/**
	 * Returns the Childframe mode height of this. Makes only sense when in Toplevel mode.
	 */
	int childFrameModeHeight() { return m_oldMainFrmHeight; };
	/**
	 * Tells the MDI system a QMenu where it can insert buttons for
	 * the system menu, undock, minimize, restore actions.
	 * If no such menu is given, K3Mdi simply overlays the buttons
	 * at the upper right-hand side of the main widget.
	 */
	virtual void setMenuForSDIModeSysButtons( KMenuBar* menuBar = 0 );

	/**
	 * @return the decoration of the window frame of docked (attached) MDI views
	 */
	static int frameDecorOfAttachedViews() { return m_frameDecoration; }

	/**
	 * An SDI application user interface is faked:
	 * @li an opened view is always maximized
	 * @li buttons for maximized childframe mode aren't inserted in the main menubar
	 * @li taskbar and windowmenu are not created/updated
	 */
	void fakeSDIApplication();

	/**
	 * @returns if we are faking an SDI application (fakeSDIApplication())
	 */
	bool isFakingSDIApplication() const { return m_bSDIApplication; }

	virtual bool eventFilter( QObject *obj, QEvent *e );
	void findRootDockWidgets( Q3PtrList<K3DockWidget>* pRootDockWidgetList, Q3ValueList<QRect>* pPositionList );

	/** We're switching something.*/
	void setSwitching( const bool switching ) { m_bSwitching = switching; }
	bool switching( void ) const { return m_bSwitching; }

public slots:
	/**
	 * addWindow demands a K3MdiChildView. This method wraps every QWidget in such an object and
	 * this way you can put every widget under MDI control.
	 */
	K3MdiChildView* createWrapper( QWidget *view, const QString& name, const QString& shortName );

	/**
	 * Adds a K3MdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or added as document-type view.
	 */
	virtual void addWindow( K3MdiChildView* pWnd, int flags = K3Mdi::StandardAdd );

	//KDE4: merge the two methods
	/**
	 * Adds a K3MdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param flags
	 * You can specify here whether:
	 * \li the view should be attached or detached.
	 * \li shown or hidden
	 * \li maximized, minimized or restored (normalized)
	 * \li added as tool view (stay-on-top and toplevel) or added as
	 * document-type view.
	 * \param index the index of the tab we should insert the new tab after.
	 * If index == -1 then the tab will just be appended to the end.
	 * Using this parameter in childview mode has no effect.
	 * \since 3.3
	 */
	void addWindow( K3MdiChildView* pWnd, int flags, int index );

	/**
	 * Adds a K3MdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param pos move the child view to the specified position
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or
	 * added as document-type view.
	 */
	virtual void addWindow( K3MdiChildView* pWnd, QPoint pos, int flags = K3Mdi::StandardAdd );

	/**
	 * Adds a K3MdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param rectNormal Sets the geometry for this child view
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or
	 * added as document-type view.
	 */
	virtual void addWindow( K3MdiChildView* pWnd, QRect rectNormal, int flags = K3Mdi::StandardAdd );

	/**
	 * Usually called from addWindow() when adding a tool view window. It reparents the given widget
	 * as toplevel and stay-on-top on the application's main widget.
	 */
	virtual K3MdiToolViewAccessor *addToolWindow( QWidget* pWnd, K3DockWidget::DockPosition pos = K3DockWidget::DockNone,
	                                             QWidget* pTargetWnd = 0L, int percent = 50, const QString& tabToolTip = 0,
	                                             const QString& tabCaption = 0 );

	virtual void deleteToolWindow( QWidget* pWnd );
	virtual void deleteToolWindow( K3MdiToolViewAccessor *accessor );

	/**
	 * Using this method you have to use the setWidget method of the access object, and it is very recommendet, that you use
	 * the widgetContainer() method for the parent of your newly created widget
	 */
	K3MdiToolViewAccessor *createToolWindow();

	/**
	 * Removes a K3MdiChildView from the MDI system and from the main frame`s control.
	 * The caller is responsible for deleting the view. If the view is not deleted it will
	 * be reparented to 0
	 */
	virtual void removeWindowFromMdi( K3MdiChildView *pWnd );

	/**
	 * Removes a K3MdiChildView from the MDI system and from the main frame`s control.
	 * Note: The view will be deleted!
	 */
	virtual void closeWindow( K3MdiChildView *pWnd, bool layoutTaskBar = true );

	/**
	 * Switches the K3MdiTaskBar on and off.
	 */
	virtual void slot_toggleTaskBar();

	/**
	 * Makes a main frame controlled undocked K3MdiChildView docked.
	 * Doesn't work on K3MdiChildView which aren't added to the MDI system.
	 * Use addWindow() for that.
	 */
	virtual void attachWindow( K3MdiChildView *pWnd, bool bShow = true, bool bAutomaticResize = false );

	/**
	 * Makes a docked K3MdiChildView undocked.
	 * The view window still remains under the main frame's MDI control.
	 */
	virtual void detachWindow( K3MdiChildView *pWnd, bool bShow = true );

	/**
	 * Someone wants that the MDI view to be closed. This method sends a K3MdiViewCloseEvent to itself
	 * to break the function call stack. See also event() .
	 */
	virtual void childWindowCloseRequest( K3MdiChildView *pWnd );

	/**
	 * Close all views
	 */
	virtual void closeAllViews();

	/**
	 * Iconfiy all views
	 */
	virtual void iconifyAllViews();

	/**
	 * Closes the view of the active (topchild) window
	 */
	virtual void closeActiveView();

	/**
	 * Undocks all view windows (unix-like)
	 */
	virtual void switchToToplevelMode();
	virtual void finishToplevelMode();

	/**
	 * Docks all view windows (Windows-like)
	 */
	virtual void switchToChildframeMode();
	virtual void finishChildframeMode();

	/**
	 * Docks all view windows (Windows-like)
	 */
	virtual void switchToTabPageMode();
	virtual void finishTabPageMode();

	/**
	 * Docks all view windows. Toolviews use dockcontainers
	 */
	virtual void switchToIDEAlMode();
	virtual void finishIDEAlMode( bool full = true );

	/**
	 * Sets the appearance of the IDEAl mode. See KMultiTabBar styles for the first 3 bits.
	 * @deprecated use setToolviewStyle(int flags) instead
	 */
#warning remove setIDEAlModeStyle
	void setIDEAlModeStyle( int flags );
	//KDE4: Get rid of the above.
	/**
	 * Sets the appearance of the toolview tabs.
	 * @param flags See K3Mdi::ToolviewStyle.
	 * @since 3.3
	 */
	void setToolviewStyle( int flags );
	/**
	 * @return if the view taskbar should be shown if there are MDI views
	 */
	bool isViewTaskBarOn();

	/**
	 * Shows the view taskbar. This should be connected with your "View" menu.
	 */
	virtual void showViewTaskBar();

	/**
	 * Hides the view taskbar. This should be connected with your "View" menu.
	 */
	virtual void hideViewTaskBar();

	/**
	 * Update of the window menu contents.
	 */
	virtual void fillWindowMenu();

	/**
	 * Cascades the windows without resizing them.
	 */
	virtual void cascadeWindows() { m_pMdi->cascadeWindows(); }

	/**
	 * Cascades the windows resizing them to the maximum available size.
	 */
	virtual void cascadeMaximized() { m_pMdi->cascadeMaximized(); }

	/**
	 * Maximizes only in vertical direction.
	 */
	virtual void expandVertical() { m_pMdi->expandVertical(); }

	/**
	* Maximizes only in horizontal direction.
	*/
	virtual void expandHorizontal() { m_pMdi->expandHorizontal(); }

	/**
	 * Tile Pragma
	 */
	virtual void tilePragma() { m_pMdi->tilePragma(); }

	/**
	 * Tile Anodine
	 */
	virtual void tileAnodine() { m_pMdi->tileAnodine(); }

	/**
	 * Tile Vertically
	 */
	virtual void tileVertically() { m_pMdi->tileVertically(); }

	/**
	 * Sets the decoration of the window frame of docked (attached) MDI views
	 * @deprecated Will be removed in KDE 4
	 */
	virtual void setFrameDecorOfAttachedViews( int frameDecor );

	/**
	 * If in Childframe mode, we can switch between maximized or restored shown MDI views
	 */
	virtual void setEnableMaximizedChildFrmMode( bool bEnable );

	/**
	 * Activates the next open view
	 */
	virtual void activateNextWin();

	/**
	 * Activates the previous open view
	 */
	virtual void activatePrevWin();

	/**
	 * Activates the view first viewed concerning to the access time.
	 */
	virtual void activateFirstWin();

	/**
	 * Activates the view last viewed concerning to the access time.
	 */
	virtual void activateLastWin();

	/**
	 * Activates the view with the tab page index (TabPage mode only)
	 */
	virtual void activateView( int index );

private:
	void setupToolViewsForIDEALMode();
	void setupTabbedDocumentViewSpace();
	class K3MdiDocumentViewTabWidget * m_documentTabWidget;

protected:

	virtual void resizeEvent( QResizeEvent * );

	/**
	 * Creates a new MDI taskbar (showing the MDI views as taskbar entries) and shows it.
	 */
	virtual void createTaskBar();

	/**
	 * Creates the MDI view area and connects some signals and slots with the K3MdiMainFrm widget.
	 */
	virtual void createMdiManager();

	/**
	 * prevents fillWindowMenu() from m_pWindowMenu->clear(). You have to care for it by yourself.
	 * This is useful if you want to add some actions in your overridden fillWindowMenu() method.
	 */
	void blockClearingOfWindowMenu( bool bBlocked ) { m_bClearingOfWindowMenuBlocked = bBlocked; }

	void findToolViewsDockedToMain( Q3PtrList<K3DockWidget>* list, K3DockWidget::DockPosition dprtmw );
	void dockToolViewsIntoContainers( Q3PtrList<K3DockWidget>& widgetsToReparent, K3DockWidget *container );
	QStringList prepareIdealToTabs( K3DockWidget* container );
	void idealToolViewsToStandardTabs( QStringList widgetNames, K3DockWidget::DockPosition pos, int sizee );

	/** Get tabwidget visibility */
	K3Mdi::TabWidgetVisibility tabWidgetVisibility();

	/** Set tabwidget visibility */
	void setTabWidgetVisibility( K3Mdi::TabWidgetVisibility );

	/** Returns the tabwidget used in IDEAl and Tabbed modes. Returns 0 in other modes. */
	class KTabWidget * tabWidget() const;


protected slots:  // Protected slots
	/**
	 * Sets the focus to this MDI view, raises it, activates its taskbar button and updates
	 * the system buttons in the main menubar when in maximized (Maximize mode).
	 */
	virtual void activateView( K3MdiChildView *pWnd );

	/**
	 * Activates the MDI view (see activateView() ) and popups the taskBar popup menu (see taskBarPopup() ).
	 */
	virtual void taskbarButtonRightClicked( K3MdiChildView *pWnd );

	/**
	 * Turns the system buttons for maximize mode (SDI mode) off, and disconnects them
	 */
	void switchOffMaximizeModeForMenu( K3MdiChildFrm* oldChild );

	/**
	 * Reconnects the system buttons form maximize mode (SDI mode) with the new child frame
	 */
	void updateSysButtonConnections( K3MdiChildFrm* oldChild, K3MdiChildFrm* newChild );

	/**
	 * Usually called when the user clicks an MDI view item in the "Window" menu.
	 */
	void windowMenuItemActivated( int id );

	/**
	 * Usually called when the user clicks an MDI view item in the sub-popup menu "Docking" of the "Window" menu.
	 */
	void dockMenuItemActivated( int id );

	/**
	 * Popups the "Window" menu. See also windowPopup() .
	 */
	void popupWindowMenu( QPoint p );

	/**
	 * The timer for main widget moving has elapsed -> send drag end to all concerned views.
	 */
	void dragEndTimeOut();

	/**
	 * internally used to handle click on view close button (TabPage mode, only)
	 */
	void closeViewButtonPressed();

signals:
	/**
	 * Signals the last attached K3MdiChildView has been closed
	 */
	void lastChildFrmClosed();

	/**
	 * Signals the last K3MdiChildView (that is under MDI control) has been closed
	 */
	void lastChildViewClosed();

	/**
	 * Signals that the Toplevel mode has been left
	 */
	void leftTopLevelMode();

	/**
	 * Signals that a child view has been detached (undocked to desktop)
	 */
	void childViewIsDetachedNow( QWidget* );

	/** Signals we need to collapse the overlapped containers */
	void collapseOverlapContainers();

	/** Signals the MDI mode has been changed */
	void mdiModeHasBeenChangedTo( K3Mdi::MdiMode );

	void viewActivated( K3MdiChildView* );
	void viewDeactivated( K3MdiChildView* );

public slots:
	void prevToolViewInDock();
	void nextToolViewInDock();

private slots:
	void setActiveToolDock( K3MdiDockContainer* );
	void removeFromActiveDockList( K3MdiDockContainer* );
	void slotDocCurrentChanged( QWidget* );
	void verifyToplevelHeight();
#ifdef Q_MOC_RUN
#define public_signals signals
#else
#define public_signals public
#endif
signals:


	void toggleTop();
	void toggleLeft();
	void toggleRight();
	void toggleBottom();
};

#endif //_K3MDIMAINFRM_H_

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
