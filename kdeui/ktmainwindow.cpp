/* This file is part of the KDE libraries
    Copyright  (C) 1997 Stephan Kulow (coolo@kde.org)
               (C) 1997-2000 Sven Radej (radej@kde.org)
               (C) 1997-99 Matthias Ettrich (ettrich@kde.org)
               (C) 1999 Chris Schlaeger (cs@kde.org)

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

#include <qsessionmanager.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <ktmainwindow.h>
#include <ktmlayout.h>
#include <kwm.h>

// a static pointer (too bad we cannot have static objects in libraries)
QList<KTMainWindow>* KTMainWindow::memberList = 0L;

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
	if ( KTMainWindow::memberList->first() ){
	    // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
	    // hook is usefull for better document orientation
	    KTMainWindow::memberList->first()->saveGlobalProperties(kapp->sessionConfig());
	}

	QListIterator<KTMainWindow> it(*KTMainWindow::memberList);
	int n = 0;
	KConfig* config = KApplication::kApplication()->sessionConfig();
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
	    for (it.toFirst(); it.current() && !cancelled; ++it){
		last = it.current();
		cancelled = !last->queryClose();
	    }
	    if ( !cancelled && last )
		cancelled = !last->queryExit();
	    return !cancelled;
	}
	
	// the user wants it, the user gets it
	return TRUE;
    }
};

static KTLWSessionManaged* ksm = 0;


static bool initing = FALSE;

KTMainWindow::KTMainWindow( const char *name, WFlags f )
    : QWidget( 0L, name, f )
{
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
        && !QApplication::closingDown())
    {
	  //debug ("KTM destructor: deleted toolbar");
      delete toolbar;
    }
  }
  
  if (kmenubar)
  {
    delete kmenubar; // Eeeh? (sven)
    debug ("KTM destructor: deleted menubar");
  }
  else
    debug ("KTM destructor: Menubar deleted, don´t wanna hurt myself");



  /* I'm not sure if the layoutMgr is deleted by Qt. So I leave it out until
   * I have some time to look at this further. */
//  delete layoutMgr;


  //if (!QApplication::closingDown())
    delete mHelpMenu;
  debug ("KTM destructor: end");
}


void KTMainWindow::closeEvent ( QCloseEvent *e){
  if (queryClose()) {
      e->accept();

      int not_withdrawn = 0;
      QListIterator<KTMainWindow> it(*KTMainWindow::memberList);
      for (it.toFirst(); it.current(); ++it){
	  if ( !it.current()->testWState( WState_Withdrawn ) )
	      not_withdrawn++;
      }

      if ( not_withdrawn <= 1 ) { // last window close accepted?
	  if ( queryExit() ) {            // Yes, Quit app?
	      kapp->quit();             // ...and quit aplication.
	  }  else {
	      // cancel closing, it's stupid to end up with no windows at all....
	      e->ignore();
	  }
      }
  }
  debug ("KTM CloseEvent end");
}

bool KTMainWindow::queryClose()
{
  return true;
}

bool KTMainWindow::queryExit(){
  return true;
}

int KTMainWindow::addToolBar( KToolBar *toolbar, int index )
{
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

        /* add toolbars */
	for (toolbars.first(); toolbars.current(); toolbars.next())
		if (toolbars.current()->isVisibleTo(this))
		{
			switch (toolbars.current()->barPos())
			{
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
	if (kmainwidget)
	{
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

    // store the objectName for later restorating
    config->writeEntry(QString::fromLatin1("ObjectName"), name());

    // store the className for later restorating
    config->writeEntry(QString::fromLatin1("ClassName"), className());

    //use KWM for window properties
    //config->writeEntry(QString::fromLatin1("KTWGeometry"), KWM::properties(winId()));
    entryList.clear();

    if (kstatusbar)
    {
        if (kstatusbar->isVisible())
            config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Enabled"));
        else
            config->writeEntry(QString::fromLatin1("StatusBar"), QString::fromLatin1("Disabled"));
    }

    if (kmenubar)
    {
        if (kmenubar->isVisible())
            entryList.append("Enabled");
        else
            entryList.append("Disabled");
        /* switch (kmenubar->menuBarPos())
        {
            case KMenuBar::Flat:   //ignore
            case KMenuBar::Top:
                entryList.append("Top");
                break;
            case KMenuBar::Bottom:
                entryList.append("Bottom");
                break;
            case KMenuBar::Floating:
                entryList.append("Floating");
                entryList.append(KWM::properties(kmenubar->winId()).ascii());
                break;
            case KMenuBar::FloatingSystem:
                entryList.append("FloatingSystem");
	  break;
        }                                              */
        config->writeEntry(QString::fromLatin1("MenuBar"), entryList, ';');
        entryList.clear();
    }

    KToolBar *toolbar;
    QString toolKey;
    for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next())
    {
        if (toolbar->isVisible())
            entryList.append("Enabled");
        else
            entryList.append("Disabled");
        switch (toolbar->barPos())
        {
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

    if ( config->hasKey(QString::fromLatin1("ObjectName" )) )
	setName( config->readEntry(QString::fromLatin1("ObjectName")).latin1()); // latin1 is right here


//     // Use KWM for window properties
//     QString geom = config->readEntry ("KTWGeometry");
//     if (!geom.isEmpty()){
//       setGeometry(KWM::setProperties(winId(), geom));
//     }

    if (kstatusbar)
    {
        entry = config->readEntry(QString::fromLatin1("StatusBar"));
        if (entry == QString::fromLatin1("Enabled"))
            enableStatusBar(KStatusBar::Show);
        else enableStatusBar(KStatusBar::Hide);
    }

    if (kmenubar)
    {
        i = config->readListEntry (QString::fromLatin1("MenuBar"), entryList, ';');
        if (i < 2)
        {
            //debug ("KTWreadProps: bad number of kmenubar args");
            return FALSE;
        }
	bool showmenubar = false;
        entry = entryList.first();
        if (entry==QString::fromLatin1("Enabled"))
	  showmenubar = True;
        else
	  kmenubar->hide();
	/*
        entry = entryList.next();
        if (entry == QString::fromLatin1("Top"))
            kmenubar->setMenuBarPos(KMenuBar::Top);
        else if (entry == "Bottom")
            kmenubar->setMenuBarPos(KMenuBar::Bottom);
        else if (entry == "Floating")
        {
            kmenubar->setMenuBarPos(KMenuBar::Floating);
            entry=entryList.next();
            kmenubar->setGeometry(KWM::setProperties(kmenubar->winId(), entry));
            showmenubar = true;
        }
        else if (entry == "FloatingSystem")
	    {
		kmenubar->setMenuBarPos(KMenuBar::FloatingSystem);
		showmenubar = true;
	    } */
        entryList.clear();
	if (showmenubar)
	  kmenubar->show();
    }
    KToolBar *toolbar;
    QString toolKey;
    QListIterator<KToolBar> it(toolbars); // must use own iterator

    for ( ; it.current(); ++it)
    {
        toolbar= it.current();
        toolKey.setNum(n);
        toolKey.prepend(QString::fromLatin1("ToolBar"));

        i = config->readListEntry(toolKey, entryList, ';');
        if (i < 2)
        {
            //printf ("KTWRP: bad number of toolbar%d args\n", n);
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
	/* We will get a layout hint when the view widget or a bar changes it's
	 * size constraines. Since we might have to adjust the window size we
	 * call updateGeometry.
	 * It has been added thanks to Nicolas Hadacek's hint. CS */
    if (ev->type() == QEvent::LayoutHint)
		updateRects();
	
    return QWidget::event(ev);
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
  if (!kstatusbar)
  {
    kstatusbar = new KStatusBar(this);
    updateRects ();
  }
  return kstatusbar;
}

void KTMainWindow::setStatusBar (KStatusBar *statusbar)
{
  kstatusbar = statusbar;
  if ( kstatusbar )
    connect( kstatusbar, SIGNAL( moved() ), this, SLOT( updateRects() ) );
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
  //connect ( kmenubar, SIGNAL( moved (menuPosition) ),
  //          this, SLOT( updateRects() ) );
  connect (kmenubar, SIGNAL(destroyed ()), this, SLOT(menubarKilled ()));
  updateRects();
}

KToolBar *KTMainWindow::toolBar( int ID )
{
  KToolBar* result = 0;
  if (ID < int(toolbars.count()))
    result = toolbars.at( ID )
      ;
  if (!result) {
    bool honor_mode = (ID == 0) ? true : false;
    result  = new KToolBar(this, 0, -1, honor_mode);
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
    debug ("KTM: ACK mb kill, local kill, NOT zeroed");
    return;
  }

  // No dead souls in here.
  const QObject *dyer = sender (); // Who needs last rites?
  if (dyer)                  // Doe he live still
  {
    kmenubar = 0L;
    debug ("KTM: ACK mb kill, zeroed");
  }
  else
  {
   debug ("KTM: ACK mb kill, dyer zero, NOT zeroed");
  }

}

void KTMainWindow::toolbarKilled()
{
  if (localKill)
  {
    debug ("KTM: ACK tb kill, local kill, NOT removed from list");
    return;
  }

  // No dead souls in here.
  const QObject *dyer = sender (); // Who need last rites?

  if (dyer)
  {
    toolbars.removeRef((KToolBar *) dyer); // remove it from the list;
    debug ("KTM: ACK tb kill, removed from list");
    //else
      //debug ("KTM: ACK tb kill, NOT removed from list");
  }
  else
    debug ("KTM: ACK tb kill, dyer zero, NOT removed from list");
}



QPopupMenu* KTMainWindow::helpMenu( const QString &aboutAppText,
				    bool showWhatsThis )
{
  if( mHelpMenu == 0 )
  {
    mHelpMenu = new KHelpMenu( this, aboutAppText, showWhatsThis );
    if( mHelpMenu == 0 ) { return( 0 ); }
    connect( mHelpMenu, SIGNAL(showAboutApplication()),
	     this, SLOT(showAboutApplication()));
  }

  return( mHelpMenu->menu() );
}



void KTMainWindow::appHelpActivated( void )
{
  if( mHelpMenu == 0 )
  {
    mHelpMenu = new KHelpMenu( this );
    if( mHelpMenu == 0 ) { return; }
  }
  mHelpMenu->appHelpActivated();
}


void KTMainWindow::showAboutApplication( void )
{
  // Just an empty virtual slot
}



#include "ktmainwindow.moc"
