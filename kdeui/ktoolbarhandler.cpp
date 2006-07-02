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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <assert.h>

#include <qdom.h>

#include <kaction.h>
#include <kactionmenu.h>
#include <kauthorized.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenu.h>
#include <ktoggletoolbaraction.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>

#include "ktoolbarhandler.h"

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
      BarActionBuilder( KActionCollection *actionCollection, KMainWindow *mainWindow,
                        QLinkedList<KToolBar*> &oldToolBarList )
        : m_actionCollection( actionCollection ), m_mainWindow( mainWindow ), m_needsRebuild( false )
      {
        QList<KToolBar*> toolBars = qFindChildren<KToolBar*>( m_mainWindow );

        foreach( KToolBar * toolBar, toolBars) {
          if ( toolBar->mainWindow() != m_mainWindow )
            continue;

          if ( !oldToolBarList.contains( toolBar ) )
            m_needsRebuild = true;

          m_toolBars.append( toolBar );
        }

        if ( !m_needsRebuild )
          m_needsRebuild = ( oldToolBarList.count() != m_toolBars.count() );
      }

      bool needsRebuild() const
      {
        return m_needsRebuild;
      }

      QList<KAction*> create()
      {
        QList<KAction*> actions;

        if ( !m_needsRebuild )
          return actions;

        foreach ( KToolBar* bar, m_toolBars )
          handleToolBar( bar );

        if ( m_toolBarActions.count() == 0 )
          return actions;

        if ( m_toolBarActions.count() == 1 ) {
          KToggleToolBarAction* action = static_cast<KToggleToolBarAction *>( m_toolBarActions.first() );
          action->setText( i18n( "Show Toolbar" ) );
          action->setCheckedState( i18n( "Hide Toolbar" ) );
          return m_toolBarActions;
        }

        KActionMenu *menuAction = new KActionMenu( i18n( "Toolbars" ), m_actionCollection, "toolbars_submenu_action" );

        foreach ( KAction* action, m_toolBarActions )
          menuAction->kMenu()->addAction( action );

        actions.append( menuAction );

        return actions;
      }

      const QLinkedList<KToolBar*> &toolBars() const
      {
        return m_toolBars;
      }

    private:
      void handleToolBar( KToolBar *toolBar )
      {
        KToggleToolBarAction *action = new KToggleToolBarAction(
              toolBar,
              toolBar->windowTitle(),
              m_actionCollection,
              toolBar->objectName().toLatin1() );

        // ## tooltips, whatsthis?
        m_toolBarActions.append( action );
      }

      KActionCollection *m_actionCollection;
      KMainWindow *m_mainWindow;

      QLinkedList<KToolBar*> m_toolBars;
      QList<KAction*> m_toolBarActions;

      bool m_needsRebuild : 1;
  };
}

using namespace KDEPrivate;

class ToolBarHandler::Private
{
  public:
    Private( ToolBarHandler *_parent )
      : parent( _parent )
    {
    }

    void clientAdded( KXMLGUIClient *client )
    {
      if ( client == parent )
        parent->setupActions();
    }

    void init( KMainWindow *mainWindow );
    void connectToActionContainers();
    void connectToActionContainer( KAction *action );
    void connectToActionContainer( QWidget *container );

    ToolBarHandler *parent;
    QPointer<KMainWindow> mainWindow;
    QList<KAction*> actions;
    QLinkedList<KToolBar*> toolBars;
};

void ToolBarHandler::Private::init( KMainWindow *mw )
{
  mainWindow = mw;

  QObject::connect( mainWindow->guiFactory(), SIGNAL( clientAdded( KXMLGUIClient * ) ),
                    parent, SLOT( clientAdded( KXMLGUIClient * ) ) );

  if ( parent->domDocument().documentElement().isNull() ) {

    QString completeDescription = QString::fromLatin1( guiDescription )
                                  .arg( actionListName );

    parent->setXML( completeDescription, false /*merge*/ );
  }
}

void ToolBarHandler::Private::connectToActionContainers()
{
  foreach ( KAction* action, actions )
    connectToActionContainer( action );
}

void ToolBarHandler::Private::connectToActionContainer( KAction *action )
{
  uint containerCount = action->associatedWidgets().count();

  for ( uint i = 0; i < containerCount; ++i )
    connectToActionContainer( action->container( i ) );
}

void ToolBarHandler::Private::connectToActionContainer( QWidget *container )
{
  QMenu *popupMenu = qobject_cast<QMenu *>( container );
  if ( !popupMenu )
    return;

  connect( popupMenu, SIGNAL( aboutToShow() ),
           parent, SLOT( setupActions() ) );
}

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow )
  : QObject( mainWindow ), KXMLGUIClient( mainWindow ),
    d( new Private( this ) )
{
  d->init( mainWindow );
}

ToolBarHandler::ToolBarHandler( KMainWindow *mainWindow, QObject *parent )
  : QObject( parent ), KXMLGUIClient( mainWindow ),
    d( new Private( this ) )
{
  d->init( mainWindow );
}

ToolBarHandler::~ToolBarHandler()
{
  qDeleteAll( d->actions );
  d->actions.clear();

  delete d;
}

KAction *ToolBarHandler::toolBarMenuAction()
{
  assert( d->actions.count() == 1 );
  return d->actions.first();
}

void ToolBarHandler::setupActions()
{
  if ( !factory() || !d->mainWindow )
      return;

  BarActionBuilder builder( actionCollection(), d->mainWindow, d->toolBars );

  if ( !builder.needsRebuild() )
      return;

  unplugActionList( actionListName );

  qDeleteAll( d->actions );
  d->actions.clear();

  d->actions = builder.create();

  d->toolBars = builder.toolBars();

  if ( KAuthorized::authorizeKAction( "options_show_toolbar" ) )
    plugActionList( actionListName, d->actions );

  d->connectToActionContainers();
}

#include "ktoolbarhandler.moc"
