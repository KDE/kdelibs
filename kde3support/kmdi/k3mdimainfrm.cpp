//----------------------------------------------------------------------------
//    filename             : k3mdimainfrm.cpp
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
#include <qobject.h>
#include <q3popupmenu.h>
#include <qmenubar.h>

#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qtabwidget.h>
#include <klocale.h>
#include <kxmlguifactory.h>

#include <kiconloader.h>
#include <k3mdidockcontainer.h>


#include <qtoolbutton.h>
#include <q3dockarea.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qmap.h>
#include <q3valuelist.h>
#include <QResizeEvent>
#include <QFocusEvent>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

#include "k3mdimainfrm.h"
#include "k3mditaskbar.h"
#include "k3mdichildfrm.h"
#include "k3mdichildarea.h"
#include "k3mdichildview.h"
#include "k3mdidockcontainer.h"
#include "k3mditoolviewaccessor_p.h"
#include "k3mdifocuslist.h"
#include "k3mdidocumentviewtabwidget.h"
#include "k3mdiguiclient.h"

#include "win_undockbutton.xpm"
#include "win_minbutton.xpm"
#include "win_restorebutton.xpm"
#include "win_closebutton.xpm"
#include "kde_undockbutton.xpm"
#include "kde_minbutton.xpm"
#include "kde_restorebutton.xpm"
#include "kde_closebutton.xpm"
#include "kde2_undockbutton.xpm"
#include "kde2_minbutton.xpm"
#include "kde2_restorebutton.xpm"
#include "kde2_closebutton.xpm"
#include "kde2laptop_undockbutton.xpm"
#include "kde2laptop_minbutton.xpm"
#include "kde2laptop_restorebutton.xpm"
#include "kde2laptop_closebutton.xpm"
#include "kde2laptop_closebutton_menu.xpm"

#ifdef Q_WS_X11
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

K3Mdi::FrameDecor K3MdiMainFrm::m_frameDecoration = K3Mdi::KDELook;

class K3MdiMainFrmPrivate
{
public:
	K3MdiMainFrmPrivate() : focusList( 0 )
	{
		for ( int i = 0;i < 4;i++ )
		{
			activeDockPriority[ i ] = 0;
			m_styleIDEAlMode = 0;
			m_toolviewStyle = 0;
		}
	}
	~K3MdiMainFrmPrivate()
	{}
	K3MdiDockContainer* activeDockPriority[ 4 ];
	K3MdiFocusList *focusList;
	int m_styleIDEAlMode;
	int m_toolviewStyle;
	KAction *closeWindowAction;
};

//============ constructor ============//
K3MdiMainFrm::K3MdiMainFrm( QWidget* parentWidget, const char* name, K3Mdi::MdiMode mdiMode, Qt::WFlags flags )
		: KParts::DockMainWindow3( parentWidget, name, flags )
		, m_mdiMode( K3Mdi::UndefinedMode )
		, m_pMdi( 0L )
		, m_pTaskBar( 0L )
		, m_pDocumentViews( 0L )
		, m_pCurrentWindow( 0L )
		, m_pWindowPopup( 0L )
		, m_pTaskBarPopup( 0L )
		, m_pWindowMenu( 0L )
		, m_pDockMenu( 0L )
		, m_pMdiModeMenu( 0L )
		, m_pPlacingMenu( 0L )
		, m_pMainMenuBar( 0L )
		, m_pUndockButtonPixmap( 0L )
		, m_pMinButtonPixmap( 0L )
		, m_pRestoreButtonPixmap( 0L )
		, m_pCloseButtonPixmap( 0L )
		, m_pUndock( 0L )
		, m_pMinimize( 0L )
		, m_pRestore( 0L )
		, m_pClose( 0L )
		, m_bMaximizedChildFrmMode( false )
		, m_oldMainFrmHeight( 0 )
		, m_oldMainFrmMinHeight( 0 )
		, m_oldMainFrmMaxHeight( 0 )
		, m_bSDIApplication( false )
		, m_pDockbaseAreaOfDocumentViews( 0L )
		, m_pTempDockSession( 0L )
		, m_bClearingOfWindowMenuBlocked( false )
		, m_pDragEndTimer( 0L )
		, m_bSwitching( false )
		, m_leftContainer( 0 )
		, m_rightContainer( 0 )
		, m_topContainer( 0 )
		, m_bottomContainer( 0 )
		, d( new K3MdiMainFrmPrivate() )
		, m_mdiGUIClient( 0 )
		, m_managedDockPositionMode( false )
		, m_documentTabWidget( 0 )
{
	kdDebug(760) << k_funcinfo << endl;
	// Create the local lists of windows
	m_pDocumentViews = new Q3PtrList<K3MdiChildView>;
	m_pDocumentViews->setAutoDelete( false );
	m_pToolViews = new QMap<QWidget*, K3MdiToolViewAccessor*>;

	// This seems to be needed (re-check it after Qt2.0 comed out)
	setFocusPolicy( Qt::ClickFocus );

	// create the central widget
	createMdiManager();

	// cover K3Mdi's childarea by a dockwidget
	m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover" );
	m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
	m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
	m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockCorner );
	m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
	// set this dock to main view
	setView( m_pDockbaseAreaOfDocumentViews );
	setMainDockWidget( m_pDockbaseAreaOfDocumentViews );

	// Apply options for the MDI manager
	applyOptions();

	m_pTaskBarPopup = new Q3PopupMenu( this, "taskbar_popup_menu" );
	m_pWindowPopup = new Q3PopupMenu( this, "window_popup_menu" );

	m_pWindowMenu = new Q3PopupMenu( this, "window_menu" );
	m_pWindowMenu->setCheckable( true );
	QObject::connect( m_pWindowMenu, SIGNAL( aboutToShow() ), this, SLOT( fillWindowMenu() ) );

	m_pDockMenu = new Q3PopupMenu( this, "dock_menu" );
	m_pDockMenu->setCheckable( true );

	m_pMdiModeMenu = new Q3PopupMenu( this, "mdimode_menu" );
	m_pMdiModeMenu->setCheckable( true );

	m_pPlacingMenu = new Q3PopupMenu( this, "placing_menu" );

	d->closeWindowAction = new KAction(i18n("&Close"),
#ifdef Q_WS_WIN
		Qt::CTRL|Qt::Key_F4,
#else
		0,
#endif
		this, SLOT(closeActiveView()), actionCollection(), "window_close");

	// the MDI view taskbar
	createTaskBar();

	// this is only a hack, but prevents us from crash because the buttons are otherwise
	// not created before we switch the modes where we need them !!!
	setMenuForSDIModeSysButtons( menuBar() );

	switch ( mdiMode )
	{
	case K3Mdi::IDEAlMode:
		kdDebug(760) << k_funcinfo << "Switching to IDEAl mode" << endl;
		switchToIDEAlMode();
		break;
	case K3Mdi::TabPageMode:
		kdDebug(760) << k_funcinfo << "Switching to tab page mode" << endl;
		switchToTabPageMode();
		break;
	case K3Mdi::ToplevelMode:
		kdDebug(760) << k_funcinfo << "Switching to top level mode" << endl;
		switchToToplevelMode();
		break;
	default:
		m_mdiMode = K3Mdi::ChildframeMode;
		kdDebug(760) << k_funcinfo << "Switching to child frame mode" << endl;
		break;
	}

	// drag end timer
	m_pDragEndTimer = new QTimer();
	connect( m_pDragEndTimer, SIGNAL( timeout() ), this, SLOT( dragEndTimeOut() ) );
	connect( guiFactory(), SIGNAL( clientAdded( KXMLGUIClient* ) ),
	         this, SLOT( verifyToplevelHeight() ) );
	connect( guiFactory(), SIGNAL( clientRemoved( KXMLGUIClient* ) ),
	         this, SLOT( verifyToplevelHeight() ) );
}

void K3MdiMainFrm::verifyToplevelHeight()
{
	if ( m_mdiMode != K3Mdi::ToplevelMode )
		return;
	
	//kdDebug(760) << k_funcinfo << endl;
	int topDockHeight = topDock() ? topDock()->height() : 0;
	int menuBarHeight = hasMenuBar() ? menuBar()->height() : 0;
	setFixedHeight( topDockHeight + menuBarHeight );
	resize( width(), height() );
}

void K3MdiMainFrm::setStandardMDIMenuEnabled( bool showModeMenu )
{
	m_mdiGUIClient = new K3MDIPrivate::K3MDIGUIClient( this, showModeMenu );
	connect( m_mdiGUIClient, SIGNAL( toggleTop() ), this, SIGNAL( toggleTop() ) );
	connect( m_mdiGUIClient, SIGNAL( toggleLeft() ), this, SIGNAL( toggleLeft() ) );
	connect( m_mdiGUIClient, SIGNAL( toggleRight() ), this, SIGNAL( toggleRight() ) );
	connect( m_mdiGUIClient, SIGNAL( toggleBottom() ), this, SIGNAL( toggleBottom() ) );

	if ( m_mdiMode == K3Mdi::IDEAlMode )
	{
		if ( m_topContainer )
			connect( this, SIGNAL( toggleTop() ), m_topContainer->getWidget(), SLOT( toggle() ) );
		if ( m_leftContainer )
			connect( this, SIGNAL( toggleLeft() ), m_leftContainer->getWidget(), SLOT( toggle() ) );
		if ( m_rightContainer )
			connect( this, SIGNAL( toggleRight() ), m_rightContainer->getWidget(), SLOT( toggle() ) );
		if ( m_bottomContainer )
			connect( this, SIGNAL( toggleBottom() ), m_bottomContainer->getWidget(), SLOT( toggle() ) );
	}

	emit mdiModeHasBeenChangedTo( m_mdiMode );
}

//============ ~K3MdiMainFrm ============//
K3MdiMainFrm::~K3MdiMainFrm()
{
	//save the children first to a list, as removing invalidates our iterator
	Q3ValueList<K3MdiChildView*> children;
	for ( K3MdiChildView * w = m_pDocumentViews->first();w;w = m_pDocumentViews->next() )
		children.append( w );

	// safely close the windows so properties are saved...
	Q3ValueListIterator<K3MdiChildView*> childIt;
	for ( childIt = children.begin(); childIt != children.end(); ++childIt )
	{
		closeWindow( *childIt, false ); // without re-layout taskbar!
	}

	emit lastChildViewClosed();
	delete m_pDocumentViews;
	delete m_pToolViews;
	m_pToolViews = 0;
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
	delete m_pTempDockSession;
	m_mdiGUIClient = 0;
	delete d;
	d = 0;
}

//============ applyOptions ============//
//FIXME something wrong with this function. dunno what though
void K3MdiMainFrm::applyOptions()
{
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		QWidget* childFrame = 0L;
		if ( ( *it )->mdiParent() )
		{
			kdDebug(760) << k_funcinfo << "using child view's mdi parent for resize hack" << endl;
			childFrame = ( *it )->mdiParent();
		}
		else
		{
			kdDebug(760) << k_funcinfo << "using child view for resize hack" << endl;
			childFrame = ( *it );
		}
		
		int w = childFrame->width();
		int h = childFrame->height();
		childFrame->resize( w + 1, h + 1 );
		childFrame->resize( w - 1, h - 1 );
	}
}

//============ createMdiManager ============//
void K3MdiMainFrm::createMdiManager()
{
	kdDebug(760) << k_funcinfo << "creating MDI manager" << endl;
	m_pMdi = new K3MdiChildArea( this );
	setCentralWidget( m_pMdi );
	QObject::connect( m_pMdi, SIGNAL( nowMaximized( bool ) ),
	                  this, SLOT( setEnableMaximizedChildFrmMode( bool ) ) );
	QObject::connect( m_pMdi, SIGNAL( noMaximizedChildFrmLeft( K3MdiChildFrm* ) ),
	                  this, SLOT( switchOffMaximizeModeForMenu( K3MdiChildFrm* ) ) );
	QObject::connect( m_pMdi, SIGNAL( sysButtonConnectionsMustChange( K3MdiChildFrm*, K3MdiChildFrm* ) ),
	                  this, SLOT( updateSysButtonConnections( K3MdiChildFrm*, K3MdiChildFrm* ) ) );
	QObject::connect( m_pMdi, SIGNAL( popupWindowMenu( QPoint ) ),
	                  this, SLOT( popupWindowMenu( QPoint ) ) );
	QObject::connect( m_pMdi, SIGNAL( lastChildFrmClosed() ),
	                  this, SIGNAL( lastChildFrmClosed() ) );
}

//============ createTaskBar ==============//
void K3MdiMainFrm::createTaskBar()
{
	m_pTaskBar = new K3MdiTaskBar( this, Qt::DockBottom );
	m_pTaskBar->installEventFilter( this );
}

void K3MdiMainFrm::slot_toggleTaskBar()
{
	if ( !m_pTaskBar )
		return;
	m_pTaskBar->switchOn( !m_pTaskBar->isSwitchedOn() );
}

void K3MdiMainFrm::resizeEvent( QResizeEvent *e )
{
	if ( ( m_mdiMode == K3Mdi::ToplevelMode ) && !parentWidget() )
	{
		if ( e->oldSize().height() != e->size().height() )
			return ;
	}
	KParts::DockMainWindow3::resizeEvent( e );
	if ( !m_mdiGUIClient )
		return ;
	setSysButtonsAtMenuPosition();
}

//================ setMinimumSize ===============//

void K3MdiMainFrm::setMinimumSize( int minw, int minh )
{
	if ( ( m_mdiMode == K3Mdi::ToplevelMode ) && !parentWidget() )
		return ;
	DockMainWindow3::setMinimumSize( minw, minh );
}

//================ wrapper ===============//

K3MdiChildView* K3MdiMainFrm::createWrapper( QWidget *view, const QString& name, const QString& shortName )
{
	Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

	K3MdiChildView* pMDICover = new K3MdiChildView( name /*caption*/, 0L /*parent*/,
	                                              name.latin1() );
	QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout" );
	view->reparent( pMDICover, QPoint( 0, 0 ) );
	pLayout->addWidget( view );
	//  pMDICover->setName(name);
	pMDICover->setTabCaption( shortName );
	pMDICover->setCaption( name );

	const QPixmap* wndIcon = view->icon();
	if ( wndIcon )
		pMDICover->setIcon( *wndIcon );

	pMDICover->trackIconAndCaptionChanges( view );
	return pMDICover;
}

//================ addWindow ===============//

void K3MdiMainFrm::addWindow( K3MdiChildView* pWnd, int flags )
{
	addWindow( pWnd, flags, -1 );
}

void K3MdiMainFrm::addWindow( K3MdiChildView* pWnd, int flags, int index )
{
	if ( windowExists( pWnd, AnyView ) ) //already added
		return;

	if ( flags & K3Mdi::ToolWindow )
	{
		addToolWindow( pWnd );
		// some kind of cascading
		pWnd->move( m_pMdi->mapToGlobal( m_pMdi->getCascadePoint() ) );

		return ;
	}

	d->closeWindowAction->setEnabled(true);

	// common connections used when under MDI control
	QObject::connect( pWnd, SIGNAL( clickedInWindowMenu( int ) ), this, SLOT( windowMenuItemActivated( int ) ) );
	QObject::connect( pWnd, SIGNAL( focusInEventOccurs( K3MdiChildView* ) ), this, SLOT( activateView( K3MdiChildView* ) ) );
	QObject::connect( pWnd, SIGNAL( childWindowCloseRequest( K3MdiChildView* ) ), this, SLOT( childWindowCloseRequest( K3MdiChildView* ) ) );
	QObject::connect( pWnd, SIGNAL( attachWindow( K3MdiChildView*, bool ) ), this, SLOT( attachWindow( K3MdiChildView*, bool ) ) );
	QObject::connect( pWnd, SIGNAL( detachWindow( K3MdiChildView*, bool ) ), this, SLOT( detachWindow( K3MdiChildView*, bool ) ) );
	QObject::connect( pWnd, SIGNAL( clickedInDockMenu( int ) ), this, SLOT( dockMenuItemActivated( int ) ) );
	QObject::connect( pWnd, SIGNAL( activated( K3MdiChildView* ) ), this, SIGNAL( viewActivated( K3MdiChildView* ) ) );
	QObject::connect( pWnd, SIGNAL( deactivated( K3MdiChildView* ) ), this, SIGNAL( viewDeactivated( K3MdiChildView* ) ) );
	
	if ( index == -1 )
		m_pDocumentViews->append( pWnd );
	else
		m_pDocumentViews->insert( index, pWnd );
	
	if ( m_pTaskBar )
	{
		K3MdiTaskBarButton* but = m_pTaskBar->addWinButton( pWnd );
		QObject::connect( pWnd, SIGNAL( tabCaptionChanged( const QString& ) ), but, SLOT( setNewText( const QString& ) ) );
	}

	// embed the view depending on the current MDI mode
	if ( m_mdiMode == K3Mdi::TabPageMode || m_mdiMode == K3Mdi::IDEAlMode )
	{
		QPixmap pixmap;
		if ( pWnd->icon() )
			pixmap = *( pWnd->icon() );
		
		m_documentTabWidget->insertTab( pWnd, pixmap, pWnd->tabCaption(), index );

		connect( pWnd, SIGNAL( iconUpdated( QWidget*, QPixmap ) ), m_documentTabWidget, SLOT( updateIconInView( QWidget*, QPixmap ) ) );
		connect( pWnd, SIGNAL( captionUpdated( QWidget*, const QString& ) ), m_documentTabWidget, SLOT( updateCaptionInView( QWidget*, const QString& ) ) );
	}
	else
	{
		if ( ( flags & K3Mdi::Detach ) || ( m_mdiMode == K3Mdi::ToplevelMode ) )
		{
			detachWindow( pWnd, !( flags & K3Mdi::Hide ) );
			emit childViewIsDetachedNow( pWnd ); // fake it because detach won't call it in this case of addWindow-to-MDI
		}
		else
			attachWindow( pWnd, !( flags & K3Mdi::Hide ), flags & K3Mdi::UseK3MdiSizeHint );

		if ( ( m_bMaximizedChildFrmMode && ( !m_bSDIApplication && ( flags & K3Mdi::Detach ) )
		       && m_mdiMode != K3Mdi::ToplevelMode ) || ( flags & K3Mdi::Maximize ) )
		{
			if ( !pWnd->isMaximized() )
				pWnd->maximize();
		}
		
		if ( !m_bSDIApplication || ( flags & K3Mdi::Detach ) )
		{
			if ( flags & K3Mdi::Minimize )
				pWnd->minimize();
			
			if ( !( flags & K3Mdi::Hide ) )
			{
				if ( pWnd->isAttached() )
					pWnd->mdiParent()->show();
				else
					pWnd->show();
			}
		}
	}
}

//============ addWindow ============//
void K3MdiMainFrm::addWindow( K3MdiChildView* pWnd, QRect rectNormal, int flags )
{
	addWindow( pWnd, flags );
	if ( m_bMaximizedChildFrmMode && pWnd->isAttached() )
		pWnd->setRestoreGeometry( rectNormal );
	else
		pWnd->setGeometry( rectNormal );
}

//============ addWindow ============//
void K3MdiMainFrm::addWindow( K3MdiChildView* pWnd, QPoint pos, int flags )
{
	addWindow( pWnd, flags );
	if ( m_bMaximizedChildFrmMode && pWnd->isAttached() )
		pWnd->setRestoreGeometry( QRect( pos, pWnd->restoreGeometry().size() ) );
	else
		pWnd->move( pos );
}



K3MdiToolViewAccessor *K3MdiMainFrm::createToolWindow()
{
	return new K3MdiToolViewAccessor( this );
}


void K3MdiMainFrm::deleteToolWindow( QWidget* pWnd )
{
	if ( m_pToolViews->contains( pWnd ) )
		deleteToolWindow( ( *m_pToolViews ) [ pWnd ] );
}

void K3MdiMainFrm::deleteToolWindow( K3MdiToolViewAccessor *accessor )
{
	delete accessor;
}

//============ addWindow ============//
K3MdiToolViewAccessor *K3MdiMainFrm::addToolWindow( QWidget* pWnd, K3DockWidget::DockPosition pos, QWidget* pTargetWnd,
                                                  int percent, const QString& tabToolTip, const QString& tabCaption )
{
	QWidget* tvta = pWnd;
	K3DockWidget* pDW = dockManager->getDockWidgetFromName( pWnd->name() );
	QString newTabCaption = !tabCaption.isEmpty() ? pWnd->caption() : tabCaption;
	if ( pDW )
	{
		// probably readDockConfig already created the widgetContainer, use that
		pDW->setWidget( pWnd );

		if ( pWnd->icon() )
			pDW->setPixmap( *pWnd->icon() );

		pDW->setTabPageLabel( newTabCaption );
		pDW->setToolTipString( tabToolTip );
		dockManager->removeFromAutoCreateList( pDW );
		pWnd = pDW;
	}

	QRect r = pWnd->geometry();

	K3MdiToolViewAccessor *mtva = new K3MdiToolViewAccessor( this, pWnd, tabToolTip, newTabCaption );
	m_pToolViews->insert( tvta, mtva );

	if ( pos == K3DockWidget::DockNone )
	{
		mtva->d->widgetContainer->setEnableDocking( K3DockWidget::DockNone );
		mtva->d->widgetContainer->reparent( this, Qt::WType_TopLevel | Qt::WType_Dialog, r.topLeft(), true ); //pToolView->isVisible());
	}
	else //add and dock the toolview as a dockwidget view
		mtva->place( pos, pTargetWnd, percent );


	return mtva;
}

//============ attachWindow ============//
void K3MdiMainFrm::attachWindow( K3MdiChildView *pWnd, bool bShow, bool bAutomaticResize )
{
	pWnd->installEventFilter( this );

	// decide whether window shall be cascaded
	bool bCascade = false;
	QApplication::sendPostedEvents();
	QRect frameGeo = pWnd->frameGeometry();
	QPoint topLeftScreen = pWnd->mapToGlobal( QPoint( 0, 0 ) );
	QPoint topLeftMdiChildArea = m_pMdi->mapFromGlobal( topLeftScreen );
	QRect childAreaGeo = m_pMdi->geometry();
	if ( topLeftMdiChildArea.x() < 0 || topLeftMdiChildArea.y() < 0 ||
	     ( topLeftMdiChildArea.x() + frameGeo.width() > childAreaGeo.width() ) ||
	     ( topLeftMdiChildArea.y() + frameGeo.height() > childAreaGeo.height() ) )
	{
		bCascade = true;
	}

	// create frame and insert child view
	K3MdiChildFrm *lpC = new K3MdiChildFrm( m_pMdi );
	pWnd->hide();
	if ( !bCascade )
		lpC->move( topLeftMdiChildArea );

	lpC->setClient( pWnd, bAutomaticResize );
	lpC->setFocus();
	pWnd->youAreAttached( lpC );
	if ( ( m_mdiMode == K3Mdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		switchToChildframeMode();
	}

	m_pMdi->manageChild( lpC, false, bCascade );
	if ( m_pMdi->topChild() && m_pMdi->topChild() ->isMaximized() )
	{
		QRect r = lpC->geometry();
		lpC->setGeometry( -lpC->m_pClient->x(), -lpC->m_pClient->y(),
		                  m_pMdi->width() + K3MDI_CHILDFRM_DOUBLE_BORDER,
		                  m_pMdi->height() + lpC->captionHeight() + K3MDI_CHILDFRM_SEPARATOR + K3MDI_CHILDFRM_DOUBLE_BORDER );
		lpC->setRestoreGeometry( r );
	}

	if ( bShow )
	{
		lpC->show();
	}

#undef FocusIn
	QFocusEvent fe( QEvent::FocusIn );
	QApplication::sendEvent( pWnd, &fe );

	m_pCurrentWindow = pWnd;  // required for checking the active item
}

//============= detachWindow ==============//
void K3MdiMainFrm::detachWindow( K3MdiChildView *pWnd, bool bShow )
{
	if ( pWnd->isAttached() )
	{
		pWnd->removeEventFilter( this );
		pWnd->youAreDetached();
		// this is only if it was attached and you want to detach it
		if ( pWnd->parent() )
		{
			K3MdiChildFrm * lpC = pWnd->mdiParent();
			if ( lpC )
			{
				if ( lpC->icon() )
				{
					QPixmap pixm( *( lpC->icon() ) );
					pWnd->setIcon( pixm );
				}
				QString capt( lpC->caption() );
				if ( !bShow )
					lpC->hide();
				lpC->unsetClient( m_undockPositioningOffset );
				m_pMdi->destroyChildButNotItsView( lpC, false ); //Do not focus the new top child , we loose focus...
				pWnd->setCaption( capt );
			}
		}
	}
	else
	{
		if ( pWnd->size().isEmpty() || ( pWnd->size() == QSize( 1, 1 ) ) )
		{
			if ( m_pCurrentWindow )
			{
				pWnd->setGeometry( QRect( m_pMdi->getCascadePoint( m_pDocumentViews->count() - 1 ), m_pCurrentWindow->size() ) );
			}
			else
			{
				pWnd->setGeometry( QRect( m_pMdi->getCascadePoint( m_pDocumentViews->count() - 1 ), defaultChildFrmSize() ) );
			}
		}
#ifdef Q_WS_X11
		if ( mdiMode() == K3Mdi::ToplevelMode )
		{
			XSetTransientForHint( QX11Info::display(), pWnd->winId(), topLevelWidget() ->winId() );
		}
#endif

		return ;
	}

#ifdef Q_WS_X11
	if ( mdiMode() == K3Mdi::ToplevelMode )
	{
		XSetTransientForHint( QX11Info::display(), pWnd->winId(), topLevelWidget() ->winId() );
	}
#endif

	// this will show it...
	if ( bShow )
	{
		activateView( pWnd );
	}

	emit childViewIsDetachedNow( pWnd );
}

//============== removeWindowFromMdi ==============//
void K3MdiMainFrm::removeWindowFromMdi( K3MdiChildView *pWnd )
{
	Q_UNUSED( pWnd );
	//Closes a child window. sends no close event : simply deletes it
	//FIXME something wrong with this, but nobody knows whatcart
#if 0
	if ( !( m_pWinList->removeRef( pWnd ) ) )
		return ;
	if ( m_pWinList->count() == 0 )
		m_pCurrentWindow = 0L;

	QObject::disconnect( pWnd, SIGNAL( attachWindow( K3MdiChildView*, bool ) ), this, SLOT( attachWindow( K3MdiChildView*, bool ) ) );
	QObject::disconnect( pWnd, SIGNAL( detachWindow( K3MdiChildView*, bool ) ), this, SLOT( detachWindow( K3MdiChildView*, bool ) ) );
	QObject::disconnect( pWnd, SIGNAL( focusInEventOccurs( K3MdiChildView* ) ), this, SLOT( activateView( K3MdiChildView* ) ) );
	QObject::disconnect( pWnd, SIGNAL( childWindowCloseRequest( K3MdiChildView* ) ), this, SLOT( childWindowCloseRequest( K3MdiChildView* ) ) );
	QObject::disconnect( pWnd, SIGNAL( clickedInWindowMenu( int ) ), this, SLOT( windowMenuItemActivated( int ) ) );
	QObject::disconnect( pWnd, SIGNAL( clickedInDockMenu( int ) ), this, SLOT( dockMenuItemActivated( int ) ) );

	if ( m_pTaskBar )
	{
		K3MdiTaskBarButton * but = m_pTaskBar->getButton( pWnd );
		if ( but != 0L )
		{
			QObject::disconnect( pWnd, SIGNAL( tabCaptionChanged( const QString& ) ), but, SLOT( setNewText( const QString& ) ) );
		}
		m_pTaskBar->removeWinButton( pWnd );
	}

	if ( m_mdiMode == K3Mdi::TabPageMode )
	{
		if ( m_pWinList->count() == 0 )
		{
			if ( !m_pDockbaseAreaOfDocumentViews )
			{
				m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover" );
				m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );

				m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
				setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
			}
			m_pDockbaseOfTabPage->setDockSite( K3DockWidget::DockFullSite );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, K3DockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
			m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
			m_pClose->hide();
		}
		K3DockWidget* pDockW = ( K3DockWidget* ) pWnd->parentWidget();
		pWnd->reparent( 0L, QPoint( 0, 0 ) );
		pDockW->setWidget( 0L );
		if ( pDockW == m_pDockbaseOfTabPage )
		{
			QTabWidget * pTab = ( QTabWidget* ) pDockW->parentWidget() ->parentWidget();
			int cnt = pTab->count();
			m_pDockbaseOfTabPage = ( K3DockWidget* ) pTab->page( cnt - 2 );
			if ( pDockW == m_pDockbaseOfTabPage )
			{
				m_pDockbaseOfTabPage = ( K3DockWidget* ) pTab->page( cnt - 1 ); // different to the one deleted next
			}
		}
		delete pDockW;
		if ( m_pWinList->count() == 1 )
		{
			m_pWinList->last() ->activate(); // all other views are activated by tab switch
		}
	}
	else if ( pWnd->isAttached() )
	{
		pWnd->mdiParent() ->hide();
		m_pMdi->destroyChildButNotItsView( pWnd->mdiParent() );
	}
	else
	{
		// is not attached
		if ( m_pMdi->getVisibleChildCount() > 0 )
		{
			setActiveWindow();
			m_pCurrentWindow = 0L;
			K3MdiChildView* pView = m_pMdi->topChild() ->m_pClient;
			if ( pView )
			{
				pView->activate();
			}
		}
		else if ( m_pWinList->count() > 0 )
		{
			//crash?         m_pWinList->last()->activate();
			//crash?         m_pWinList->last()->setFocus();
		}
	}

	if ( pWnd->isToolView() )
		pWnd->m_bToolView = false;

	if ( !m_pCurrentWindow )
		emit lastChildViewClosed();
#endif
}

//============== closeWindow ==============//
void K3MdiMainFrm::closeWindow( K3MdiChildView *pWnd, bool layoutTaskBar )
{
	if ( !pWnd )
		return ;
	//Closes a child window. sends no close event : simply deletes it
	m_pDocumentViews->removeRef( pWnd );
	if ( m_pDocumentViews->count() == 0 )
		m_pCurrentWindow = 0L;

	if ( m_pTaskBar )
	{
		m_pTaskBar->removeWinButton( pWnd, layoutTaskBar );
	}

	if ( ( m_mdiMode == K3Mdi::TabPageMode ) || ( m_mdiMode == K3Mdi::IDEAlMode ) )
	{
		if ( !m_documentTabWidget )
			return ; //oops
		if ( m_pDocumentViews->count() == 0 )
			m_pClose->hide();
		pWnd->reparent( 0L, QPoint( 0, 0 ) );
		kdDebug(760) << "-------- 1" << endl;
		if ( m_pDocumentViews->count() == 1 )
		{
			m_pDocumentViews->last() ->activate(); // all other views are activated by tab switch
		}
	}
	if ( ( m_mdiMode == K3Mdi::TabPageMode ) || ( m_mdiMode == K3Mdi::IDEAlMode ) )
	{
		if ( m_pDocumentViews->count() == 0 )
		{
			if ( !m_pDockbaseAreaOfDocumentViews )
			{
				m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover" );
				m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
				m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
				setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
			}
#if 0
			m_pDockbaseOfTabPage->setDockSite( K3DockWidget::DockFullSite );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, K3DockWidget::DockCenter );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
			m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
#endif

			m_pClose->hide();
		}
#if 0
		K3DockWidget* pDockW = ( K3DockWidget* ) pWnd->parentWidget();
		pWnd->reparent( 0L, QPoint( 0, 0 ) );
		pDockW->setWidget( 0L );
		if ( pDockW == m_pDockbaseOfTabPage )
		{
			QTabWidget * pTab = ( QTabWidget* ) pDockW->parentWidget() ->parentWidget();
			int cnt = pTab->count();
			m_pDockbaseOfTabPage = ( K3DockWidget* ) pTab->page( cnt - 2 );
			if ( pDockW == m_pDockbaseOfTabPage )
			{
				m_pDockbaseOfTabPage = ( K3DockWidget* ) pTab->page( cnt - 1 ); // different to the one deleted next
			}
		}
		delete pDockW;
#endif

		delete pWnd;
		if ( m_pDocumentViews->count() == 1 )
		{
			m_pDocumentViews->last() ->activate(); // all other views are activated by tab switch
		}
	}
	else if ( pWnd->isAttached() )
	{
		m_pMdi->destroyChild( pWnd->mdiParent() );
	}
	else
	{
		delete pWnd;
		// is not attached
		if ( m_pMdi->getVisibleChildCount() > 0 )
		{
			setActiveWindow();
			m_pCurrentWindow = 0L;
			K3MdiChildView* pView = m_pMdi->topChild() ->m_pClient;
			if ( pView )
			{
				pView->activate();
			}
		}
		else if ( m_pDocumentViews->count() > 0 )
		{
			if ( m_pDocumentViews->current() )
			{
				m_pDocumentViews->current() ->activate();
				m_pDocumentViews->current() ->setFocus();
			}
			else
			{
				m_pDocumentViews->last() ->activate();
				m_pDocumentViews->last() ->setFocus();
			}
		}
	}

	if ( !m_pCurrentWindow )
	{
		d->closeWindowAction->setEnabled(false);
		emit lastChildViewClosed();
	}
}

//================== findWindow =================//
K3MdiChildView* K3MdiMainFrm::findWindow( const QString& caption )
{
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		if ( ( *it )->caption() == caption )
			return ( *it );
	}
	return 0L;
}

//================== activeWindow ===================//
K3MdiChildView* K3MdiMainFrm::activeWindow()
{
	return m_pCurrentWindow;
}

//================== windowExists ? =================//
bool K3MdiMainFrm::windowExists( K3MdiChildView *pWnd, ExistsAs as )
{
	if ( ( as == ToolView ) || ( as == AnyView ) )
	{
		if ( m_pToolViews->contains( pWnd ) )
			return true;
		if ( as == ToolView )
			return false;
	}
	
	if ( m_pDocumentViews->findRef( pWnd ) != -1  )
		return true;

	return false;
}

Q3PopupMenu * K3MdiMainFrm::windowPopup( K3MdiChildView * pWnd, bool bIncludeTaskbarPopup )
{
	m_pWindowPopup->clear();
	if ( bIncludeTaskbarPopup )
	{
		m_pWindowPopup->insertItem( i18n( "Window" ), taskBarPopup( pWnd, false ) );
		m_pWindowPopup->insertSeparator();
	}
	return m_pWindowPopup;
}

//================ taskBarPopup =================//
Q3PopupMenu* K3MdiMainFrm::taskBarPopup( K3MdiChildView *pWnd, bool /*bIncludeWindowPopup*/ )
{
	//returns the g_pTaskBarPopup filled according to the K3MdiChildView state
	m_pTaskBarPopup->clear();
	if ( pWnd->isAttached() )
	{
		m_pTaskBarPopup->insertItem( i18n( "Undock" ), pWnd, SLOT( detach() ) );
		m_pTaskBarPopup->insertSeparator();
		if ( pWnd->isMinimized() || pWnd->isMaximized() )
			m_pTaskBarPopup->insertItem( i18n( "Restore" ), pWnd, SLOT( restore() ) );
		if ( !pWnd->isMaximized() )
			m_pTaskBarPopup->insertItem( i18n( "Maximize" ), pWnd, SLOT( maximize() ) );
		if ( !pWnd->isMinimized() )
			m_pTaskBarPopup->insertItem( i18n( "Minimize" ), pWnd, SLOT( minimize() ) );
	}
	else
		m_pTaskBarPopup->insertItem( i18n( "Dock" ), pWnd, SLOT( attach() ) );
	m_pTaskBarPopup->insertSeparator();
	m_pTaskBarPopup->insertItem( i18n( "Close" ), pWnd, SLOT( close() ) );
	// the window has a view...get the window popup
	m_pTaskBarPopup->insertSeparator();
	m_pTaskBarPopup->insertItem( i18n( "Operations" ), windowPopup( pWnd, false ) );  //alvoid recursion
	return m_pTaskBarPopup;
}

void K3MdiMainFrm::slotDocCurrentChanged( QWidget* pWidget )
{
	K3MdiChildView * pWnd = static_cast<K3MdiChildView*>( pWidget );
	pWnd->m_bMainframesActivateViewIsPending = true;

	bool bActivateNecessary = true;
	if ( m_pCurrentWindow != pWnd )
		m_pCurrentWindow = pWnd;

	if ( m_pTaskBar )
		m_pTaskBar->setActiveButton( pWnd );

	if ( m_documentTabWidget && ( m_mdiMode == K3Mdi::TabPageMode || m_mdiMode == K3Mdi::IDEAlMode ) )
	{
		m_documentTabWidget->showPage( pWnd );
		pWnd->activate();
	}
	else
	{
		if ( pWnd->isAttached() )
		{
			if ( bActivateNecessary && ( m_pMdi->topChild() == pWnd->mdiParent() ) )
				pWnd->activate();
			
			pWnd->mdiParent()->raiseAndActivate();
		}
		if ( !pWnd->isAttached() )
		{
			if ( bActivateNecessary )
				pWnd->activate();
			
			m_pMdi->setTopChild( 0L ); // lose focus in the mainframe window
			if ( !pWnd->isActiveWindow() )
				pWnd->setActiveWindow();
			
			pWnd->raise();
		}
	}
	if ( !switching() )
		activeWindow()->updateTimeStamp();
	emit collapseOverlapContainers();
	pWnd->m_bMainframesActivateViewIsPending = false;
}


void K3MdiMainFrm::activateView( K3MdiChildView* pWnd )
{
	pWnd->m_bMainframesActivateViewIsPending = true;

	bool bActivateNecessary = true;
	if ( m_pCurrentWindow != pWnd )
		m_pCurrentWindow = pWnd;
	else
	{
		bActivateNecessary = false;
		// if this method is called as answer to view->activate(),
		// interrupt it because it's not necessary
		pWnd->m_bInterruptActivation = true;
	}

	if ( m_pTaskBar )
		m_pTaskBar->setActiveButton( pWnd );

	if ( m_documentTabWidget && m_mdiMode == K3Mdi::TabPageMode || m_mdiMode == K3Mdi::IDEAlMode )
	{
		m_documentTabWidget->showPage( pWnd );
		pWnd->activate();
	}
	else
	{
		if ( pWnd->isAttached() )
		{
			if ( bActivateNecessary && ( m_pMdi->topChild() == pWnd->mdiParent() ) )
				pWnd->activate();
			
			pWnd->mdiParent() ->raiseAndActivate();
		}
		if ( !pWnd->isAttached() )
		{
			if ( bActivateNecessary )
				pWnd->activate();
			
			m_pMdi->setTopChild( 0L ); // lose focus in the mainframe window
			if ( !pWnd->isActiveWindow() )
				pWnd->setActiveWindow();
			
			pWnd->raise();
		}
	}

	emit collapseOverlapContainers();

	pWnd->m_bMainframesActivateViewIsPending = false;
}

void K3MdiMainFrm::taskbarButtonRightClicked( K3MdiChildView *pWnd )
{
	activateView( pWnd ); // set focus
	//QApplication::sendPostedEvents();
	taskBarPopup( pWnd, true ) ->popup( QCursor::pos() );
}

void K3MdiMainFrm::childWindowCloseRequest( K3MdiChildView *pWnd )
{
	K3MdiViewCloseEvent * ce = new K3MdiViewCloseEvent( pWnd );
	QApplication::postEvent( this, ce );
}

bool K3MdiMainFrm::event( QEvent* e )
{
	if ( e->type() == QEvent::User )
	{
		K3MdiChildView * pWnd = ( K3MdiChildView* ) ( ( K3MdiViewCloseEvent* ) e )->data();
		if ( pWnd != 0L )
			closeWindow( pWnd );
		return true;
		// A little hack: If MDI child views are moved implicietly by moving
		// the main widget they should know this too. Unfortunately there seems to
		// be no way to catch the move start / move stop situations for the main
		// widget in a clean way. (There is no MouseButtonPress/Release or
		// something like that.) Therefore we do the following: When we get the
		// "first" move event we start a timer and interprete it as "drag begin".
		// If we get the next move event and the timer is running we restart the
		// timer and don't do anything else. If the timer elapses (this meens we
		// haven't had any move event for a while) we interprete this as "drag
		// end". If the moving didn't stop actually, we will later get another
		// "drag begin", so we get a drag end too much, but this would be the same
		// as if the user would stop moving for a little while.
		// Actually we seem to be lucky that the timer does not elapse while we
		// are moving -> so we have no obsolete drag end / begin
	}
	else if ( isVisible() && e->type() == QEvent::Move )
	{
		if ( m_pDragEndTimer->isActive() )
		{
			// this is not the first move -> stop old timer
			m_pDragEndTimer->stop();
		}
		else
		{
			// this is the first move -> send the drag begin to all concerned views
			Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
			for ( ; ( *it ); ++it )
			{
				K3MdiChildFrmDragBeginEvent dragBeginEvent( 0L );
				QApplication::sendEvent( ( *it ), &dragBeginEvent );
			}
		}
		m_pDragEndTimer->start( 200, true ); // single shot after 200 ms
	}

	return DockMainWindow3::event( e );
}

bool K3MdiMainFrm::eventFilter( QObject * /*obj*/, QEvent *e )
{
	if ( e->type() == QEvent::Resize && m_mdiMode == K3Mdi::ToplevelMode )
	{
		verifyToplevelHeight();
		return false; //let the rest of the resize magic do its work
	}
	
	if ( e->type() == QEvent::FocusIn )
	{
		QFocusEvent * pFE = ( QFocusEvent* ) e;
		if ( pFE->reason() == Qt::ActiveWindowFocusReason )
		{
			if ( m_pCurrentWindow && !m_pCurrentWindow->isHidden() &&
			     !m_pCurrentWindow->isAttached() && m_pMdi->topChild() )
			{
				return true;   // eat the event
			}
		}
		if ( m_pMdi )
		{
			static bool focusTCIsPending = false;
			if ( !focusTCIsPending && m_mdiMode == K3Mdi::ChildframeMode )
			{
				focusTCIsPending = true;
				m_pMdi->focusTopChild();
				focusTCIsPending = false;
			}
		}
	}
	else if ( e->type() == QEvent::KeyRelease )
	{
		if ( switching() )
		{
			KAction * a = actionCollection() ->action( "view_last_window" ) ;
			if ( a )
			{
				const KShortcut cut( a->shortcut() );
				const KKeySequence& seq = cut.seq( 0 );
				const KKey& key = seq.key( 0 );
				int modFlags = key.modFlags();
				int state = ( ( QKeyEvent * ) e ) ->state();
				KKey key2( ( QKeyEvent * ) e );

				/** these are quite some assumptions:
				 *   The key combination uses exactly one modifier key
				 *   The WIN button in KDE is the meta button in Qt
				 **/
				if ( state != ( ( QKeyEvent * ) e ) ->stateAfter() &&
				        ( ( modFlags & KKey::CTRL ) > 0 ) == ( ( state & Qt::ControlModifier ) > 0 ) &&
				        ( ( modFlags & KKey::ALT ) > 0 ) == ( ( state & Qt::AltModifier ) > 0 ) &&
				        ( ( modFlags & KKey::WIN ) > 0 ) == ( ( state & Qt::MetaModifier ) > 0 ) )
				{
					activeWindow() ->updateTimeStamp();
					setSwitching( false );
				}
				return true;
			}
			else
			{
				kdDebug(760) << "KAction( \"view_last_window\") not found." << endl;
			}
		}
	}
	return false;  // standard event processing
}

/**
 * close all views
 */
void K3MdiMainFrm::closeAllViews()
{
	//save the children first to a list, as removing invalidates our iterator
	Q3ValueList<K3MdiChildView*> children;
	for ( K3MdiChildView * w = m_pDocumentViews->first();w;w = m_pDocumentViews->next() )
	{
		children.append( w );
	}
	Q3ValueListIterator<K3MdiChildView *> childIt;
	for ( childIt = children.begin(); childIt != children.end(); ++childIt )
	{
		( *childIt )->close();
	}
}


/**
 * iconify all views
 */
void K3MdiMainFrm::iconifyAllViews()
{
	kdDebug(760) << k_funcinfo << "minimizing all the views" << endl;
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
		( *it )->minimize();
}

/**
 * closes the view of the active (topchild) window
 */
void K3MdiMainFrm::closeActiveView()
{
	kdDebug(760) << k_funcinfo << "closing the active view" << endl;
	if ( m_pCurrentWindow )
		m_pCurrentWindow->close();
}

/** find the root dockwidgets and store their geometry */
void K3MdiMainFrm::findRootDockWidgets( Q3PtrList<K3DockWidget>* rootDockWidgetList, Q3ValueList<QRect>* positionList )
{
	//nothing is valid
	if ( !rootDockWidgetList && !positionList )
		return ;

	// since we set some windows to toplevel, we must consider the window manager's window frame
	const int frameBorderWidth = 7;  // @todo: Can we / do we need to ask the window manager?
	const int windowTitleHeight = 10; // @todo:    -"-

	QList<K3DockWidget *> pObjList = findChildren<K3DockWidget *>();
	
	// FIXME: Is this still valid in kde4?
	//if ( pObjList.isEmpty() )
	//	pObjList = findChildren<K3DockWidget_Compat::K3DockWidget *>();

	// for all dockwidgets (which are children of this mainwindow)
	foreach(K3DockWidget *it, pObjList)
	{
		K3DockWidget* dockWindow = 0L; /* pDockW */
		K3DockWidget* rootDockWindow = 0L; /* pRootDockWindow */
		K3DockWidget* undockCandidate = 0L; /* pUndockCandidate */
		QWidget* pW = static_cast<QWidget*>( ( it ) );
		
		// find the oldest ancestor of the current dockwidget that can be undocked
		while ( !pW->isTopLevel() )
		{
			if ( qobject_cast<K3DockWidget*>( pW ) ||  pW->inherits( "K3DockWidget_Compat::K3DockWidget" ) )
			{
				undockCandidate = static_cast<K3DockWidget*>( pW );
				if ( undockCandidate->enableDocking() != K3DockWidget::DockNone )
					rootDockWindow = undockCandidate;
			}
			pW = pW->parentWidget();
		}
		
		if ( rootDockWindow )
		{
			// if that oldest ancestor is not already in the list, append it
			bool found = false;
			if ( !rootDockWidgetList->isEmpty() )
			{
				Q3PtrListIterator<K3DockWidget> it2( *rootDockWidgetList );
				for ( ; it2.current() && !found; ++it2 )
				{
					dockWindow = it2.current();
					if ( dockWindow == rootDockWindow )
						found = true;
				}
			}
			
			if ( !found || rootDockWidgetList->isEmpty() )
			{
					rootDockWidgetList->append( dockWindow );
					kdDebug(760) << k_funcinfo << "Appending " << rootDockWindow << " to our list of " <<
						"root dock windows" << endl;
					QPoint p = rootDockWindow->mapToGlobal( rootDockWindow->pos() ) - rootDockWindow->pos();
					QRect r( p.x(), p.y() + m_undockPositioningOffset.y(),
					         rootDockWindow->width() - windowTitleHeight - frameBorderWidth * 2,
					         rootDockWindow->height() - windowTitleHeight - frameBorderWidth * 2 );
					positionList->append( r );
			}
		}
	}
}

/**
 * undocks all view windows (unix-like)
 */
void K3MdiMainFrm::switchToToplevelMode()
{
	if ( m_mdiMode == K3Mdi::ToplevelMode )
	{
		emit mdiModeHasBeenChangedTo( K3Mdi::ToplevelMode );
		return ;
	}

	K3Mdi::MdiMode oldMdiMode = m_mdiMode;

	const int frameBorderWidth = 7;  // @todo: Can we / do we need to ask the window manager?
	setUndockPositioningOffset( QPoint( 0, ( m_pTaskBar ? m_pTaskBar->height() : 0 ) + frameBorderWidth ) );

	// 1.) select the dockwidgets to be undocked and store their geometry
	Q3PtrList<K3DockWidget> rootDockWidgetList;
	Q3ValueList<QRect> positionList;

	// 2.) undock the MDI views of K3MDI
	switch( oldMdiMode )
	{
	case K3Mdi::ChildframeMode:
		finishChildframeMode();
		break;
	case K3Mdi::TabPageMode:
		finishTabPageMode();
		break;
	case K3Mdi::IDEAlMode:
		finishIDEAlMode();
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		break;
	default:
		break; //do nothing
	}
	
	// 3.) undock all these found oldest ancestors (being K3DockWidgets)
	Q3PtrListIterator<K3DockWidget> kdwit( rootDockWidgetList );
	for ( ; ( *kdwit ); ++kdwit )
		( *kdwit )->undock();

	// 4.) recreate the MDI childframe area and hide it
	if ( oldMdiMode == K3Mdi::TabPageMode || oldMdiMode == K3Mdi::IDEAlMode )
	{
		if ( !m_pDockbaseAreaOfDocumentViews )
		{
			m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover" );
			m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
			m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
			m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockCorner );
			m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
		}
		// set this dock to main view
		setView( m_pDockbaseAreaOfDocumentViews );
		setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
	}
	//	QApplication::sendPostedEvents(); //why do we need to empty the event queue?
	if ( !parentWidget() )
	{
		//if we don't have a parent widget ( which i expect we wouldn't )
		//make sure we take into account the size of the docks provided by
		//QMainWindow
		int topDockHeight = topDock() ? topDock()->height() : 0;
		int bottomDockHeight = bottomDock() ? bottomDock()->height() : 0;
		int menuBarHeight = hasMenuBar() ? menuBar()->height() : 0;
		if ( m_pDocumentViews->count() != 0 )
			setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height() );
		else
		{
			kdDebug(760) << k_funcinfo << "height is: " << height() << endl;
			kdDebug(760) << k_funcinfo << "top dock height: " << topDockHeight << endl;
			kdDebug(760) << k_funcinfo << "bottom dock height: " << bottomDockHeight << endl;
			kdDebug(760) << k_funcinfo << "menu bar height: " << menuBarHeight << endl;
			kdDebug(760) << k_funcinfo << "dock base area height: " << m_pDockbaseAreaOfDocumentViews->height() << endl;
			setFixedHeight( topDockHeight + menuBarHeight );
		}
	} 
   
	//FIXME although i don't know what to fix
	// 5. show the child views again
	Q3PtrListIterator<K3MdiChildView> k3mdicvit( *m_pDocumentViews );
	for ( k3mdicvit.toFirst(); ( *k3mdicvit ); ++k3mdicvit )
	{
#ifdef Q_WS_X11
		XSetTransientForHint( QX11Info::display(), ( *k3mdicvit )->winId(), winId() );
#endif
		( *k3mdicvit )->show();
	}

	// 6.) reset all memorized positions of the undocked ones and show them again
	Q3ValueList<QRect>::Iterator qvlqrit;
	Q3ValueList<QRect>::Iterator qvlEnd = positionList.end();
	for ( k3mdicvit.toFirst(), qvlqrit = positionList.begin() ; ( *k3mdicvit ) && qvlqrit != qvlEnd; ++k3mdicvit, ++qvlqrit )
	{
		( *k3mdicvit )->setGeometry( ( *qvlqrit ) );
		( *k3mdicvit )->show();
	}

	m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockNone );
	m_mdiMode = K3Mdi::ToplevelMode;
	
	kdDebug(760) << k_funcinfo << "Switch to toplevel mode completed" << endl;
	emit mdiModeHasBeenChangedTo( K3Mdi::ToplevelMode );

}

void K3MdiMainFrm::finishToplevelMode()
{
	m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockCorner );
}

/**
 * docks all view windows (Windows-like)
 */
void K3MdiMainFrm::switchToChildframeMode()
{
	if ( m_mdiMode == K3Mdi::ChildframeMode )
	{
		emit mdiModeHasBeenChangedTo( K3Mdi::ChildframeMode );
		return ;
	}

	Q3PtrList<K3DockWidget> rootDockWidgetList;
	Q3ValueList<QRect> positionList;
	
	if ( m_mdiMode == K3Mdi::TabPageMode )
	{
		kdDebug(760) << k_funcinfo << "finishing tab page mode" << endl;
		// select the dockwidgets to be undocked and store their geometry
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		kdDebug(760) << k_funcinfo << "Found " << rootDockWidgetList.count() << " widgets to undock" << endl;
		
		// undock all these found oldest ancestors (being K3DockWidgets)
		Q3PtrListIterator<K3DockWidget> it( rootDockWidgetList );
		for ( ; ( *it ) ; ++it )
			( *it )->undock();

		finishTabPageMode();
	}
	else if ( m_mdiMode == K3Mdi::ToplevelMode )
	{
		finishToplevelMode();
	}
	else if ( m_mdiMode == K3Mdi::IDEAlMode )
	{
		kdDebug(760) << k_funcinfo << "finishing ideal mode" << endl;
		finishIDEAlMode( false );

		// select the dockwidgets to be undocked and store their geometry
		findRootDockWidgets( &rootDockWidgetList, &positionList );
		kdDebug(760) << k_funcinfo << "Found " << rootDockWidgetList.count() << " widgets to undock" << endl;
		
		// undock all these found oldest ancestors (being K3DockWidgets)
		Q3PtrListIterator<K3DockWidget> it( rootDockWidgetList );
		for ( ; ( *it ) ; ++it )
			( *it )->undock();

		m_mdiMode = K3Mdi::TabPageMode;
		finishTabPageMode();
		m_mdiMode = K3Mdi::IDEAlMode;
	}

	if ( !m_pDockbaseAreaOfDocumentViews )
	{
		// cover K3Mdi's childarea by a dockwidget
		m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover" );
		m_pDockbaseAreaOfDocumentViews->setDockWindowTransient( this, true );
		m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
		m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockCorner );
		m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi );
		kdDebug(760) << k_funcinfo << "childarea is now covered by a dockwidget" << endl;
	}
	
	if ( m_pDockbaseAreaOfDocumentViews->isTopLevel() )
	{
		// set this dock to main view
		setView( m_pDockbaseAreaOfDocumentViews );
		setMainDockWidget( m_pDockbaseAreaOfDocumentViews );
		m_pDockbaseAreaOfDocumentViews->setEnableDocking( K3DockWidget::DockNone );
		m_pDockbaseAreaOfDocumentViews->setDockSite( K3DockWidget::DockCorner );
		kdDebug(760) << k_funcinfo << "Dock base area has been set to the main view" << endl;
	}
	m_pDockbaseAreaOfDocumentViews->setWidget( m_pMdi ); //JW
	m_pDockbaseAreaOfDocumentViews->show();
	
	if ( ( m_mdiMode == K3Mdi::TabPageMode ) || ( m_mdiMode == K3Mdi::IDEAlMode ) )
	{
		kdDebug(760) << k_funcinfo << "trying to dock back the undock toolviews" << endl;
		Q3PtrListIterator<K3DockWidget> it( rootDockWidgetList );
		for ( ; ( *it ); ++it )
			( *it )->dockBack();
	}

	if ( m_mdiMode == K3Mdi::ToplevelMode && m_pTempDockSession )
	{
		// restore the old dock scenario which we memorized at the time we switched to toplevel mode
		kdDebug(760) << k_funcinfo << "Restoring old dock scenario memorized from toplevel mode" << endl;
		QDomElement oldDockState = m_pTempDockSession->namedItem( "cur_dock_state" ).toElement();
		readDockConfig( oldDockState );
	}

	K3Mdi::MdiMode oldMdiMode = m_mdiMode;
	m_mdiMode = K3Mdi::ChildframeMode;

	//FIXME although i don't know what to fix.
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildView* pView = ( *it );
		if ( !pView->isToolView() && pView->isAttached() )
			attachWindow( pView, true );
	}
	for ( it.toFirst(); ( *it ); ++it )
	{
		K3MdiChildView* pView = ( *it );
		if ( !pView->isToolView() )
			pView->show();
	}
	if ( ( oldMdiMode == K3Mdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		kdDebug(760) << k_funcinfo << "left top level mode completely" << endl;
		emit leftTopLevelMode();
	}
	emit mdiModeHasBeenChangedTo( K3Mdi::ChildframeMode );
}

void K3MdiMainFrm::finishChildframeMode()
{
	// save the old dock scenario of the dockwidget-like tool views to a DOM tree
	kdDebug(760) << k_funcinfo << "saving the current dock scenario" << endl;
	delete m_pTempDockSession;
	m_pTempDockSession = new QDomDocument( "docksession" );
	QDomElement curDockState = m_pTempDockSession->createElement( "cur_dock_state" );
	m_pTempDockSession->appendChild( curDockState );
	writeDockConfig( curDockState );

	// detach all non-tool-views to toplevel
	kdDebug(760) << k_funcinfo << "detaching all document views and moving them to toplevel" << endl;
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildView* pView = ( *it );
		if ( pView->isToolView() )
			continue;
		if ( pView->isAttached() )
		{
			if ( pView->isMaximized() )
				pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height() );
			detachWindow( pView, false );
		}
	}
}

/**
 * Docks all view windows (Windows-like)
 */
void K3MdiMainFrm::switchToTabPageMode()
{
	if ( m_mdiMode == K3Mdi::TabPageMode )
	{
		emit mdiModeHasBeenChangedTo( K3Mdi::TabPageMode );
		return ;  // nothing need to be done
	}

	switch( m_mdiMode )
	{
	case K3Mdi::ChildframeMode:
		finishChildframeMode();
		break;
	case K3Mdi::ToplevelMode:
		finishToplevelMode();
		break;
	case K3Mdi::IDEAlMode:
		finishIDEAlMode( false );
		emit mdiModeHasBeenChangedTo( K3Mdi::TabPageMode );
		m_mdiMode = K3Mdi::TabPageMode;
		return;
		break;
	default:
		break;
	}
	
	setupTabbedDocumentViewSpace();
	m_mdiMode = K3Mdi::TabPageMode;
	if ( m_pCurrentWindow )
		m_pCurrentWindow->setFocus();

	m_pTaskBar->switchOn( false );

	if ( m_pClose )
	{
		QObject::connect( m_pClose, SIGNAL( clicked() ), this, SLOT( closeViewButtonPressed() ) );
		if ( m_pDocumentViews->count() > 0 )
			m_pClose->show();
	}
	else
		kdDebug(760) << "close button nonexistant. strange things might happen" << endl;
	
	kdDebug(760) << "Switch to tab page mode complete" << endl;
	emit mdiModeHasBeenChangedTo( K3Mdi::TabPageMode );
}

void K3MdiMainFrm::finishTabPageMode()
{
	// if tabified, release all views from their docking covers
	if ( m_mdiMode == K3Mdi::TabPageMode )
	{
		m_pClose->hide();
		QObject::disconnect( m_pClose, SIGNAL( clicked() ), this, SLOT( closeViewButtonPressed() ) );

		Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
		for ( ; it.current(); ++it )
		{
			K3MdiChildView* pView = it.current();
			if ( pView->isToolView() )
				continue;
			kdDebug(760) << "K3MdiMainFrm::finishTabPageMode: in loop" << endl;
			QSize mins = pView->minimumSize();
			QSize maxs = pView->maximumSize();
			QSize sz = pView->size();
			QWidget* pParent = pView->parentWidget();
			QPoint p( pParent->mapToGlobal( pParent->pos() ) - pParent->pos() + m_undockPositioningOffset );
			m_documentTabWidget->removePage( pView );
			pView->reparent( 0, 0, p );
			//         pView->reparent(0,0,p);
			pView->resize( sz );
			pView->setMinimumSize( mins.width(), mins.height() );
			pView->setMaximumSize( maxs.width(), maxs.height() );
			//         ((K3DockWidget*)pParent)->undock(); // this destroys the dockwiget cover, too
			//         pParent->close();
			//         delete pParent;
			//         if (centralWidget() == pParent) {
			//            setCentralWidget(0L); // avoid dangling pointer
			//         }
		}
		delete m_documentTabWidget;
		m_documentTabWidget = 0;
		m_pTaskBar->switchOn( true );
	}
}



void K3MdiMainFrm::setupTabbedDocumentViewSpace()
{
	// resize to childframe mode size of the mainwindow if we were in toplevel mode
	if ( ( m_mdiMode == K3Mdi::ToplevelMode ) && !parentWidget() )
	{
		setMinimumHeight( m_oldMainFrmMinHeight );
		setMaximumHeight( m_oldMainFrmMaxHeight );
		resize( width(), m_oldMainFrmHeight );
		m_oldMainFrmHeight = 0;
		//qDebug("TopLevelMode off");
		emit leftTopLevelMode();
		QApplication::sendPostedEvents();

		// restore the old dock szenario which we memorized at the time we switched to toplevel mode
		if ( m_pTempDockSession )
		{
			QDomElement oldDockState = m_pTempDockSession->namedItem( "cur_dock_state" ).toElement();
			readDockConfig( oldDockState );
		}
	}

#if 0
	if ( m_pDockbaseOfTabPage != m_pDockbaseAreaOfDocumentViews )
	{
		delete m_pDockbaseOfTabPage;
		m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
	}
#endif
	delete m_documentTabWidget;
	m_documentTabWidget = new K3MdiDocumentViewTabWidget( m_pDockbaseAreaOfDocumentViews );
	connect( m_documentTabWidget, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotDocCurrentChanged( QWidget* ) ) );
	m_pDockbaseAreaOfDocumentViews->setWidget( m_documentTabWidget );
	m_documentTabWidget->show();
	Q3PtrListIterator<K3MdiChildView> it4( *m_pDocumentViews );
	for ( ; it4.current(); ++it4 )
	{
		K3MdiChildView* pView = it4.current();
		m_documentTabWidget->addTab( pView, pView->icon() ? *( pView->icon() ) : QPixmap(), pView->tabCaption() );
		/*
		   connect(pView,SIGNAL(iconOrCaptionUdpated(QWidget*,QPixmap,const QString&)),
		   m_documentTabWidget,SLOT(updateView(QWidget*,QPixmap,const QString&)));
		 */
		connect( pView, SIGNAL( iconUpdated( QWidget*, QPixmap ) ), m_documentTabWidget, SLOT( updateIconInView( QWidget*, QPixmap ) ) );
		connect( pView, SIGNAL( captionUpdated( QWidget*, const QString& ) ), m_documentTabWidget, SLOT( updateCaptionInView( QWidget*, const QString& ) ) );

	}
}


void K3MdiMainFrm::setIDEAlModeStyle( int flags )
{
	d->m_styleIDEAlMode = flags; // see KMultiTabBar for the first 3 bits
	if ( m_leftContainer )
	{
		K3MdiDockContainer * tmpL = ( K3MdiDockContainer* ) ( qobject_cast<K3MdiDockContainer*>(m_leftContainer->getWidget()) );
		if ( tmpL )
			tmpL->setStyle( flags );
	}

	if ( m_rightContainer )
	{
		K3MdiDockContainer * tmpR = ( K3MdiDockContainer* ) ( qobject_cast<K3MdiDockContainer*>(m_rightContainer->getWidget()) );
		if ( tmpR )
			tmpR->setStyle( flags );
	}

	if ( m_topContainer )
	{
		K3MdiDockContainer * tmpT = ( K3MdiDockContainer* ) ( qobject_cast<K3MdiDockContainer*>(m_topContainer->getWidget()) );
		if ( tmpT )
			tmpT->setStyle( flags );
	}

	if ( m_bottomContainer )
	{
		K3MdiDockContainer * tmpB = ( K3MdiDockContainer* ) ( qobject_cast<K3MdiDockContainer*>(m_bottomContainer->getWidget()) );
		if ( tmpB )
			tmpB->setStyle( flags );
	}
}

void K3MdiMainFrm::setToolviewStyle( int flag )
{
	if ( m_mdiMode == K3Mdi::IDEAlMode )
	{
		setIDEAlModeStyle( flag );
	}
	d->m_toolviewStyle = flag;
	bool toolviewExists = false;
	QMap<QWidget*, K3MdiToolViewAccessor*>::Iterator it;
	for ( it = m_pToolViews->begin(); it != m_pToolViews->end(); ++it )
	{
		K3DockWidget *dockWidget = dynamic_cast<K3DockWidget*>( it.data()->wrapperWidget() );
		if ( dockWidget )
		{
			switch ( flag )
			{
			case K3Mdi::IconOnly:
				dockWidget->setTabPageLabel( QString() );
				dockWidget->setPixmap( *( it.data()->wrappedWidget()->icon() ) );
				break;
			case K3Mdi::TextOnly:
				dockWidget->setPixmap(); //FIXME: Does not hide the icon in the IDEAl mode.
				dockWidget->setTabPageLabel( it.data()->wrappedWidget()->caption() );
				break;
			case K3Mdi::TextAndIcon:
				dockWidget->setPixmap( *( it.data()->wrappedWidget()->icon() ) );
				dockWidget->setTabPageLabel( it.data()->wrappedWidget()->caption() );
			default:
				break;
			}
			toolviewExists = true;
		}
	}
	
	if ( toolviewExists )
	{
		//workaround for the above FIXME to make switching to TextOnly mode work in IDEAl as well. Be sure that this version of switch* is called.
		if ( m_mdiMode == K3Mdi::IDEAlMode && flag == K3Mdi::TextOnly )
		{
			K3MdiMainFrm::switchToTabPageMode();
			K3MdiMainFrm::switchToIDEAlMode();
		}
		else
		{
			writeDockConfig();
			readDockConfig();
		}
	}
}

/**
 * Docks all view windows (Windows-like)
 */
void K3MdiMainFrm::switchToIDEAlMode()
{
	kdDebug(760) << k_funcinfo << "switching to IDEAl mode" << endl;
	
	if ( m_mdiMode == K3Mdi::IDEAlMode )
	{
		emit mdiModeHasBeenChangedTo( K3Mdi::IDEAlMode );
		return ;  // nothing need to be done
	}
	
	switch( m_mdiMode )
	{
	case K3Mdi::ChildframeMode:
		finishChildframeMode();
		break;
	case K3Mdi::ToplevelMode:
		finishToplevelMode();
		break;
	case K3Mdi::TabPageMode:
		m_mdiMode = K3Mdi::IDEAlMode;
		setupToolViewsForIDEALMode();
		emit mdiModeHasBeenChangedTo( K3Mdi::IDEAlMode );
		return;
		break;
	default:
		break;
	}

	setupTabbedDocumentViewSpace();
	m_mdiMode = K3Mdi::IDEAlMode;
	setupToolViewsForIDEALMode();

	if ( m_pCurrentWindow )
		m_pCurrentWindow->setFocus();

	m_pTaskBar->switchOn( false );

	if ( m_pClose )
	{
		QObject::connect( m_pClose, SIGNAL( clicked() ), this, SLOT( closeViewButtonPressed() ) );
		if ( m_pDocumentViews->count() > 0 )
			m_pClose->show();
	}
	else
		kdWarning(760) << k_funcinfo << "close button pointer does not exist!" << endl;

	kdDebug(760) << k_funcinfo << "switch to IDEAl mode complete" << endl;

	emit mdiModeHasBeenChangedTo( K3Mdi::IDEAlMode );
}


void K3MdiMainFrm::dockToolViewsIntoContainers( Q3PtrList<K3DockWidget>& widgetsToReparent, K3DockWidget *container )
{
	Q3PtrListIterator<K3DockWidget> it( widgetsToReparent );
	for ( ; ( *it ); ++it )
	{
		( *it )->manualDock( container, K3DockWidget::DockCenter, 20 );
		( *it )->loseFormerBrotherDockWidget();
	}
}

void K3MdiMainFrm::findToolViewsDockedToMain( Q3PtrList<K3DockWidget>* list, K3DockWidget::DockPosition dprtmw )
{
	K3DockWidget* mainDock = getMainDockWidget();
	if ( mainDock->parentDockTabGroup() )
	{
		mainDock = dynamic_cast<K3DockWidget*>( mainDock->parentDockTabGroup()->parent() );
		// FIXME: will likely crash below due to unchecked cast
	}

	if ( !mainDock )
	{
		kdDebug(760) << k_funcinfo << "mainDock invalid. No main dock widget found." << endl;
		return;
	}
	
	K3DockWidget* widget = mainDock->findNearestDockWidget( dprtmw );
	if ( widget && widget->parentDockTabGroup() )
	{
		widget = static_cast<K3DockWidget*>( widget->parentDockTabGroup() ->parent() );

		if ( widget )
		{
			K3DockTabGroup* tg = dynamic_cast<K3DockTabGroup*>( widget->getWidget() );
			if ( tg )
			{
				kdDebug(760) << k_funcinfo << "K3DockTabGroup found" << endl;
				for ( int i = 0;i < tg->count();i++ )
					list->append( static_cast<K3DockWidget*>( tg->page( i ) ) );
			}
			else
				list->append( widget );
		}
		else
			kdDebug(760) << k_funcinfo << "no widget found" << endl;
	}
	else
		kdDebug(760) << "No main dock widget found" << endl;
}


void K3MdiMainFrm::setupToolViewsForIDEALMode()
{
	m_leftContainer = createDockWidget( "K3MdiDock::leftDock", SmallIcon( "misc" ), 0L, "Left Dock" );
	m_rightContainer = createDockWidget( "K3MdiDock::rightDock", SmallIcon( "misc" ), 0L, "Right Dock" );
	m_topContainer = createDockWidget( "K3MdiDock::topDock", SmallIcon( "misc" ), 0L, "Top Dock" );
	m_bottomContainer = createDockWidget( "K3MdiDock::bottomDock", SmallIcon( "misc" ), 0L, "Bottom Dock" );

	K3DockWidget *mainDock = getMainDockWidget();
	K3DockWidget *w = mainDock;
	if ( mainDock->parentDockTabGroup() )
		w = static_cast<K3DockWidget*>( mainDock->parentDockTabGroup()->parent() );

	Q3PtrList<K3DockWidget> leftReparentWidgets;
	Q3PtrList<K3DockWidget> rightReparentWidgets;
	Q3PtrList<K3DockWidget> bottomReparentWidgets;
	Q3PtrList<K3DockWidget> topReparentWidgets;

	if ( mainDock->parentDockTabGroup() )
		mainDock = static_cast<K3DockWidget*>( mainDock->parentDockTabGroup() ->parent() );

	findToolViewsDockedToMain( &leftReparentWidgets, K3DockWidget::DockLeft );
	findToolViewsDockedToMain( &rightReparentWidgets, K3DockWidget::DockRight );
	findToolViewsDockedToMain( &bottomReparentWidgets, K3DockWidget::DockBottom );
	findToolViewsDockedToMain( &topReparentWidgets, K3DockWidget::DockTop );

	mainDock->setEnableDocking( K3DockWidget::DockNone ); //::DockCorner);
	mainDock->setDockSite( K3DockWidget::DockCorner );


	K3MdiDockContainer *tmpDC;
	m_leftContainer->setWidget( tmpDC = new K3MdiDockContainer( m_leftContainer, this, K3DockWidget::DockLeft, d->m_styleIDEAlMode ) );
	m_leftContainer->setEnableDocking( K3DockWidget::DockLeft );
	m_leftContainer->manualDock( mainDock, K3DockWidget::DockLeft, 20 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, SIGNAL( toggleLeft() ), tmpDC, SLOT( toggle() ) );
	connect( this, SIGNAL( collapseOverlapContainers() ), tmpDC, SLOT( collapseOverlapped() ) );
	connect( tmpDC, SIGNAL( activated( K3MdiDockContainer* ) ), this, SLOT( setActiveToolDock( K3MdiDockContainer* ) ) );
	connect( tmpDC, SIGNAL( deactivated( K3MdiDockContainer* ) ), this, SLOT( removeFromActiveDockList( K3MdiDockContainer* ) ) );

	m_rightContainer->setWidget( tmpDC = new K3MdiDockContainer( m_rightContainer, this, K3DockWidget::DockRight, d->m_styleIDEAlMode ) );
	m_rightContainer->setEnableDocking( K3DockWidget::DockRight );
	m_rightContainer->manualDock( mainDock, K3DockWidget::DockRight, 80 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, SIGNAL( toggleRight() ), tmpDC, SLOT( toggle() ) );
	connect( this, SIGNAL( collapseOverlapContainers() ), tmpDC, SLOT( collapseOverlapped() ) );
	connect( tmpDC, SIGNAL( activated( K3MdiDockContainer* ) ), this, SLOT( setActiveToolDock( K3MdiDockContainer* ) ) );
	connect( tmpDC, SIGNAL( deactivated( K3MdiDockContainer* ) ), this, SLOT( removeFromActiveDockList( K3MdiDockContainer* ) ) );

	m_topContainer->setWidget( tmpDC = new K3MdiDockContainer( m_topContainer, this, K3DockWidget::DockTop, d->m_styleIDEAlMode ) );
	m_topContainer->setEnableDocking( K3DockWidget::DockTop );
	m_topContainer->manualDock( mainDock, K3DockWidget::DockTop, 20 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, SIGNAL( toggleTop() ), tmpDC, SLOT( toggle() ) );
	connect( this, SIGNAL( collapseOverlapContainers() ), tmpDC, SLOT( collapseOverlapped() ) );
	connect( tmpDC, SIGNAL( activated( K3MdiDockContainer* ) ), this, SLOT( setActiveToolDock( K3MdiDockContainer* ) ) );
	connect( tmpDC, SIGNAL( deactivated( K3MdiDockContainer* ) ), this, SLOT( removeFromActiveDockList( K3MdiDockContainer* ) ) );

	m_bottomContainer->setWidget( tmpDC = new K3MdiDockContainer( m_bottomContainer, this, K3DockWidget::DockBottom, d->m_styleIDEAlMode ) );
	m_bottomContainer->setEnableDocking( K3DockWidget::DockBottom );
	m_bottomContainer->manualDock( mainDock, K3DockWidget::DockBottom, 80 );
	tmpDC->init();
	if ( m_mdiGUIClient )
		connect ( this, SIGNAL( toggleBottom() ), tmpDC, SLOT( toggle() ) );
	connect( this, SIGNAL( collapseOverlapContainers() ), tmpDC, SLOT( collapseOverlapped() ) );
	connect( tmpDC, SIGNAL( activated( K3MdiDockContainer* ) ), this, SLOT( setActiveToolDock( K3MdiDockContainer* ) ) );
	connect( tmpDC, SIGNAL( deactivated( K3MdiDockContainer* ) ), this, SLOT( removeFromActiveDockList( K3MdiDockContainer* ) ) );

	m_leftContainer->setDockSite( K3DockWidget::DockCenter );
	m_rightContainer->setDockSite( K3DockWidget::DockCenter );
	m_topContainer->setDockSite( K3DockWidget::DockCenter );
	m_bottomContainer->setDockSite( K3DockWidget::DockCenter );

	dockToolViewsIntoContainers( leftReparentWidgets, m_leftContainer );
	dockToolViewsIntoContainers( rightReparentWidgets, m_rightContainer );
	dockToolViewsIntoContainers( bottomReparentWidgets, m_bottomContainer );
	dockToolViewsIntoContainers( topReparentWidgets, m_topContainer );


	dockManager->setSpecialLeftDockContainer( m_leftContainer );
	dockManager->setSpecialRightDockContainer( m_rightContainer );
	dockManager->setSpecialTopDockContainer( m_topContainer );
	dockManager->setSpecialBottomDockContainer( m_bottomContainer );


	( ( K3MdiDockContainer* ) ( m_leftContainer->getWidget() ) ) ->hideIfNeeded();
	( ( K3MdiDockContainer* ) ( m_rightContainer->getWidget() ) ) ->hideIfNeeded();
	( ( K3MdiDockContainer* ) ( m_topContainer->getWidget() ) ) ->hideIfNeeded();
	( ( K3MdiDockContainer* ) ( m_bottomContainer->getWidget() ) ) ->hideIfNeeded();

}



void K3MdiMainFrm::finishIDEAlMode( bool full )
{
	// if tabified, release all views from their docking covers
	if ( m_mdiMode == K3Mdi::IDEAlMode )
	{
		assert( m_pClose );
		m_pClose->hide();
		QObject::disconnect( m_pClose, SIGNAL( clicked() ), this, SLOT( closeViewButtonPressed() ) );


		QStringList leftNames;
		leftNames = prepareIdealToTabs( m_leftContainer );
		int leftWidth = m_leftContainer->width();

		QStringList rightNames;
		rightNames = prepareIdealToTabs( m_rightContainer );
		int rightWidth = m_rightContainer->width();

		QStringList topNames;
		topNames = prepareIdealToTabs( m_topContainer );
		int topHeight = m_topContainer->height();

		QStringList bottomNames;
		bottomNames = prepareIdealToTabs( m_bottomContainer );
		int bottomHeight = m_bottomContainer->height();


		kdDebug(760) << "leftNames" << leftNames << endl;
		kdDebug(760) << "rightNames" << rightNames << endl;
		kdDebug(760) << "topNames" << topNames << endl;
		kdDebug(760) << "bottomNames" << bottomNames << endl;

		delete m_leftContainer;
		m_leftContainer = 0;
		delete m_rightContainer;
		m_rightContainer = 0;
		delete m_bottomContainer;
		m_bottomContainer = 0;
		delete m_topContainer;
		m_topContainer = 0;


		idealToolViewsToStandardTabs( bottomNames, K3DockWidget::DockBottom, bottomHeight );
		idealToolViewsToStandardTabs( leftNames, K3DockWidget::DockLeft, leftWidth );
		idealToolViewsToStandardTabs( rightNames, K3DockWidget::DockRight, rightWidth );
		idealToolViewsToStandardTabs( topNames, K3DockWidget::DockTop, topHeight );

		QApplication::sendPostedEvents();

		if ( !full )
			return ;

		Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
		for ( ; it.current(); ++it )
		{
			K3MdiChildView* pView = it.current();
			if ( pView->isToolView() )
				continue;
			QSize mins = pView->minimumSize();
			QSize maxs = pView->maximumSize();
			QSize sz = pView->size();
			QWidget* pParent = pView->parentWidget();
			QPoint p( pParent->mapToGlobal( pParent->pos() ) - pParent->pos() + m_undockPositioningOffset );
			pView->reparent( 0, 0, p );
			pView->reparent( 0, 0, p );
			pView->resize( sz );
			pView->setMinimumSize( mins.width(), mins.height() );
			pView->setMaximumSize( maxs.width(), maxs.height() );
			K3DockWidget* pDockW = 0L;
			// find the oldest ancestor of the current dockwidget that can be undocked
			do
			{
				if ( pParent->inherits( "K3DockWidget" ) || pParent->inherits( "K3DockWidget_Compat::K3DockWidget" ) )
				{
					pDockW = ( K3DockWidget* ) pParent;
					pDockW->undock(); // this destroys the dockwiget cover, too
					if ( pParent != m_pDockbaseAreaOfDocumentViews )
					{
						pParent->close();
						delete pParent;
					}
				}
				else
				{
					pParent = pParent->parentWidget();
				}
			}
			while ( pParent && !pDockW );
			if ( centralWidget() == pParent )
			{
				setCentralWidget( 0L ); // avoid dangling pointer
			}
		}
		m_pTaskBar->switchOn( true );

	}

}

QStringList K3MdiMainFrm::prepareIdealToTabs( K3DockWidget* container )
{
	K3DockContainer * pDW = dynamic_cast<K3DockContainer*>( container->getWidget() );
	QStringList widgetNames = ( ( K3MdiDockContainer* ) pDW ) ->containedWidgets();
	for ( QStringList::iterator it = widgetNames.begin();it != widgetNames.end();++it )
	{
		K3DockWidget* dw = ( K3DockWidget* ) manager() ->getDockWidgetFromName( *it );
		dw->undock();
		dw->setLatestK3DockContainer( 0 );
		dw->loseFormerBrotherDockWidget();
	}
	return widgetNames;
}

void K3MdiMainFrm::idealToolViewsToStandardTabs( QStringList widgetNames, K3DockWidget::DockPosition pos, int size )
{
	Q_UNUSED( size )

	K3DockWidget * mainDock = getMainDockWidget();
	if ( mainDock->parentDockTabGroup() )
	{
		mainDock = static_cast<K3DockWidget*>( mainDock->parentDockTabGroup() ->parent() );
	}

	if ( widgetNames.count() > 0 )
	{
		QStringList::iterator it = widgetNames.begin();
		K3DockWidget *dwpd = manager() ->getDockWidgetFromName( *it );
		if ( !dwpd )
		{
			kdDebug(760) << "Fatal error in finishIDEAlMode" << endl;
			return ;
		}
		dwpd->manualDock( mainDock, pos, 20 );
		++it;
		for ( ;it != widgetNames.end();++it )
		{
			K3DockWidget *tmpdw = manager() ->getDockWidgetFromName( *it );
			if ( !tmpdw )
			{
				kdDebug(760) << "Fatal error in finishIDEAlMode" << endl;
				return ;
			}
			tmpdw->manualDock( dwpd, K3DockWidget::DockCenter, 20 );
		}

#if 0
		QWidget *wid = dwpd->parentDockTabGroup();
		if ( !wid )
			wid = dwpd;
		wid->setGeometry( 0, 0, 20, 20 );
		/*  wid->resize(
		    ((pos==K3DockWidget::DockLeft) || (pos==K3DockWidget::DockRight))?size:wid->width(),
		    ((pos==K3DockWidget::DockLeft) || (pos==K3DockWidget::DockRight))?wid->height():size);
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
void K3MdiMainFrm::setMenuForSDIModeSysButtons( KMenuBar* pMenuBar )
{
	if ( m_bSDIApplication )   // there are no buttons in the menubar in this mode (although the view is always maximized)
		return ;

	m_pMainMenuBar = pMenuBar;
	if ( m_pMainMenuBar == 0L )
		return ;  // use setMenuForSDIModeSysButtons( 0L) for unsetting the external main menu!

	if ( !m_pUndock )
		m_pUndock = new QToolButton( pMenuBar );
	if ( !m_pRestore )
		m_pRestore = new QToolButton( pMenuBar );
	if ( !m_pMinimize )
		m_pMinimize = new QToolButton( pMenuBar );
	if ( !m_pClose )
		m_pClose = new QToolButton( pMenuBar );
	m_pUndock->setAutoRaise( false );
	m_pMinimize->setAutoRaise( false );
	m_pRestore->setAutoRaise( false );
	m_pClose->setAutoRaise( false );

	setSysButtonsAtMenuPosition();

	delete m_pUndockButtonPixmap;
	delete m_pMinButtonPixmap;
	delete m_pRestoreButtonPixmap;
	delete m_pCloseButtonPixmap;
	// create the decoration pixmaps
	if ( frameDecorOfAttachedViews() == K3Mdi::Win95Look )
	{
		m_pUndockButtonPixmap = new QPixmap( win_undockbutton );
		m_pMinButtonPixmap = new QPixmap( win_minbutton );
		m_pRestoreButtonPixmap = new QPixmap( win_restorebutton );
		m_pCloseButtonPixmap = new QPixmap( win_closebutton );
	}
	else if ( frameDecorOfAttachedViews() == K3Mdi::KDE1Look )
	{
		m_pUndockButtonPixmap = new QPixmap( kde_undockbutton );
		m_pMinButtonPixmap = new QPixmap( kde_minbutton );
		m_pRestoreButtonPixmap = new QPixmap( kde_restorebutton );
		m_pCloseButtonPixmap = new QPixmap( kde_closebutton );
		m_pUndock->setAutoRaise( true );
		m_pMinimize->setAutoRaise( true );
		m_pRestore->setAutoRaise( true );
		m_pClose->setAutoRaise( true );
	}
	else if ( frameDecorOfAttachedViews() == K3Mdi::KDELook )
	{
		m_pUndockButtonPixmap = new QPixmap( kde2_undockbutton );
		m_pMinButtonPixmap = new QPixmap( kde2_minbutton );
		m_pRestoreButtonPixmap = new QPixmap( kde2_restorebutton );
		m_pCloseButtonPixmap = new QPixmap( kde2_closebutton );
	}
	else
	{   // kde2laptop look
		m_pUndockButtonPixmap = new QPixmap( kde2laptop_undockbutton );
		m_pMinButtonPixmap = new QPixmap( kde2laptop_minbutton );
		m_pRestoreButtonPixmap = new QPixmap( kde2laptop_restorebutton );
		m_pCloseButtonPixmap = new QPixmap( kde2laptop_closebutton );
	}

	m_pUndock->hide();
	m_pMinimize->hide();
	m_pRestore->hide();
	m_pClose->hide();

	m_pUndock->setPixmap( *m_pUndockButtonPixmap );
	m_pMinimize->setPixmap( *m_pMinButtonPixmap );
	m_pRestore->setPixmap( *m_pRestoreButtonPixmap );
	m_pClose->setPixmap( *m_pCloseButtonPixmap );
}

void K3MdiMainFrm::setSysButtonsAtMenuPosition()
{
	if ( m_pMainMenuBar == 0L )
		return ;
	if ( m_pMainMenuBar->parentWidget() == 0L )
		return ;

	int menuW = m_pMainMenuBar->parentWidget() ->width();
	int h;
	int y;
	if ( frameDecorOfAttachedViews() == K3Mdi::Win95Look )
		h = 16;
	else if ( frameDecorOfAttachedViews() == K3Mdi::KDE1Look )
		h = 20;
	else if ( frameDecorOfAttachedViews() == K3Mdi::KDELook )
		h = 16;
	else
		h = 14;
	y = m_pMainMenuBar->height() / 2 - h / 2;

	if ( frameDecorOfAttachedViews() == K3Mdi::KDELaptopLook )
	{
		int w = 27;
		m_pUndock->setGeometry( ( menuW - ( w * 3 ) - 5 ), y, w, h );
		m_pMinimize->setGeometry( ( menuW - ( w * 2 ) - 5 ), y, w, h );
		m_pRestore->setGeometry( ( menuW - w - 5 ), y, w, h );
	}
	else
	{
		m_pUndock->setGeometry( ( menuW - ( h * 4 ) - 5 ), y, h, h );
		m_pMinimize->setGeometry( ( menuW - ( h * 3 ) - 5 ), y, h, h );
		m_pRestore->setGeometry( ( menuW - ( h * 2 ) - 5 ), y, h, h );
		m_pClose->setGeometry( ( menuW - h - 5 ), y, h, h );
	}
}

/** Activates the next open view */
void K3MdiMainFrm::activateNextWin()
{
	K3MdiIterator<K3MdiChildView*>* it = createIterator();
	K3MdiChildView* aWin = activeWindow();
	for ( it->first(); !it->isDone(); it->next() )
	{
		if ( it->currentItem() == aWin )
		{
			it->next();
			if ( !it->currentItem() )
			{
				it->first();
			}
			if ( it->currentItem() )
			{
				activateView( it->currentItem() );
			}
			break;
		}
	}
	delete it;
}

/** Activates the previous open view */
void K3MdiMainFrm::activatePrevWin()
{
	K3MdiIterator<K3MdiChildView*>* it = createIterator();
	K3MdiChildView* aWin = activeWindow();
	for ( it->first(); !it->isDone(); it->next() )
	{
		if ( it->currentItem() == aWin )
		{
			it->prev();
			if ( !it->currentItem() )
			{
				it->last();
			}
			if ( it->currentItem() )
			{
				activateView( it->currentItem() );
			}
			break;
		}
	}
	delete it;
}

/** Activates the view we accessed the most time ago */
void K3MdiMainFrm::activateFirstWin()
{
	m_bSwitching= true; // flag that we are currently switching between windows
	K3MdiIterator<K3MdiChildView*>* it = createIterator();
	QMap<QDateTime, K3MdiChildView*> m;
	for ( it->first(); !it->isDone(); it->next() )
	{
		m.insert( it->currentItem() ->getTimeStamp(), it->currentItem() );
	}

	if ( !activeWindow() )
		return ;

	QDateTime current = activeWindow() ->getTimeStamp();
	QMap<QDateTime, K3MdiChildView*>::iterator pos( m.find( current ) );
	QMap<QDateTime, K3MdiChildView*>::iterator newPos = pos;
	if ( pos != m.end() )
	{
		++newPos;
	}
	if ( newPos != m.end() )
	{ // look ahead
		++pos;
	}
	else
	{
		pos = m.begin();
	}
	activateView( pos.data() );
	delete it;
}

/** Activates the previously accessed view before this one was activated */
void K3MdiMainFrm::activateLastWin()
{
	m_bSwitching= true; // flag that we are currently switching between windows
	K3MdiIterator<K3MdiChildView*>* it = createIterator();
	QMap<QDateTime, K3MdiChildView*> m;
	for ( it->first(); !it->isDone(); it->next() )
	{
		m.insert( it->currentItem() ->getTimeStamp(), it->currentItem() );
	}

	if ( !activeWindow() )
		return ;

	QDateTime current = activeWindow() ->getTimeStamp();
	QMap<QDateTime, K3MdiChildView*>::iterator pos( m.find( current ) );
	if ( pos != m.begin() )
	{
		--pos;
	}
	else
	{
		pos = m.end();
		--pos;
	}
	activateView( pos.data() );
	delete it;
}

/** Activates the view with a certain index (TabPage mode only) */
void K3MdiMainFrm::activateView( int index )
{
	K3MdiChildView * pView = m_pDocumentViews->first();
	for ( int i = 0; pView && ( i < index ); i++ )
	{
		pView = m_pDocumentViews->next();
	}
	if ( pView )
	{
		pView->activate();
	}
}

/** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
void K3MdiMainFrm::setEnableMaximizedChildFrmMode( bool enableMaxChildFrameMode )
{
	if ( enableMaxChildFrameMode )
	{
		kdDebug(760) << k_funcinfo << "Turning on maximized child frame mode" << endl;
		m_bMaximizedChildFrmMode = true;
		
		K3MdiChildFrm* pCurrentChild = m_pMdi->topChild();
		
		//If we have no child or there is no menubar, we do nothing
		if ( !pCurrentChild || !m_pMainMenuBar )
			return ;

		QObject::connect( m_pUndock, SIGNAL( clicked() ), pCurrentChild, SLOT( undockPressed() ) );
		QObject::connect( m_pMinimize, SIGNAL( clicked() ), pCurrentChild, SLOT( minimizePressed() ) );
		QObject::connect( m_pRestore, SIGNAL( clicked() ), pCurrentChild, SLOT( maximizePressed() ) );
		m_pMinimize->show();
		m_pUndock->show();
		m_pRestore->show();

		if ( frameDecorOfAttachedViews() == K3Mdi::KDELaptopLook )
		{
			m_pMainMenuBar->insertItem( QPixmap( kde2laptop_closebutton_menu ), m_pMdi->topChild(), SLOT( closePressed() ), 0, -1, 0 );
		}
		else
		{
			m_pMainMenuBar->insertItem( *pCurrentChild->icon(), pCurrentChild->systemMenu(), -1, 0 );
			if ( m_pClose )
			{
				QObject::connect( m_pClose, SIGNAL( clicked() ), pCurrentChild, SLOT( closePressed() ) );
				m_pClose->show();
			}
			else
				kdDebug(760) << k_funcinfo << "no close button. things won't behave correctly" << endl;
		}
	}
	else
	{
		if ( !m_bMaximizedChildFrmMode )
			return ;  // already set, nothing to do

		kdDebug(760) << k_funcinfo << "Turning off maximized child frame mode" << endl;
		m_bMaximizedChildFrmMode = false;

		K3MdiChildFrm* pFrmChild = m_pMdi->topChild();
		if ( pFrmChild && pFrmChild->m_pClient && pFrmChild->state() == K3MdiChildFrm::Maximized )
		{
			pFrmChild->m_pClient->restore();
			switchOffMaximizeModeForMenu( pFrmChild );
		}
	}
}

/** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
void K3MdiMainFrm::switchOffMaximizeModeForMenu( K3MdiChildFrm* oldChild )
{
	// if there is no menubar given, those system buttons aren't possible
	if ( !m_pMainMenuBar )
		return ;
	
	m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt( 0 ) );
	
	if ( oldChild )
	{
		Q_ASSERT( m_pClose );
		QObject::disconnect( m_pUndock, SIGNAL( clicked() ), oldChild, SLOT( undockPressed() ) );
		QObject::disconnect( m_pMinimize, SIGNAL( clicked() ), oldChild, SLOT( minimizePressed() ) );
		QObject::disconnect( m_pRestore, SIGNAL( clicked() ), oldChild, SLOT( maximizePressed() ) );
		QObject::disconnect( m_pClose, SIGNAL( clicked() ), oldChild, SLOT( closePressed() ) );
	}
	m_pUndock->hide();
	m_pMinimize->hide();
	m_pRestore->hide();
	m_pClose->hide();
}

/** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
void K3MdiMainFrm::updateSysButtonConnections( K3MdiChildFrm* oldChild, K3MdiChildFrm* newChild )
{
	//qDebug("updateSysButtonConnections");
	// if there is no menubar given, those system buttons aren't possible
	if ( !m_pMainMenuBar )
		return ;

	if ( newChild )
	{
		if ( frameDecorOfAttachedViews() == K3Mdi::KDELaptopLook )
			m_pMainMenuBar->insertItem( QPixmap( kde2laptop_closebutton_menu ), newChild, SLOT( closePressed() ), 0, -1, 0 );
		else
			m_pMainMenuBar->insertItem( *newChild->icon(), newChild->systemMenu(), -1, 0 );
	}
	
	if ( oldChild )
	{
		m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt( 1 ) );
		Q_ASSERT( m_pClose );
		QObject::disconnect( m_pUndock, SIGNAL( clicked() ), oldChild, SLOT( undockPressed() ) );
		QObject::disconnect( m_pMinimize, SIGNAL( clicked() ), oldChild, SLOT( minimizePressed() ) );
		QObject::disconnect( m_pRestore, SIGNAL( clicked() ), oldChild, SLOT( maximizePressed() ) );
		QObject::disconnect( m_pClose, SIGNAL( clicked() ), oldChild, SLOT( closePressed() ) );
	}
	if ( newChild )
	{
		Q_ASSERT( m_pClose );
		QObject::connect( m_pUndock, SIGNAL( clicked() ), newChild, SLOT( undockPressed() ) );
		QObject::connect( m_pMinimize, SIGNAL( clicked() ), newChild, SLOT( minimizePressed() ) );
		QObject::connect( m_pRestore, SIGNAL( clicked() ), newChild, SLOT( maximizePressed() ) );
		QObject::connect( m_pClose, SIGNAL( clicked() ), newChild, SLOT( closePressed() ) );
	}
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
bool K3MdiMainFrm::isViewTaskBarOn()
{
	if ( m_pTaskBar )
		return m_pTaskBar->isSwitchedOn();
	else
		return false;
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void K3MdiMainFrm::showViewTaskBar()
{
	if ( m_pTaskBar )
		m_pTaskBar->switchOn( true );
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void K3MdiMainFrm::hideViewTaskBar()
{
	if ( m_pTaskBar )
		m_pTaskBar->switchOn( false );
}

//=============== fillWindowMenu ===============//
void K3MdiMainFrm::fillWindowMenu()
{
	bool tabPageMode = false;
	if ( m_mdiMode == K3Mdi::TabPageMode )
		tabPageMode = true;
	
	bool IDEAlMode = false;
	if ( m_mdiMode == K3Mdi::IDEAlMode )
		IDEAlMode = true;

	bool noViewOpened = false;
	if ( m_pDocumentViews->isEmpty() )
		noViewOpened = true;

	// construct the menu and its submenus
	if ( !m_bClearingOfWindowMenuBlocked )
		m_pWindowMenu->clear();

	d->closeWindowAction->plug(m_pWindowMenu);

	int closeAllId = m_pWindowMenu->insertItem( i18n( "Close &All" ), this, SLOT( closeAllViews() ) );
	if ( noViewOpened )
	{
		d->closeWindowAction->setEnabled(false);
		m_pWindowMenu->setItemEnabled( closeAllId, false );
	}
	
	if ( !tabPageMode && !IDEAlMode )
	{
		int iconifyId = m_pWindowMenu->insertItem( i18n( "&Minimize All" ), this, SLOT( iconifyAllViews() ) );
		if ( noViewOpened )
			m_pWindowMenu->setItemEnabled( iconifyId, false );
	}
	
	m_pWindowMenu->insertSeparator();
	m_pWindowMenu->insertItem( i18n( "&MDI Mode" ), m_pMdiModeMenu );
	m_pMdiModeMenu->clear();
	m_pMdiModeMenu->insertItem( i18n( "&Toplevel Mode" ), this, SLOT( switchToToplevelMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "C&hildframe Mode" ), this, SLOT( switchToChildframeMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "Ta&b Page Mode" ), this, SLOT( switchToTabPageMode() ) );
	m_pMdiModeMenu->insertItem( i18n( "I&DEAl Mode" ), this, SLOT( switchToIDEAlMode() ) );
	switch ( m_mdiMode )
	{
	case K3Mdi::ToplevelMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 0 ), true );
		break;
	case K3Mdi::ChildframeMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 1 ), true );
		break;
	case K3Mdi::TabPageMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 2 ), true );
		break;
	case K3Mdi::IDEAlMode:
		m_pMdiModeMenu->setItemChecked( m_pMdiModeMenu->idAt( 3 ), true );
		break;
	default:
		break;
	}
	
	m_pWindowMenu->insertSeparator();
	if ( !tabPageMode && !IDEAlMode )
	{
		int placMenuId = m_pWindowMenu->insertItem( i18n( "&Tile" ), m_pPlacingMenu );
		m_pPlacingMenu->clear();
		m_pPlacingMenu->insertItem( i18n( "Ca&scade Windows" ), m_pMdi, SLOT( cascadeWindows() ) );
		m_pPlacingMenu->insertItem( i18n( "Cascade &Maximized" ), m_pMdi, SLOT( cascadeMaximized() ) );
		m_pPlacingMenu->insertItem( i18n( "Expand &Vertically" ), m_pMdi, SLOT( expandVertical() ) );
		m_pPlacingMenu->insertItem( i18n( "Expand &Horizontally" ), m_pMdi, SLOT( expandHorizontal() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile &Non-Overlapped" ), m_pMdi, SLOT( tileAnodine() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile Overla&pped" ), m_pMdi, SLOT( tilePragma() ) );
		m_pPlacingMenu->insertItem( i18n( "Tile V&ertically" ), m_pMdi, SLOT( tileVertically() ) );
		if ( m_mdiMode == K3Mdi::ToplevelMode )
		{
			m_pWindowMenu->setItemEnabled( placMenuId, false );
		}
		m_pWindowMenu->insertSeparator();
		int dockUndockId = m_pWindowMenu->insertItem( i18n( "&Dock/Undock" ), m_pDockMenu );
		m_pDockMenu->clear();
		m_pWindowMenu->insertSeparator();
		if ( noViewOpened )
		{
			m_pWindowMenu->setItemEnabled( placMenuId, false );
			m_pWindowMenu->setItemEnabled( dockUndockId, false );
		}
	}
	int entryCount = m_pWindowMenu->count();

	// for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
	int i = 100;
	K3MdiChildView* pView = 0L;
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	Q3ValueList<QDateTime> timeStamps;
	for ( ; it.current(); ++it )
	{
		pView = it.current();
		QDateTime timeStamp( pView->getTimeStamp() );

		if ( pView->isToolView() )
		{
			continue;
		}

		QString item;
		// set titles of minimized windows in brackets
		if ( pView->isMinimized() )
		{
			item += "(";
			item += pView->caption();
			item += ")";
		}
		else
		{
			item += " ";
			item += pView->caption();
		}

		// insert the window entry sorted by access time
		unsigned int indx;
		unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
		bool inserted = false;
		QString tmpString;
		Q3ValueList<QDateTime>::iterator timeStampIterator = timeStamps.begin();
		for ( indx = 0; indx <= windowItemCount; indx++, ++timeStampIterator )
		{
			bool putHere = false;
			if ( ( *timeStampIterator ) < timeStamp )
			{
				putHere = true;
				timeStamps.insert( timeStampIterator, timeStamp );
			}
			if ( putHere )
			{
				m_pWindowMenu->insertItem( item, pView, SLOT( slot_clickedInWindowMenu() ), 0, -1, indx + entryCount );
				if ( pView == m_pCurrentWindow )
				{
					m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx + entryCount ), true );
				}
				pView->setWindowMenuID( i );
				if ( !tabPageMode )
				{
					m_pDockMenu->insertItem( item, pView, SLOT( slot_clickedInDockMenu() ), 0, -1, indx );
					if ( pView->isAttached() )
					{
						m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx ), true );
					}
				}
				inserted = true;
				break;
				indx = windowItemCount + 1;  // break the loop
			}
		}
		if ( !inserted )
		{  // append it
			m_pWindowMenu->insertItem( item, pView, SLOT( slot_clickedInWindowMenu() ), 0, -1, windowItemCount + entryCount );
			if ( pView == m_pCurrentWindow )
			{
				m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( windowItemCount + entryCount ), true );
			}
			pView->setWindowMenuID( i );
			if ( !tabPageMode )
			{
				m_pDockMenu->insertItem( item, pView, SLOT( slot_clickedInDockMenu() ), 0, -1, windowItemCount );
				if ( pView->isAttached() )
				{
					m_pDockMenu->setItemChecked( m_pDockMenu->idAt( windowItemCount ), true );
				}
			}
		}
		i++;
	}
}

//================ windowMenuItemActivated ===============//

void K3MdiMainFrm::windowMenuItemActivated( int id )
{
	if ( id < 100 )
		return ;
	id -= 100;
	K3MdiChildView *pView = m_pDocumentViews->at( id );
	if ( !pView )
		return ;
	if ( pView->isMinimized() )
		pView->minimize();
	if ( m_mdiMode != K3Mdi::TabPageMode )
	{
		K3MdiChildFrm * pTopChild = m_pMdi->topChild();
		if ( pTopChild )
		{
			if ( ( pView == pTopChild->m_pClient ) && pView->isAttached() )
			{
				return ;
			}
		}
	}
	activateView( pView );
}

//================ dockMenuItemActivated ===============//

void K3MdiMainFrm::dockMenuItemActivated( int id )
{
	if ( id < 100 )
		return ;
	id -= 100;
	K3MdiChildView *pView = m_pDocumentViews->at( id );
	if ( !pView )
		return ;
	if ( pView->isMinimized() )
		pView->minimize();
	if ( pView->isAttached() )
	{
		detachWindow( pView, true );
	}
	else
	{   // is detached
		attachWindow( pView, true );
	}
}

//================ popupWindowMenu ===============//

void K3MdiMainFrm::popupWindowMenu( QPoint p )
{
	if ( !isFakingSDIApplication() )
	{
		m_pWindowMenu->popup( p );
	}
}

//================ dragEndTimeOut ===============//
void K3MdiMainFrm::dragEndTimeOut()
{
	// send drag end to all concerned views.
	K3MdiChildView * pView;
	for ( m_pDocumentViews->first(); ( pView = m_pDocumentViews->current() ) != 0L; m_pDocumentViews->next() )
	{
		K3MdiChildFrmDragEndEvent dragEndEvent( 0L );
		QApplication::sendEvent( pView, &dragEndEvent );
	}
}

//================ setFrameDecorOfAttachedViews ===============//

void K3MdiMainFrm::setFrameDecorOfAttachedViews( int frameDecor )
{
	switch ( frameDecor )
	{
	case 0:
		m_frameDecoration = K3Mdi::Win95Look;
		break;
	case 1:
		m_frameDecoration = K3Mdi::KDE1Look;
		break;
	case 2:
		m_frameDecoration = K3Mdi::KDELook;
		break;
	case 3:
		m_frameDecoration = K3Mdi::KDELaptopLook;
		break;
	default:
		qDebug( "unknown MDI decoration" );
		break;
	}
	setMenuForSDIModeSysButtons( m_pMainMenuBar );
	Q3PtrListIterator<K3MdiChildView> it( *m_pDocumentViews );
	for ( ; it.current(); ++it )
	{
		K3MdiChildView* pView = it.current();
		if ( pView->isToolView() )
			continue;
		if ( pView->isAttached() )
			pView->mdiParent() ->redecorateButtons();
	}
}

void K3MdiMainFrm::fakeSDIApplication()
{
	m_bSDIApplication = true;
	if ( m_pTaskBar )
		m_pTaskBar->close();
	m_pTaskBar = 0L;
}

void K3MdiMainFrm::closeViewButtonPressed()
{
	K3MdiChildView * pView = activeWindow();
	if ( pView )
	{
		pView->close();
	}
}

void K3MdiMainFrm::setManagedDockPositionModeEnabled( bool enabled )
{
	m_managedDockPositionMode = enabled;
}

void K3MdiMainFrm::setActiveToolDock( K3MdiDockContainer* td )
{
	if ( td == d->activeDockPriority[ 0 ] )
		return ;
	if ( d->activeDockPriority[ 0 ] == 0 )
	{
		d->activeDockPriority[ 0 ] = td;
		//        d->focusList=new K3MdiFocusList(this);
		//        if (m_pMdi)  d->focusList->addWidgetTree(m_pMdi);
		//        if (m_documentTabWidget) d->focusList->addWidgetTree(m_documentTabWidget);
		return ;
	}
	for ( int dst = 3, src = 2;src >= 0;dst--, src-- )
	{
		if ( d->activeDockPriority[ src ] == td )
			src--;
		if ( src < 0 )
			break;
		d->activeDockPriority[ dst ] = d->activeDockPriority[ src ];
	}
	d->activeDockPriority[ 0 ] = td;
}

void K3MdiMainFrm::removeFromActiveDockList( K3MdiDockContainer* td )
{
	for ( int i = 0;i < 4;i++ )
	{
		if ( d->activeDockPriority[ i ] == td )
		{
			for ( ;i < 3;i++ )
				d->activeDockPriority[ i ] = d->activeDockPriority[ i + 1 ];
			d->activeDockPriority[ 3 ] = 0;
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

void K3MdiMainFrm::prevToolViewInDock()
{
	K3MdiDockContainer * td = d->activeDockPriority[ 0 ];
	if ( !td )
		return ;
	td->prevToolView();
}

void K3MdiMainFrm::nextToolViewInDock()
{
	K3MdiDockContainer * td = d->activeDockPriority[ 0 ];
	if ( !td )
		return ;
	td->nextToolView();
}

K3Mdi::TabWidgetVisibility K3MdiMainFrm::tabWidgetVisibility()
{
	if ( m_documentTabWidget )
		return m_documentTabWidget->tabWidgetVisibility();

	return K3Mdi::NeverShowTabs;
}

void K3MdiMainFrm::setTabWidgetVisibility( K3Mdi::TabWidgetVisibility visibility )
{
	if ( m_documentTabWidget )
		m_documentTabWidget->setTabWidgetVisibility( visibility );
}

KTabWidget * K3MdiMainFrm::tabWidget() const
{
	return m_documentTabWidget;
}

#include "k3mdimainfrm.moc"

// vim: ts=2 sw=2 et
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
