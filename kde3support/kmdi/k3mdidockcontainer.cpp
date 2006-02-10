/* This file is part of the KDE project
 Copyright (C) 2002 Christoph Cullmann <cullmann@kde.org>
 Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/

#include "k3mdimainfrm.h"
#include "k3mdidockcontainer.h"
#include "k3mdidockcontainer.moc"

#include "k3dockwidget_private.h"

#include <q3widgetstack.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <kmultitabbar.h>
#include <kglobalsettings.h>
#include <QMouseEvent>

#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>

//TODO: Well, this is already defined in kdeui/k3dockwidget.cpp
static const char* const k3mdi_not_close_xpm[] =
    {
        "5 5 2 1",
        "# c black",
        ". c None",
        "#####",
        "#...#",
        "#...#",
        "#...#",
        "#####"
    };

K3MdiDockContainer::K3MdiDockContainer( QWidget *parent, QWidget *win, int position, int flags )
		: QWidget( parent ), K3DockContainer()
{
	m_tabSwitching = false;
	m_block = false;
	m_inserted = -1;
	m_mainWin = win;
	oldtab = -1;
	mTabCnt = 0;
	m_position = position;
	m_previousTab = -1;
	m_separatorPos = 18000;
	m_movingState = NotMoving;
	m_startEvent = 0;
	kDebug( 760 ) << k_funcinfo << endl;

	QBoxLayout *l;
	m_horizontal = ( ( position == K3DockWidget::DockTop ) || ( position == K3DockWidget::DockBottom ) );


	if ( m_horizontal )
		l = new QVBoxLayout( this ); //vertical layout for top and bottom docks
	else
		l = new QHBoxLayout( this ); //horizontal layout for left and right docks

	l->setAutoAdd( false );

	m_tb = new KMultiTabBar( m_horizontal ? KMultiTabBar::Horizontal : KMultiTabBar::Vertical, this );

	m_tb->setStyle( KMultiTabBar::KMultiTabBarStyle( flags ) );
	m_tb->showActiveTabTexts( true );

	KMultiTabBar::KMultiTabBarPosition kmtbPos;
	switch( position )
	{
	case K3DockWidget::DockLeft:
		kmtbPos = KMultiTabBar::Left;
		break;
	case K3DockWidget::DockRight:
		kmtbPos = KMultiTabBar::Right;
		break;
	case K3DockWidget::DockTop:
		kmtbPos = KMultiTabBar::Top;
		break;
	case K3DockWidget::DockBottom:
		kmtbPos = KMultiTabBar::Bottom;
		break;
	default:
		kmtbPos = KMultiTabBar::Right;
		break;
	}
	m_tb->setPosition( kmtbPos );

	m_ws = new Q3WidgetStack( this );

	m_ws->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

	//layout the tabbar
	if ( position == K3DockWidget::DockLeft || position == K3DockWidget::DockTop )
	{
		//add the tabbar then the widget stack
		l->add( m_tb );
		l->add( m_ws );
	}
	else
	{
		//add the widget stack then the tabbar
		l->add( m_ws );
		l->add( m_tb );
	}

	l->activate();
	m_ws->hide();

}

void K3MdiDockContainer::setStyle( int style )
{
	if ( m_tb )
		m_tb->setStyle( KMultiTabBar::KMultiTabBarStyle( style ) );
}

K3MdiDockContainer::~K3MdiDockContainer()
{
	QMap<K3DockWidget*, int>::iterator it;
	while ( m_map.count() )
	{
		it = m_map.begin();
		K3DockWidget *w = it.key();
		if ( m_overlapButtons.contains( w ) )
		{
			qobject_cast<K3DockWidgetHeader*>( w->getHeader() )->removeButton( m_overlapButtons[w] );
			m_overlapButtons.remove( w );
		}
		m_map.remove( w );
		w->undock();
	}
	deactivated( this );
}


void K3MdiDockContainer::init()
{
	bool overlap = isOverlapMode();
	kDebug( 760 ) << k_funcinfo << endl;
	if ( !m_horizontal )
	{
		kDebug( 760 ) << k_funcinfo << "Horizontal tabbar. Setting forced fixed width." << endl;
		parentDockWidget()->setForcedFixedWidth( m_tb->width() );
		activateOverlapMode( m_tb->width() );
	}
	else
	{
		kDebug( 760 ) << k_funcinfo << "Vertical tabbar. Setting forced fixed height." << endl;
		parentDockWidget()->setForcedFixedHeight( m_tb->height() );
		activateOverlapMode( m_tb->height() );
	}

	if (!overlap) deactivateOverlapMode();

	// try to restore splitter size
	if ( parentDockWidget() && parentDockWidget()->parent() )
	{
		K3DockSplitter * sp = (K3DockSplitter*)qobject_cast<K3DockSplitter*>( parentDockWidget()->parent() );
		if ( sp )
			sp->setSeparatorPosX( m_separatorPos );
	}
}

K3DockWidget* K3MdiDockContainer::parentDockWidget()
{
	return ( ( K3DockWidget* ) parent() );
}

void K3MdiDockContainer::insertWidget ( K3DockWidget *dwdg, QPixmap pixmap, const QString &text, int & )
{
	kDebug( 760 ) << k_funcinfo << "Adding a dockwidget to the dock container" << endl;
	K3DockWidget* w = dwdg;
	int tab;
	bool alreadyThere = m_map.contains( w );

	if ( alreadyThere )
	{
		tab = m_map[ w ];
		if ( m_ws->addWidget( w, tab ) != tab )
			kDebug( 760 ) << "ERROR COULDN'T READD WIDGET" << endl;

		kDebug( 760 ) << k_funcinfo << "Readded widget " << dwdg << endl;
	}
	else
	{
		tab = m_ws->addWidget( w );
		m_map.insert( w, tab );
		m_revMap.insert( tab, w );

		if ( ( ( K3DockWidget* ) parentWidget() ) ->mayBeShow() )
			( ( K3DockWidget* ) parentWidget() ) ->dockBack();

		if ( qobject_cast<K3DockWidgetHeader*>(w->getHeader()) )
		{
			kDebug( 760 ) << k_funcinfo << "The dockwidget we're adding has a header" << endl;
			kDebug( 760 ) << k_funcinfo << "Adding our overlap mode button to it" << endl;

			K3DockWidgetHeader *hdr = qobject_cast<K3DockWidgetHeader*>(w->getHeader());
			K3DockButton_Private *btn = new K3DockButton_Private( hdr, "OverlapButton" );

			btn->setToolTip(i18n( "Switch between overlap and side by side mode", "Overlap" ) );

			btn->setToggleButton( true );
			btn->setPixmap( const_cast< const char** >( k3mdi_not_close_xpm ) );
			hdr->addButton( btn );
			m_overlapButtons.insert( w, btn );
			btn->setOn( !isOverlapMode() );

			connect( btn, SIGNAL( clicked() ), this, SLOT( changeOverlapMode() ) );
		}

		m_tb->appendTab( pixmap.isNull() ? SmallIcon( "misc" ) : pixmap, tab, w->tabPageLabel() );
		m_tb->tab( tab )->installEventFilter( this );
		kDebug( 760 ) << k_funcinfo << "Added tab with label " << w->tabPageLabel() <<
			" to the tabbar" << endl;

		connect( m_tb->tab( tab ), SIGNAL( clicked( int ) ), this, SLOT( tabClicked( int ) ) );

		mTabCnt++;
		m_inserted = tab;
		int dummy = 0;
		K3DockContainer::insertWidget( w, pixmap, text, dummy );
		itemNames.append( w->name() );
		tabCaptions.insert( w->name(), w->tabPageLabel() );
		tabTooltips.insert( w->name(), w->toolTipString() );
	}

	//FB  m_ws->raiseWidget(tab);
}


bool K3MdiDockContainer::eventFilter( QObject *obj, QEvent *event )
{
	switch ( event->type() )
	{
	case QEvent::MouseButtonPress:
	{
		KMultiTabBarTab* kmtbTab = dynamic_cast<KMultiTabBarTab*>( obj );
		if ( !kmtbTab )
		{
			kDebug(760) << k_funcinfo << "Got a mouse button press but we have no tab" << endl;
			break;
		}

		K3DockWidget* w = m_revMap[ kmtbTab->id() ];
		if ( !w )
		{
			kDebug(760) << k_funcinfo << "Got a mouse button press but we have no widget" << endl;
			break;
		}

		if ( !w->getHeader() )
		{
			kDebug(760) << k_funcinfo << "Got a mouse button press but we have no header" << endl;
			break;
		}

		K3DockWidgetHeader *hdr = qobject_cast<K3DockWidgetHeader*>( w->getHeader() );
		if ( !hdr )
		{
			kDebug(760) << "Wrong header type in K3MdiDockContainer::eventFilter" << endl;
			break;
		}

		m_dockManager = w->dockManager();
		m_dragPanel = hdr->dragPanel();

		if ( m_dragPanel )
			m_movingState = WaitingForMoveStart;

		delete m_startEvent;
		m_startEvent = new QMouseEvent( * ( ( QMouseEvent* ) event ) );
	}
	break;
	case QEvent::MouseButtonRelease:
		if ( m_movingState == Moving )
		{
			m_movingState = NotMoving;
			QApplication::postEvent( m_dragPanel, new QMouseEvent( * ( ( QMouseEvent* ) event ) ) );
			delete m_startEvent;
			m_startEvent = 0;
		}
	case QEvent::MouseMove:
		if ( m_movingState == WaitingForMoveStart )
		{
			QPoint p( ( ( QMouseEvent* ) event )->pos() - m_startEvent->pos() );
			if ( p.manhattanLength() > KGlobalSettings::dndEventDelay() )
			{
				m_dockManager->eventFilter( m_dragPanel, m_startEvent );
				m_dockManager->eventFilter( m_dragPanel, event );
				m_movingState = Moving;
			}
		}
		else if ( m_movingState == Moving )
			m_dockManager->eventFilter( m_dragPanel, event );

		break;
	default:
		break;

	}
	return false;

}

void K3MdiDockContainer::showWidget( K3DockWidget *w )
{
	if ( !m_map.contains( w ) )
		return ;

	int id = m_map[ w ];
	m_tb->setTab( id, true );
	tabClicked( id );
}

void K3MdiDockContainer::changeOverlapMode()
{
	const K3DockButton_Private * btn = dynamic_cast<const K3DockButton_Private*>( sender() );

	if ( !btn )
		return ;

	if ( !btn->isOn() )
	{
		kDebug( 760 ) << k_funcinfo << "Activating overlap mode" << endl;
		if ( !m_horizontal )
			activateOverlapMode( m_tb->width() );
		else
			activateOverlapMode( m_tb->height() );

	}
	else
	{
		kDebug( 760 ) << k_funcinfo << "Deactivating overlap mode" << endl;
		deactivateOverlapMode();
	}

	QMap<K3DockWidget*, K3DockButton_Private*>::iterator it;
	for ( it = m_overlapButtons.begin(); it != m_overlapButtons.end(); ++it )
		it.data()->setOn( !isOverlapMode() );
}

void K3MdiDockContainer::hideIfNeeded()
{
	if ( itemNames.count() == 0 )
	{
		kDebug( 760 ) << k_funcinfo << "Hiding the dock container" << endl;
		( ( K3DockWidget* ) parentWidget() )->undock();
	}
}

void K3MdiDockContainer::removeWidget( K3DockWidget* dwdg )
{
	K3DockWidget * w = dwdg;
	if ( !m_map.contains( w ) )
		return; //we don't have this widget in our container

	kDebug( 760 ) << k_funcinfo << endl;
	//lower the tab. ( TODO: needed? )
	int id = m_map[ w ];
	if ( m_tb->isTabRaised( id ) )
	{
		m_tb->setTab( id, false );
		tabClicked( id );
	}

	m_tb->removeTab( id );
	m_ws->removeWidget( w );
	m_map.remove( w );
	m_revMap.remove( id );
	if ( m_overlapButtons.contains( w ) )
	{
		( (K3DockWidgetHeader*)qobject_cast<K3DockWidgetHeader*>( w->getHeader() ) )->removeButton( m_overlapButtons[ w ] );
		m_overlapButtons.remove( w );
	}
	K3DockContainer::removeWidget( w );
	itemNames.remove( w->name() );
	tabCaptions.remove( w->name() );
	tabTooltips.remove( w->name() );
	hideIfNeeded();
}

void K3MdiDockContainer::undockWidget( K3DockWidget *dwdg )
{
	K3DockWidget * w = dwdg;

	if ( !m_map.contains( w ) )
		return ;

	int id = m_map[ w ];
	if ( m_tb->isTabRaised( id ) )
	{
		kDebug( 760 ) << k_funcinfo << "Widget has been undocked, setting tab down" << endl;
		m_tb->setTab( id, false );
		tabClicked( id );
	}
}

void K3MdiDockContainer::tabClicked( int t )
{
	bool call_makeVisible = !m_tabSwitching;
	m_tabSwitching = true;
	if ( m_tb->isTabRaised( t ) )
	{
		kDebug( 760 ) << k_funcinfo << "Tab " << t << " was just activated" << endl;
		if ( m_ws->isHidden() )
		{
			kDebug( 760 ) << k_funcinfo << "Showing widgetstack for tab just clicked" << endl;
			m_ws->show();
			parentDockWidget()->restoreFromForcedFixedSize();
		}

		if ( !m_ws->widget( t ) )
		{
			kDebug( 760 ) << k_funcinfo << "Widget tab was clicked for is not in our stack" << endl;
			kDebug( 760 ) << k_funcinfo << "Docking it back in" << endl;
			m_revMap[t]->manualDock( parentDockWidget(), K3DockWidget::DockCenter, 20 );
			if ( call_makeVisible )
				m_revMap[t]->makeDockVisible();
			m_tabSwitching = false;
			emit activated( this );
			return ;
		}

		if ( m_ws->widget( t ) )
		{
			m_ws->raiseWidget( t );
			K3DockWidget * tmpDw = (K3DockWidget*)qobject_cast<K3DockWidget*>( m_ws->widget( t ) );
			if ( tmpDw )
			{
				if ( tmpDw->getWidget() )
					tmpDw->getWidget()->setFocus();
			}
			else
				kDebug( 760 ) << k_funcinfo << "Something really weird is going on" << endl;
		}
		else
			kDebug( 760 ) << k_funcinfo << "We have no widget to handle in our stack." << endl;

		if ( oldtab != t )
			m_tb->setTab( oldtab, false );

		m_tabSwitching = true;
		oldtab = t;
		emit activated( this );
	}
	else
	{
		kDebug( 760 ) << k_funcinfo << "Tab " << t << " was just deactiviated" << endl;
		// try save splitter position
		if ( parentDockWidget() && parentDockWidget()->parent() )
		{
			K3DockSplitter * sp = (K3DockSplitter*)static_cast<K3DockSplitter*>( parentDockWidget()->parent() );
			if ( sp )
				m_separatorPos = sp->separatorPos();
		}
		m_previousTab = t;
		//    oldtab=-1;
		if ( m_block )
			return ;
		emit deactivated( this );
		m_block = true;
		if ( m_ws->widget( t ) )
		{
			//    ((K3DockWidget*)m_ws->widget(t))->undock();
		}
		m_block = false;
		m_ws->hide ();


		kDebug( 760 ) << k_funcinfo << "Fixed Width:" << m_tb->width() << endl;
		if ( !m_horizontal )
			parentDockWidget()->setForcedFixedWidth( m_tb->width() ); // strange why it worked before at all
		else
			parentDockWidget()->setForcedFixedHeight( m_tb->height() ); // strange why it worked before at all
	}
	m_tabSwitching = false;
}

void K3MdiDockContainer::setToolTip ( K3DockWidget* w, QString &s )
{
	kDebug( 760 ) << k_funcinfo << "Setting tooltip '" << s << "' for widget " << w << endl;
	int tabId = m_map[w];
	KMultiTabBarTab *mbTab = m_tb->tab( tabId );
	mbTab->setToolTip(s);
}

void K3MdiDockContainer::setPixmap( K3DockWidget* widget , const QPixmap& pixmap )
{
	int id = m_ws->id( widget );
	if ( id == -1 )
		return ;
	KMultiTabBarTab *tab = m_tb->tab( id );
	tab->setIcon( pixmap.isNull() ? SmallIcon( "misc" ) : pixmap );
}

void K3MdiDockContainer::save( QDomElement& dockEl )
{
	QDomDocument doc = dockEl.ownerDocument();
	QDomElement el;
	el = doc.createElement( "name" );
	el.appendChild( doc.createTextNode( QString( "%1" ).arg( parent() ->name() ) ) );
	dockEl.appendChild( el );
	el = doc.createElement( "overlapMode" );
	el.appendChild( doc.createTextNode( isOverlapMode() ? "true" : "false" ) );
	dockEl.appendChild( el );
	QList<KMultiTabBarTab *> tl = m_tb->tabs();
	QList<KMultiTabBarTab *>::ConstIterator it = tl.begin();
	QStringList::ConstIterator it2 = itemNames.begin();
	int i = 0;
	for ( ;it != tl.end() && it2 != itemNames.end() ;++it, ++it2, ++i )
	{
		el = doc.createElement( "child" );
		el.setAttribute( "pos", QString( "%1" ).arg( i ) );
		QString s = tabCaptions[ *it2 ];
		if ( !s.isEmpty() )
		{
			el.setAttribute( "tabCaption", s );
		}
		s = tabTooltips[ *it2 ];
		if ( !s.isEmpty() )
		{
			el.setAttribute( "tabTooltip", s );
		}
		el.appendChild( doc.createTextNode( *it2 ) );
		dockEl.appendChild( el );
		if ( m_tb->isTabRaised( (*it)->id() ) )
		{
			QDomElement el2 = doc.createElement( "raised" );
			el2.appendChild( doc.createTextNode( m_ws->widget( (*it)->id() ) ->name() ) );
			el.appendChild( el2 );
		}
	}


}

void K3MdiDockContainer::load( QDomElement& dockEl )
{
	QString raise;

	for ( QDomNode n = dockEl.firstChild();!n.isNull();n = n.nextSibling() )
	{
		QDomElement el = n.toElement();
		if ( el.isNull() )
			continue;
		if ( el.tagName() == "overlapMode" )
		{
			if ( el.attribute( "overlapMode" ) != "false" )
				activateOverlapMode( m_horizontal?m_tb->height():m_tb->width() );
			else
				deactivateOverlapMode();
		}
		else if ( el.tagName() == "child" )
		{
			K3DockWidget * dw = ( ( K3DockWidget* ) parent() ) ->dockManager() ->getDockWidgetFromName( el.text() );
			if ( dw )
			{
				if ( el.hasAttribute( "tabCaption" ) )
				{
					dw->setTabPageLabel( el.attribute( "tabCaption" ) );
				}
				if ( el.hasAttribute( "tabTooltip" ) )
				{
					dw->setToolTipString( el.attribute( "tabTooltip" ) );
				}
				dw->manualDock( ( K3DockWidget* ) parent(), K3DockWidget::DockCenter );
			}
		}
	}

	m_ws->hide();
	if ( !m_horizontal )
		parentDockWidget()->setForcedFixedWidth( m_tb->width() );
	else
		parentDockWidget()->setForcedFixedHeight( m_tb->height() );

	QList<KMultiTabBarTab *> tl = m_tb->tabs();
	QList<KMultiTabBarTab *>::ConstIterator it1 = tl.begin();
	for ( ; it1 != tl.end(); ++it1 )
		m_tb->setTab( (*it1)->id(), false );

	kapp->syncX();
	m_delayedRaise = -1;

	for ( QMap<K3DockWidget*, K3DockButton_Private*>::iterator it = m_overlapButtons.begin();
	        it != m_overlapButtons.end();++it )
		it.data() ->setOn( !isOverlapMode() );

	if ( !raise.isEmpty() )
	{
		for ( QMap<K3DockWidget*, int>::iterator it = m_map.begin();it != m_map.end();++it )
		{
			if ( it.key() ->name() == raise )
			{
				m_delayedRaise = it.data();
				QTimer::singleShot( 0, this, SLOT( delayedRaise() ) );
				kDebug( 760 ) << k_funcinfo << "raising " << it.key()->name() << endl;
				break;
			}
		}

	}
	if ( m_delayedRaise == -1 )
		QTimer::singleShot( 0, this, SLOT( init() ) );
}

void K3MdiDockContainer::save( KConfig* cfg, const QString& group_or_prefix )
{
	QString grp = cfg->group();
	cfg->deleteGroup( group_or_prefix + QString( "::%1" ).arg( parent() ->name() ) );
	cfg->setGroup( group_or_prefix + QString( "::%1" ).arg( parent() ->name() ) );

	if ( isOverlapMode() )
		cfg->writeEntry( "overlapMode", "true" );
	else
		cfg->writeEntry( "overlapMode", "false" );

	// try to save the splitter position
	if ( parentDockWidget() && parentDockWidget() ->parent() )
	{
		K3DockSplitter * sp = (K3DockSplitter*) qobject_cast<K3DockSplitter*>( parentDockWidget()->parent() );
		if ( sp )
			cfg->writeEntry( "separatorPosition", m_separatorPos );
	}

	QList<KMultiTabBarTab *> tl = m_tb->tabs();
	QList<KMultiTabBarTab *>::ConstIterator it = tl.begin();
	QStringList::ConstIterator it2 = itemNames.begin();
	int i = 0;
	for ( ;it != tl.end() && it2 != itemNames.end() ;++it, ++it2, ++i )
	{
		//    cfg->writeEntry(QString("widget%1").arg(i),m_ws->widget((*it)->id())->name());
		cfg->writeEntry( QString( "widget%1" ).arg( i ), ( *it2 ) );
		QString s = tabCaptions[ *it2 ];
		if ( !s.isEmpty() )
		{
			cfg->writeEntry( QString( "widget%1-tabCaption" ).arg( i ), s );
		}
		s = tabTooltips[ *it2 ];
		if ( !s.isEmpty() )
		{
			cfg->writeEntry( QString( "widget%1-tabTooltip" ).arg( i ), s );
		}
		//    kDebug(760)<<"****************************************Saving: "<<m_ws->widget((*it)->id())->name()<<endl;
		if ( m_tb->isTabRaised( (*it) ->id() ) )
			cfg->writeEntry( m_ws->widget( (*it) ->id() ) ->name(), true );
		++i;
	}
	cfg->sync();
	cfg->setGroup( grp );

}

void K3MdiDockContainer::load( KConfig* cfg, const QString& group_or_prefix )
{
	QString grp = cfg->group();
	cfg->setGroup( group_or_prefix + QString( "::%1" ).arg( parent() ->name() ) );

	if ( cfg->readEntry( "overlapMode" ) != "false" )
		activateOverlapMode( m_horizontal?m_tb->height():m_tb->width() );
	else
		deactivateOverlapMode();

	m_separatorPos = cfg->readEntry( "separatorPosition", QVariant(18000 )).toInt();

	int i = 0;
	QString raise;
	while ( true )
	{
		QString dwn = cfg->readEntry( QString( "widget%1" ).arg( i ), QString() );
		if ( dwn.isEmpty() )
			break;
		kDebug( 760 ) << k_funcinfo << "configuring dockwidget :" << dwn << endl;
		K3DockWidget *dw = ( ( K3DockWidget* ) parent() ) ->dockManager() ->getDockWidgetFromName( dwn );
		if ( dw )
		{
			QString s = cfg->readEntry( QString( "widget%1-tabCaption" ).arg( i ), QString() );
			if ( !s.isEmpty() )
			{
				dw->setTabPageLabel( s );
			}
			s = cfg->readEntry( QString( "widget%1-tabTooltip" ).arg( i ), QString() );
			if ( !s.isEmpty() )
			{
				dw->setToolTipString( s );
			}
			dw->manualDock( ( K3DockWidget* ) parent(), K3DockWidget::DockCenter );
		}
		if ( cfg->readEntry( dwn, QVariant(false )).toBool() )
			raise = dwn;
		i++;

	}

	m_ws->hide();
	if ( !m_horizontal )
		parentDockWidget() ->setForcedFixedWidth( m_tb->width() );
	else
		parentDockWidget() ->setForcedFixedHeight( m_tb->height() );
	QList<KMultiTabBarTab *> tl = m_tb->tabs();
	QList<KMultiTabBarTab *>::ConstIterator it1 = tl.begin();
	for ( ; it1 != tl.end(); ++it1 )
		m_tb->setTab( (*it1)->id(), false );

	kapp->syncX();
	m_delayedRaise = -1;

	for ( QMap<K3DockWidget*, K3DockButton_Private*>::iterator it = m_overlapButtons.begin();
	        it != m_overlapButtons.end();++it )
		it.data() ->setOn( !isOverlapMode() );

	if ( !raise.isEmpty() )
	{
		for ( QMap<K3DockWidget*, int>::iterator it = m_map.begin();it != m_map.end();++it )
		{
			if ( it.key() ->name() == raise )
			{
				/*        tabClicked(it.data());
				        m_tb->setTab(it.data(),true);
				        tabClicked(it.data());
				        m_ws->raiseWidget(it.key());
				        kapp->sendPostedEvents();
				        kapp->syncX();*/

				m_delayedRaise = it.data();
				QTimer::singleShot( 0, this, SLOT( delayedRaise() ) );
				kDebug( 760 ) << k_funcinfo << "raising" << it.key() ->name() << endl;
				break;
			}
		}

	}
	if ( m_delayedRaise == -1 )
		QTimer::singleShot( 0, this, SLOT( init() ) );
	cfg->setGroup( grp );

}

void K3MdiDockContainer::delayedRaise()
{
	m_tb->setTab( m_delayedRaise, true );
	tabClicked( m_delayedRaise );
}

void K3MdiDockContainer::collapseOverlapped()
{
	//don't collapse if we're switching tabs
	if ( m_tabSwitching )
		return;

	if ( isOverlapMode() )
	{
		QList<KMultiTabBarTab *> tl = m_tb->tabs();
		QList<KMultiTabBarTab *>::ConstIterator it = tl.begin();
		for ( ; it != tl.end(); ++it )
		{
			if ( (*it)->isOn() )
			{
				kDebug( 760 ) << k_funcinfo << "lowering tab with id " << ( *it )->id() << endl;
				(*it)->setState( false );
				tabClicked( ( *it )->id() );
			}
		}
	}
}

void K3MdiDockContainer::toggle()
{
	kDebug( 760 ) << k_funcinfo << endl;

	if ( m_tb->isTabRaised( oldtab ) )
	{
		kDebug( 760 ) << k_funcinfo << "lowering tab" << endl;
		m_tb->setTab( oldtab, false );
		tabClicked( oldtab );
		K3MdiMainFrm *mainFrm = dynamic_cast<K3MdiMainFrm*>( m_mainWin );
		if ( mainFrm && mainFrm->activeWindow() )
			mainFrm->activeWindow()->setFocus();
	}
	else
	{
		kDebug( 760 ) << k_funcinfo << "raising tab" << endl;
		if ( m_tb->tab( m_previousTab ) == 0 )
		{
			if ( m_tb->tabs().count() == 0 )
				return ;

			m_previousTab = m_tb->tabs().first()->id();
		}
		m_tb->setTab( m_previousTab, true );
		tabClicked( m_previousTab );
	}
}

void K3MdiDockContainer::prevToolView()
{
	kDebug( 760 ) << k_funcinfo << endl;
	QList<KMultiTabBarTab *> tabs = m_tb->tabs();
	int pos = tabs.indexOf( m_tb->tab( oldtab ) );

	if ( pos == -1 )
		return ;

	pos--;
	if ( pos < 0 )
		pos = tabs.count() - 1;

	KMultiTabBarTab *tab = tabs.at( pos );
	if ( !tab )
		return ; //can never happen here, but who knows

	m_tb->setTab( tab->id(), true );
	tabClicked( tab->id() );
}

void K3MdiDockContainer::nextToolView()
{
	kDebug( 760 ) << k_funcinfo << endl;
	QList<KMultiTabBarTab *> tabs = m_tb->tabs();
	int pos = tabs.indexOf( m_tb->tab( oldtab ) );

	if ( pos == -1 )
		return ;

	pos++;
	if ( pos >= ( int ) tabs.count() )
		pos = 0;

	KMultiTabBarTab *tab = tabs.at( pos );
	if ( !tab )
		return ; //can never happen here, but who knows

	m_tb->setTab( tab->id(), true );
	tabClicked( tab->id() );
}

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
