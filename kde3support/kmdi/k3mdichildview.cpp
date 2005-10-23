//----------------------------------------------------------------------------
//    filename             : k3mdichildview.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create a
//                           -06/2000      stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           02/2001       by Eva Brucherseifer (eva@rt.e-technik.tu-darmstadt.de)
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

#include "k3mdichildview.h"
#include "k3mdichildview.moc"

#include <qdatetime.h>
#include <qobject.h>

#include "k3mdimainfrm.h"
#include "k3mdichildfrm.h"
#include "k3mdidefines.h"
#include <kdebug.h>
#include <klocale.h>
#include <qicon.h>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>

//============ K3MdiChildView ============//

K3MdiChildView::K3MdiChildView( const QString& caption, QWidget* parentWidget, const char* name, Qt::WFlags f )
	: QWidget( parentWidget, name, f )
	, m_focusedChildWidget( 0L )
	, m_firstFocusableChildWidget( 0L )
	, m_lastFocusableChildWidget( 0L )
	, m_stateChanged( true )
	, m_bToolView( false )
	, m_bInterruptActivation( false )
	, m_bMainframesActivateViewIsPending( false )
	, m_bFocusInEventIsPending( false )
	, m_trackChanges( 0 )
{
	setGeometry( 0, 0, 0, 0 );  // reset
	if ( !caption.isEmpty() )
		m_szCaption = caption;
	else
		m_szCaption = i18n( "Unnamed" );
	
	m_sTabCaption = m_szCaption;
	setFocusPolicy( Qt::ClickFocus );
	installEventFilter( this );
	
	// store the current time
	updateTimeStamp();
}


//============ K3MdiChildView ============//

K3MdiChildView::K3MdiChildView( QWidget* parentWidget, const char* name, Qt::WFlags f )
	: QWidget( parentWidget, name, f )
	, m_focusedChildWidget( 0L )
	, m_firstFocusableChildWidget( 0L )
	, m_lastFocusableChildWidget( 0L )
	, m_stateChanged( true )
	, m_bToolView( false )
	, m_bInterruptActivation( false )
	, m_bMainframesActivateViewIsPending( false )
	, m_bFocusInEventIsPending( false )
	, m_trackChanges( 0 )
{
	setGeometry( 0, 0, 0, 0 );  // reset
	m_szCaption = i18n( "Unnamed" );
	m_sTabCaption = m_szCaption;
	setFocusPolicy( Qt::ClickFocus );
	installEventFilter( this );

	// store the current time
	updateTimeStamp();
}

//============ ~K3MdiChildView ============//

K3MdiChildView::~K3MdiChildView()
{
	kdDebug( 760 ) << k_funcinfo << endl;
}

void K3MdiChildView::trackIconAndCaptionChanges( QWidget *view )
{
	m_trackChanges = view;
}


//============== internal geometry ==============//

QRect K3MdiChildView::internalGeometry() const
{
	if ( mdiParent() )
	{ // is attached
		// get the client area coordinates inside the MDI child frame
		QRect posInFrame = geometry();
		// map these values to the parent of the MDI child frame
		// (this usually is the MDI child area) and return
		QPoint ptTopLeft = mdiParent() ->mapToParent( posInFrame.topLeft() );
		QSize sz = size();
		return QRect( ptTopLeft, sz );
	}
	else
	{
		QRect geo = geometry();
		QRect frameGeo = externalGeometry();
		return QRect( frameGeo.x(), frameGeo.y(), geo.width(), geo.height() );
		//      return geometry();
	}
}

//============== set internal geometry ==============//

void K3MdiChildView::setInternalGeometry( const QRect& newGeometry )
{
	if ( mdiParent() )
	{ // is attached
		// retrieve the frame size
		QRect geo = internalGeometry();
		QRect frameGeo = externalGeometry();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the QWidget::setGeometry() method
		QRect newGeoQt;
		newGeoQt.setX( newGeometry.x() - nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() - nFrameSizeTop );

		newGeoQt.setWidth( newGeometry.width() + nFrameSizeLeft + K3MDI_CHILDFRM_DOUBLE_BORDER / 2 );
		newGeoQt.setHeight( newGeometry.height() + nFrameSizeTop + K3MDI_CHILDFRM_DOUBLE_BORDER / 2 );
		//      newGeoQt.setWidth(newGeometry.width()+K3MDI_MDI_CHILDFRM_DOUBLE_BORDER);
		//      newGeoQt.setHeight(newGeometry.height()+mdiParent()->captionHeight()+K3MDI_MDI_CHILDFRM_DOUBLE_BORDER);

		// set the geometry
		mdiParent()->setGeometry( newGeoQt );
	}
	else
	{
		// retrieve the frame size
		QRect geo = internalGeometry();
		QRect frameGeo = externalGeometry();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the QWidget::setGeometry() method
		QRect newGeoQt;

		newGeoQt.setX( newGeometry.x() - nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() - nFrameSizeTop );

		newGeoQt.setWidth( newGeometry.width() );
		newGeoQt.setHeight( newGeometry.height() );

		// set the geometry
		setGeometry( newGeoQt );
	}
}

//============== external geometry ==============//

QRect K3MdiChildView::externalGeometry() const
{
	return mdiParent() ? mdiParent()->frameGeometry() : frameGeometry();
}

//============== set external geometry ==============//

void K3MdiChildView::setExternalGeometry( const QRect& newGeometry )
{
	if ( mdiParent() )
	{ // is attached
		mdiParent() ->setGeometry( newGeometry );
	}
	else
	{
		// retrieve the frame size
		QRect geo = internalGeometry();
		QRect frameGeo = externalGeometry();
		int nTotalFrameWidth = frameGeo.width() - geo.width();
		int nTotalFrameHeight = frameGeo.height() - geo.height();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the QWidget::setGeometry() method
		// not attached => the window system makes the frame
		QRect newGeoQt;
		newGeoQt.setX( newGeometry.x() + nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() + nFrameSizeTop );
		newGeoQt.setWidth( newGeometry.width() - nTotalFrameWidth );
		newGeoQt.setHeight( newGeometry.height() - nTotalFrameHeight );

		// set the geometry
		setGeometry( newGeoQt );
	}
}

//============== minimize ==============//

void K3MdiChildView::minimize( bool bAnimate )
{
	if ( mdiParent() )
	{
		if ( !isMinimized() )
		{
			mdiParent() ->setState( K3MdiChildFrm::Minimized, bAnimate );
		}
	}
	else
		showMinimized();
}

void K3MdiChildView::showMinimized()
{
	emit isMinimizedNow();
	QWidget::showMinimized();
}

//slot:
void K3MdiChildView::minimize()
{
	minimize( true );
}

//============= maximize ==============//

void K3MdiChildView::maximize( bool bAnimate )
{
	if ( mdiParent() )
	{
		if ( !isMaximized() )
		{
			mdiParent() ->setState( K3MdiChildFrm::Maximized, bAnimate );
			emit mdiParentNowMaximized( true );
		}
	}
	else
		showMaximized();
}

void K3MdiChildView::showMaximized()
{
	emit isMaximizedNow();
	QWidget::showMaximized();
}

//slot:
void K3MdiChildView::maximize()
{
	maximize( true );
}

//============== restoreGeometry ================//

QRect K3MdiChildView::restoreGeometry()
{
	if ( mdiParent() )
		return mdiParent() ->restoreGeometry();
	else //FIXME not really supported, may be we must use Windows or X11 funtions
		return geometry();
}

//============== setRestoreGeometry ================//

void K3MdiChildView::setRestoreGeometry( const QRect& newRestGeo )
{
	if ( mdiParent() )
		mdiParent()->setRestoreGeometry( newRestGeo );
}

//============== attach ================//

void K3MdiChildView::attach()
{
	emit attachWindow( this, true );
}

//============== detach =================//

void K3MdiChildView::detach()
{
	emit detachWindow( this, true );
}

//=============== isMinimized ? =================//

bool K3MdiChildView::isMinimized() const
{
	if ( mdiParent() )
		return ( mdiParent()->state() == K3MdiChildFrm::Minimized );
	else
		return QWidget::isMinimized();
}

//============== isMaximized ? ==================//

bool K3MdiChildView::isMaximized() const
{
	if ( mdiParent() )
		return ( mdiParent()->state() == K3MdiChildFrm::Maximized );
	else
		return QWidget::isMaximized();
}

//============== restore ================//

void K3MdiChildView::restore()
{
	if ( mdiParent() )
	{
		if ( isMaximized() )
			emit mdiParentNowMaximized( false );
		
		if ( isMinimized() || isMaximized() )
			mdiParent()->setState( K3MdiChildFrm::Normal );
	}
	else
		showNormal();
}

void K3MdiChildView::showNormal()
{
	emit isRestoredNow();
	QWidget::showNormal();
}

//=============== youAreAttached ============//

void K3MdiChildView::youAreAttached( K3MdiChildFrm *lpC )
{
	lpC->setCaption( m_szCaption );
	emit isAttachedNow();
}

//================ youAreDetached =============//

void K3MdiChildView::youAreDetached()
{
	setCaption( m_szCaption );

	setTabCaption( m_sTabCaption );
	if ( myIconPtr() )
		setIcon( *( myIconPtr() ) );
	
	setFocusPolicy( Qt::StrongFocus );

	emit isDetachedNow();
}

//================ setCaption ================//
// this set the caption of only the window
void K3MdiChildView::setCaption( const QString& szCaption )
{
	// this will work only for window
	m_szCaption = szCaption;
	if ( mdiParent() )
		mdiParent() ->setCaption( m_szCaption );
	else //have to call the parent one
		QWidget::setCaption( m_szCaption );

	emit windowCaptionChanged( m_szCaption );
}

//============== closeEvent ================//

void K3MdiChildView::closeEvent( QCloseEvent *e )
{
	e->ignore(); //we ignore the event , and then close later if needed.
	emit childWindowCloseRequest( this );
}

//================ myIconPtr =================//

QPixmap* K3MdiChildView::myIconPtr()
{
	return 0;
}

//============= focusInEvent ===============//

void K3MdiChildView::focusInEvent( QFocusEvent *e )
{
	QWidget::focusInEvent( e );

	// every widget get a focusInEvent when a popup menu is opened!?! -> maybe bug of QT
	if ( e && ( ( e->reason() ) == Qt::PopupFocusReason ) )
		return ;


	m_bFocusInEventIsPending = true;
	activate();
	m_bFocusInEventIsPending = false;

	emit gotFocus( this );
}

//============= activate ===============//

void K3MdiChildView::activate()
{
	// avoid circularity
	static bool s_bActivateIsPending = false;
	if ( s_bActivateIsPending )
		return ;
	
	s_bActivateIsPending = true;

	// raise the view and push the taskbar button
	if ( !m_bMainframesActivateViewIsPending )
		emit focusInEventOccurs( this );

	// if this method was called directly, check if the mainframe wants that we interrupt
	if ( m_bInterruptActivation )
		m_bInterruptActivation = false;
	else
	{
		if ( !m_bFocusInEventIsPending )
			setFocus();
		
		kdDebug( 760 ) << k_funcinfo << endl;
		emit activated( this );
	}

	if ( m_focusedChildWidget != 0L )
		m_focusedChildWidget->setFocus();
	else
	{
		if ( m_firstFocusableChildWidget != 0L )
		{
			m_firstFocusableChildWidget->setFocus();
			m_focusedChildWidget = m_firstFocusableChildWidget;
		}
	}
	s_bActivateIsPending = false;
}

//============= focusOutEvent ===============//

void K3MdiChildView::focusOutEvent( QFocusEvent* e )
{
	QWidget::focusOutEvent( e );
	emit lostFocus( this );
}

//============= resizeEvent ===============//

void K3MdiChildView::resizeEvent( QResizeEvent* e )
{
	QWidget::resizeEvent( e );

	if ( m_stateChanged )
	{
		m_stateChanged = false;
		if ( isMaximized() )
		{ //maximized
			emit isMaximizedNow();
		}
		else if ( isMinimized() )
		{ //minimized
			emit isMinimizedNow();
		}
		else
		{ //is restored
			emit isRestoredNow();
		}
	}
}

void K3MdiChildView::slot_childDestroyed()
{
	// do what we do if a child is removed

	// if we lost a child we uninstall ourself as event filter for the lost
	// child and its children
	QObject* pLostChild = const_cast<QObject*>( QObject::sender() );
	if ( pLostChild && ( pLostChild->isWidgetType() ) )
	{
		QList<QWidget *> list = findChildren<QWidget *>();
		list.insert( 0, qobject_cast<QWidget *>(pLostChild) );        // add the lost child to the list too, just to save code

		foreach(QWidget *widg, list)
		{ // for each found object...
			widg->removeEventFilter( this );
			if ( m_firstFocusableChildWidget == widg )
				m_firstFocusableChildWidget = 0L;   // reset first widget
			
			if ( m_lastFocusableChildWidget == widg )
				m_lastFocusableChildWidget = 0L;    // reset last widget
			
			if ( m_focusedChildWidget == widg )
				m_focusedChildWidget = 0L;          // reset focused widget
		}
	}
}

//============= eventFilter ===============//
bool K3MdiChildView::eventFilter( QObject *obj, QEvent *e )
{
	if ( e->type() == QEvent::KeyPress && isAttached() )
	{
		QKeyEvent* ke = ( QKeyEvent* ) e;
		if ( ke->key() == Qt::Key_Tab )
		{
			QWidget* w = ( QWidget* ) obj;
			Qt::FocusPolicy wfp = w->focusPolicy();
			if ( wfp == Qt::StrongFocus || wfp == Qt::TabFocus || w->focusPolicy() == Qt::WheelFocus )
			{
				if ( m_lastFocusableChildWidget != 0 )
				{
					if ( w == m_lastFocusableChildWidget )
					{
						if ( w != m_firstFocusableChildWidget )
							m_firstFocusableChildWidget->setFocus();
					}
				}
			}
		}
	}
	else if ( e->type() == QEvent::FocusIn )
	{
		if ( obj->isWidgetType() )
		{
			QObjectList list = queryList( "QWidget" );
			if ( list.indexOf( obj ) != -1 )
				m_focusedChildWidget = ( QWidget* ) obj;
		}
		if ( !isAttached() )
		{   // is toplevel, for attached views activation is done by main frame event filter
			static bool m_bActivationIsPending = false;
			if ( !m_bActivationIsPending )
			{
				m_bActivationIsPending = true;
				activate(); // sets the focus
				m_bActivationIsPending = false;
			}
		}
	}
	else if ( e->type() == QEvent::ChildRemoved )
	{
		// if we lost a child we uninstall ourself as event filter for the lost
		// child and its children
		QObject * pLostChild = ( ( QChildEvent* ) e ) ->child();
		if ( ( pLostChild != 0L ) && ( pLostChild->isWidgetType() ) )
		{
			QObjectList list = pLostChild->queryList( "QWidget" );
			list.insert( 0, pLostChild );        // add the lost child to the list too, just to save code
			QObjectList::iterator it = list.begin();          // iterate over all lost child widgets
			QObject * o;
			while ( ( o = (*it) ) != 0 )
			{ // for each found object...
				QWidget * widg = ( QWidget* ) o;
				++it;
				widg->removeEventFilter( this );
				Qt::FocusPolicy wfp = widg->focusPolicy();
				if ( wfp == Qt::StrongFocus || wfp == Qt::TabFocus || widg->focusPolicy() == Qt::WheelFocus )
				{
					if ( m_firstFocusableChildWidget == widg )
						m_firstFocusableChildWidget = 0L;   // reset first widget
					
					if ( m_lastFocusableChildWidget == widg )
						m_lastFocusableChildWidget = 0L;    // reset last widget
				}
			}
		}
	}
	else if ( e->type() == QEvent::ChildInserted )
	{
		// if we got a new child and we are attached to the MDI system we
		// install ourself as event filter for the new child and its children
		// (as we did when we were added to the MDI system).
		QObject * pNewChild = ( ( QChildEvent* ) e ) ->child();
		if ( ( pNewChild != 0L ) && ( pNewChild->isWidgetType() ) )
		{
			QWidget * pNewWidget = ( QWidget* ) pNewChild;
			if ( (pNewWidget->windowFlags() & Qt::Dialog) == Qt::Dialog &&
			     pNewWidget->testAttribute( Qt::WA_ShowModal ) )
				return false;
			QObjectList list = pNewWidget->queryList( "QWidget" );
			list.insert( 0, pNewChild );         // add the new child to the list too, just to save code
			foreach ( QObject *o, list ) {
				QWidget * widg = ( QWidget* ) o;
				widg->installEventFilter( this );
				connect( widg, SIGNAL( destroyed() ), this, SLOT( slot_childDestroyed() ) );
				Qt::FocusPolicy wfp = widg->focusPolicy();
				if ( wfp == Qt::StrongFocus || wfp == Qt::TabFocus || widg->focusPolicy() == Qt::WheelFocus )
				{
					if ( m_firstFocusableChildWidget == 0 )
						m_firstFocusableChildWidget = widg;  // first widge
					
					m_lastFocusableChildWidget = widg; // last widget
				}
			}
		}
	}
	else
	{
		if ( e->type() == QEvent::WindowIconChange )
		{
			//            qDebug("KMDiChildView:: QEvent:IconChange intercepted\n");
			if ( obj == this )
				iconUpdated( this, icon() ? ( *icon() ) : QPixmap() );
			else if ( obj == m_trackChanges )
				setIcon( m_trackChanges->icon() ? ( *( m_trackChanges->icon() ) ) : QPixmap() );
		}
		if ( e->type() == QEvent::WindowTitleChange )
		{
			if ( obj == this )
				captionUpdated( this, caption() );
		}
	}

	return false;                           // standard event processing
}

/** Switches interposing in event loop of all current child widgets off. */
void K3MdiChildView::removeEventFilterForAllChildren()
{
	QObjectList list = queryList( "QWidget" );
	QObjectList::iterator it = list.begin();          // iterate over all child widgets
	QObject* obj;
	while ( ( obj = (*it) ) != 0 )
	{ // for each found object...
		QWidget* widg = ( QWidget* ) obj;
		++it;
		widg->removeEventFilter( this );
	}
}

QWidget* K3MdiChildView::focusedChildWidget()
{
	return m_focusedChildWidget;
}

void K3MdiChildView::setFirstFocusableChildWidget( QWidget* firstFocusableChildWidget )
{
	m_firstFocusableChildWidget = firstFocusableChildWidget;
}

void K3MdiChildView::setLastFocusableChildWidget( QWidget* lastFocusableChildWidget )
{
	m_lastFocusableChildWidget = lastFocusableChildWidget;
}

/** Set a new value of  the task bar button caption  */
void K3MdiChildView::setTabCaption ( const QString& stbCaption )
{
	m_sTabCaption = stbCaption;
	emit tabCaptionChanged( m_sTabCaption );
}

void K3MdiChildView::setMDICaption ( const QString& caption )
{
	setCaption( caption );
	setTabCaption( caption );
}

/** sets an ID  */
void K3MdiChildView::setWindowMenuID( int id )
{
	m_windowMenuID = id;
}

//============= slot_clickedInWindowMenu ===============//

/** called if someone click on the "Window" menu item for this child frame window */
void K3MdiChildView::slot_clickedInWindowMenu()
{
	updateTimeStamp();
	emit clickedInWindowMenu( m_windowMenuID );
}

//============= slot_clickedInDockMenu ===============//

/** called if someone click on the "Dock/Undock..." menu item for this child frame window */
void K3MdiChildView::slot_clickedInDockMenu()
{
	emit clickedInDockMenu( m_windowMenuID );
}

//============= setMinimumSize ===============//

void K3MdiChildView::setMinimumSize( int minw, int minh )
{
	QWidget::setMinimumSize( minw, minh );
	if ( mdiParent() && mdiParent()->state() != K3MdiChildFrm::Minimized )
	{
		mdiParent() ->setMinimumSize( minw + K3MDI_CHILDFRM_DOUBLE_BORDER,
		                              minh + K3MDI_CHILDFRM_DOUBLE_BORDER + K3MDI_CHILDFRM_SEPARATOR + mdiParent() ->captionHeight() );
	}
}

//============= setMaximumSize ===============//

void K3MdiChildView::setMaximumSize( int maxw, int maxh )
{
	if ( mdiParent() && mdiParent()->state() == K3MdiChildFrm::Normal )
	{
		int w = maxw + K3MDI_CHILDFRM_DOUBLE_BORDER;
		if ( w > QWIDGETSIZE_MAX )
			w = QWIDGETSIZE_MAX;

		int h = maxh + K3MDI_CHILDFRM_DOUBLE_BORDER + K3MDI_CHILDFRM_SEPARATOR + mdiParent() ->captionHeight();
		if ( h > QWIDGETSIZE_MAX )
			h = QWIDGETSIZE_MAX;

		mdiParent()->setMaximumSize( w, h );
	}
	QWidget::setMaximumSize( maxw, maxh );
}

//============= show ===============//

void K3MdiChildView::show()
{
	if ( mdiParent() )
		mdiParent()->show();

	QWidget::show();
}

//============= hide ===============//

void K3MdiChildView::hide()
{
	if ( mdiParent() )
		mdiParent()->hide();
	
	QWidget::hide();
}

//============= raise ===============//

void K3MdiChildView::raise()
{
	if ( mdiParent() )  //TODO Check Z-order
		mdiParent()->raise();

	QWidget::raise();
}

// kate: space-indent off; replace-tabs off; indent-mode csands; tab-width 4;
