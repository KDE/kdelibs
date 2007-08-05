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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "dockmainwindow3.h"
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>
#include <kstatusbar.h>
#include <kcomponentdata.h>
#include <khelpmenu.h>
#include <kstandarddirs.h>
#include <QtGui/QApplication>

#include <kdebug.h>
#include <kxmlguifactory.h>

#include <assert.h>

using namespace KParts;

namespace KParts
{
class DockMainWindow3Private
{
public:
  DockMainWindow3Private()
  {
    m_activePart = 0;
    m_bShellGUIActivated = false;
    m_helpMenu = 0;
  }
  ~DockMainWindow3Private()
  {
  }

  QPointer<Part> m_activePart;
  bool m_bShellGUIActivated;
  KHelpMenu *m_helpMenu;
};
}

DockMainWindow3::DockMainWindow3( QWidget* parent, const char *name, Qt::WFlags f )
    : K3DockMainWindow( parent, name, f )
{
    d = new DockMainWindow3Private();
    PartBase::setPartObject( this );
    setAttribute( Qt::WA_DeleteOnClose );
}

DockMainWindow3::~DockMainWindow3()
{
  delete d;
}

void DockMainWindow3::createGUI( Part * part )
{
  kDebug(1000) << QString("DockMainWindow3::createGUI for %1").arg(part?part->name():"0L");

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  Q3PtrList<Plugin> plugins;

  if ( d->m_activePart )
  {
    kDebug(1000) << QString("deactivating GUI for %1").arg(d->m_activePart->name());

    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    disconnect( d->m_activePart, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    loadPlugins( this, this, KGlobal::mainComponent() );
    createShellGUI();
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

  }

  setUpdatesEnabled( true );

  d->m_activePart = part;
}

void DockMainWindow3::slotSetStatusBarText( const QString & text )
{
  statusBar()->message( text );
}

void DockMainWindow3::createShellGUI( bool create )
{
    assert( d->m_bShellGUIActivated != create );
    d->m_bShellGUIActivated = create;
    if ( create )
    {
        if ( isHelpMenuEnabled() )
            d->m_helpMenu = new KHelpMenu( this, componentData().aboutData(), true, actionCollection() );

        QString f = xmlFile();
        setXMLFile( KStandardDirs::locate( "config", "ui/ui_standards.rc", componentData() ) );
        if ( !f.isEmpty() )
            setXMLFile( f, true );
        else
        {
            QString auto_file( componentData().componentName() + "ui.rc" );
            setXMLFile( auto_file, true );
        }

        GUIActivateEvent ev( true );
        QApplication::sendEvent( this, &ev );

        guiFactory()->addClient( this );

    }
    else
    {
        GUIActivateEvent ev( false );
        QApplication::sendEvent( this, &ev );

        guiFactory()->removeClient( this );
    }
}

#include "dockmainwindow3.moc"
