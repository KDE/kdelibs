/* This file is part of the KDE libraries
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
  based on ktoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#include "k3mdiguiclient.h"
#include "k3mdiguiclient.moc"

#include <q3popupmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kaction.h>
#include <kxmlguifactory.h>
#include <qstring.h>
#include <assert.h>
#include <kdebug.h>
#include <k3dockwidget.h>
#include <kdeversion.h>
#include "k3mdimainfrm.h"
#include "k3mditoolviewaccessor.h"
#include "k3mditoolviewaccessor_p.h"
namespace
{
const char *actionListName = "show_k3mdi_document_tool_view_actions";

const char *guiDescription = ""
                             "<!DOCTYPE kpartgui><kpartgui name=\"K3MDIViewActions\">"
                             "<MenuBar>"
                             "    <Menu name=\"view\">"
                             "        <ActionList name=\"%1\" />"
                             "    </Menu>"
                             "</MenuBar>"
                             "</kpartgui>";

const char *resourceFileName = "k3mdiviewactions.rc";

}


using namespace K3MDIPrivate;



ToggleToolViewAction::ToggleToolViewAction( const QString& text, const KShortcut& cut, K3DockWidget *dw, K3MdiMainFrm *mdiMainFrm,
        KActionCollection* parent, const char* name )
		: KToggleAction( text, cut, parent, name ), m_dw( dw ), m_mdiMainFrm( mdiMainFrm )
{
	if ( m_dw )
	{
		connect( this, SIGNAL( toggled( bool ) ), this, SLOT( slotToggled( bool ) ) );
		connect( m_dw->dockManager(), SIGNAL( change() ), this, SLOT( anDWChanged() ) );
		connect( m_dw, SIGNAL( destroyed() ), this, SLOT( slotWidgetDestroyed() ) );
		setChecked( m_dw->mayBeHide() );
	}
}


ToggleToolViewAction::~ToggleToolViewAction()
{
	unplugAll();
}

void ToggleToolViewAction::anDWChanged()
{
	if ( isChecked() && m_dw->mayBeShow() )
		setChecked( false );
	else if ( ( !isChecked() ) && m_dw->mayBeHide() )
		setChecked( true );
	else if ( isChecked() && ( m_dw->parentDockTabGroup() &&
	                           ( qobject_cast<K3DockWidget*>( m_dw->parentDockTabGroup() ->
	                                                          parent() )->mayBeShow() ) ) )
		setChecked( false );
}


void ToggleToolViewAction::slotToggled( bool t )
{
	//  m_mw->mainDock->setDockSite( K3DockWidget::DockCorner );

	if ( ( !t ) && m_dw->mayBeHide() )
		m_dw->undock();
	else
		if ( t && m_dw->mayBeShow() )
			m_mdiMainFrm->makeDockVisible( m_dw );

	//  m_mw->mainDock->setDockSite( K3DockWidget::DockNone );
}

void ToggleToolViewAction::slotWidgetDestroyed()
{
	disconnect( m_dw->dockManager(), SIGNAL( change() ), this, SLOT( anDWChanged() ) );
	disconnect( this, SIGNAL( toggled( bool ) ), 0, 0 );
	unplugAll();
	deleteLater();
}


K3MDIGUIClient::K3MDIGUIClient( K3MdiMainFrm* mdiMainFrm, bool showMDIModeAction, const char* name ) : QObject( mdiMainFrm, name ),
		KXMLGUIClient( mdiMainFrm )
{
	m_mdiMode = K3Mdi::ChildframeMode;
	m_mdiMainFrm = mdiMainFrm;
	connect( mdiMainFrm->guiFactory(), SIGNAL( clientAdded( KXMLGUIClient * ) ),
	         this, SLOT( clientAdded( KXMLGUIClient * ) ) );

	/* re-use an existing resource file if it exists. can happen if the user launches the
	 * toolbar editor */
	/*
	setXMLFile( resourceFileName );
	*/

	if ( domDocument().documentElement().isNull() )
	{

		QString completeDescription = QString::fromLatin1( guiDescription )
		                              .arg( actionListName );

		setXML( completeDescription, false /*merge*/ );
	}

	if ( actionCollection() ->kaccel() == 0 )
		actionCollection() ->setWidget( mdiMainFrm );
	m_toolMenu = new KActionMenu( i18n( "Tool &Views" ), actionCollection(), "k3mdi_toolview_menu" );
	if ( showMDIModeAction )
	{
		m_mdiModeAction = new KSelectAction( i18n( "MDI Mode" ), 0, actionCollection() );
		QStringList modes;
		modes << i18n( "&Toplevel Mode" ) << i18n( "C&hildframe Mode" ) << i18n( "Ta&b Page Mode" ) << i18n( "I&DEAl Mode" );
		m_mdiModeAction->setItems( modes );
		connect( m_mdiModeAction, SIGNAL( activated( int ) ), this, SLOT( changeViewMode( int ) ) );
	}
	else
		m_mdiModeAction = 0;

	connect( m_mdiMainFrm, SIGNAL( mdiModeHasBeenChangedTo( K3Mdi::MdiMode ) ),
	         this, SLOT( mdiModeHasBeenChangedTo( K3Mdi::MdiMode ) ) );

	m_gotoToolDockMenu = new KActionMenu( i18n( "Tool &Docks" ), actionCollection(), "k3mdi_tooldock_menu" );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Switch Top Dock" ), Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_T, this, SIGNAL( toggleTop() ),
	                            actionCollection(), "k3mdi_activate_top" ) );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Switch Left Dock" ), Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_L, this, SIGNAL( toggleLeft() ),
	                            actionCollection(), "k3mdi_activate_left" ) );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Switch Right Dock" ), Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_R, this, SIGNAL( toggleRight() ),
	                            actionCollection(), "k3mdi_activate_right" ) );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Switch Bottom Dock" ), Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_B, this, SIGNAL( toggleBottom() ),
	                            actionCollection(), "k3mdi_activate_bottom" ) );
	m_gotoToolDockMenu->insert( new KActionSeparator( actionCollection(), "k3mdi_goto_menu_separator" ) );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Previous Tool View" ), Qt::ALT + Qt::CTRL + Qt::Key_Left, m_mdiMainFrm, SLOT( prevToolViewInDock() ),
	                            actionCollection(), "k3mdi_prev_toolview" ) );
	m_gotoToolDockMenu->insert( new KAction( i18n( "Next Tool View" ), Qt::ALT + Qt::CTRL + Qt::Key_Right, m_mdiMainFrm, SLOT( nextToolViewInDock() ),
	                            actionCollection(), "k3mdi_next_toolview" ) );

	actionCollection() ->readShortcutSettings( "Shortcuts", kapp->config() );
}

K3MDIGUIClient::~K3MDIGUIClient()
{

	//     actionCollection()->writeShortcutSettings( "K3MDI Shortcuts", kapp->config() );
	for ( uint i = 0;i < m_toolViewActions.count();i++ )
		disconnect( m_toolViewActions.at( i ), 0, this, 0 );

	m_toolViewActions.setAutoDelete( false );
	m_toolViewActions.clear();
	m_documentViewActions.setAutoDelete( false );
	m_documentViewActions.clear();
}

void K3MDIGUIClient::changeViewMode( int id )
{
	switch ( id )
	{
	case 0:
		m_mdiMainFrm->switchToToplevelMode();
		break;
	case 1:
		m_mdiMainFrm->switchToChildframeMode();
		break;
	case 2:
		m_mdiMainFrm->switchToTabPageMode();
		break;
	case 3:
		m_mdiMainFrm->switchToIDEAlMode();
		break;
	default:
		Q_ASSERT( 0 );
	}
}

void K3MDIGUIClient::setupActions()
{
	if ( !factory() || !m_mdiMainFrm )
		return ;

	//    BarActionBuilder builder( actionCollection(), m_mainWindow, m_toolBars );

	//    if ( !builder.needsRebuild() )
	//        return;


	unplugActionList( actionListName );

	//    m_actions.setAutoDelete( true );
	//    m_actions.clear();
	//    m_actions.setAutoDelete( false );

	//    m_actions = builder.create();

	//    m_toolBars = builder.toolBars();

	//    m_toolViewActions.append(new KAction( "TESTK3MDIGUICLIENT", QString::null, 0,
	//             this, SLOT(blah()),actionCollection(),"nothing"));

	Q3PtrList<KAction> addList;
	if ( m_toolViewActions.count() < 3 )
		for ( uint i = 0;i < m_toolViewActions.count();i++ )
			addList.append( m_toolViewActions.at( i ) );
	else
		addList.append( m_toolMenu );
	if ( m_mdiMode == K3Mdi::IDEAlMode )
		addList.append( m_gotoToolDockMenu );
	if ( m_mdiModeAction )
		addList.append( m_mdiModeAction );
	kdDebug( 760 ) << "K3MDIGUIClient::setupActions: plugActionList" << endl;
	plugActionList( actionListName, addList );

	//    connectToActionContainers();
}

void K3MDIGUIClient::addToolView( K3MdiToolViewAccessor* mtva )
{
	kdDebug( 760 ) << "*****void K3MDIGUIClient::addToolView(K3MdiToolViewAccessor* mtva)*****" << endl;
	//	kdDebug()<<"name: "<<mtva->wrappedWidget()->name()<<endl;
	QString aname = QString( "k3mdi_toolview_" ) + mtva->wrappedWidget() ->name();

	// try to read the action shortcut
	KShortcut sc;
	KConfig *cfg = kapp->config();
	QString _grp = cfg->group();
	cfg->setGroup( "Shortcuts" );
	// 	if ( cfg->hasKey( aname ) )
	sc = KShortcut( cfg->readEntry( aname, "" ) );
	cfg->setGroup( _grp );
	KAction *a = new ToggleToolViewAction( i18n( "Show %1" ).arg( mtva->wrappedWidget() ->caption() ),
	                                       /*QString::null*/sc, dynamic_cast<K3DockWidget*>( mtva->wrapperWidget() ),
	                                       m_mdiMainFrm, actionCollection(), aname.latin1() );
#if KDE_IS_VERSION(3,2,90)

	( ( ToggleToolViewAction* ) a ) ->setCheckedState( i18n( "Hide %1" ).arg( mtva->wrappedWidget() ->caption() ) );
#endif

	connect( a, SIGNAL( destroyed( QObject* ) ), this, SLOT( actionDeleted( QObject* ) ) );
	m_toolViewActions.append( a );
	m_toolMenu->insert( a );
	mtva->d->action = a;

	setupActions();
}

void K3MDIGUIClient::actionDeleted( QObject* a )
{
	m_toolViewActions.remove( static_cast<KAction*>( a ) );
	/*	if (!m_toolMenu.isNull()) m_toolMenu->remove(static_cast<KAction*>(a));*/
	setupActions();
}


void K3MDIGUIClient::clientAdded( KXMLGUIClient *client )
{
	if ( client == this )
		setupActions();
}


void K3MDIGUIClient::mdiModeHasBeenChangedTo( K3Mdi::MdiMode mode )
{
	kdDebug( 760 ) << "K3MDIGUIClient::mdiModeHasBennChangeTo" << endl;
	m_mdiMode = mode;
	if ( m_mdiModeAction )
	{
		switch ( mode )
		{
		case K3Mdi::ToplevelMode:
			m_mdiModeAction->setCurrentItem( 0 );
			break;
		case K3Mdi::ChildframeMode:
			m_mdiModeAction->setCurrentItem( 1 );
			break;
		case K3Mdi::TabPageMode:
			m_mdiModeAction->setCurrentItem( 2 );
			break;
		case K3Mdi::IDEAlMode:
			m_mdiModeAction->setCurrentItem( 3 );
			break;
		default:
			Q_ASSERT( 0 );
		}
	}
	setupActions();

}


// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
