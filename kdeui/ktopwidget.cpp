/* This file is part of the KDE libraries
    Copyright  (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
               (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
               (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

/* On next binary incompatible change, someone please undefine KTW_BINCOMPAT */

/**************************************************************************/

//undefine this to have binary compatibility (setBorderwidth doesn't work)
#define KTW_BINCOMPAT

// undefine this to have old apps default-exit when lastwindow closed
#define KTW_NOOLDDEFEXIT

/**************************************************************************/

#include <ktopwidget.h>
#include <ktopwidget.moc>
#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <qstrlist.h>
#include <kwm.h>
#include <qobjcoll.h>

#ifdef KTW_BINCOMPAT
#define usesNewStyle borderwidth
#endif

// a static pointer (too bad we cannot have static objects in libraries)
QList<KTopLevelWidget>* KTopLevelWidget::memberList = 0L;

KTopLevelWidget::KTopLevelWidget( const char *name )
    : QWidget( 0L, name )
{

    usesNewStyle = 1;

    kmenubar = 0L;
    kmainwidget = 0L;
    kstatusbar = 0L;

#ifndef KTW_BINCOMPAT
    borderwidth = 0;
#endif

    // set the specified icons
    KWM::setIcon(winId(), kapp->getIcon());
    KWM::setMiniIcon(winId(), kapp->getMiniIcon());

    kmainwidgetframe = new QFrame( this );
    CHECK_PTR( kmainwidgetframe );
    kmainwidgetframe ->setFrameStyle( QFrame::Panel | QFrame::Sunken);
    kmainwidgetframe ->setLineWidth(0);
    kmainwidgetframe ->hide();

    // If the application does not yet have a top widget, make it this one


    // Enable session management (Matthias)
    setUnsavedData(False);
    if( !kapp->topWidget() ){
      kapp->setTopWidget( this );
      kapp->enableSessionManagement();
      connect(kapp, SIGNAL(saveYourself()), SLOT(saveYourself()));
    }

    // see if there already is a member list
    if( !memberList )
      memberList = new QList<KTopLevelWidget>;
    
    // enter the widget in the list of all KTWs
    memberList->append( this );

    // finally set the caption
    setCaption(kapp->getCaption());
}

KTopLevelWidget::~KTopLevelWidget()
{
  if (usesNewStyle==1)
  {
    int n=0;
    // THANK YOU FOR THIS, TROLLS AND GOD BLESS YOU!!
    if (!KApplication::closingDown())
    {
      debug ("KTW destructor: deleteing *Bars");

      toolbars.setAutoDelete(true);
      //  Must use iterator since deleting of toolbar will cause her
      //  to embed itself back and  call ktopwidget's updateRects
      //  which will count toolbars with first()/next()
      //  Or, maybe not.
      //  God knows how many events and signals are pending
      //  and waiting for me to die and than to molest it
      QListIterator<KToolBar>  tit(toolbars);
      while ( tit.current() )
      {
        toolbars.remove(tit.current()); // will point to next by itself
        ++n;
        //++tit;
      }
      if (kmenubar)
        delete kmenubar;
      printf ("KTW destructor: deleted %d toolbar(s)\n", n);
    }
    else
    {
      debug ("KTW destructor: Aha! K/QApp closingDown!");
      // Don't touch anything!!!
    }
  }
  else // we're used by old application
  {
    toolbars.setAutoDelete(false);
    toolbars.clear(); // this prevents post-mortem scavengers (?)
    debug ("KTW destructor: old style used");
  }
  

	// remove this widget from the member list
	memberList->remove( this );

	// if this was the topWidget, find a new one to be it
	if( kapp && kapp->topWidget() == this ){
	    KTopLevelWidget* pTemp = 0;
	    if( ( pTemp = memberList->getFirst() ) )
	      kapp->setTopWidget( pTemp );
	}
	if(kapp && memberList->isEmpty()){
	  // if there is no mainWidget left: bad luck
	  // Matthias:
	  //        Nope, not bad luck. We should simply
	  //        exit the application in this case.
          //        (But emit a signal before)
          // Sven: Matthias what about kfm and like?
          // for now I'll fix it for old apps but you
          // found a way to fix it for new. There might
          // be apps who want to live after lastwindow closed.
	  
          kapp->setTopWidget( 0 );
          debug ("KTW destructor: postmortem: topwidget 0");
#ifdef KTW_NOOLDDEFEXIT
          if (usesNewStyle == 1) // damn!
            kapp->quit();
#else
          kapp->quit();
#endif
          debug ("KTW destructor: postmortem: quit");
        }
        debug ("KTW destructor: dead as a dodo (exiting)");
}

/*  The problem is with old apps who do not reimplement
    closeEvent (karm, kwindowtest)... */
void KTopLevelWidget::closeEvent ( QCloseEvent *e){
  if (usesNewStyle != 1)
  {
    debug ("KTW closeEvent: doing it old style");
    QWidget::closeEvent(e);
  }
  else
    
  if (memberList->count() > 1 || queryExit()){
    e->accept();
    delete this;
  }
}

bool KTopLevelWidget::queryExit(){
  return true;
}


int KTopLevelWidget::addToolBar( KToolBar *toolbar, int index )
{
  debug ("addToolBar() is deprecated and will be removed. Use toolBar()");
  usesNewStyle = 0;
  if ( index == -1 )
    toolbars.append( toolbar );
  else
    toolbars.insert( index, toolbar );
  index = toolbars.at();
  connect ( toolbar, SIGNAL( moved (BarPosition) ),
            this, SLOT( updateRects() ) );
  updateRects();
  return index;
}

void KTopLevelWidget::setView( QWidget *view, bool show_frame )
{
  kmainwidget = view;
  if( show_frame ){

    // Set a default frame borderwidth, for a toplevelwidget with
    // frame.
#ifndef KTW_BINCOMPAT
    if(borderwidth == 0 )
      setFrameBorderWidth(0);
#endif
    kmainwidgetframe->show();
  }

  // In the case setView(...,TRUE),
  // we leave the default frame borderwith at 0 so that we don't get
  // an unwanted border -- after all we didn't request a frame. If you
  // still want a border ( though no frame, call setFrameBorderWidth()
  // before setView(...,FALSE).

}

void KTopLevelWidget::setMenu( KMenuBar *menu )
{
  debug ("setMenu() is deprecated and will be removed. Use menuBar()");
  usesNewStyle = 0;
  kmenubar = menu;
  connect ( kmenubar, SIGNAL( moved (menuPosition) ),
            this, SLOT( updateRects() ) );
  updateRects();
}

void KTopLevelWidget::setStatusBar( KStatusBar *statusbar )
{
    debug ("setStatusBar() is deprecated and will be removed. Use statusBar()");
    kstatusbar = statusbar;
}

void KTopLevelWidget::focusInEvent( QFocusEvent *)
{
  repaint( FALSE );
}

void KTopLevelWidget::focusOutEvent( QFocusEvent *)
{
  repaint( FALSE );
}

void KTopLevelWidget::show ()
{
    QWidget::show();
    updateRects();
}

void KTopLevelWidget::updateRects()
{
    //debug ("Update");
    int t=0, b=0, l=0, r=0;
    int to=-1, bo=-1, lo=-1, ro=-1;
    int h, w;
    int freeHeight, freeWidth;

    bool fixedY = FALSE;

    if (kmainwidget && kmainwidget->minimumSize().height() == kmainwidget->maximumSize().height())
        fixedY = TRUE;
    
    if (kmainwidget && kmainwidget->minimumSize() == kmainwidget->maximumSize())
    {
        KToolBar *toolbar;
        w = kmainwidget->width()+2*borderwidth;
        h = kmainwidget->height()+2*borderwidth;

        // left toolbars first
        for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next() )
            if ( toolbar->barPos() == KToolBar::Left && toolbar->isVisible() )
            {
                toolbar->setMaxHeight(h);   // Sven: You have to do this here
                toolbar->updateRects (TRUE);        // Sven: You have to this
                if ( lo < 0 )
                {
                    lo = 0;
                    l += toolbar->width();
                }
                if ( lo + toolbar->height() > h)
                {
                    lo = 0;
                    l += toolbar->width();
                }
                toolbar->move( l-toolbar->width(), t  + lo );
                lo += toolbar->height();
            }

        w+=l; //AAAARRRGGGHHH!!!
        
        // Now right (I'm ok now)
        for ( toolbar = toolbars.first();
              toolbar != 0L; toolbar = toolbars.next() )
            if ( toolbar->barPos() == KToolBar::Right && toolbar->isVisible() )
            {
                toolbar->setMaxHeight(h);   // Sven: You have to do this here
                toolbar->updateRects (TRUE);   // Sven: You have to this
                if ( ro < 0 )
                {
                    ro = 0;
                    r += toolbar->width();
                }
                if (ro + toolbar->height() > h)
                {
                    ro = 0;
                    r += toolbar->width();
                }
                toolbar->move(w+r-toolbar->width(), t + ro);
                ro += toolbar->height();
            }
        w+=r;
        
        // Now kmenubar and top toolbars
        // No, first I'll have a beer.
        // Aaah, kold beer.. my nerves ar circulating better now...
        
        if (kmenubar && kmenubar->isVisible())   // (gulp)
            if (kmenubar->menuBarPos() == KMenuBar::Top)      // !? This beer isn't cold!
            {
		int mh = kmenubar->heightForWidth(w);
                kmenubar->setGeometry(0, 0, w, mh);
                t += mh;
            }

        // Top toolbars
        for ( toolbar = toolbars.first() ;
              toolbar != 0L ; toolbar = toolbars.next() )
            if ( toolbar->barPos() == KToolBar::Top && toolbar->isVisible() )
            {
                toolbar->setMaxWidth(w);
                toolbar->updateRects (TRUE);     // Sven: You have to do this
                if ( to < 0 )
                {
                    to = 0;
                    t += toolbar->height();
                }
                if (to + toolbar->width() > w)
                {
                    to = 0;
                    t += toolbar->height();
                }
                toolbar->move( to, t-toolbar->height() );
                to += toolbar->width();
            }
        h+=t;

        // move vertical toolbar for t down.
        for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next())
            if (toolbar->isVisible())
                if (toolbar->barPos() == KToolBar::Left ||
                    toolbar->barPos() == KToolBar::Right)
                    toolbar->move(toolbar->x(), t);
        
        // Bottom toolbars
        for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next())
            if ( toolbar->barPos() == KToolBar::Bottom && toolbar->isVisible() )
            {
                toolbar->setMaxWidth(w);
                toolbar->updateRects (TRUE);   // Sven: You have to this
                if ( bo < 0 )
                {
                    bo = 0;
                    b += toolbar->height();
                }
                if (bo + toolbar->width() > w)
                {
                    bo = 0;
                    b += toolbar->height();
                }
                toolbar->move(bo, h+b-toolbar->height());
                bo += toolbar->width();
            }

        // Statusbar
        if ( kstatusbar && kstatusbar->isVisible() )
        {
            kstatusbar->setGeometry(0, h+b, w, kstatusbar->height());
            b += kstatusbar->height();
        }
        
        // Bottom menubar
        if (kmenubar && kmenubar->isVisible())
            if (kmenubar->menuBarPos() == KMenuBar::Bottom)
            {
		int mh = kmenubar->heightForWidth(w);
                kmenubar->setGeometry(0, h+b, w, mh);
                b+=mh;
            }
        h+=b;
        
        // Move everything
        if (kmainwidgetframe)
            kmainwidgetframe->move(l, t);
        if (kmainwidget)
            kmainwidget->move(l+borderwidth, t+borderwidth);
        
        // Set geometry
        setFixedSize(w, h);
        resize(w,h);
        return;
    }
    else // resizable and y-fixed widgets
    {
        KToolBar *toolbar;
        if (fixedY)
            h=kmainwidget->height()+2*borderwidth;
        else
            h = height();
        w = width();

        // menubar if on top
        if (kmenubar && kmenubar->isVisible())
            if (kmenubar->menuBarPos() == KMenuBar::Top)
            {
		int mh = kmenubar->heightForWidth(w);
                kmenubar->setGeometry(0, 0, w, mh);
                t += mh;
            }

        // top toolbars
        for (toolbar = toolbars.first(); toolbar != 0L ; toolbar = toolbars.next())
            if ( toolbar->barPos() == KToolBar::Top && toolbar->isVisible() )
            {
                toolbar->updateRects (TRUE);     // Sven: You have to do this
                if ( to < 0 )
                {
                    to = 0;
                    t += toolbar->height();
                }
                if (to + toolbar->width() > width())
                {
                    to = 0;
                    t += toolbar->height();
                }
                toolbar->move( to, t-toolbar->height() );
                to += toolbar->width();
            }

        if (fixedY == TRUE)
        {
            // Bottom toolbars
            for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next())
                if ( toolbar->barPos() == KToolBar::Bottom && toolbar->isVisible() )
                {
                    toolbar->updateRects (TRUE);   // Sven: You have to this
                    if ( bo < 0 )
                    {
                        bo = 0;
                        b += toolbar->height();
                    }
                    if (bo + toolbar->width() > w)
                    {
                        bo = 0;
                        b += toolbar->height();
                    }
                    toolbar->move(bo, h+b+t-toolbar->height());
                    bo += toolbar->width();
                }
            
            // Statusbar
            if ( kstatusbar && kstatusbar->isVisible() )
            {
                b += kstatusbar->height();
                kstatusbar->setGeometry(0, h+b+t-kstatusbar->height(), w, kstatusbar->height());
            }

            // menubar if on bottom
            if (kmenubar && kmenubar->isVisible())
                if (kmenubar->menuBarPos() == KMenuBar::Bottom)
                {
                    b+=kmenubar->height();
                    kmenubar->setGeometry(0, h+b+t-kmenubar->height(), w, kmenubar->height());
                }
            freeHeight = h;
        }
        else
        {
            // menubar if on bottom
            if (kmenubar && kmenubar->isVisible())
                if (kmenubar->menuBarPos() == KMenuBar::Bottom)
                {
                    b+=kmenubar->height();
                    kmenubar->setGeometry(0, h-b, w, kmenubar->height());
                }

            // statusbar
            if ( kstatusbar && kstatusbar->isVisible() )
            {
                b += kstatusbar->height();
                kstatusbar->setGeometry(0, h-b, w, kstatusbar->height());
            }

            // Bottom toolbars
            for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next())
                if ( toolbar->barPos() == KToolBar::Bottom && toolbar->isVisible() )
                {
                    toolbar->updateRects (TRUE);   // Sven: You have to this
                    if ( bo < 0 )
                    {
                        bo = 0;
                        b += toolbar->height();
                    }
                    if (bo + toolbar->width() > w)
                    {
                        bo = 0;
                        b += toolbar->height();
                    }
                    toolbar->move(bo, h-b);
                    bo += toolbar->width();
                }

            freeHeight = h-b-t;
        }
        // left toolbars
        for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next() )
            if ( toolbar->barPos() == KToolBar::Left && toolbar->isVisible() )
            {
                toolbar->setMaxHeight(freeHeight);   // Sven: You have to do this here
                toolbar->updateRects (TRUE);        // Sven: You have to this
                if ( lo < 0 )
                {
                    lo = 0;
                    l += toolbar->width();
                }
                if ( lo + toolbar->height() > freeHeight)
                {
                    lo = 0;
                    l += toolbar->width();
                }
                toolbar->move( l-toolbar->width(), t  + lo );
                lo += toolbar->height();
            }
        
        // right toolbars
        for ( toolbar = toolbars.first();
              toolbar != 0L; toolbar = toolbars.next() )
            if ( toolbar->barPos() == KToolBar::Right && toolbar->isVisible() )
            {
                toolbar->setMaxHeight(freeHeight);   // Sven: You have to do this here
                toolbar->updateRects (TRUE);   // Sven: You have to this
                if ( ro < 0 )
                {
                    ro = 0;
                    r += toolbar->width();
                }
                if (ro + toolbar->height() > freeHeight)
                {
                    ro = 0;
                    r += toolbar->width();
                }
                toolbar->move(w-r, t + ro);
                ro += toolbar->height();
            }

        freeWidth = w-l-r;

        // set geometry of everything
        if (kmainwidgetframe)
            kmainwidgetframe->setGeometry(l, t, freeWidth, freeHeight);
        if (kmainwidget)
            kmainwidget->setGeometry(l+borderwidth, t+borderwidth,
                                     freeWidth-2*borderwidth,
                                     freeHeight-2*borderwidth);
        // set public variables (for compatibility - kghostview)
        view_left = l;
        view_right = width()-r;
        view_top = t;
        view_bottom = height()-b;
        if (fixedY == TRUE)
        {
            setMaximumSize(9999, h+t+b);
            setMinimumSize(kmainwidget->minimumSize().width()+(2*borderwidth)+l+r, h+t+b);
            resize (width(), h+t+b);  // Carefull now!
        }

        
    }
}


void KTopLevelWidget::saveYourself(){
  // Do session management (Matthias)
  if (kapp->topWidget() != this)
    return;
  QListIterator<KTopLevelWidget> it(*memberList);
  int n = 0;
  KConfig* config = KApplication::getKApplication()->getSessionConfig();
  config->setGroup("Number");
  config->writeEntry("NumberOfWindows", memberList->count());
  for (it.toFirst(); it.current(); ++it){
    n++;
    it.current()->savePropertiesInternal(config, n); 
  }
  config->sync();
}


//Matthias
void KTopLevelWidget::savePropertiesInternal (KConfig* config, int number)
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
            config->writeEntry("StatusBar", "Enabled");
        else
            config->writeEntry("StatusBar", "Disabled");
    }
    
    if (kmenubar)
    {
        if (kmenubar->isVisible())
            entryList.append("Enabled");
        else
            entryList.append("Disabled");
        switch (kmenubar->menuBarPos())
        {
            case KMenuBar::Top:
                entryList.append("Top");
                break;
            case KMenuBar::Bottom:
                entryList.append("Bottom");
                break;
            case KMenuBar::Floating:
                entryList.append("Floating");
                entryList.append(KWM::getProperties(kmenubar->winId()));
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
                entryList.append(KWM::getProperties(toolbar->winId()));
                break;
        }
        toolKey.setNum(n);
        toolKey.prepend("ToolBar");
        config->writeEntry(toolKey.data(), entryList, ';');
        entryList.clear();
        n++;
    }

    s.setNum(number);
    config->setGroup(s);
    saveProperties(config);
}

//Matthias
bool KTopLevelWidget::readPropertiesInternal (KConfig* config, int number)
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
    if (config->hasKey("KTWGeometry") == FALSE) // No global, return False
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
	bool showmenubar = False;  //Matthias
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
	    kmenubar->show();
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

        i = config->readListEntry(toolKey.data(), entryList, ';');
        if (i < 2)
        {
            //printf ("KTWRP: bad number of toolbar%d args\n", n);
            return FALSE;
        }
        
	bool showtoolbar = False;  //Matthias
        entry = entryList.first();
        if (entry=="Enabled")
	  showtoolbar = True; //Matthias
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

void KTopLevelWidget::setFrameBorderWidth(int size){
#ifndef KTW_BINCOMPAT
  borderwidth = size;
#endif
}

//Matthias
bool KTopLevelWidget::canBeRestored(int number){
  if (!kapp->isRestored())
    return False;
  KConfig *config = kapp->getSessionConfig();
  if (!config)
    return False;
  config->setGroup("Number");
  int n = config->readNumEntry("NumberOfWindows", 0);
  return (number >= 1 && number <= n);
}

//Matthias
const QString KTopLevelWidget::classNameOfToplevel(int number){
  if (!kapp->isRestored())
    return "";
  KConfig *config = kapp->getSessionConfig();
  if (!config)
    return False;
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
bool KTopLevelWidget::restore(int number){
  if (!canBeRestored(number))
    return False;
  KConfig *config = kapp->getSessionConfig();
  if (readPropertiesInternal(config, number)){
    show();
    return True;
  }
  return False;
}
// Matthias
void KTopLevelWidget::setUnsavedData( bool b){
  KWM::setUnsavedDataHint(winId(), b);
}


void KTopLevelWidget::resizeEvent( QResizeEvent *ev )
{
    if (kmainwidget && kmainwidget->minimumSize() == kmainwidget->maximumSize())
        return;
    if (kmainwidget && kmainwidget->minimumSize().height() == kmainwidget->maximumSize().height())
        if (ev->oldSize() == ev->size())
            return;
    updateRects();
}

KStatusBar *KTopLevelWidget::statusBar()
{
  if (!kstatusbar) 
    kstatusbar = new KStatusBar(this);
  return kstatusbar;
}

KMenuBar *KTopLevelWidget::menuBar()
{
  if (!kmenubar) {
    kmenubar = new KMenuBar(this);
    connect ( kmenubar, SIGNAL( moved (menuPosition) ),
	      this, SLOT( updateRects() ) );
    updateRects();
  }
  return kmenubar;
}

KToolBar *KTopLevelWidget::toolBar( int ID )
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
    updateRects();
  }
  return result;
}

void KTopLevelWidget::enableToolBar( KToolBar::BarStatus stat, int ID )
{
  KToolBar *t = toolbars.at( ID );
  if ( t )
    t->enable( stat );
  updateRects();
}

void KTopLevelWidget::enableStatusBar( KStatusBar::BarStatus stat )
{
  CHECK_PTR( kstatusbar );
  if ( ( stat == KStatusBar::Toggle && kstatusbar->isVisible() )
       || stat == KStatusBar::Hide )
    kstatusbar->hide();
  else
    kstatusbar->show();
  updateRects();
}


