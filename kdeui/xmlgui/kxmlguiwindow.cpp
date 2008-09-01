 /* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)
     (C) 2005-2006 Hamish Rodda (rodda@kde.org)

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

#include "kxmlguiwindow.h"
#include "kmainwindow_p.h"
#include "kactioncollection.h"
#include "kmainwindowiface_p.h"
#include "ktoolbarhandler.h"
#include "kwhatsthismanager_p.h"
#include "kxmlguifactory.h"
#include "kcmdlineargs.h"
#include "ktoggleaction.h"
#include "ksessionmanager.h"
#include "kstandardaction.h"

#include <config.h>

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QtXml/QDomDocument>
#include <QtGui/QLayout>
#include <QtCore/QObject>
#include <QtGui/QSessionManager>
#include <QtGui/QStyle>
#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtCore/QList>
#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kwindowsystem.h>
#include <kconfiggroup.h>

#if defined Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#include <kstartupinfo.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

class KXmlGuiWindowPrivate : public KMainWindowPrivate {
public:
    bool showHelpMenu:1;
    bool saveFlag:1;
    QSize defaultSize;

    KDEPrivate::ToolBarHandler *toolBarHandler;
    KToggleAction *showStatusBarAction;
    QPointer<KEditToolBar> toolBarEditor;
    KXMLGUIFactory *factory;
};

KXmlGuiWindow::KXmlGuiWindow( QWidget* parent, Qt::WFlags f )
    : KMainWindow(*new KXmlGuiWindowPrivate, parent, f), KXMLGUIBuilder( this )
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = true;
    d->saveFlag = false;
    d->toolBarHandler = 0;
    d->showStatusBarAction = 0;
    d->factory = 0;
    new KMainWindowInterface(this);
}


QAction *KXmlGuiWindow::toolBarMenuAction()
{
    K_D(KXmlGuiWindow);
    if ( !d->toolBarHandler )
	return 0;

    return d->toolBarHandler->toolBarMenuAction();
}


void KXmlGuiWindow::setupToolbarMenuActions()
{
    K_D(KXmlGuiWindow);
    if ( d->toolBarHandler )
        d->toolBarHandler->setupActions();
}


KXmlGuiWindow::~KXmlGuiWindow()
{
}

bool KXmlGuiWindow::event( QEvent* ev )
{
    bool ret = KMainWindow::event(ev);
    if (ev->type()==QEvent::Polish) {
        QDBusConnection::sessionBus().registerObject(dbusName() + "/actions", actionCollection(),
                                                     QDBusConnection::ExportScriptableSlots |
                                                     QDBusConnection::ExportScriptableProperties |
                                                     QDBusConnection::ExportNonScriptableSlots |
                                                     QDBusConnection::ExportNonScriptableProperties |
                                                     QDBusConnection::ExportChildObjects);
    }
    return ret;
}

void KXmlGuiWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = showHelpMenu;
}

bool KXmlGuiWindow::isHelpMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return d->showHelpMenu;
}

KXMLGUIFactory *KXmlGuiWindow::guiFactory()
{
    K_D(KXmlGuiWindow);
    if (!d->factory)
        d->factory = new KXMLGUIFactory( this, this );
    return d->factory;
}

void KXmlGuiWindow::configureToolbars()
{
    K_D(KXmlGuiWindow);
    KConfigGroup cg(KGlobal::config(), QString());
    saveMainWindowSettings(cg);
    if (!d->toolBarEditor) {
      d->toolBarEditor = new KEditToolBar(guiFactory(), this);
      d->toolBarEditor->setAttribute(Qt::WA_DeleteOnClose);
      connect(d->toolBarEditor, SIGNAL(newToolBarConfig()), SLOT(saveNewToolbarConfig()));
    }
    d->toolBarEditor->show();
}

void KXmlGuiWindow::saveNewToolbarConfig()
{
    // createGUI(xmlFile()); // this loses any plugged-in guiclients, so we use remove+add instead.

    guiFactory()->removeClient(this);
    guiFactory()->addClient(this);

    KConfigGroup cg(KGlobal::config(), QString());
    applyMainWindowSettings(cg);
}

void KXmlGuiWindow::setupGUI( StandardWindowOptions options, const QString & xmlfile ) {
    setupGUI(QSize(), options, xmlfile);
}

void KXmlGuiWindow::setupGUI( const QSize & defaultSize, StandardWindowOptions options, const QString & xmlfile ) {
    K_D(KXmlGuiWindow);

    if( options & Keys ){
        KStandardAction::keyBindings(guiFactory(),
                    SLOT(configureShortcuts()), actionCollection());
    }

    if( (options & StatusBar) && statusBar() ){
        createStandardStatusBarAction();
    }

    if( options & ToolBar ){
        setStandardToolBarMenuEnabled( true );
        KStandardAction::configureToolbars(this,
                      SLOT(configureToolbars() ), actionCollection());
    }

    d->saveFlag = bool(options & Save);
    d->defaultSize = defaultSize;

    if( options & Create ){
        createGUI(xmlfile);
    }
}
void KXmlGuiWindow::createGUI( const QString &xmlfile )
{
    K_D(KXmlGuiWindow);
    // disabling the updates prevents unnecessary redraws
    //setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    QMenuBar* mb = menuBar();
    if ( mb )
        mb->clear();

    qDeleteAll( toolBars() ); // delete all toolbars

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        delete d->helpMenu;
        // we always want a help menu
        d->helpMenu = new KHelpMenu(this, componentData().aboutData(), true, actionCollection());
    }

    // we always want to load in our global standards file
    setXMLFile(KStandardDirs::locate("config", "ui/ui_standards.rc", componentData()));

    // now, merge in our local xml file.  if this is null, then that
    // means that we will be only using the global file
    if ( !xmlfile.isNull() ) {
        setXMLFile( xmlfile, true );
    } else {
        QString auto_file(componentData().componentName() + "ui.rc");
        setXMLFile( auto_file, true );
    }

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument( QDomDocument() );

    // do the actual GUI building
    guiFactory()->addClient( this );

    //  setUpdatesEnabled( true );
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate)
{
  stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate,
                                   bool reverse)
{
  stateChanged(newstate,
               reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::setStandardToolBarMenuEnabled( bool enable )
{
    K_D(KXmlGuiWindow);
    if ( enable ) {
        if ( d->toolBarHandler )
            return;

        d->toolBarHandler = new KDEPrivate::ToolBarHandler( this );

        if ( factory() )
            factory()->addClient( d->toolBarHandler );
    } else {
        if ( !d->toolBarHandler )
            return;

        if ( factory() )
            factory()->removeClient( d->toolBarHandler );

        delete d->toolBarHandler;
        d->toolBarHandler = 0;
    }
}

bool KXmlGuiWindow::isStandardToolBarMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return ( d->toolBarHandler );
}

void KXmlGuiWindow::createStandardStatusBarAction(){
    K_D(KXmlGuiWindow);
    if(!d->showStatusBarAction){
        d->showStatusBarAction = KStandardAction::showStatusbar(this, SLOT(setSettingsDirty()), actionCollection());
        KStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
        connect(d->showStatusBarAction, SIGNAL(toggled(bool)), sb, SLOT(setVisible(bool)));
        d->showStatusBarAction->setChecked(sb->isHidden());
    } else {
        // If the language has changed, we'll need to grab the new text and whatsThis
	KAction *tmpStatusBar = KStandardAction::showStatusbar(NULL, NULL, NULL);
        d->showStatusBarAction->setText(tmpStatusBar->text());
        d->showStatusBarAction->setWhatsThis(tmpStatusBar->whatsThis());
	delete tmpStatusBar;
    }
}

void KXmlGuiWindow::finalizeGUI( bool /*force*/ )
{
    K_D(KXmlGuiWindow);

    if (d->saveFlag) {
        if(initialGeometrySet())
        {
          // Do nothing...
        }
        else if(d->defaultSize.isValid())
        {
          resize(d->defaultSize);
        }
        else if(isHidden())
        {
          adjustSize();
        }
        setAutoSaveSettings();
        // We only pretend to call this one time. If setupGUI(... | Save | ...) is called this wil
        // be set again to true.
        d->saveFlag = false;
    }

    // FIXME: this really needs to be removed with a code more like the one we had on KDE3.
    //        what we need to do here is to position correctly toolbars so they don't overlap.
    //        Also, take in count plugins could provide their own toolbars and those also need to
    //        be restored.
    if (autoSaveSettings() && autoSaveConfigGroup().isValid()) {
        applyMainWindowSettings(autoSaveConfigGroup());
    }
    // END OF FIXME
}

void KXmlGuiWindow::applyMainWindowSettings(const KConfigGroup &config, bool force)
{
    K_D(KXmlGuiWindow);
    KMainWindow::applyMainWindowSettings(config, force);
    KStatusBar *sb = qFindChild<KStatusBar *>(this);
    if (sb && d->showStatusBarAction)
        d->showStatusBarAction->setChecked(!sb->isHidden());
}

// KDE5 TODO: change it to "using KXMLGUIBuilder::finalizeGUI;" in the header
// and remove the reimplementation
void KXmlGuiWindow::finalizeGUI( KXMLGUIClient *client )
{ KXMLGUIBuilder::finalizeGUI( client ); }

#include "kxmlguiwindow.moc"

