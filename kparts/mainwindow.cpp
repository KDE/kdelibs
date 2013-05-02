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
#include <kactioncollection.h>
#include <kedittoolbar.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>
#include <khelpmenu.h>
#include <kaboutdata.h>
#include <kxmlguifactory.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kdebug.h>

#include <QAction>
#include <QApplication>
#include <QPointer>
#include <QStatusBar>
#include <qstandardpaths.h>


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
          m_helpMenu(0)
    {
    }
    ~MainWindowPrivate()
    {
    }

    QPointer<Part> m_activePart;
    bool m_bShellGUIActivated;
    KHelpMenu *m_helpMenu;
};
}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags f )
    : KXmlGuiWindow( parent, f ), d(new MainWindowPrivate())
{
  PartBase::setPartObject( this );
}

#ifndef KDE_NO_DEPRECATED
MainWindow::MainWindow( QWidget* parent, const char *name, Qt::WindowFlags f )
  : KXmlGuiWindow( parent, f ),d(new MainWindowPrivate())
{
  setObjectName( name );
  PartBase::setPartObject( this );
}
#endif

MainWindow::~MainWindow()
{
  delete d;
}

void MainWindow::createGUI( Part * part )
{
#if 0
  kDebug(1000) << "part=" << part
                << ( part ? part->metaObject()->className() : "" )
                << ( part ? part->objectName() : "" );
#endif
  KXMLGUIFactory *factory = guiFactory();

  assert( factory );

  if ( d->m_activePart )
  {
#if 0
    kDebug(1000) << "deactivating GUI for" << d->m_activePart
                  << d->m_activePart->metaObject()->className()
                  << d->m_activePart->objectName();
#endif

    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL(setWindowCaption(QString)),
             this, SLOT(setCaption(QString)) );
    disconnect( d->m_activePart, SIGNAL(setStatusBarText(QString)),
             this, SLOT(slotSetStatusBarText(QString)) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    loadPlugins(this, this, KAboutData::applicationData());
    createShellGUI();
    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    // do this before sending the activate event
    connect( part, SIGNAL(setWindowCaption(QString)),
             this, SLOT(setCaption(QString)) );
    connect( part, SIGNAL(setStatusBarText(QString)),
             this, SLOT(slotSetStatusBarText(QString)) );

    factory->addClient( part );

    GUIActivateEvent ev( true );
    QApplication::sendEvent( part, &ev );
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
        if ( isHelpMenuEnabled() && !d->m_helpMenu ) {
            d->m_helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);

            KActionCollection *actions = actionCollection();
            QAction *helpContentsAction = d->m_helpMenu->action(KHelpMenu::menuHelpContents);
            QAction *whatsThisAction = d->m_helpMenu->action(KHelpMenu::menuWhatsThis);
            QAction *reportBugAction = d->m_helpMenu->action(KHelpMenu::menuReportBug);
            QAction *switchLanguageAction = d->m_helpMenu->action(KHelpMenu::menuSwitchLanguage);
            QAction *aboutAppAction = d->m_helpMenu->action(KHelpMenu::menuAboutApp);
            QAction *aboutKdeAction = d->m_helpMenu->action(KHelpMenu::menuAboutKDE);

            if (helpContentsAction)
                actions->addAction(helpContentsAction->objectName(), helpContentsAction);
            if (whatsThisAction)
                actions->addAction(whatsThisAction->objectName(), whatsThisAction);
            if (reportBugAction)
                actions->addAction(reportBugAction->objectName(), reportBugAction);
            if (switchLanguageAction)
                actions->addAction(switchLanguageAction->objectName(), switchLanguageAction);
            if (aboutAppAction)
                actions->addAction(aboutAppAction->objectName(), aboutAppAction);
            if (aboutKdeAction)
                actions->addAction(aboutKdeAction->objectName(), aboutKdeAction);
        }

        QString f = xmlFile();
        setXMLFile(QStandardPaths::locate(QStandardPaths::ConfigLocation, "ui/ui_standards.rc"));
        if ( !f.isEmpty() )
            setXMLFile( f, true );
        else
        {
            QString auto_file( componentName() + "ui.rc" );
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
    createGUI(d->m_activePart);
    KConfigGroup cg(KSharedConfig::openConfig(), QString());
    applyMainWindowSettings(cg);
}

void KParts::MainWindow::configureToolbars()
{
    // No difference with base class anymore.
    KXmlGuiWindow::configureToolbars();
}

