 /* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)

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

#include "kmainwindow.h"
#include "kmainwindowiface.h"
#include <qsessionmanager.h>
#include <qobjectlist.h>
#include <qstyle.h>
#include <qlayout.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#ifndef Q_WS_QWS
#include <netwm.h>
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
    QString autoSaveGroup;
    KAccel * kaccel;
    KMainWindowInterface *m_interface;
    KToolBarMenuAction *toolbarMenu;
};

QPtrList<KMainWindow>* KMainWindow::memberList = 0L;
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
        if ( KMainWindow::memberList->first() ){
            // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
            // hook is usefull for better document orientation
            KMainWindow::memberList->first()->saveGlobalProperties(config);
        }

        QPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
        int n = 0;
        config->setGroup(QString::fromLatin1("Number"));
        config->writeEntry(QString::fromLatin1("NumberOfWindows"), KMainWindow::memberList->count());
        for (it.toFirst(); it.current(); ++it){
            n++;
            it.current()->savePropertiesInternal(config, n);
        }
        return TRUE;
    }

    bool commitData( QSessionManager& sm )
    {
        // not really a fast method but the only compatible one
        if ( sm.allowsInteraction() ) {
            bool cancelled = false;
            QPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
            ::no_query_exit = true;
            for (it.toFirst(); it.current() && !cancelled;){
                KMainWindow *window = *it;
                ++it; // Update now, the current window might get deleted
                if ( !window->testWState( Qt::WState_ForceHide ) ) {
                    QCloseEvent e;
                    QApplication::sendEvent( window, &e );
                    cancelled = !e.isAccepted();
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
            if (cancelled)
               return false;

            KMainWindow* last = 0;
            for (it.toFirst(); it.current() && !cancelled; ++it){
                KMainWindow *window = *it;
                if ( !window->testWState( Qt::WState_ForceHide ) ) {
                    last = window;
                }
            }
            if ( last )
                return last->queryExit();
            // else
            return true;
        }

        // the user wants it, the user gets it
        return TRUE;
    }
};

static bool beeing_first = true;

KMainWindow::KMainWindow( QWidget* parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initKMainWindow(true,name);
}

KMainWindow::KMainWindow( QWidget* parent, const char *name,WFlags f, bool createToolbarAction)
    : QMainWindow( parent, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    initKMainWindow(createToolbarAction,name);
}


void KMainWindow::initKMainWindow(bool createToolbarAction,const char *name)
{
  if (createToolbarAction)
    kdDebug(200) << "KMainWindow::init called with createToolbarAction=true"<< endl;

    setDockMenuEnabled( FALSE );
    mHelpMenu = 0;
    kapp->setTopWidget( this );
    actionCollection()->setWidget( this );
    connect(kapp, SIGNAL(shutDown()), this, SLOT(shuttingDown()));
    if( !memberList )
        memberList = new QPtrList<KMainWindow>;

    if ( !ksm )
        ksm = ksmd.setObject(new KMWSessionManaged());
    if ( !name ) {
        // set a unique object name. Required by session management.
        QCString s;
        int unusedNumber= 0;
        KMainWindow *existingWin;
        do {
            s.setNum( ++unusedNumber );
            s= kapp->instanceName() + "-mainwindow#" + s;
            for ( existingWin= memberList->first(); existingWin!=0;
                  existingWin= memberList->next() )
                if ( existingWin->name() == s )
                    break;
        } while ( existingWin!=0 );
        setName( s );
    }
    memberList->append( this );

    d = new KMainWindowPrivate;
    d->showHelpMenu = true;
    d->settingsDirty = false;
    d->autoSaveSettings = false;
    d->autoSaveWindowSize = true; // for compatibility
    d->kaccel = actionCollection()->kaccel();
    if (createToolbarAction)
    {
       d->toolbarMenu=new KToolBarMenuAction(this,"options_show_toolbar");
    } else d->toolbarMenu=0;
    if ((d->care_about_geometry = beeing_first)) {
        beeing_first = false;
        if ( kapp->geometryArgument().isNull() ) // if there is no geometry, it doesn't mater
            d->care_about_geometry = false;
        else
            parseGeometry(false);
    }

    setCaption( kapp->caption() );
	// attach dcop interface
	d->m_interface = new KMainWindowInterface(this);
}

KToolBarMenuAction *KMainWindow::toolBarMenuAction()
{
	return d->toolbarMenu;
}

void KMainWindow::parseGeometry(bool parsewidth)
{
    assert ( !kapp->geometryArgument().isNull() );
    assert ( d->care_about_geometry );

#ifndef Q_WS_QWS
    // FIXME: (E) Implement something similar for Qt Embedded (or decide we don't need it)
    int x, y;
    int w, h;
    int m = XParseGeometry( kapp->geometryArgument().latin1(), &x, &y, (unsigned int*)&w, (unsigned int*)&h);
    if (parsewidth) {
        QSize minSize = minimumSize();
        QSize maxSize = maximumSize();
        if ( (m & WidthValue) == 0 )
            w = width();
        if ( (m & HeightValue) == 0 )
            h = height();
         w = QMIN(w,maxSize.width());
         h = QMIN(h,maxSize.height());
         w = QMAX(w,minSize.width());
         h = QMAX(h,minSize.height());
         resize(w, h);
    } else {
        if ( parsewidth && (m & XValue) == 0 )
            x = geometry().x();
        if ( parsewidth && (m & YValue) == 0 )
            y = geometry().y();
        if ( (m & XNegative) )
            x = KApplication::desktop()->width()  + x - w;
        if ( (m & YNegative) )
            y = KApplication::desktop()->height() + y - h;
        move(x, y);
    }
#endif
}

KMainWindow::~KMainWindow()
{
    QMenuBar* mb = internalMenuBar();
    delete mb;
    delete d->m_interface;
    delete d;
    memberList->remove( this );
}

KPopupMenu* KMainWindow::helpMenu( const QString &aboutAppText, bool showWhatsThis )
{
    if( mHelpMenu == 0 ) {
        if ( aboutAppText.isEmpty() )
            mHelpMenu = new KHelpMenu( this, instance()->aboutData(), showWhatsThis);
        else
            mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );

        if ( mHelpMenu == 0 )
            return 0;
        connect( mHelpMenu, SIGNAL( showAboutApplication() ),
                 this, SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

KPopupMenu* KMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( mHelpMenu == 0 ) {
        mHelpMenu = new KHelpMenu( this, QString::null, showWhatsThis );
        connect( mHelpMenu, SIGNAL( showAboutApplication() ),
                 this, SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

bool KMainWindow::canBeRestored( int number )
{
    if ( !kapp->isRestored() )
        return FALSE;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return FALSE;
    config->setGroup( QString::fromLatin1("Number") );
    int n = config->readNumEntry( QString::fromLatin1("NumberOfWindows") , 1 );
    return number >= 1 && number <= n;
}

const QString KMainWindow::classNameOfToplevel( int number )
{
    if ( !kapp->isRestored() )
        return QString::null;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
        return QString::null;
    QString s;
    s.setNum( number );
    s.prepend( QString::fromLatin1("WindowProperties") );
    config->setGroup( s );
    if ( !config->hasKey( QString::fromLatin1("ClassName") ) )
        return QString::null;
    else
        return config->readEntry( QString::fromLatin1("ClassName") );
}

bool KMainWindow::restore( int number, bool show )
{
    if ( !canBeRestored( number ) )
        return FALSE;
    KConfig *config = kapp->sessionConfig();
    if ( readPropertiesInternal( config, number ) ){
        if ( show )
            KMainWindow::show();
        return FALSE;
    }
    return FALSE;
}

KXMLGUIFactory *KMainWindow::guiFactory()
{
    if ( !factory_ )
        factory_ = new KXMLGUIFactory( this, this, "guifactory" );
    return factory_;
}

void KMainWindow::createGUI( const QString &xmlfile, bool _conserveMemory )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    QMenuBar* mb = internalMenuBar();
    if ( mb )
        mb->clear();

    (void)toolBarIterator(); // make sure toolbarList is most-up-to-date
    toolbarList.setAutoDelete( true );
    toolbarList.clear();
    toolbarList.setAutoDelete( false );

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        // we always want a help menu
        if (helpMenu2 == 0)
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

    // try and get back *some* of our memory
    if ( _conserveMemory )
    {
      // before freeing the memory allocated by the DOM document we also
      // free all memory allocated internally in the KXMLGUIFactory for
      // the menubar and the toolbars . This however implies that we
      // have to take care of deleting those widgets ourselves. For
      // destruction this is no problem, but when rebuilding we have
      // to take care of that (and we want to rebuild the GUI when
      // using stuff like the toolbar editor ).
      // In addition we have to take care of not removing containers
      // like popupmenus, defined in the XML document.
      // this code should probably go into a separate method in KMainWindow.
      // there's just one problem: I'm bad in finding names ;-) , so
      // I skipped this ;-)

      QDomDocument doc = domDocument();

      QDomElement e = doc.documentElement().firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
          if ( e.tagName().lower() == "toolbar" )
              factory_->resetContainer( e.attribute( "name" ) );
          else if ( e.tagName().lower() == "menubar" )
              factory_->resetContainer( e.tagName(), true );
      }

      conserveMemory();
    }

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
    setPlainCaption( kapp->makeStdCaption(caption) );
}

void KMainWindow::setCaption( const QString &caption, bool modified )
{
    setPlainCaption( kapp->makeStdCaption(caption, true, modified) );
}

void KMainWindow::setPlainCaption( const QString &caption )
{
    QMainWindow::setCaption( caption );
#ifndef Q_WS_QWS
    NETWinInfo info( qt_xdisplay(), winId(), qt_xrootwin(), 0 );
    info.setName( caption.utf8().data() );
#endif
}

void KMainWindow::appHelpActivated( void )
{
    if( mHelpMenu == 0 ) {
        mHelpMenu = new KHelpMenu( this );
        if ( mHelpMenu == 0 )
            return;
    }
    mHelpMenu->appHelpActivated();
}

void KMainWindow::slotStateChanged(const QString &newstate)
{
  stateChanged(newstate);
}

void KMainWindow::closeEvent ( QCloseEvent *e )
{
    if (queryClose()) {
        e->accept();

        // Save settings if auto-save is enabled, and settings have changed
        if (d->settingsDirty && d->autoSaveSettings)
        {
            //kdDebug(200) << "KMainWindow::closeEvent -> saving settings" << endl;
            saveMainWindowSettings( KGlobal::config(), d->autoSaveGroup );
            KGlobal::config()->sync();
            d->settingsDirty = false;
        }

        int not_withdrawn = 0;
        QPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
        for (it.toFirst(); it.current(); ++it){
            if ( !it.current()->isHidden() && it.current()->isTopLevel() && it.current() != this )
                not_withdrawn++;
        }

        if ( !no_query_exit && not_withdrawn <= 0 ) { // last window close accepted?
            if ( queryExit() ) {            // Yes, Quit app?
                // We saved the toolbars already
                disconnect(kapp, SIGNAL(shutDown()), this, SLOT(shuttingDown()));
                kapp->deref();             // ...and quit aplication.
            }  else {
                // cancel closing, it's stupid to end up with no windows at all....
                e->ignore();
            }
        }
    }
}

bool KMainWindow::queryExit()
{
    return TRUE;
}

bool KMainWindow::queryClose()
{
    return TRUE;
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
    s.prepend(QString::fromLatin1("WindowProperties"));
    config->setGroup(s);

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    config->writeEntry(QString::fromLatin1("ObjectName"), name());
    config->writeEntry(QString::fromLatin1("ClassName"), className());

    saveMainWindowSettings(config); // Menubar, statusbar and Toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);

    d->autoSaveWindowSize = oldASWS;
}

void KMainWindow::saveMainWindowSettings(KConfig *config, const QString &configGroup)
{
    kdDebug(200) << "KMainWindow::saveMainWindowSettings " << configGroup << endl;
    QString entry;
    QStrList entryList;

    if (!configGroup.isEmpty())
       config->setGroup(configGroup);

    // Called by session management - or if we want to save the window size anyway
    if ( d->autoSaveWindowSize )
        saveWindowSize( config );

    QStatusBar* sb = internalStatusBar();
    if (sb) {
        entryList.clear();
        if ( sb->isHidden() )
            entryList.append("Disabled");
        else
            entryList.append("Enabled");
        config->writeEntry(QString::fromLatin1("StatusBar"), entryList, ';');
    }

    QMenuBar* mb = internalMenuBar();
    if (mb) {
        entryList.clear();
        if ( mb->isHidden() )
            entryList.append("Disabled");
        else
            entryList.append("Enabled");
        config->writeEntry(QString::fromLatin1("MenuBar"), entryList, ';');
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    KToolBar *toolbar = 0;
    QString toolKey;
    QPtrListIterator<KToolBar> it( toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
        ++it;
        QString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? QString::number(n) : QString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->saveSettings(config, group);
        n++;
    }
}

bool KMainWindow::readPropertiesInternal( KConfig *config, int number )
{
    if ( number == 1 )
        readGlobalProperties( config );

    // in order they are in toolbar list
    QString s;
    s.setNum(number);
    s.prepend(QString::fromLatin1("WindowProperties"));

    config->setGroup(s);

    // restore the object name (window role)
    if ( config->hasKey(QString::fromLatin1("ObjectName" )) )
        setName( config->readEntry(QString::fromLatin1("ObjectName")).latin1()); // latin1 is right here

    applyMainWindowSettings(config); // Menubar, statusbar and toolbar settings.

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return true;
}

void KMainWindow::applyMainWindowSettings(KConfig *config, const QString &configGroup)
{
    kdDebug(200) << "KMainWindow::applyMainWindowSettings" << endl;
    QString entry;
    QStrList entryList;
    int i = 0; // Number of entries in list

    if (!configGroup.isEmpty())
       config->setGroup(configGroup);

    restoreWindowSize(config);

    QStatusBar* sb = internalStatusBar();
    if (sb) {
        entryList.clear();
        i = config->readListEntry (QString::fromLatin1("StatusBar"), entryList, ';');
        entry = entryList.first();
        if (entry == QString::fromLatin1("Disabled"))
            sb->hide();
        else
            sb->show();
    }

    QMenuBar* mb = internalMenuBar();
    if (mb) {
        entryList.clear();
        i = config->readListEntry (QString::fromLatin1("MenuBar"), entryList, ';');
        entry = entryList.first();
        if (entry==QString::fromLatin1("Disabled"))
            mb->hide();
        else
            mb->show();
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    KToolBar *toolbar;
    QString toolKey;
    QPtrListIterator<KToolBar> it( toolBarIterator() ); // must use own iterator

    for ( ; it.current(); ++it) {
        toolbar= it.current();
        QString group;
        if (!configGroup.isEmpty())
        {
           // Give a number to the toolbar, but prefer a name if there is one,
           // because there's no real guarantee on the ordering of toolbars
           group = (!::qstrcmp(toolbar->name(), "unnamed") ? QString::number(n) : QString(" ")+toolbar->name());
           group.prepend(" Toolbar");
           group.prepend(configGroup);
        }
        toolbar->applySettings(config, group);
        n++;
    }

    finalizeGUI( true );
}

void KMainWindow::finalizeGUI( bool force )
{
    //kdDebug(200) << "KMainWindow::finalizeGUI force=" << force << endl;
    // The whole reason for this is that moveToolBar relies on the indexes
    // of the other toolbars, so in theory it should be called only once per
    // toolbar, but in increasing order of indexes.
    // Since we can't do that immediately, we move them, and _then_
    // we call positionYourself again for each of them, but this time
    // the toolbariterator should give them in the proper order.
    // Both the XMLGUI and applySettings call this, hence "force" for the latter.
    d->toolbarMenu->clear();
    QPtrListIterator<KToolBar> it( toolBarIterator() );
    for ( ; it.current() ; ++ it )
        {
            d->toolbarMenu->addToolbar(it.current());
            it.current()->positionYourself( force );
        }

    d->settingsDirty = false;
}

void KMainWindow::saveWindowSize( KConfig * config ) const
{
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    config->writeEntry(QString::fromLatin1("Width %1").arg(desk.width()), width() );
    config->writeEntry(QString::fromLatin1("Height %1").arg(desk.height()), height() );
}

void KMainWindow::restoreWindowSize( KConfig * config )
{
    if (d->care_about_geometry) {
        parseGeometry(true);
    } else {
        // restore the size
        int scnum = QApplication::desktop()->screenNumber(parentWidget());
        QRect desk = QApplication::desktop()->screenGeometry(scnum);
        QSize size( config->readNumEntry( QString::fromLatin1("Width %1").arg(desk.width()), 0 ),
                    config->readNumEntry( QString::fromLatin1("Height %1").arg(desk.height()), 0 ) );
        if (size.isEmpty()) {
            // try the KDE 2.0 way
            size = QSize( config->readNumEntry( QString::fromLatin1("Width"), 0 ),
                          config->readNumEntry( QString::fromLatin1("Height"), 0 ) );
            if (!size.isEmpty()) {
                // make sure the other resolutions don't get old settings
                config->writeEntry( QString::fromLatin1("Width"), 0 );
                config->writeEntry( QString::fromLatin1("Height"), 0 );
            }
        }
        if ( !size.isEmpty() )
            resize( size );
    }
}

bool KMainWindow::initialGeometrySet() const
{
    return d->care_about_geometry;
}

void KMainWindow::setSettingsDirty()
{
    //kdDebug(200) << "KMainWindow::setSettingsDirty" << endl;
    d->settingsDirty = true;
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
    // Get notified when the user moves a toolbar around
    connect( this, SIGNAL( dockWindowPositionChanged( QDockWindow * ) ),
             this, SLOT( setSettingsDirty() ) );
    // Now read the previously saved settings
    applyMainWindowSettings( KGlobal::config(), groupName );
}

void KMainWindow::resetAutoSaveSettings()
{
    d->autoSaveSettings = false;
}

void KMainWindow::resizeEvent( QResizeEvent * )
{
    if ( d->autoSaveWindowSize )
        setSettingsDirty();
}

KMenuBar *KMainWindow::menuBar()
{
    KMenuBar * mb = static_cast<KMenuBar *>(internalMenuBar());
    if ( !mb )
        mb = new KMenuBar( this );
    return mb;
}

KStatusBar *KMainWindow::statusBar()
{
    KStatusBar * sb = static_cast<KStatusBar *>(internalStatusBar());
    if ( !sb )
        sb = new KStatusBar( this );
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

QMenuBar *KMainWindow::internalMenuBar()
{
    QObjectList *l = queryList( "QMenuBar" );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    QMenuBar *m = (QMenuBar*)l->first();
    delete l;
    return m;
}

QStatusBar *KMainWindow::internalStatusBar()
{
    QObjectList *l = queryList( "QStatusBar" );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    QStatusBar *s = (QStatusBar*)l->first();
    delete l;
    return s;
}

void KMainWindow::childEvent( QChildEvent* e)
{
    QMainWindow::childEvent( e );
}

KToolBar *KMainWindow::toolBar( const char * name )
{
    if (!name)
       name = "mainToolBar";
    KToolBar *tb = (KToolBar*)child( name, "KToolBar" );
    if ( tb )
        return tb;
    bool honor_mode = (name == "mainToolBar");

    if ( builderClient() )
        return new KToolBar(this, name, honor_mode); // XMLGUI constructor
    else
        return new KToolBar(this, DockTop, false, name, honor_mode ); // non-XMLGUI
}

QPtrListIterator<KToolBar> KMainWindow::toolBarIterator()
{
    toolbarList.clear();
    QPtrList<QToolBar> lst;
    for ( int i = (int)QMainWindow::DockUnmanaged; i <= (int)DockMinimized; ++i ) {
        lst = toolBars( (ToolBarDock)i );
        for ( QToolBar *tb = lst.first(); tb; tb = lst.next() ) {
            if ( !tb->inherits( "KToolBar" ) )
                continue;
            toolbarList.append( (KToolBar*)tb );
        }
    }
    return QPtrListIterator<KToolBar>( toolbarList );
}

KAccel * KMainWindow::accel()
{
    if ( !d->kaccel )
        d->kaccel = new KAccel( this, "kmw-kaccel" );
    return d->kaccel;
}

void KMainWindow::paintEvent( QPaintEvent * )
{
    // do nothing
}

QSize KMainWindow::sizeForCentralWidgetSize(QSize size)
{
    KToolBar *tb = toolBar();
    if (!tb->isHidden()) {
        switch( tb->barPos() )
        {
          case KToolBar::Top:
          case KToolBar::Bottom:
            size += QSize(0, tb->sizeHint().height());
            break;

          case KToolBar::Left:
          case KToolBar::Right:
            size += QSize(toolBar()->sizeHint().width(), 0);
            break;

          case KToolBar::Flat:
            size += QSize(0, 3+kapp->style().pixelMetric( QStyle::PM_DockWindowHandleExtent ));
            break;

          default:
            break;
        }
    }
    KMenuBar *mb = menuBar();
    if (!mb->isHidden()) {
        size += QSize(0,mb->heightForWidth(size.width()));
        if (style().styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, this))
           size += QSize( 0, dockWindowsMovable() ? 1 : 2);
    }
    QStatusBar *sb = internalStatusBar();
    if( sb && !sb->isHidden() )
       size += QSize(0, sb->sizeHint().height());

#if QT_VERSION == 302
    size += QSize(0,2); // Qt 3.0.2 insists on wasting 2 pixels it seems.
#endif

    return size;
}

// why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
void KMainWindow::finalizeGUI( KXMLGUIClient *client )
{ return KXMLGUIBuilder::finalizeGUI( client ); }

void KMainWindow::virtual_hook( int id, void* data )
{ KXMLGUIBuilder::virtual_hook( id, data );
  KXMLGUIClient::virtual_hook( id, data ); }



#include "kmainwindow.moc"

