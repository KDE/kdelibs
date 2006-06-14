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
#include "config.h"

#include "kactioncollection.h"
#include "kmainwindow.h"
#include "kmainwindowiface_p.h"
#include "ktoolbarhandler.h"
#include "kwhatsthismanager_p.h"
#include "kxmlguifactory.h"
#include "kcmdlineargs.h"
#include "ktoggleaction.h"
#include "kstdaction.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDockWidget>
#include <qlayout.h>
#include <qobject.h>
#include <qsessionmanager.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qlist.h>
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
#include <kstaticdeleter.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kwin.h>

#if defined Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#include <kstartupinfo.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

class KMainWindowPrivate {
public:
    bool showHelpMenu:1;

    bool autoSaveSettings:1;
    bool settingsDirty:1;
    bool autoSaveWindowSize:1;
    bool care_about_geometry:1;
    bool shuttingDown:1;
    QString autoSaveGroup;
    KDEPrivate::ToolBarHandler *toolBarHandler;
    QTimer* settingsTimer;
    KToggleAction *showStatusBarAction;
    QRect defaultWindowSize;
    KEditToolbar* toolBarEditor;
};

QList<KMainWindow*> KMainWindow::sMemberList;
static bool no_query_exit = false;
static KMWSessionManaged* ksm = 0;
static KStaticDeleter<KMWSessionManaged> ksmd;

class KMWSessionManaged : public KSessionManaged
{
public:
    KMWSessionManaged()
    {
    };
    ~KMWSessionManaged()
    {
    }
    bool saveState( QSessionManager& )
    {
        KConfig* config = KApplication::kApplication()->sessionConfig();
        if ( KMainWindow::memberList().count() ){
            // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
            // hook is useful for better document orientation
            KMainWindow::memberList().first()->saveGlobalProperties(config);
        }

        int n = 0;
        foreach (KMainWindow* mw, KMainWindow::memberList()) {
            n++;
            mw->savePropertiesInternal(config, n);
        }

        KConfigGroup group( config, QLatin1String("Number") );
        group.writeEntry(QLatin1String("NumberOfWindows"), n );
        return true;
    }

    bool commitData( QSessionManager& sm )
    {
        // not really a fast method but the only compatible one
        if ( sm.allowsInteraction() ) {
            bool canceled = false;
            ::no_query_exit = true;

            for (QList<KMainWindow*>::ConstIterator it = KMainWindow::memberList().constBegin(); it != KMainWindow::memberList().constEnd()  && !canceled; ++it) {
                KMainWindow *window = *it;
                ++it; // Update now, the current window might get deleted
                if ( !window->testAttribute( Qt::WA_WState_Hidden ) ) {
                    QCloseEvent e;
                    QApplication::sendEvent( window, &e );
                    canceled = !e.isAccepted();
		    /* Don't even think_about deleting widgets with
		     Qt::WDestructiveClose flag set at this point. We
		     are faking a close event, but we are *not*_
		     closing the window. The purpose of the faked
		     close event is to prepare the application so it
		     can safely be quit without the user losing data
		     (possibly showing a message box "do you want to
		     save this or that?"). It is possible that the
		     session manager quits the application later
		     (emitting QApplication::aboutToQuit() when this
		     happens), but it is also possible that the user
		     cancels the shutdown, so the application will
		     continue to run.
		     */
                }
            }
            ::no_query_exit = false;
            if (canceled)
               return false;

            KMainWindow* last = 0;
            for (QList<KMainWindow*>::ConstIterator it = KMainWindow::memberList().constBegin(); it != KMainWindow::memberList().constEnd()  && !canceled; ++it) {
                KMainWindow *window = *it;
                if ( !window->testAttribute( Qt::WA_WState_Hidden ) ) {
                    last = window;
                }
            }
            if ( last )
                return last->queryExit();
            // else
            return true;
        }

        // the user wants it, the user gets it
        return true;
    }
};

static bool being_first = true;

KMainWindow::KMainWindow( QWidget* parent, const char* name, Qt::WindowFlags f )
    : QMainWindow( parent, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    setObjectName( name );
    initKMainWindow();
}

KMainWindow::KMainWindow( QWidget* parent, Qt::WFlags f )
    : QMainWindow( parent, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initKMainWindow();
}

void KMainWindow::initKMainWindow()
{
    setAttribute( Qt::WA_DeleteOnClose );

    KWhatsThisManager::init ();

    mHelpMenu = 0;

#if defined Q_WS_X11
    // set the app startup notification window property
    if ( kapp )
        KStartupInfo::setWindowStartupId( winId(), kapp->startupId() );
#endif

    //actionCollection()->setWidget( this );
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shuttingDown()));

    if ( !ksm )
        ksm = ksmd.setObject(ksm, new KMWSessionManaged());

    sMemberList.append( this );

    d = new KMainWindowPrivate;
    d->showHelpMenu = true;
    d->settingsDirty = false;
    d->autoSaveSettings = false;
    d->autoSaveWindowSize = true; // for compatibility
    //d->kaccel = actionCollection()->kaccel();
    d->toolBarHandler = 0;
    d->settingsTimer = 0;
    d->toolBarEditor = 0;
    d->showStatusBarAction = NULL;
    d->shuttingDown = false;
    if ((d->care_about_geometry = being_first)) {
        being_first = false;

        QString geometry;
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
        if (args && args->isSet("geometry"))
            geometry = args->getOption("geometry");

        if ( geometry.isNull() ) // if there is no geometry, it doesn't matter
            d->care_about_geometry = false;
        else
            parseGeometry(false);
    }

    if ( kapp )
        setWindowTitle( kapp->caption() );
    new KMainWindowInterface(this);

    // Get notified when settings change
    connect( this, SIGNAL( iconSizeChanged(const QSize&) ), SLOT( setSettingsDirty() ) );
    connect( this, SIGNAL( toolButtonStyleChanged(Qt::ToolButtonStyle) ), SLOT( setSettingsDirty() ) );
}

void KMainWindow::setUniqueName()
{
    // Set a unique object name. Required by session management, window management, and for the dbus interface.
    QString objname;
    QString s;
    int unusedNumber = 1;
    const QString name = objectName();
    bool startNumberingImmediately = true;
    if ( name.isEmpty() )
    {   // no name given
        objname = "MainWindow_";
    }
    else if( name.endsWith( QLatin1Char( '#' ) ) )
    {   // trailing # - always add a number
        objname = name.left( name.length() - 1 ); // lose the hash
    }
    else
    {
        objname = name;
        startNumberingImmediately = false;
    }

    // Clean up for dbus usage: any non-alphanumeric char should be turned into '_'
    const int len = objname.length();
    for ( int i = 0; i < len; ++i ) {
        if ( !objname[i].isLetterOrNumber() )
            objname[i] = QLatin1Char('_');
    }

    objname.prepend( qApp->applicationName() + '/' );
    s = objname;
    if ( startNumberingImmediately )
        s += '1';

    for(;;) {
        QList<QWidget*> list = qApp->topLevelWidgets();
        bool found = false;
        foreach ( QWidget* w, list ) {
            if( w != this && w->objectName() == s )
            {
                found = true;
                break;
            }
        }
        if( !found )
            break;
        s.setNum( ++unusedNumber );
        s = objname + s;
    }
    setObjectName( s );

    const QString pathname = "/" + objectName();
    QDBus::sessionBus().registerObject(pathname, this, QDBusConnection::ExportAllSlots |
                                       QDBusConnection::ExportAllProperties |
                                       QDBusConnection::ExportAdaptors);
    QDBus::sessionBus().registerObject(pathname + "/actions", actionCollection(),
                                       QDBusConnection::ExportAllSlots |
                                       QDBusConnection::ExportAllProperties |
                                       QDBusConnection::ExportChildObjects);

}

KAction *KMainWindow::toolBarMenuAction()
{
    if ( !d->toolBarHandler )
	return 0;

    return d->toolBarHandler->toolBarMenuAction();
}


void KMainWindow::setupToolbarMenuActions()
{
    if ( d->toolBarHandler )
        d->toolBarHandler->setupActions();
}

void KMainWindow::parseGeometry(bool parsewidth)
{
    QString cmdlineGeometry;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args->isSet("geometry"))
        cmdlineGeometry = args->getOption("geometry");

    assert ( !cmdlineGeometry.isNull() );
    assert ( d->care_about_geometry );

#if defined Q_WS_X11
    int x, y;
    int w, h;
    int m = XParseGeometry( cmdlineGeometry.toLatin1(), &x, &y, (unsigned int*)&w, (unsigned int*)&h);
    if (parsewidth) {
        QSize minSize = minimumSize();
        QSize maxSize = maximumSize();
        if ( !(m & WidthValue) )
            w = width();
        if ( !(m & HeightValue) )
            h = height();
         w = qMin(w,maxSize.width());
         h = qMin(h,maxSize.height());
         w = qMax(w,minSize.width());
         h = qMax(h,minSize.height());
         resize(w, h);
    } else {
        if ( (m & XNegative) )
            x = KApplication::desktop()->width()  + x - w;
        else if ( (m & XValue) )
            x = geometry().x();
        if ( (m & YNegative) )
            y = KApplication::desktop()->height() + y - h;
        else if ( (m & YValue) )
            y = geometry().y();

        move(x, y);
    }
#endif
}

KMainWindow::~KMainWindow()
{
    delete d->settingsTimer;
    QMenuBar* mb = internalMenuBar();
    delete mb;
    delete d;
    sMemberList.removeAll( this );
}

KMenu* KMainWindow::helpMenu( const QString &aboutAppText, bool showWhatsThis )
{
    if( !mHelpMenu ) {
        if ( aboutAppText.isEmpty() )
            mHelpMenu = new KHelpMenu( this, instance()->aboutData(), showWhatsThis);
        else
            mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );

        if ( !mHelpMenu )
            return 0;
        connect( mHelpMenu, SIGNAL( showAboutApplication() ),
                 this, SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

KMenu* KMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this, QString(), showWhatsThis );
        connect( mHelpMenu, SIGNAL( showAboutApplication() ),
                 this, SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

bool KMainWindow::canBeRestored( int number )
{
    if ( !qApp->isSessionRestored() )
        return false;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return false;

    KConfigGroup group( config, QLatin1String("Number") );
    int n = group.readEntry( QLatin1String("NumberOfWindows") , 1 );
    return number >= 1 && number <= n;
}

const QString KMainWindow::classNameOfToplevel( int number )
{
    if ( !qApp->isSessionRestored() )
        return QString();
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return QString();
    QString s;
    s.setNum( number );
    s.prepend( QLatin1String("WindowProperties") );

    KConfigGroup group( config, s );
    if ( !group.hasKey( QLatin1String("ClassName") ) )
        return QString();
    else
        return group.readEntry( "ClassName" );
}

bool KMainWindow::restore( int number, bool show )
{
    if ( !canBeRestored( number ) )
        return false;
    KConfig *config = kapp->sessionConfig();
    if ( readPropertiesInternal( config, number ) ){
        if ( show )
            KMainWindow::show();
        return false;
    }
    return false;
}

KXMLGUIFactory *KMainWindow::guiFactory()
{
    if ( !factory_ ) {
        factory_ = new KXMLGUIFactory( this, this );
        factory_->setObjectName( "guifactory" );
    }
    return factory_;
}

void KMainWindow::configureToolbars()
{
    saveMainWindowSettings(KGlobal::config());
    if (!d->toolBarEditor) {
      d->toolBarEditor = new KEditToolbar(actionCollection(), xmlFile(), true, this);
      connect(d->toolBarEditor, SIGNAL(newToolbarConfig()), SLOT(saveNewToolbarConfig()));
    }
    d->toolBarEditor->show();
}

void KMainWindow::saveNewToolbarConfig()
{
    createGUI(xmlFile());
    applyMainWindowSettings( KGlobal::config() );
}

void KMainWindow::setupGUI( StandardWindowOptions options, const QString & xmlfile ) {
    setupGUI(QSize(), options, xmlfile);
}

void KMainWindow::setupGUI( QSize defaultSize, StandardWindowOptions options, const QString & xmlfile ) {
    if( options & Keys ){
        KStdAction::keyBindings(guiFactory(),
                    SLOT(configureShortcuts()), actionCollection());
    }

    if( (options & StatusBar) && internalStatusBar() ){
        createStandardStatusBarAction();
    }

    if( options & ToolBar ){
        setStandardToolBarMenuEnabled( true );
        KStdAction::configureToolbars(this,
                      SLOT(configureToolbars() ), actionCollection());
    }

    if( options & Create ){
        createGUI(xmlfile);
    }

    if( options & Save ){
        // setupGUI() is typically called in the constructor before show(),
        // so the default window size will be incorrect unless the application
        // hard coded the size which they should try not to do (i.e. use
        // size hints).
        if(initialGeometrySet())
        {
          // Do nothing...
        }
        else if(defaultSize.isValid())
        {
          resize(defaultSize);
        }
        else if(isHidden())
        {
          adjustSize();
        }
        setAutoSaveSettings();
    }

}

void KMainWindow::createGUI( const QString &xmlfile )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    QMenuBar* mb = internalMenuBar();
    if ( mb )
        mb->clear();

    qDeleteAll( toolBars() ); // delete all toolbars

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        // we always want a help menu
        if (!helpMenu2)
            helpMenu2 = new KHelpMenu(this, instance()->aboutData(), true,
                                      actionCollection());
    }

    // we always want to load in our global standards file
    setXMLFile( locate( "config", "ui/ui_standards.rc", instance() ) );

    // now, merge in our local xml file.  if this is null, then that
    // means that we will be only using the global file
    if ( !xmlfile.isNull() ) {
        setXMLFile( xmlfile, true );
    } else {
        QString auto_file(instance()->instanceName() + "ui.rc");
        setXMLFile( auto_file, true );
    }

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument( QDomDocument() );

    // do the actual GUI building
    guiFactory()->addClient( this );

    setUpdatesEnabled( true );
    updateGeometry();
}

void KMainWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    d->showHelpMenu = showHelpMenu;
}

bool KMainWindow::isHelpMenuEnabled()
{
    return d->showHelpMenu;
}

void KMainWindow::setCaption( const QString &caption )
{
    setPlainCaption( KInstance::makeStdCaption(caption) );
}

void KMainWindow::setCaption( const QString &caption, bool modified )
{
    KInstance::CaptionFlags flags = KInstance::HIGCompliantCaption;

    if ( modified )
    {
        flags |= KInstance::ModifiedCaption;
    }

    setPlainCaption( KInstance::makeStdCaption(caption, flags) );
}

void KMainWindow::setPlainCaption( const QString &caption )
{
	setWindowTitle(caption);

#if defined Q_WS_X11
    NETWinInfo info( QX11Info::display(), winId(), QX11Info::appRootWindow(), 0 );
    info.setName( caption.toUtf8().data() );
#endif
}

void KMainWindow::appHelpActivated( void )
{
    if( !mHelpMenu ) {
        mHelpMenu = new KHelpMenu( this );
        if ( !mHelpMenu )
            return;
    }
    mHelpMenu->appHelpActivated();
}

void KMainWindow::slotStateChanged(const QString &newstate)
{
  stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

void KMainWindow::slotStateChanged(const QString &newstate,
                                   bool reverse)
{
  stateChanged(newstate,
               reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
}

void KMainWindow::closeEvent ( QCloseEvent *e )
{
    // Save settings if auto-save is enabled, and settings have changed
    if (d->settingsDirty && d->autoSaveSettings)
        saveAutoSaveSettings();

    if (queryClose()) {
        e->accept();

        int not_withdrawn = 0;
        foreach (KMainWindow* mw, KMainWindow::memberList()) {
            if ( !mw->isHidden() && mw->isTopLevel() && mw != this )
                not_withdrawn++;
        }

        if ( !no_query_exit && not_withdrawn <= 0 ) { // last window close accepted?
            if ( queryExit() && ( !kapp || !kapp->sessionSaving() ) && !d->shuttingDown ) { // Yes, Quit app?
                // don't call queryExit() twice
                disconnect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shuttingDown()));
                d->shuttingDown = true;
                KGlobal::deref();             // ...and quit application.
                // ###### what if this doesn't quit?
            }  else {
                // cancel closing, it's stupid to end up with no windows at all....
                e->ignore();
            }
        }
    } else e->ignore(); //if the window should not be closed, don't close it
}

bool KMainWindow::queryExit()
{
    return true;
}

bool KMainWindow::queryClose()
{
    return true;
}

void KMainWindow::saveGlobalProperties( KConfig*  )
{
}

void KMainWindow::readGlobalProperties( KConfig*  )
{
}

#if defined(KDE_COMPAT)
void KMainWindow::updateRects()
{
}
#endif

void KMainWindow::showAboutApplication()
{
}

void KMainWindow::savePropertiesInternal( KConfig *config, int number )
{
    bool oldASWS = d->autoSaveWindowSize;
    d->autoSaveWindowSize = true; // make saveMainWindowSettings save the window size

    QString s;
    s.setNum(number);
    s.prepend(QLatin1String("WindowProperties"));
    config->setGroup(s);

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    config->writeEntry(QLatin1String("ObjectName"), objectName());
    config->writeEntry(QLatin1String("ClassName"), metaObject()->className());

    saveMainWindowSettings(config); // Menubar, statusbar and Toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);

    d->autoSaveWindowSize = oldASWS;
}

void KMainWindow::saveMainWindowSettings(KConfig *config, const QString &configGroup)
{
    kDebug(200) << "KMainWindow::saveMainWindowSettings " << configGroup << endl;
    QString oldGroup;

    if (!configGroup.isEmpty())
    {
       oldGroup = config->group();
       config->setGroup(configGroup);
    }

    // Called by session management - or if we want to save the window size anyway
    if ( d->autoSaveWindowSize )
        saveWindowSize( config );

    QStatusBar* sb = internalStatusBar();
    if (sb) {
       if(!config->hasDefault("StatusBar") && !sb->isHidden() )
           config->revertToDefault("StatusBar");
       else
           config->writeEntry("StatusBar", sb->isHidden() ? "Disabled" : "Enabled");
    }

    QMenuBar* mb = internalMenuBar();
    if (mb) {
       QString MenuBar = QLatin1String("MenuBar");
       if(!config->hasDefault("MenuBar") && !mb->isHidden() )
           config->revertToDefault("MenuBar");
       else
           config->writeEntry("MenuBar", mb->isHidden() ? "Disabled" : "Enabled");
    }

    // Utilise the QMainWindow::saveState() functionality
    QByteArray state = saveState();
    config->writeEntry("State", state.toBase64());
    // One day will need to save the version number, but for now, assume 0

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    foreach (KToolBar* toolbar, toolBars()) {
        QString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (toolbar->objectName().isEmpty() ? QString::number(n) : QString(" ")+toolbar->objectName());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->saveSettings(config, group);
        n++;
    }
    if (!configGroup.isEmpty())
       config->setGroup(oldGroup);
}

void KMainWindow::setStandardToolBarMenuEnabled( bool enable )
{
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

bool KMainWindow::isStandardToolBarMenuEnabled() const
{
    return ( d->toolBarHandler );
}

void KMainWindow::createStandardStatusBarAction(){
  if(!d->showStatusBarAction){
    d->showStatusBarAction = KStdAction::showStatusbar(this, SLOT(setSettingsDirty()), actionCollection());
    KStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
    connect(d->showStatusBarAction, SIGNAL(toggled(bool)), sb, SLOT(setVisible(bool)));
    d->showStatusBarAction->setChecked(sb->isHidden());
  }
}

bool KMainWindow::readPropertiesInternal( KConfig *config, int number )
{
    if ( number == 1 )
        readGlobalProperties( config );

    // in order they are in toolbar list
    QString s;
    s.setNum(number);
    s.prepend(QLatin1String("WindowProperties"));

    config->setGroup(s);

    // restore the object name (window role)
    if ( config->hasKey(QLatin1String("ObjectName" )) )
        setObjectName( config->readEntry("ObjectName").toLatin1()); // latin1 is right here

    applyMainWindowSettings(config); // Menubar, statusbar and toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return true;
}

void KMainWindow::applyMainWindowSettings(KConfig *config, const QString &configGroup,bool force)
{
    kDebug(200) << "KMainWindow::applyMainWindowSettings" << endl;

    KConfigGroup cg( config, configGroup.isEmpty() ? config->group() : configGroup );

    restoreWindowSize(&cg);

    QStatusBar* sb = internalStatusBar();
    if (sb) {
        QString entry = cg.readEntry("StatusBar", "Enabled");
        if ( entry == "Disabled" )
           sb->hide();
        else
           sb->show();
        if(d->showStatusBarAction)
            d->showStatusBarAction->setChecked(!sb->isHidden());
    }

    QMenuBar* mb = internalMenuBar();
    if (mb) {
        QString entry = cg.readEntry ("MenuBar", "Enabled");
        if ( entry == "Disabled" )
           mb->hide();
        else
           mb->show();
    }

    // Utilise the QMainWindow::restoreState() functionality
    if (cg.hasKey("State")) {
        QByteArray state;
        state = cg.readEntry("State", state);
        state = QByteArray::fromBase64(state);
        // One day will need to load the version number, but for now, assume 0
        restoreState(state);
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    foreach (KToolBar* toolbar, toolBars()) {
        QString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (toolbar->objectName().isEmpty() ? QString::number(n) : QString(" ")+toolbar->objectName());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->applySettings(config, group, force);
        n++;
    }

    finalizeGUI( true );
}

void KMainWindow::finalizeGUI( bool force )
{
    //kDebug(200) << "KMainWindow::finalizeGUI force=" << force << endl;
    // The whole reason for this is that moveToolBar relies on the indexes
    // of the other toolbars, so in theory it should be called only once per
    // toolbar, but in increasing order of indexes.
    // Since we can't do that immediately, we move them, and _then_
    // we call positionYourself again for each of them, but this time
    // the toolbariterator should give them in the proper order.
    // Both the XMLGUI and applySettings call this, hence "force" for the latter.
    /* FIXME KAction port - not needed?
    foreach (KToolBar* toolbar, toolBars()) {
        toolbar->positionYourself( force );
    }*/

    d->settingsDirty = false;
}

void KMainWindow::saveWindowSize( KConfig * config ) const
{
  int scnum = QApplication::desktop()->screenNumber(parentWidget());
  QRect desk = QApplication::desktop()->screenGeometry(scnum);
  int w, h;
#if defined Q_WS_X11
  // save maximalization as desktop size + 1 in that direction
  KWin::WindowInfo info = KWin::windowInfo( winId(), NET::WMState );
  w = info.state() & NET::MaxHoriz ? desk.width() + 1 : width();
  h = info.state() & NET::MaxVert ? desk.height() + 1 : height();
#else
  if (isMaximized()) {
    w = desk.width() + 1;
    h = desk.height() + 1;
  }
  //TODO: add "Maximized" property instead "+1" hack
#endif
  QRect size( desk.width(), w, desk.height(), h );
  bool defaultSize = (size == d->defaultWindowSize);
  QString widthString = QString::fromLatin1("Width %1").arg(desk.width());
  QString heightString = QString::fromLatin1("Height %1").arg(desk.height());
  if (!config->hasDefault(widthString) && defaultSize)
     config->revertToDefault(widthString);
  else
     config->writeEntry(widthString, w );

  if (!config->hasDefault(heightString) && defaultSize)
     config->revertToDefault(heightString);
  else
     config->writeEntry(heightString, h );
}

void KMainWindow::restoreWindowSize( KConfigBase * config )
{
    if (d->care_about_geometry) {
        parseGeometry(true);
    } else {
        // restore the size
        int scnum = QApplication::desktop()->screenNumber(parentWidget());
        QRect desk = QApplication::desktop()->screenGeometry(scnum);
        if ( d->defaultWindowSize.isNull() ) // only once
          d->defaultWindowSize = QRect(desk.width(), width(), desk.height(), height()); // store default values
        QSize size( config->readEntry( QString::fromLatin1("Width %1").arg(desk.width()), 0 ),
                    config->readEntry( QString::fromLatin1("Height %1").arg(desk.height()), 0 ) );
        if (size.isEmpty()) {
            // try the KDE 2.0 way
            size = QSize( config->readEntry( QLatin1String("Width"), 0 ),
                          config->readEntry( QLatin1String("Height"), 0 ) );
            if (!size.isEmpty()) {
                // make sure the other resolutions don't get old settings
                config->writeEntry( QLatin1String("Width"), 0 );
                config->writeEntry( QLatin1String("Height"), 0 );
            }
        }
        if ( !size.isEmpty() ) {
#ifdef Q_WS_X11
            int state = ( size.width() > desk.width() ? NET::MaxHoriz : 0 )
                        | ( size.height() > desk.height() ? NET::MaxVert : 0 );
            if(( state & NET::Max ) == NET::Max )
                ; // no resize
            else if(( state & NET::MaxHoriz ) == NET::MaxHoriz )
                resize( width(), size.height());
            else if(( state & NET::MaxVert ) == NET::MaxVert )
                resize( size.width(), height());
            else
                resize( size );
            // QWidget::showMaximized() is both insufficient and broken
            KWin::setState( winId(), state );
#else
            if (size.width() > desk.width() || size.height() > desk.height())
              setWindowState( Qt::WindowMaximized );
            else
              resize( size );
#endif
        }
    }
}

bool KMainWindow::initialGeometrySet() const
{
    return d->care_about_geometry;
}

void KMainWindow::ignoreInitialGeometry()
{
    d->care_about_geometry = false;
}

void KMainWindow::setSettingsDirty()
{
    //kDebug(200) << "KMainWindow::setSettingsDirty" << endl;
    d->settingsDirty = true;
    if ( d->autoSaveSettings )
    {
        // Use a timer to save "immediately" user-wise, but not too immediately
        // (to compress calls and save only once, in case of multiple changes)
        if ( !d->settingsTimer )
        {
           d->settingsTimer = new QTimer( this );
           connect( d->settingsTimer, SIGNAL( timeout() ), SLOT( saveAutoSaveSettings() ) );
        }
        d->settingsTimer->setInterval(500);
        d->settingsTimer->setSingleShot(true);
        d->settingsTimer->start();
    }
}

bool KMainWindow::settingsDirty() const
{
    return d->settingsDirty;
}

QString KMainWindow::settingsGroup() const
{
    return d->autoSaveGroup;
}

void KMainWindow::setAutoSaveSettings( const QString & groupName, bool saveWindowSize )
{
    d->autoSaveSettings = true;
    d->autoSaveGroup = groupName;
    d->autoSaveWindowSize = saveWindowSize;

    // Now read the previously saved settings
    applyMainWindowSettings( KGlobal::config(), groupName );
}

void KMainWindow::resetAutoSaveSettings()
{
    d->autoSaveSettings = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

bool KMainWindow::autoSaveSettings() const
{
    return d->autoSaveSettings;
}

QString KMainWindow::autoSaveGroup() const
{
    return d->autoSaveGroup;
}

void KMainWindow::saveAutoSaveSettings()
{
    Q_ASSERT( d->autoSaveSettings );
    //kDebug(200) << "KMainWindow::saveAutoSaveSettings -> saving settings" << endl;
    saveMainWindowSettings( KGlobal::config(), d->autoSaveGroup );
    KGlobal::config()->sync();
    d->settingsDirty = false;
    if ( d->settingsTimer )
        d->settingsTimer->stop();
}

bool KMainWindow::event( QEvent* ev )
{
    switch( ev->type() ) {
    case QEvent::Resize:
        if ( d->autoSaveWindowSize )
            setSettingsDirty();
        break;
    case QEvent::Polish:
        setUniqueName();
        break;
    default:
        break;
    }
    return QMainWindow::event( ev );
}

bool KMainWindow::hasMenuBar()
{
    return (internalMenuBar());
}

KMenuBar *KMainWindow::menuBar()
{
    KMenuBar * mb = internalMenuBar();
    if ( !mb ) {
        mb = new KMenuBar( this );
        // trigger a re-layout and trigger a call to the private
        // setMenuBar method.
        setMenuBar(mb);
    }
    return mb;
}

KStatusBar *KMainWindow::statusBar()
{
    KStatusBar * sb = internalStatusBar();
    if ( !sb ) {
        sb = new KStatusBar( this );
        // trigger a re-layout and trigger a call to the private
        // setStatusBar method.
        setStatusBar(sb);
    }
    return sb;
}

void KMainWindow::shuttingDown()
{
    // Needed for Qt <= 3.0.3 at least to prevent reentrancy
    // when queryExit() shows a dialog. Check before removing!
    static bool reentrancy_protection = false;
    if (!reentrancy_protection)
    {
       reentrancy_protection = true;
       // call the virtual queryExit
       queryExit();
       reentrancy_protection = false;
    }

}

KMenuBar *KMainWindow::internalMenuBar()
{
    return qFindChild<KMenuBar *>(this);
}

KStatusBar *KMainWindow::internalStatusBar()
{
    return qFindChild<KStatusBar *>(this);
}

KToolBar *KMainWindow::toolBar( const QString& name )
{
    QString childName = name;
    if (childName.isEmpty())
       childName = "mainToolBar";

    KToolBar *tb = findChild<KToolBar*>(childName);
    if ( tb )
        return tb;
    bool honor_mode = name != "mainToolBar";

    KToolBar* toolbar;
    if ( builderClient() )
        toolbar = new KToolBar(this, honor_mode); // XMLGUI constructor
    else
        toolbar = new KToolBar(this, false, honor_mode ); // non-XMLGUI

    toolbar->setObjectName(childName);
    addToolBar(toolbar);

    return toolbar;
}

QList<KToolBar*> KMainWindow::toolBars() const
{
    QList<KToolBar*> ret;

    foreach (QObject* child, children())
        if (KToolBar* toolBar = qobject_cast<KToolBar*>(child))
            ret.append(toolBar);

    return ret;
}

const QList<KMainWindow*>& KMainWindow::memberList() { return sMemberList; }

// why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
// DF: because they compile KDE much faster :)
void KMainWindow::finalizeGUI( KXMLGUIClient *client )
{ KXMLGUIBuilder::finalizeGUI( client ); }

void KMainWindow::virtual_hook( int id, void* data )
{ KXMLGUIBuilder::virtual_hook( id, data );
  KXMLGUIClient::virtual_hook( id, data ); }


#include "kmainwindow.moc"

