/* This file is part of the KDE libraries
    Copyright  (C) 1997 Stephan Kulow (coolo@kde.org)
               (C) 1997 Sven Radej (sven.radej@iname.com)
               (C) 1997 Matthias Ettrich (ettrich@kde.org)

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
#include <ktmainwindow.moc>
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
    XSetIconName( qt_xdisplay(), winId(), kapp->getCaption() );

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
      memberList = new QList<KTMainWindow>;

    // enter the widget in the list of all KTWs
    memberList->append( this );

    // finally set the caption
    setCaption(kapp->getCaption());

    localKill = false;
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


void KTMainWindow::updateRects()
{
  // This thing is first against the wall as
  // soon as I have some free time.

  //debug ("Update");
  int t=0, b=0, l=0, r=0;
  int to=-1, bo=-1, lo=-1, ro=-1;
  int h, w;
  int freeHeight, freeWidth;
  int widest; // To find widest toolbar
  bool fixedY = FALSE;
  int flatX=0;
  int flatY=0;
  int FlatHeight=0;

#define vSpace 2

  if (kmainwidget && kmainwidget->minimumSize().height() ==
      kmainwidget->maximumSize().height())
    fixedY = TRUE;

  if (kmainwidget && kmainwidget->minimumSize() ==
      kmainwidget->maximumSize())
    {
      KToolBar *toolbar;
      w = kmainwidget->width()+2*borderwidth;
      h = kmainwidget->height()+2*borderwidth;

      // left toolbars first
      widest = 0;
      for (toolbar=toolbars.first(); toolbar != 0L; toolbar=toolbars.next())
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
          flatY=t;
          flatX=0;
        }
        else if (kmenubar->menuBarPos() == KMenuBar::Flat) //flat menubar
        {
          kmenubar->move(0, 0);
          FlatHeight=kmenubar->height();
          flatX += kmenubar->width() + vSpace;
          flatY=0;
        }
      //flat toolbars
      for (toolbar=toolbars.first(); toolbar != 0L; toolbar=toolbars.next())
        if (toolbar->barPos() == KToolBar::Flat && toolbar->isVisible() )
        {
          toolbar->move(flatX, flatY);
          flatX+=toolbar->width()+vSpace;
          FlatHeight=toolbar->height();
        }

      if (FlatHeight)
        t+=FlatHeight;

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
      view_left = l;
      view_right = width()-r;
      view_top = t;
      view_bottom = height()-b;
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
          t = mh;
          flatY=t;
          flatX=0;
        }
        else if (kmenubar->menuBarPos() == KMenuBar::Flat) //flat menubar
        {
          kmenubar->move(0, 0);
          FlatHeight=kmenubar->height();
          flatX = kmenubar->width() + vSpace;
          flatY=0;
        }
      //flat toolbars
      for (toolbar=toolbars.first(); toolbar != 0L; toolbar=toolbars.next())
        if (toolbar->barPos() == KToolBar::Flat && toolbar->isVisible() )
        {
          toolbar->move(flatX, flatY);
          flatX+=toolbar->width()+vSpace;
          FlatHeight=toolbar->height();
        }

      if (FlatHeight)
        t+=FlatHeight;

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
      widest=0;
      for (toolbar = toolbars.first(); toolbar != 0L; toolbar = toolbars.next() )
        if ( toolbar->barPos() == KToolBar::Left && toolbar->isVisible() )
        {
          toolbar->setMaxHeight(freeHeight);   // Sven: You have to do this here
          toolbar->updateRects (TRUE);        // Sven: You have to this
          widest = (widest>toolbar->width())?widest:toolbar->width();
          if ( lo < 0 )
          {
            lo = 0;
            l += toolbar->width();
            toolbar->move( l-toolbar->width(), t  + lo );
            lo += toolbar->height();
          }
          else if ( lo + toolbar->height() > freeHeight)
          {
            lo = 0;
            l += toolbar->width();
            toolbar->move( l-toolbar->width(), t  + lo );
            lo += toolbar->height();
          }
          else
          {
            l=widest;
            toolbar->move( l-toolbar->width(), t  + lo );
            lo += toolbar->height();
          }
        }

      // right toolbars
      widest=0;
      for ( toolbar = toolbars.first();
            toolbar != 0L; toolbar = toolbars.next() )
        if ( toolbar->barPos() == KToolBar::Right && toolbar->isVisible() )
        {
          toolbar->setMaxHeight(freeHeight);   // Sven: You have to do this here
          toolbar->updateRects (TRUE);   // Sven: You have to this
          widest = (widest>toolbar->width())?widest:toolbar->width();
          if ( ro < 0 )
          {
            ro = 0;
            r += toolbar->width();
            toolbar->move(w-r, t + ro);
            ro += toolbar->height();
          }
          else if (ro + toolbar->height() > freeHeight)
          {
            ro = 0;
            r += toolbar->width();
            toolbar->move(w-r, t + ro);
            ro += toolbar->height();
          }
          else
          {
            r=widest;
            toolbar->move(w-r, t + ro);
            ro += toolbar->height();
          }
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
            case KMenuBar::Flat:   //ignore
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

void KTMainWindow::setFrameBorderWidth(int size){

  borderwidth = size;

}

//Matthias
bool KTMainWindow::canBeRestored(int number){
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
const QString KTMainWindow::classNameOfToplevel(int number){
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
bool KTMainWindow::restore(int number){
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
void KTMainWindow::setUnsavedData( bool b){
  KWM::setUnsavedDataHint(winId(), b);
}


void KTMainWindow::resizeEvent( QResizeEvent *ev )
{
    if (kmainwidget && kmainwidget->minimumSize() == kmainwidget->maximumSize())
        return;
    if (kmainwidget && kmainwidget->minimumSize().height() == kmainwidget->maximumSize().height())
        if (ev->oldSize() == ev->size())
            return;
    updateRects();
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

