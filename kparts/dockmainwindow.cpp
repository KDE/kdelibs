/* This file is part of the KDE project
   Copyright (C) 2000 Falk Brettschneider <gigafalk@yahoo.com>
             (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kparts/dockmainwindow.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>

#include <qapplication.h>

#include <kdebug.h>

using namespace KParts;

namespace KParts
{
class DockMainWindowPrivate
{
public:
  DockMainWindowPrivate()
  {
    m_activePart = 0;
    m_bShellGUIActivated = false;
  }
  ~DockMainWindowPrivate()
  {
  }

  QGuardedPtr<Part> m_activePart;
  bool m_bShellGUIActivated;
};
};

DockMainWindow::DockMainWindow( const char *name, WFlags /*f*/ )
  : KDockMainWindow( name )
{
  d = new DockMainWindowPrivate();
  PartBase::setPartObject( this );
}

DockMainWindow::~DockMainWindow()
{
  delete d;
}

void DockMainWindow::createGUI( Part * part )
{
  kdDebug(1000) << QString("DockMainWindow::createGUI for %1").arg(part?part->name():"0L") << endl;

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  QValueList<KXMLGUIClient *> plugins;
  QValueList<KXMLGUIClient *>::ConstIterator pIt, pBegin, pEnd;

  if ( d->m_activePart )
  {
    kdDebug(1000) << QString("deactivating GUI for %1").arg(d->m_activePart->name()) << endl;
    
    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

    plugins = Plugin::pluginClients( d->m_activePart );
    pIt = plugins.fromLast();
    pBegin = plugins.begin();

    for (; pIt != pBegin ; --pIt )
      factory->removeClient( *pIt );

    if ( pIt != plugins.end() )
      factory->removeClient( *pIt );

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    disconnect( d->m_activePart, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    GUIActivateEvent ev( true );
    QApplication::sendEvent( this, &ev );

    factory->addClient( this );

    plugins = Plugin::pluginClients( this );
    pIt = plugins.begin();
    pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
      factory->addClient( *pIt );

    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    // do this before sending the activate event
    connect( part, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    connect( part, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );

    factory->addClient( part );
    
    GUIActivateEvent ev( true );
    QApplication::sendEvent( part, &ev );

    plugins = Plugin::pluginClients( part );
    pIt = plugins.begin();
    pEnd = plugins.end();

    for (; pIt != pEnd; ++pIt )
      factory->addClient( *pIt );
  }

  setUpdatesEnabled( true );
  updateRects();

  d->m_activePart = part;
}

void DockMainWindow::slotSetStatusBarText( const QString & text )
{
  statusBar()->message( text );
}

#include "dockmainwindow.moc"
