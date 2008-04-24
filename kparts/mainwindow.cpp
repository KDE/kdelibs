/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
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

#include "mainwindow.h"
#include <kedittoolbar.h>
#include <kparts/mainwindow.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>
#include <kcomponentdata.h>
#include <kstatusbar.h>
#include <khelpmenu.h>
#include <kstandarddirs.h>
#include <QtGui/QApplication>
#include <kxmlguifactory.h>
#include <kconfiggroup.h>

#include <kdebug.h>

#include <assert.h>

using namespace KParts;

namespace KParts
{
class MainWindowPrivate
{
public:
    MainWindowPrivate()
        : m_activePart(0),
          m_bShellGUIActivated(false),
          m_helpMenu(0),
          toolBarEditor(0)
    {
    }
    ~MainWindowPrivate()
    {
    }

    QPointer<Part> m_activePart;
    bool m_bShellGUIActivated;
    KHelpMenu *m_helpMenu;
    QPointer<KEditToolBar> toolBarEditor;
};
}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags f )
    : KXmlGuiWindow( parent, f ), d(new MainWindowPrivate())
{
  PartBase::setPartObject( this );
}

MainWindow::MainWindow( QWidget* parent, const char *name, Qt::WindowFlags f )
  : KXmlGuiWindow( parent, f ),d(new MainWindowPrivate())
{
  setObjectName( name );
  PartBase::setPartObject( this );
}

MainWindow::~MainWindow()
{
  delete d;
}

void MainWindow::createGUI( Part * part )
{
  kDebug(1000) << "MainWindow::createGUI, part=" << part << " "
                << ( part ? part->metaObject()->className() : "" )
                << " " << ( part ? part->objectName() : "" )
                << endl;

  KXMLGUIFactory *factory = guiFactory();

  assert( factory );

    if (autoSaveSettings() && settingsDirty()) {
        // Save any unsaved window settings before we change the gui,
        // so that the applyMainWindowSettings call below will re-apply the right settings.
        // Testcase: hiding a konqueror toolbar and switching tabs immediately.
        saveAutoSaveSettings();
    }

  if ( d->m_activePart )
  {
    kDebug(1000) << "deactivating GUI for " << d->m_activePart << " "
                  << d->m_activePart->metaObject()->className()
                  << " " << d->m_activePart->objectName() << endl;

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

    // If the saved mainwindow settings contain settings for a part-specific
    // toolbar, that just appeared right now (when activating this part for the first time),
    // then those settings need to be applied --> done by applyMainWindowSettings below.
    // Technically, this isn't KParts specific so it should probably be done inside
    // KXMLGUIFactory/KXMLGUIBuilder, though...
    if ( autoSaveSettings() ) {
        QWidget *focus = QApplication::focusWidget();
        KConfigGroup cg(KGlobal::config(), autoSaveGroup());
        applyMainWindowSettings(cg);

        //The call to applyMainWindowSettings likes to steal focus via the
        //call to QMainWindow::restoreState.  Don't let it.
        if (focus)
            focus->setFocus();
    }
  }

  d->m_activePart = part;
}

void MainWindow::slotSetStatusBarText( const QString & text )
{
  statusBar()->showMessage( text );
}

void MainWindow::createShellGUI( bool create )
{
    assert( d->m_bShellGUIActivated != create );
    d->m_bShellGUIActivated = create;
    if ( create )
    {
        if ( isHelpMenuEnabled() && !d->m_helpMenu )
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

void KParts::MainWindow::saveNewToolbarConfig()
{
    createGUI( d->m_activePart );
    KConfigGroup cg(KGlobal::config(), "");
    applyMainWindowSettings(cg);
}

void KParts::MainWindow::configureToolbars()
{
    KConfigGroup cg(KGlobal::config(), QString());
    saveMainWindowSettings(cg);
    if (!d->toolBarEditor) {
      d->toolBarEditor = new KEditToolBar(guiFactory(), this);
      d->toolBarEditor->setAttribute(Qt::WA_DeleteOnClose);
      connect(d->toolBarEditor, SIGNAL(newToolBarConfig()), SLOT(saveNewToolbarConfig()));
    }
    d->toolBarEditor->show();
}

#include "mainwindow.moc"
