/* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-99 Matthias Ettrich (ettrich@kde.org)
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

#include "ktmainwindow.h"

#include <qsessionmanager.h>
#include <qobjectlist.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>

#include <klocale.h>
#include <kstddirs.h>

#include <stdlib.h>
#include <ctype.h>

QList<KTMainWindow>* KTMainWindow::memberList = 0L;
static bool no_query_exit = false;
static KTLWSessionManaged* ksm = 0;
static bool initing = FALSE;

class KTLWSessionManaged : public KSessionManaged
{
public:
    KTLWSessionManaged()
    {
    };
    ~KTLWSessionManaged()
    {
    }
    bool saveState( QSessionManager& )
    {
	KConfig* config = KApplication::kApplication()->sessionConfig();
	if ( KTMainWindow::memberList->first() ){
	    // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
	    // hook is usefull for better document orientation
	    KTMainWindow::memberList->first()->saveGlobalProperties(config);
	}

	QListIterator<KTMainWindow> it(*KTMainWindow::memberList);
	int n = 0;
	config->setGroup(QString::fromLatin1("Number"));
	config->writeEntry(QString::fromLatin1("NumberOfWindows"), KTMainWindow::memberList->count());
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
	    QListIterator<KTMainWindow> it(*KTMainWindow::memberList);
	    KTMainWindow* last = 0;
	    ::no_query_exit = true;
	    for (it.toFirst(); it.current() && !cancelled; ++it){
		if ( !it.current()->testWState( Qt::WState_ForceHide ) ) {
		    last = it.current();
		    QCloseEvent e;
		    QApplication::sendEvent( last, &e );
		    cancelled = !e.isAccepted();
		}
	    }
	    no_query_exit = FALSE;
	    if ( !cancelled && last )
		cancelled = !last->queryExit();
	    return !cancelled;
	}

	// the user wants it, the user gets it
	return TRUE;
    }
};

KTMainWindow::KTMainWindow( const char *name, WFlags f )
    : QMainWindow( 0, name, f ), KXMLGUIBuilder( this ), helpMenu2( 0 ), factory_( 0 )
{
    setDockMenuEnabled( FALSE );
    mHelpMenu = 0;
    initing = TRUE;
    kapp->setTopWidget( this );
    connect(kapp, SIGNAL(shutDown()), this, SLOT(shuttingDown()));
    if( !memberList )
	memberList = new QList<KTMainWindow>;

    if ( !ksm )
	ksm = new KTLWSessionManaged();
    memberList->append( this );
    if ( !name ) {
	// set a unique object name. Required by session management.
	QCString s;
	s.setNum( memberList->count() );
	setName( kapp->instanceName() + "-mainwindow#" + s );
    }
    initing = FALSE;
}

KTMainWindow::~KTMainWindow()
{
    memberList->remove( this );
}

QPopupMenu* KTMainWindow::helpMenu( const QString &aboutAppText, bool showWhatsThis )
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

QPopupMenu* KTMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( mHelpMenu == 0 ) {
        mHelpMenu = new KHelpMenu( this, QString::null, showWhatsThis );
	connect( mHelpMenu, SIGNAL( showAboutApplication() ),
		 this, SLOT( showAboutApplication() ) );
    }

    return mHelpMenu->menu();
}

bool KTMainWindow::canBeRestored( int number )
{
    if ( !kapp->isRestored() )
	return FALSE;
    KConfig *config = kapp->sessionConfig();
    if ( !config )
	return FALSE;
    config->setGroup( QString::fromLatin1("Number") );
    int n = config->readNumEntry( QString::fromLatin1("NumberOfWindows") , 0 );
    return number >= 1 && number <= n;
}

const QString KTMainWindow::classNameOfToplevel( int number )
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

bool KTMainWindow::restore( int number, bool show )
{
    if ( !canBeRestored( number ) )
	return FALSE;
    KConfig *config = kapp->sessionConfig();
    if ( readPropertiesInternal( config, number ) ){
	if ( show )
	    KTMainWindow::show();
	return FALSE;
    }
    return FALSE;
}

KXMLGUIFactory *KTMainWindow::guiFactory()
{
    if ( !factory_ )
        factory_ = new KXMLGUIFactory( this );
    return factory_;
}

void KTMainWindow::createGUI( const QString &xmlfile, bool _conserveMemory )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    if ( internalMenuBar() )
	internalMenuBar()->clear();

    // we always want a help menu
    if (helpMenu2 == 0)
        helpMenu2 = new KHelpMenu(this, instance()->aboutData(), true,
				  actionCollection());

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
    setContainerStates(QMap<QString,QByteArray>());

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
      // this code should probably go into a separate method in KTMainWindow.
      // there's just one problem: I'm bad in finding names ;-) , so
      // I skipped this ;-)

      QDomDocument doc = domDocument();

      QDomElement e = doc.documentElement().firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
	  if ( e.tagName().lower().latin1() == "toolbar" )
	      factory_->resetContainer( e.attribute( "name" ) );
	  else if ( e.tagName().lower().latin1() == "menubar" )
	      factory_->resetContainer( e.tagName(), true );
      }

      conserveMemory();
    }

    setUpdatesEnabled( true );
    updateGeometry();
}

void KTMainWindow::setCaption( const QString &caption )
{
    if ( initing ) // we are in our constructor
	return;
    QWidget::setCaption( kapp->makeStdCaption(caption) );
}

void KTMainWindow::setCaption( const QString &caption, bool modified )
{
    if ( initing ) // we are in our constructor
	return;
    QWidget::setCaption( kapp->makeStdCaption(caption, true, modified) );
}

void KTMainWindow::setPlainCaption( const QString &caption )
{
    QWidget::setCaption( caption );
}

void KTMainWindow::appHelpActivated( void )
{
    if( mHelpMenu == 0 ) {
	mHelpMenu = new KHelpMenu( this );
	if ( mHelpMenu == 0 )
	    return;
    }
    mHelpMenu->appHelpActivated();
}

void KTMainWindow::closeEvent ( QCloseEvent *e )
{
    saveToolBars();
    if (queryClose()) {
	e->accept();

	int not_withdrawn = 0;
	QListIterator<KTMainWindow> it(*KTMainWindow::memberList);
	for (it.toFirst(); it.current(); ++it){
	    if ( !it.current()->testWState( WState_ForceHide ) )
		not_withdrawn++;
	}

	if ( !no_query_exit && not_withdrawn <= 1 ) { // last window close accepted?
	    if ( queryExit() ) {            // Yes, Quit app?
		kapp->deref();             // ...and quit aplication.
	    }  else {
		// cancel closing, it's stupid to end up with no windows at all....
		e->ignore();
	    }
	}
    }
}

bool KTMainWindow::queryExit()
{
    return TRUE;
}

bool KTMainWindow::queryClose()
{
    return TRUE;
}

void KTMainWindow::saveGlobalProperties( KConfig*  )
{
}

void KTMainWindow::readGlobalProperties( KConfig*  )
{
}

#if defined(KDE_COMPAT)
void KTMainWindow::updateRects()
{
}
#endif

void KTMainWindow::showAboutApplication( void )
{
}

void KTMainWindow::savePropertiesInternal( KConfig *config, int number )
{
    QString entry;
    QStrList entryList;
    // in order they are in toolbar list

    QString s;
    s.setNum(number);
    s.prepend(QString::fromLatin1("WindowProperties"));
    config->setGroup(s);

    // store objectName, className, Width and Height  for later restorating
    config->writeEntry(QString::fromLatin1("ObjectName"), name());
    config->writeEntry(QString::fromLatin1("ClassName"), className());
    config->writeEntry(QString::fromLatin1("Width"), width() );
    config->writeEntry(QString::fromLatin1("Height"), height() );

    entryList.clear();

    if (internalStatusBar()) {
	if (internalStatusBar()->isVisible())
	    config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Enabled"));
	else
	    config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Disabled"));
    }

    if (internalMenuBar()) {
	if (internalMenuBar()->isVisible())
	    entryList.append("Enabled");
	else
	    entryList.append("Disabled");
	config->writeEntry(QString::fromLatin1("MenuBar"), entryList, ';');
	entryList.clear();
    }

    int n = 1; // Tolbar counter. toolbars are counted from 1,
    KToolBar *toolbar = 0;
    QString toolKey;
    QListIterator<KToolBar> it( toolBarIterator() );
    while ( ( toolbar = it.current() ) ) {
	++it;
	if (toolbar->isVisible())
	    entryList.append("Enabled");
	else
	    entryList.append("Disabled");
	switch (toolbar->barPos()) {
	case KToolBar::Flat:   //ignore
	case KToolBar::Top:
	    entryList.append("Top");
	    break;
	case KToolBar::Bottom:
	    entryList.append("Bottom");
	    break;
	case KToolBar::Left:
	    entryList.append("Left");
	    break;
	case KToolBar::Right:
	    entryList.append("Right");
	    break;
	case KToolBar::Floating:
	    entryList.append("Floating");
	    break;
	default:
	    break;
	}
	toolKey.setNum(n);
	QMainWindow::ToolBarDock dock;
	int index;
	bool nl;
	int offset;
	getLocation( toolbar, dock, index, nl, offset );
	entryList.append( QString::number( index ).latin1() );
	entryList.append( nl ?  "true" : "false" );
	entryList.append( QString::number( offset ).latin1() );
	toolKey.prepend(QString::fromLatin1("ToolBar"));
	config->writeEntry(toolKey, entryList, ';');
	entryList.clear();
	n++;
    }
    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);
}

bool KTMainWindow::readPropertiesInternal( KConfig *config, int number )
{
    if ( number == 1 )
	readGlobalProperties( config );

    QString entry;
    QStrList entryList;
    // in order they are in toolbar list
    int i = 0; // Number of entries in list
    QString s;
    s.setNum(number);
    s.prepend(QString::fromLatin1("WindowProperties"));
    config->setGroup(s);

    // restore the object name (window role )
    if ( config->hasKey(QString::fromLatin1("ObjectName" )) )
	setName( config->readEntry(QString::fromLatin1("ObjectName")).latin1()); // latin1 is right here

    // restore the size
    QSize size( config->readNumEntry( QString::fromLatin1("Width"), sizeHint().width() ),
		config->readNumEntry( QString::fromLatin1("Height"), sizeHint().height() ) );
    if ( size.isValid() )
	resize( size );

    if (internalStatusBar()) {
	entry = config->readEntry(QString::fromLatin1("StatusBar"));
	if (entry == QString::fromLatin1("Enabled"))
	    internalStatusBar()->hide();
	else
	    internalStatusBar()->show();
    }

    if (internalMenuBar()) {
	i = config->readListEntry (QString::fromLatin1("MenuBar"), entryList, ';');
	bool showmenubar = false;
	entry = entryList.first();
	if (entry==QString::fromLatin1("Enabled"))
	    showmenubar = true;
	else
	    internalMenuBar()->hide();
	entryList.clear();
	if (showmenubar)
	    internalMenuBar()->show();
    }

    int n = 1; // Tolbar counter. toolbars are counted from 1,
    KToolBar *toolbar;
    QString toolKey;
    QListIterator<KToolBar> it( toolBarIterator() ); // must use own iterator

    for ( ; it.current(); ++it) {
	toolbar= it.current();
	toolKey.setNum(n);
	toolKey.prepend(QString::fromLatin1("ToolBar"));

	i = config->readListEntry(toolKey, entryList, ';');
	if (i < 2) {
	    kdDebug(200) << "KTM: bad number of toolbar args." << endl;
	    return FALSE;
	}

	bool showtoolbar = false;
	entry = entryList.first();
	if (entry==QString::fromLatin1("Enabled"))
	    showtoolbar = true;
	else
	    toolbar->enable(KToolBar::Hide);

	QMainWindow::ToolBarDock dock = Top;
	int index = 0, offset = -1;
	bool nl = FALSE;
	
	entry = entryList.next();
	if (entry == QString::fromLatin1("Top"))
	    dock = Top;
	else if (entry == QString::fromLatin1("Bottom"))
	    dock = Bottom;
	else if (entry == QString::fromLatin1("Left"))
	    dock = Left;
	else if (entry == QString::fromLatin1("Right"))
	    dock = Right;
	else if (entry == QString::fromLatin1("Floating"))
	    dock = Top; // TornOff;
	if (showtoolbar)
	    toolbar->enable(KToolBar::Show);
	
	if ( entryList.count() > 2 ) {
	    entry = entryList.next();
	    index = entry.toInt();
	    nl = ( QString( entry ).lower() == QString::fromLatin1( "true" ) ) ? TRUE : FALSE;
	    offset = entry.toInt();
	    moveToolBar( toolbar, dock, index, nl, offset );
	}
	toolbar->setBarPos( (KToolBar::BarPosition)dock );
	n++; // next toolbar
	entryList.clear();
    }

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return true;
}

KMenuBar *KTMainWindow::menuBar()
{
    if ( !internalMenuBar() )
	return new KMenuBar( this );
    return (KMenuBar*)internalMenuBar();
}

KStatusBar *KTMainWindow::statusBar()
{
    if ( !internalStatusBar() )
	return new KStatusBar( this );
    return (KStatusBar*)internalStatusBar();
}

void KTMainWindow::shuttingDown()
{
    saveToolBars();
    // call the virtual queryExit
    queryExit();
}

QMenuBar *KTMainWindow::internalMenuBar()
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

QStatusBar *KTMainWindow::internalStatusBar()
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

void KTMainWindow::childEvent( QChildEvent* e)
{
    QMainWindow::childEvent( e );
}

KToolBar *KTMainWindow::toolBar( int id )
{
    if ( idBarMap.find( id ) == idBarMap.end() ) {
	KToolBar *tb = new KToolBar( this );
	idBarMap.insert( id, tb );
	return tb;
    }
    return *idBarMap.find( id );
}

KToolBar *KTMainWindow::toolBar( const char * name )
{
    if ( nameBarMap.find( name ) == nameBarMap.end() ) {
	KToolBar *tb = (KToolBar*)child( name, "KToolBar" );
	if ( tb )
	    return tb;
	return 0;
    }

    return *nameBarMap.find( name );
}

QListIterator<KToolBar> KTMainWindow::toolBarIterator()
{
    toolbarList.clear();
    QList<QToolBar> lst;
    for ( int i = (int)QMainWindow::Unmanaged; i <= (int)Minimized; ++i ) {
	lst = toolBars( (ToolBarDock)i );
	for ( QToolBar *tb = lst.first(); tb; tb = lst.next() ) {
	    if ( !tb->inherits( "KToolBar" ) )
		continue;
	    toolbarList.append( (KToolBar*)tb );
	}
    }
    return QListIterator<KToolBar>( toolbarList );
}

void KTMainWindow::enableStatusBar( KStatusBar::BarStatus stat )
{
    KStatusBar *sb = 0;
    if ( internalStatusBar() && internalStatusBar()->inherits( "KStatusBar" ) )
	sb = (KStatusBar*)internalStatusBar();
    else
	return;
    if ( ( stat == KStatusBar::Toggle && sb->isVisible() ) || stat == KStatusBar::Hide )
	sb->hide();
    else
	sb->show();
}

void KTMainWindow::enableToolBar( KToolBar::BarStatus stat, int id )
{
    KToolBar *tb = *idBarMap.find( id );
    if ( !tb )
	return;
    bool mystat = tb->isVisible();

    if ( (stat == KToolBar::Toggle && mystat) || stat == KToolBar::Hide )
	tb->hide();
    else
	tb->show();
}

void KTMainWindow::setEnableToolBar( KToolBar::BarStatus stat, const char * name )
{
    KToolBar *tb = *nameBarMap.find( name );
    if ( !tb )
	return;
    bool mystat = tb->isVisible();

    if ( (stat == KToolBar::Toggle && mystat) || stat == KToolBar::Hide )
	tb->hide();
    else
	tb->show();
}

int KTMainWindow::addToolBar( KToolBar *toolbar, int index )
{
    int id = idBarMap.count();
    idBarMap.insert( id, toolbar );
    if ( index == -1 )
	index = toolBars( (ToolBarDock)toolbar->barPos() ).count();
    moveToolBar( toolbar, (ToolBarDock)toolbar->barPos(), FALSE, index );
    return id;
}

bool KTMainWindow::hasMenuBar()
{
    return internalMenuBar() != 0;
}

bool KTMainWindow::hasStatusBar()
{
    return internalStatusBar() != 0;
}

bool KTMainWindow::hasToolBar( int id )
{
    return idBarMap.find( id ) != idBarMap.end();
}

void KTMainWindow::saveToolBars()
{
    QListIterator<KToolBar> it( toolBarIterator() );
    KToolBar *toolbar = 0;
    while ( ( toolbar = it.current() ) ) {
	++it;
	toolbar->saveState();
    }
}

void KTMainWindow::paintEvent( QPaintEvent * )
{
    // do nothing
}


#include "ktmainwindow.moc"

