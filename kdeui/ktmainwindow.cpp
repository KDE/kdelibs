/* This file is part of the KDE libraries
    Copyright  (C) 1997 Stephan Kulow (coolo@kde.org)
               (C) 1997 Sven Radej (sven.radej@iname.com)
               (C) 1997 Matthias Ettrich (ettrich@kde.org)
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
#include <ktmainwindow.h>
#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <qstrlist.h>
#include <kwm.h>
//#include <qobjcoll.h>

// a static pointer (too bad we cannot have static objects in libraries)
QList<KTMainWindow>* KTMainWindow::memberList = 0L;

KTMainWindow::KTMainWindow( const char *name )
    : QWidget( 0L, name )
{
    kmenubar = 0L;
    kmainwidget = 0L;
    kstatusbar = 0L;
    borderwidth = 0;

    // set the specified icons
    KWM::setIcon(winId(), kapp->getIcon());
    KWM::setMiniIcon(winId(), kapp->getMiniIcon());
    // set a short icon text
    // TODO: try to make this as unicode compatible as possible
    XSetIconName( qt_xdisplay(), winId(), kapp->getCaption().ascii() );

    kmainwidgetframe = new QFrame( this );
    CHECK_PTR( kmainwidgetframe );
    kmainwidgetframe ->setFrameStyle( QFrame::Panel | QFrame::Sunken);
    kmainwidgetframe ->setLineWidth(0);
    kmainwidgetframe ->hide();

    // If the application does not yet have a top widget, make it this one


    // Enable session management (Matthias)
    setUnsavedData(false);
    if( !kapp->topWidget() ){
      kapp->setTopWidget( this );
      kapp->enableSessionManagement();
      connect(kapp, SIGNAL(saveYourself()), SLOT(saveYourself()));
    }

    // see if there already is a member list
    if( !memberList )
      memberList = new QList<KTMainWindow>;

    // enter the widget in the list of all KTWs
    memberList->append( this );

    // finally set the caption
    setCaption(kapp->getCaption());

    localKill = false;

	layoutMgr = 0;
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
      delete toolbar;
      //debug ("KTM destructor: deleted toolbar");
    }
  }
  // delete the menubar (necessary if floating)
  if (kmenubar && kmenubar->menuBarPos() == KMenuBar::Floating
      && !QApplication::closingDown())
  {
    delete kmenubar;
    //debug ("KTM destructor: deleted menubar");
  }


  // if this was the topWidget, find a new one to be it
  if( kapp && kapp->topWidget() == this ){
    KTMainWindow* pTemp = 0;
    if( ( pTemp = memberList->getFirst() ) )
	kapp->setTopWidget( pTemp );
    else
	kapp->setTopWidget( 0 );
  }

  delete layoutMgr;

  //debug ("KTM destructor: end");
}


void KTMainWindow::deleteAll(){
  KTMainWindow* w;
  if (memberList){
    for (w = memberList->first(); w; w = memberList->next()){
      delete w;
    }
    memberList->clear();
  }
}

void KTMainWindow::closeEvent ( QCloseEvent *e){
  if (queryClose())
  {
    e->accept();
    if (memberList->count() == 1) // last window close accepted?
    {
      if (queryExit())            // Yes, Quit app?
      {
        delete this;              // Yes, delete this...
        kapp->quit();             // ...and quit aplication.
      }                           //--------------------------------
    }
    else                         // It was not last window...
      delete this;               // ...so only delete this. (sven)
  }
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
    if (kmenubar && kmenubar->menuBarPos() == KMenuBar::Floating)
	kmenubar->show();
    QWidget::show();
    updateRects();
}

QRect KTMainWindow::mainViewGeometry() const
{
	return (kmainwidget ? kmainwidget->geometry() : QRect());
}

void KTMainWindow::updateRects()
{
	if (layoutMgr)
		delete layoutMgr;

	layoutMgr = new KVTBLayout(this);
	CHECK_PTR(layoutMgr);

	/* add a top or flattened menu */
	if ((kmenubar->menuBarPos() == KMenuBar::Top ||
		 kmenubar->menuBarPos() == KMenuBar::Flat) && kmenubar->isVisible())
	{
		layoutMgr->addWidget(kmenubar);
	}

	/* add top and flat toolbars */
	for (toolbars.first(); toolbars.current(); toolbars.next())
		if ((toolbars.current()->barPos() == KToolBar::Top ||
			 toolbars.current()->barPos() == KToolBar::Flat) &&
			toolbars.current()->isVisible())
		{
			layoutMgr->addWidget(toolbars.current());
		}

	/*
	 * Add the horizontal child layout manager that holds the vertical tool
	 * bars and the main widget.
	 */
	subLayoutMgr = new KHTBLayout();
	layoutMgr->addMainLayout(subLayoutMgr);

	/* add left toolbars */
	for (toolbars.first(); toolbars.current(); toolbars.next())
		if (toolbars.current()->barPos() == KToolBar::Left &&
			toolbars.current()->isVisible())
		{
			subLayoutMgr->addWidget(toolbars.current());
		}

	/* add the main widget */
	if (kmainwidget)
	{
		if (kmainwidget->maximumWidth() > 0)
			setMaximumWidth(kmainwidget->maximumWidth());
		if (kmainwidget->maximumHeight() > 0)
			setMaximumHeight(kmainwidget->maximumHeight());
		subLayoutMgr->addMainItem(kmainwidget);
	}

	/* add right toolbars */
	for (toolbars.first(); toolbars.current(); toolbars.next())
		if (toolbars.current()->barPos() == KToolBar::Right &&
			toolbars.current()->isVisible())
		{
			subLayoutMgr->addWidget(toolbars.current());
		}

	/* add bottom toolbars */
	for (toolbars.first(); toolbars.current(); toolbars.next())
		if (toolbars.current()->barPos() == KToolBar::Bottom &&
			toolbars.current()->isVisible())
		{
			layoutMgr->addWidget(toolbars.current());
		}

	/* add the status bar */
	if (kstatusbar && kstatusbar->isVisible())
		layoutMgr->addWidget(kstatusbar);

	/* add a bottom menu bar */
	if (kmenubar->menuBarPos() == KMenuBar::Bottom && kmenubar->isVisible())
		layoutMgr->addWidget(kmenubar);

	layoutMgr->activate();
}

void KTMainWindow::saveData(KConfig*)
{
}

void KTMainWindow::saveYourself(){
  // Do session management (Matthias)
  if (kapp->topWidget() != this)
    return;

  // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
  // hook is usefull for better document orientation

  saveData(kapp->getSessionConfig());

  QListIterator<KTMainWindow> it(*memberList);
  int n = 0;
  KConfig* config = KApplication::getKApplication()->getSessionConfig();
  config->setGroup("Number");
  config->writeEntry("NumberOfWindows", memberList->count());
  for (it.toFirst(); it.current(); ++it){
    n++;
    it.current()->savePropertiesInternal(config, n);
  }
  // According to Jochen, config is synced in kapp already
  // config->sync();
}


//Matthias
void KTMainWindow::savePropertiesInternal (KConfig* config, int number)
{
    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
               // in order they are in toolbar list

    QString s;
    s.setNum(number);
    s.prepend("WindowProperties");
    config->setGroup(s);

    // store the className for later restorating (Matthias)
    config->writeEntry("ClassName", className());

    //use KWM for window properties (Matthias)
    config->writeEntry("KTWGeometry", KWM::getProperties(winId()));
    entryList.clear();

    if (kstatusbar)
    {
        if (kstatusbar->isVisible())
            config->writeEntry("StatusBar", QString("Enabled"));
        else
            config->writeEntry("StatusBar", QString("Disabled"));
    }

    if (kmenubar)
    {
        if (kmenubar->isVisible())
            entryList.append("Enabled");
        else
            entryList.append("Disabled");
        switch (kmenubar->menuBarPos())
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
                entryList.append(KWM::getProperties(kmenubar->winId()).ascii());
                break;
            case KMenuBar::FloatingSystem:
                entryList.append("FloatingSystem");
	  break;
        }
        config->writeEntry("MenuBar", entryList, ';');
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
                entryList.append(KWM::getProperties(toolbar->winId()).ascii());
                break;
        }
        toolKey.setNum(n);
        toolKey.prepend("ToolBar");
        config->writeEntry(toolKey, entryList, ';');
        entryList.clear();
        n++;
    }

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);
}

//Matthias
bool KTMainWindow::readPropertiesInternal (KConfig* config, int number)
{
    // All comments by sven

    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
               // in order they are in toolbar list
    int i = 0; // Number of entries in list
    QString s;
    s.setNum(number);
    s.prepend("WindowProperties");
    config->setGroup(s);
    if (config->hasKey("KTWGeometry") == FALSE) // No global, return false
      {
	return FALSE;
      }

    // Use KWM for window properties  (Matthias)
    QString geom = config->readEntry ("KTWGeometry");
    if (!geom.isEmpty()){
      setGeometry(KWM::setProperties(winId(), geom));
    }

    if (kstatusbar)
    {
        entry = config->readEntry("StatusBar");
        if (entry == "Enabled")
            enableStatusBar(KStatusBar::Show);
        else enableStatusBar(KStatusBar::Hide);
    }

    if (kmenubar)
    {
        i = config->readListEntry ("MenuBar", entryList, ';');
        if (i < 2)
        {
            //debug ("KTWreadProps: bad number of kmenubar args");
            return FALSE;
        }
	bool showmenubar = false;  //Matthias
        entry = entryList.first();
        if (entry=="Enabled")
	  showmenubar = True; //Matthias
        else
	  kmenubar->hide();
	
        entry = entryList.next();
        if (entry == "Top")
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
	    }
        entryList.clear();
	if (showmenubar) //Matthias
	  kmenubar->show();
    }
    KToolBar *toolbar;
    QString toolKey;
    QListIterator<KToolBar> it(toolbars); // must use own iterator

    for ( ; it.current(); ++it)
    {
        toolbar= it.current();
        toolKey.setNum(n);
        toolKey.prepend("ToolBar");

        i = config->readListEntry(toolKey, entryList, ';');
        if (i < 2)
        {
            //printf ("KTWRP: bad number of toolbar%d args\n", n);
            return FALSE;
        }

	bool showtoolbar = false;  //Matthias
        entry = entryList.first();
        if (entry=="Enabled")
	  showtoolbar = true; //Matthias
        else
	  toolbar->enable(KToolBar::Hide);

        entry = entryList.next();
        if (entry == "Top")
            toolbar->setBarPos(KToolBar::Top);
        else if (entry == "Bottom")
            toolbar->setBarPos(KToolBar::Bottom);
        else if (entry == "Left")
            toolbar->setBarPos(KToolBar::Left);
        else if (entry == "Right")
            toolbar->setBarPos(KToolBar::Right);
        else if (entry == "Floating")
        {
            toolbar->setBarPos(KToolBar::Floating);
            entry=entryList.next();
            toolbar->setGeometry(KWM::setProperties(toolbar->winId(), entry));
            toolbar->updateRects(TRUE);
	    toolbar->show();
        }
	if (showtoolbar)
	  toolbar->enable(KToolBar::Show); //Matthias
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

//Matthias
bool KTMainWindow::canBeRestored(int number){
  if (!kapp->isRestored())
    return false;
  KConfig *config = kapp->getSessionConfig();
  if (!config)
    return false;
  config->setGroup("Number");
  int n = config->readNumEntry("NumberOfWindows", 0);
  return (number >= 1 && number <= n);
}

//Matthias
const QString KTMainWindow::classNameOfToplevel(int number){
  if (!kapp->isRestored())
    return "";
  KConfig *config = kapp->getSessionConfig();
  if (!config)
    return QString();
  QString s;
  s.setNum(number);
  s.prepend("WindowProperties");
  config->setGroup(s);
  if (!config->hasKey("ClassName"))
    return "";
  else
    return config->readEntry("ClassName");
}



//Matthias
bool KTMainWindow::restore(int number){
  if (!canBeRestored(number))
    return false;
  KConfig *config = kapp->getSessionConfig();
  if (readPropertiesInternal(config, number)){
    show();
    return true;
  }
  return false;
}
// Matthias
void KTMainWindow::setUnsavedData( bool b){
  KWM::setUnsavedDataHint(winId(), b);
}


void KTMainWindow::resizeEvent(QResizeEvent *ev)
{
	/*
	 * This is an ugly hack to work around a Qt layout management
	 * problem.  If the minimum or maximum size changes during the
	 * execution of resizeEvent() functions this new size is not
	 * honored. Unfortunately due to the multiple resizeEvents() this
	 * flickers like hell in opaque resize mode. CS */
	if (width() < minimumWidth())
		resize(minimumWidth(), height());
	if (maximumWidth() > 0 && width() > maximumWidth())
		resize(maximumWidth(), height());

	if (height() < minimumHeight())
		resize(width(), minimumHeight());
	if (maximumHeight() > 0 && height() > maximumHeight())
		resize(width(), maximumHeight());
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
  connect ( kmenubar, SIGNAL( moved (menuPosition) ),
            this, SLOT( updateRects() ) );
  connect (kmenubar, SIGNAL(destroyed ()), this, SLOT(menubarKilled ()));
  updateRects();
}

KToolBar *KTMainWindow::toolBar( int ID )
{
  KToolBar* result = 0;
  if (ID < int(toolbars.count()))
    result = toolbars.at( ID );
  if (!result) {
    result  = new KToolBar(this);
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
    //debug ("KTM: ACK mb kill, local kill, NOT zeroed");
    return;
  }

  // No dead souls in here.
  const QObject *dyer = sender (); // Who need last rites?
  if (dyer)                  // Doe he live still
  {
    kmenubar = 0L;
    //debug ("KTM: ACK mb kill, zeroed");
  }
//  else
//  {
   //debug ("KTM: ACK mb kill, dyer zero, NOT zeroed");
//  }

}

void KTMainWindow::toolbarKilled()
{
  if (localKill)
  {
    //debug ("KTM: ACK tb kill, local kill, NOT removed from list");
    return;
  }

  // No dead souls in here.
  const QObject *dyer = sender (); // Who need last rites?

  if (dyer)
  {
    toolbars.removeRef((KToolBar *) dyer); // remove it from the list;
      //debug ("KTM: ACK tb kill, removed from list");
    //else
      //debug ("KTM: ACK tb kill, NOT removed from list");
  }
//  else
    //debug ("KTM: ACK tb kill, dyer zero, NOT removed from list");
}
#include "ktmainwindow.moc"

