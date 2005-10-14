/* This file is part of the KDE libraries
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "ktoolbarhandler.h"

#include <QMenu>

#include <kaction.h>
#include <kauthorized.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include <assert.h>

namespace
{
    const char *actionListName = "show_menu_and_toolbar_actionlist";

    const char *guiDescription = ""
        "<!DOCTYPE kpartgui><kpartgui name=\"StandardToolBarMenuHandler\">"
        "<MenuBar>"
        "    <Menu name=\"settings\">"
        "        <ActionList name=\"%1\" />"
        "    </Menu>"
        "</MenuBar>"
        "</kpartgui>";

    const char *resourceFileName = "barhandler.rc";

    class BarActionBuilder
    {
    public:
        BarActionBuilder( KActionCollection *actionCollection, KMainWindow *mainWindow, QLinkedList<KToolBar*> &oldToolBarList )
            : m_actionCollection( actionCollection ), m_mainWindow( mainWindow ), m_needsRebuild( false )
        {
            QList<KToolBar*>  toolBars = qFindChildren<KToolBar*>(m_mainWindow);
            foreach( KToolBar * toolBar, toolBars) {
		if (toolBar->mainWindow()!=m_mainWindow) continue;
                if ( !oldToolBarList.contains( toolBar ) )
                    m_needsRebuild = true;

                m_toolBars.append( toolBar );
            }

            if ( !m_needsRebuild )
                m_needsRebuild = ( oldToolBarList.count() != m_toolBars.count() );
        }

        bool needsRebuild() const { return m_needsRebuild; }

        Q3PtrList<KAction> create()
        {
            if ( !m_needsRebuild )
                return Q3PtrList<KAction>();

            foreach(KToolBar* bar, m_toolBars)
                handleToolBar( bar );

            Q3PtrList<KAction> actions;

            if ( m_toolBarActions.count() == 0 )
                return actions;

            if ( m_toolBarActions.count() == 1 ) {
                KToggleToolBarAction* action = static_cast<KToggleToolBarAction *>( m_toolBarActions.getFirst() );
                action->setText( i18n( "Show Toolbar" ) );
                action->setCheckedState( i18n( "Hide Toolbar" ) );
                return m_toolBarActions;
            }

            KActionMenu *menuAction = new KActionMenu( i18n( "Toolbars" ), m_actionCollection, "toolbars_submenu_action" );

            Q3PtrListIterator<KAction> actionIt( m_toolBarActions );
            for ( ; actionIt.current(); ++actionIt )
                menuAction->insert( actionIt.current() );

            actions.append( menuAction );
            return actions;
        }

        const QLinkedList<KToolBar*> &toolBars() const { return m_toolBars; }

    private:
        void handleToolBar( KToolBar *toolBar )
        {
            KToggleToolBarAction *action = new KToggleToolBarAction(
                toolBar,
                toolBar->label(),
                m_actionCollection,
                toolBar->name() );
            // ## tooltips, whatsthis?
            m_toolBarActions.append( action );
        }

        KActionCollection *m_actionCollection;
        KMainWindow *m_mainWindow;

        QLinkedList<KToolBar*> m_toolBars;
        Q3PtrList<KAction> m_toolBarActions;

        bool m_needsRebuild : 1;
    };
}

using namespace KDEPrivate;

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow )
    : QObject( mainWindow ), KXMLGUIClient( mainWindow )
{
    init( mainWindow );
}

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow, QObject *parent )
    : QObject( parent ), KXMLGUIClient( mainWindow )
{
    init( mainWindow );
}

ToolBarHandler::~ToolBarHandler()
{
    m_actions.setAutoDelete( true );
    m_actions.clear();
}

KAction *ToolBarHandler::toolBarMenuAction()
{
    assert( m_actions.count() == 1 );
    return m_actions.getFirst();
}

void ToolBarHandler::setupActions()
{
    if ( !factory() || !m_mainWindow )
        return;

    BarActionBuilder builder( actionCollection(), m_mainWindow, m_toolBars );

    if ( !builder.needsRebuild() )
        return;

    unplugActionList( actionListName );

    m_actions.setAutoDelete( true );
    m_actions.clear();
    m_actions.setAutoDelete( false );

    m_actions = builder.create();

    /*
    for (  QPtrListIterator<KToolBar> toolBarIt( m_toolBars );
           toolBarIt.current(); ++toolBarIt )
        toolBarIt.current()->disconnect( this );
        */

    m_toolBars = builder.toolBars();

    /*
    for (  QPtrListIterator<KToolBar> toolBarIt( m_toolBars );
           toolBarIt.current(); ++toolBarIt )
        connect( toolBarIt.current(), SIGNAL( destroyed() ),
                 this, SLOT( setupActions() ) );
                 */

    if (KAuthorized::authorizeKAction("options_show_toolbar"))
	plugActionList( actionListName, m_actions );

    connectToActionContainers();
}

void ToolBarHandler::clientAdded( KXMLGUIClient *client )
{
    if ( client == this )
        setupActions();
}

void ToolBarHandler::init( KMainWindow *mainWindow )
{
    d = 0;
    m_mainWindow = mainWindow;

    connect( m_mainWindow->guiFactory(), SIGNAL( clientAdded( KXMLGUIClient * ) ),
             this, SLOT( clientAdded( KXMLGUIClient * ) ) );

    /* re-use an existing resource file if it exists. can happen if the user launches the
     * toolbar editor */
    /*
    setXMLFile( resourceFileName );
    */

    if ( domDocument().documentElement().isNull() ) {

        QString completeDescription = QString::fromLatin1( guiDescription )
            .arg( actionListName );

        setXML( completeDescription, false /*merge*/ );
    }
}

void ToolBarHandler::connectToActionContainers()
{
    Q3PtrListIterator<KAction> actionIt( m_actions );
    for ( ; actionIt.current(); ++actionIt )
        connectToActionContainer( actionIt.current() );
}

void ToolBarHandler::connectToActionContainer( KAction *action )
{
    uint containerCount = action->containerCount();
    for ( uint i = 0; i < containerCount; ++i )
        connectToActionContainer( action->container( i ) );
}

void ToolBarHandler::connectToActionContainer( QWidget *container )
{
    QMenu *popupMenu = dynamic_cast<QMenu *>( container );
    if ( !popupMenu )
        return;

    connect( popupMenu, SIGNAL( aboutToShow() ),
             this, SLOT( setupActions() ) );
}

#include "ktoolbarhandler.moc"

/* vim: et sw=4 ts=4
 */
