/* This file is part of the KDE libraries
     Copyright  (C) 1997 Stephan Kulow (coolo@kde.org)
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

#include <qsessionmanager.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <ktmainwindow.h>
#include <ktmlayout.h>
#include <kwm.h>

#include <klocale.h>
#include <kstddirs.h>

// a static pointer (too bad we cannot have static objects in libraries)
QList<KTMainWindow>* KTMainWindow::memberList = 0L;
static bool no_query_exit = false;

template class QList<KTMainWindow>;
template class QList<KToolBar>;

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

class KTMainWindowPrivate
{
public:
    KTMainWindowPrivate()
    {
	m_factory   = 0L;
	m_helpMenu  = 0L;
	m_indicator = 0L;
    }
    ~KTMainWindowPrivate()
    {
	if (m_factory) delete m_factory;
	if (m_helpMenu) delete m_helpMenu;
	if (m_indicator) delete m_indicator;
    }
    KXMLGUIFactory *m_factory;
    KHelpMenu *m_helpMenu;
    QWidget *m_indicator;
};

static KTLWSessionManaged* ksm = 0;


static bool initing = FALSE;

KTMainWindow::KTMainWindow( const char *name, WFlags f )
    : QWidget( 0L, name, f ), KXMLGUIBuilder( this )
{
    d = new KTMainWindowPrivate();

    initing = TRUE;

    kmenubar = 0L;
    kmainwidget = 0L;
    kstatusbar = 0L;
    borderwidth = 0;
    mHelpMenu = 0L;

    kmainwidgetframe = new QFrame( this );
    CHECK_PTR( kmainwidgetframe );
    kmainwidgetframe ->setFrameStyle( QFrame::Panel | QFrame::Sunken);
    kmainwidgetframe ->setLineWidth(0);
    kmainwidgetframe ->hide();

    kapp->setTopWidget( this );

    connect(kapp, SIGNAL(shutDown()), this, SLOT(shuttingDown()));

    // see if there already is a member list
    if( !memberList )
	memberList = new QList<KTMainWindow>;

    if ( !ksm )
	ksm = new KTLWSessionManaged();

    // enter the widget in the list of all KTWs
    memberList->append( this );


    if ( !name ) {
	// set a unique object name. Required by session management.
	QCString s;
	s.setNum( memberList->count() );
	setName( kapp->instanceName() + "-mainwindow#" + s );
    }

    localKill = false;
    layoutMgr = 0;

    initing = FALSE;
}

KTMainWindow::~KTMainWindow()
{
    localKill = true;

    // remove this widget from the member list
    memberList->remove( this );

    // delete all toolbars (necessary if they are floating)
    KToolBar *toolbar = 0;
    for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next() ) {
	if (toolbar->barPos() == KToolBar::Floating
	    && !QApplication::closingDown()) {
	    //kdDebug(200) << "KTM destructor: deleted toolbar" << endl;
	    delete toolbar;
	}
    }

    if (kmenubar) {
	delete kmenubar; // Eeeh? (sven)
	kdDebug(200) << "KTM destructor: deleted menubar" << endl;
    } else {
	kdDebug(200) << "KTM destructor: Menubar deleted, don´t wanna hurt myself" << endl;
    }



    /* I'm not sure if the layoutMgr is deleted by Qt. So I leave it out until
     * I have some time to look at this further. */
    //  delete layoutMgr;


    //if (!QApplication::closingDown())
    delete mHelpMenu;

    delete d;

    kdDebug(200) << "KTM destructor: end" << endl;
}


void KTMainWindow::closeEvent ( QCloseEvent *e){
    // quickly save the toolbar state
    KToolBar *toolbar;
    for (toolbar = toolbars.first(); toolbar != 0; toolbar = toolbars.next())
      toolbar->saveState();

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
		kapp->quit();             // ...and quit aplication.
	    }  else {
		// cancel closing, it's stupid to end up with no windows at all....
		e->ignore();
	    }
	}
    }
    kdDebug(200) << "KTM CloseEvent end" << endl;
}

bool KTMainWindow::queryClose()
{
    return true;
}

bool KTMainWindow::queryExit(){
    return true;
}

void KTMainWindow::shuttingDown()
{
   // quickly save the toolbar state
  KToolBar *toolbar;
  for (toolbar = toolbars.first(); toolbar != 0; toolbar = toolbars.next())
    toolbar->saveState();

  // call the virtual queryExit
  queryExit();
}

int KTMainWindow::addToolBar( KToolBar *toolbar, int index )
{
    // this is *somewhat* of a hack.. but we need to do this here
    // since the xmlgui code (suprisingly) doesn't really have all
    // that much knowledge of the xml file itself
    if ( !xmlFile().isNull() )
        toolbar->setXML( xmlFile(), domDocument() );

    if ( index == -1 )
	toolbars.append( toolbar );
    else
	toolbars.insert( index, toolbar );
    index = toolbars.at();
    connect ( toolbar, SIGNAL( moved (BarPosition) ),
	      this, SLOT( updateRects() ) );
    connect (toolbar, SIGNAL(destroyed ()), this, SLOT(toolbarKilled ()));
    updateRects();
    return index;
}

void KTMainWindow::setView( QWidget *view, bool show_frame )
{
    kmainwidget = view;
    if( show_frame ){

	// Set a default frame borderwidth, for a toplevelwidget with
	// frame.

	if(borderwidth == 0 )
	    setFrameBorderWidth(0);

	kmainwidgetframe->show();
    }

    // In the case setView(...,TRUE),
    // we leave the default frame borderwith at 0 so that we don't get
    // an unwanted border -- after all we didn't request a frame. If you
    // still want a border ( though no frame, call setFrameBorderWidth()
    // before setView(...,FALSE).

}

void KTMainWindow::focusInEvent( QFocusEvent *)
{
    repaint( FALSE );
}

void KTMainWindow::focusOutEvent( QFocusEvent *)
{
    repaint( FALSE );
}

void KTMainWindow::show ()
{
    QWidget::show();
    updateRects();
}


void KTMainWindow::setCaption( const QString &caption )
{
    if ( initing ) // we are in our constructor
	return;
    QWidget::setCaption( kapp->makeStdCaption(caption) );
}


void KTMainWindow::setPlainCaption( const QString &caption )
{
    QWidget::setCaption( caption );
}

QSize KTMainWindow::sizeHint() const
{
    if (layoutMgr)
	return layoutMgr->sizeHint();

    return (QSize(-1, -1));
}

QRect KTMainWindow::mainViewGeometry() const
{
    return (kmainwidget ? kmainwidget->geometry() : QRect());
}

void KTMainWindow::updateRects()
{
    if (!isUpdatesEnabled())
	return;

    delete layoutMgr;

    layoutMgr = new KTMLayout(this);
    CHECK_PTR(layoutMgr);

    /* add menu bar */
    if (kmenubar && kmenubar->isVisibleTo(this))
	layoutMgr->addTopMenuBar(kmenubar);

    /* add indicator widget */
    if (d->m_indicator)
	layoutMgr->addIndicatorWidget(d->m_indicator);

    /* add toolbars */
    for (toolbars.first(); toolbars.current(); toolbars.next())
	if (toolbars.current()->isVisibleTo(this)) {
	    switch (toolbars.current()->barPos()) {
	    case KToolBar::Flat:
		layoutMgr->addFlatBar(toolbars.current());
		break;
	    case KToolBar::Top:
		layoutMgr->addTopToolBar(toolbars.current());
		break;
	    case KToolBar::Bottom:
		layoutMgr->addBottomToolBar(toolbars.current());
		break;
	    case KToolBar::Left:
		layoutMgr->addLeftToolBar(toolbars.current());
		break;
	    case KToolBar::Right:
		layoutMgr->addRightToolBar(toolbars.current());
		break;
	    default:
		break;
	    }	
	}

    /* add the main widget */
    if (kmainwidget) {
	if (kmainwidget->maximumWidth() > 0)
	    setMaximumWidth(kmainwidget->maximumWidth());
	if (kmainwidget->maximumHeight() > 0)
	    setMaximumHeight(kmainwidget->maximumHeight());
	layoutMgr->addMainItem(kmainwidget);
    }

    /* add the status bar */
    if (kstatusbar && kstatusbar->isVisibleTo(this))
	layoutMgr->addStatusBar(kstatusbar);

    layoutMgr->activate();

    /* show the toolbars */
    for (toolbars.first(); toolbars.current(); toolbars.next())
	if (toolbars.current()->isVisibleTo(this))
	    toolbars.current()->show();
}

void KTMainWindow::saveGlobalProperties(KConfig*)
{
}

void KTMainWindow::readGlobalProperties(KConfig*)
{
}



void KTMainWindow::savePropertiesInternal (KConfig* config, int number)
{
    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
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

    if (kstatusbar) {
	if (kstatusbar->isVisible())
	    config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Enabled"));
	else
	    config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Disabled"));
    }

    if (kmenubar) {
	if (kmenubar->isVisible())
	    entryList.append("Enabled");
	else
	    entryList.append("Disabled");
	config->writeEntry(QString::fromLatin1("MenuBar"), entryList, ';');
	entryList.clear();
    }

    KToolBar *toolbar;
    QString toolKey;
    for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next()) {
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
	    entryList.append(KWM::properties(toolbar->winId()).ascii());
	    break;
	}
	toolKey.setNum(n);
	toolKey.prepend(QString::fromLatin1("ToolBar"));
	config->writeEntry(toolKey, entryList, ';');
	entryList.clear();
	n++;
    }

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);
}

bool KTMainWindow::readPropertiesInternal (KConfig* config, int number)
{
    if ( number == 1 )
	readGlobalProperties( config );

    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
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

    if (kstatusbar) {
	entry = config->readEntry(QString::fromLatin1("StatusBar"));
	if (entry == QString::fromLatin1("Enabled"))
	    enableStatusBar(KStatusBar::Show);
	else enableStatusBar(KStatusBar::Hide);
    }

    if (kmenubar) {
	i = config->readListEntry (QString::fromLatin1("MenuBar"), entryList, ';');
	bool showmenubar = false;
	entry = entryList.first();
	if (entry==QString::fromLatin1("Enabled"))
	    showmenubar = True;
	else
	    kmenubar->hide();
	entryList.clear();
	if (showmenubar)
	    kmenubar->show();
    }
    KToolBar *toolbar;
    QString toolKey;
    QListIterator<KToolBar> it(toolbars); // must use own iterator

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

	entry = entryList.next();
	if (entry == QString::fromLatin1("Top"))
	    toolbar->setBarPos(KToolBar::Top);
	else if (entry == QString::fromLatin1("Bottom"))
	    toolbar->setBarPos(KToolBar::Bottom);
	else if (entry == QString::fromLatin1("Left"))
	    toolbar->setBarPos(KToolBar::Left);
	else if (entry == QString::fromLatin1("Right"))
	    toolbar->setBarPos(KToolBar::Right);
	else if (entry == QString::fromLatin1("Floating"))
	    {
		toolbar->setBarPos(KToolBar::Floating);
		entry=entryList.next();
		toolbar->setGeometry(KWM::setProperties(toolbar->winId(), entry));
		toolbar->updateRects(TRUE);
		toolbar->show();
	    }
	if (showtoolbar)
	    toolbar->enable(KToolBar::Show);
	n++; // next toolbar
	entryList.clear();
    }

    s.setNum(number);
    config->setGroup(s);
    readProperties(config);
    return True;
}

void KTMainWindow::setFrameBorderWidth(int size){

    borderwidth = size;

}

void KTMainWindow::setMaximumToolBarWraps(unsigned int wraps)
{
    layoutMgr->setMaximumWraps(wraps);
}

bool KTMainWindow::canBeRestored(int number){
    if (!kapp->isRestored())
	return false;
    KConfig *config = kapp->sessionConfig();
    if (!config)
	return false;
    config->setGroup(QString::fromLatin1("Number"));
    int n = config->readNumEntry(QString::fromLatin1("NumberOfWindows"), 0);
    return (number >= 1 && number <= n);
}

const QString KTMainWindow::classNameOfToplevel(int number){
    if (!kapp->isRestored())
	return QString::null;
    KConfig *config = kapp->sessionConfig();
    if (!config)
	return QString::null;
    QString s;
    s.setNum(number);
    s.prepend(QString::fromLatin1("WindowProperties"));
    config->setGroup(s);
    if (!config->hasKey(QString::fromLatin1("ClassName")))
	return QString::null;
    else
	return config->readEntry(QString::fromLatin1("ClassName"));
}



bool KTMainWindow::restore(int number){
    if (!canBeRestored(number))
	return false;
    KConfig *config = kapp->sessionConfig();
    if (readPropertiesInternal(config, number)){
	show();
	return true;
    }
    return false;
}

bool KTMainWindow::event(QEvent* ev)
{
	if ( QWidget::event(ev) ) return TRUE;
    /* We will get a layout hint when the view widget or a bar changes it's
     * size constraines. Since we might have to adjust the window size we
     * call updateGeometry.
     * It has been added thanks to Nicolas Hadacek's hint. CS */
    if (ev->type() == QEvent::LayoutHint) {
		updateRects();
		return TRUE;
	}
    return FALSE;
}

void KTMainWindow::resizeEvent(QResizeEvent* )
{
    /* This is an ugly hack to work around a Qt layout management
     * problem.  If the minimum or maximum size changes during the
     * execution of resizeEvent() functions this new size is not
     * honored. Unfortunately due to the multiple resizeEvents() this
     * flickers a lot in opaque resize mode when using kwm. No
     * flickering with kwin, yeah!
     *
     * To avoid endless recursions we make sure that we only call resize
     * once. I arbitrarily favored height adjustment over width adjustment
     * since it fits into our height for width dominated widget world. CS */
    if (height() < minimumHeight())
	resize(width(), minimumHeight());
    else if (maximumHeight() > 0 && height() > maximumHeight())
	resize(width(), maximumHeight());
    else if (width() < minimumWidth())
	resize(minimumWidth(), height());
    else if (maximumWidth() > 0 && width() > maximumWidth())
	resize(maximumWidth(), height());

    /* Matthias believes that this may lead to endless loops, but I think
     * it's save, though I have no proof. I did some heavy testing on
     * both slow and fast machines without problems. CS */
}

KStatusBar *KTMainWindow::statusBar()
{
    if (!kstatusbar) {
	kstatusbar = new KStatusBar(this);
	updateRects ();
    }
    return kstatusbar;
}

void KTMainWindow::setStatusBar (KStatusBar *statusbar)
{
    kstatusbar = statusbar;
    updateRects ();
}

KMenuBar *KTMainWindow::menuBar()
{
    if (!kmenubar) {
	setMenu(new KMenuBar(this));
    }
    return kmenubar;
}

void KTMainWindow::setMenu (KMenuBar *menubar)
{
    kmenubar = menubar;
    connect (kmenubar, SIGNAL(destroyed ()), this, SLOT(menubarKilled ()));
    updateRects();
}

void KTMainWindow::setIndicatorWidget( QWidget *ind)
{
    d->m_indicator = ind;
    updateRects();
}

QWidget *KTMainWindow::indicator()
{
    return d->m_indicator;
}

KToolBar *KTMainWindow::toolBar( int ID )
{
    KToolBar* result = 0;
    if (ID < int(toolbars.count()))
	result = toolbars.at( ID )
	    ;
    if (!result) {
	bool honor_mode = (ID == 0) ? true : false;
	result  = new KToolBar(this, 0, honor_mode);
	toolbars.append( result );
	while (int(toolbars.count()) < ID){
	    toolbars.append( result );
	}
	connect ( result, SIGNAL( moved (BarPosition) ),
		  this, SLOT( updateRects() ) );
	connect (result, SIGNAL(destroyed ()), this, SLOT(toolbarKilled ()));
	updateRects();
    }
    return result;
}

KToolBar *KTMainWindow::toolBar( const QString& name )
{
    KToolBar* result = 0L;

    if ( factory() ) {
	QWidget *widget = factory()->container( name, this );
	if ( widget->inherits("KToolBar") )
	    result = (KToolBar*)widget;
    }
    return result;
}

void KTMainWindow::setEnableToolBar( KToolBar::BarStatus stat, const QString& name)
{
    KToolBar *t = toolBar( name );
    if ( t )
	t->enable( stat );
    updateRects();
}

void KTMainWindow::enableToolBar( KToolBar::BarStatus stat, int ID )
{
    KToolBar *t = toolbars.at( ID );
    if ( t )
	t->enable( stat );
    updateRects();
}

void KTMainWindow::enableStatusBar( KStatusBar::BarStatus stat )
{
    if ( ( stat == KStatusBar::Toggle && statusBar()->isVisible() )
	 || stat == KStatusBar::Hide )
	statusBar()->hide();
    else
	statusBar()->show();
    updateRects();
}


bool KTMainWindow::hasMenuBar(){
    return kmenubar != 0;
}

bool KTMainWindow::hasStatusBar(){
    return kstatusbar != 0;
}

bool KTMainWindow::hasToolBar( int ID ){
    return toolbars.at( ID ) != 0;
}

void KTMainWindow::menubarKilled()
{
    if (localKill)
	{
	    kdDebug(200) << "KTM: ACK mb kill, local kill, NOT zeroed" << endl;
	    return;
	}

    // No dead souls in here.
    const QObject *dyer = sender (); // Who needs last rites?
    if (dyer) {                 // Doe he live still
	kmenubar = 0L;
	kdDebug(200) << "KTM: ACK mb kill, zeroed" << endl;
    } else {
	kdDebug(200) << "KTM: ACK mb kill, dyer zero, NOT zeroed" << endl;
    }

}

void KTMainWindow::toolbarKilled()
{
    if (localKill) {
	kdDebug(200) << "KTM: ACK tb kill, local kill, NOT removed from list" << endl;
	return;
    }

    // No dead souls in here.
    const QObject *dyer = sender (); // Who need last rites?

    if (dyer) {
	toolbars.removeRef((KToolBar *) dyer); // remove it from the list;
	kdDebug(200) << "KTM: ACK tb kill, removed from list" << endl;
	//else
	//kdDebug(200) << "KTM: ACK tb kill, NOT removed from list");
    } else {
	kdDebug(200) << "KTM: ACK tb kill, dyer zero, NOT removed from list" << endl;
    }
}



QPopupMenu* KTMainWindow::helpMenu( const QString &aboutAppText,
				    bool showWhatsThis )
{
    if( mHelpMenu == 0 ) {
        if (aboutAppText.isEmpty())
           mHelpMenu = new KHelpMenu( this, instance()->aboutData(), showWhatsThis);
        else
           mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );

	if( mHelpMenu == 0 ) { return( 0 ); }
	connect( mHelpMenu, SIGNAL(showAboutApplication()),
		 this, SLOT(showAboutApplication()));
    }

    return( mHelpMenu->menu() );
}



QPopupMenu* KTMainWindow::customHelpMenu( bool showWhatsThis )
{
    if( mHelpMenu == 0 ) {
        mHelpMenu = new KHelpMenu( this, QString::null, showWhatsThis );
	connect( mHelpMenu, SIGNAL(showAboutApplication()),
		 this, SLOT(showAboutApplication()));
    }

    return( mHelpMenu->menu() );
}



void KTMainWindow::appHelpActivated( void )
{
    if( mHelpMenu == 0 ) {
	mHelpMenu = new KHelpMenu( this );
	if( mHelpMenu == 0 ) { return; }
    }
    mHelpMenu->appHelpActivated();
}


void KTMainWindow::showAboutApplication( void )
{
    // Just an empty virtual slot
}

KXMLGUIFactory *KTMainWindow::guiFactory()
{
    if ( !d->m_factory )
        d->m_factory = new KXMLGUIFactory( this );
    return d->m_factory;
}

void KTMainWindow::createGUI( const QString &xmlfile, bool _conserveMemory )
{
    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // we always want a help menu
    if (d->m_helpMenu == 0)
        d->m_helpMenu = new KHelpMenu(this, instance()->aboutData(), true,
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
      conserveMemory();

    setUpdatesEnabled( true );

    updateRects();
}

#include "ktmainwindow.moc"
