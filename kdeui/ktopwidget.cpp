#include <ktopwidget.h>
#include <ktopwidget.moc>
#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <qstrlist.h>
//#include <qobjcoll.h>

// a static pointer (too bad we cannot have static objects in libraries)
QList<KTopLevelWidget>* KTopLevelWidget::memberList = NULL;

KTopLevelWidget::KTopLevelWidget( const char *name )
    : QWidget( 0L, name )
{
    kmenubar = NULL;
    kmainwidget = NULL;
    kstatusbar = NULL;
    borderwidth = 0;

    kmainwidgetframe = new QFrame( this );
    CHECK_PTR( kmainwidgetframe );
    kmainwidgetframe ->setFrameStyle( QFrame::Panel | QFrame::Sunken);
    kmainwidgetframe ->setLineWidth(0);
    kmainwidgetframe ->hide();

	// If the application does not yet have a main widget, make it this one
	if( !kapp->mainWidget() )
	  kapp->setMainWidget( this );

	// see if there already is a member list
	if( !memberList )
	  memberList = new QList<KTopLevelWidget>;

	// enter the widget in the list of all KTWs
	memberList->append( this );
}

KTopLevelWidget::~KTopLevelWidget()
{
    // This doesn't work; dunno how to do it - Sven
    /*
    int n = 0;

    QObjectList  *toolbars = queryList( 0, "KToolBar", FALSE );
    QObjectListIt tit( *toolbars );
    while ( tit.current() )
    {
        delete tit.current();
        n++;
        ++tit;
    }

    printf ("KTW destructor: deleted %d toolbar(s)\n", n);
    n=0;
    
    QObjectList  *menubars = queryList( 0, "KMenuBar", FALSE );
    QObjectListIt mit( *menubars );
    while ( mit.current() )
    {
        delete mit.current();
        n++;
        ++mit;
    }

    printf ("KTW destructor: deleted %d menubar(s)\n", n);

    delete menubars;
    delete toolbars;
    */
    KDEBUG (KDEBUG_INFO, 151, "KTopLevelWidget destructor: finished");

	// remove this widget from the member list
	memberList->remove( this );

	// if this was the mainWidget, find a new one to be it
	if( kapp->mainWidget() == this )
	  {
		KTopLevelWidget* pTemp = NULL;
		if( ( pTemp = memberList->getFirst() ) )
		  kapp->setMainWidget( pTemp );
		// if there is no mainWidget left: bad luck
		else
		  {
			KDEBUG( KDEBUG_FATAL, 151, "No main widget left" );
			kapp->setMainWidget( NULL );

			// but since it is the last one, it can at least deallocate
			// the member list...
			delete memberList;
		  }
	  }
}

int KTopLevelWidget::addToolBar( KToolBar *toolbar, int index )
{
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

void KTopLevelWidget::setMenu( KMenuBar *menu )
{
  kmenubar = menu;
  connect ( kmenubar, SIGNAL( moved (menuPosition) ),
            this, SLOT( updateRects() ) );
  updateRects();
}

void KTopLevelWidget::setStatusBar( KStatusBar *statusbar )
{
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
        for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next() )
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
              toolbar != NULL; toolbar = toolbars.next() )
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
                kmenubar->setGeometry(0, 0, w, kmenubar->height());
                t+=kmenubar->height();
            }

        // Top toolbars
        for ( toolbar = toolbars.first() ;
              toolbar != NULL ; toolbar = toolbars.next() )
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
        for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next())
            if (toolbar->isVisible())
                if (toolbar->barPos() == KToolBar::Left ||
                    toolbar->barPos() == KToolBar::Right)
                    toolbar->move(toolbar->x(), t);
        
        // Bottom toolbars
        for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next())
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
                kmenubar->setGeometry(0, h+b, w, kmenubar->height());
                b+=kmenubar->height();
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
                kmenubar->setGeometry(0, 0, w, kmenubar->height());
                t+=kmenubar->height();
            }

        // top toolbars
        for (toolbar = toolbars.first(); toolbar != NULL ; toolbar = toolbars.next())
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
            for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next())
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
            for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next())
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
        for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next() )
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
              toolbar != NULL; toolbar = toolbars.next() )
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

bool KTopLevelWidget::saveProperties (bool global)
{
    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
               // in order they are in toolbar list
    
    KConfig *config = KApplication::getKApplication()->getConfig();

    if (global == TRUE)
        config->setGroup("WindowProperties");
    else
    {
        if (caption() == 0)
            return FALSE;
        config->setGroup(caption());
    }

    entry.setNum(x());
    entryList.append(entry.data());
    entry.setNum(y());
    entryList.append(entry.data());
    entry.setNum(width());
    entryList.append(entry.data());
    entry.setNum(height());
    entryList.append(entry.data());
    config->writeEntry("KTWGeometry", entryList, ';');
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
                entry.setNum(kmenubar->x());
                entryList.append(entry.data());
                entry.setNum(kmenubar->y());
                entryList.append(entry.data());
                entry.setNum(kmenubar->width());
                entryList.append(entry.data());
                entry.setNum(kmenubar->height());
                entryList.append(entry.data());
                break;
        }
        config->writeEntry("MenuBar", entryList, ';');
        entryList.clear();
    }

    KToolBar *toolbar;
    QString toolKey;
    for (toolbar = toolbars.first(); toolbar != NULL; toolbar = toolbars.next())
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
                entry.setNum(toolbar->x());
                entryList.append(entry.data());
                entry.setNum(toolbar->y());
                entryList.append(entry.data());
                entry.setNum(toolbar->width());
                entryList.append(entry.data());
                entry.setNum(toolbar->height());
                entryList.append(entry.data());
                break;
        }
        toolKey.setNum(n);
        toolKey.prepend("ToolBar");
        config->writeEntry(toolKey.data(), entryList, ';');
        entryList.clear();
        n++;
    }

    config->sync();
    return TRUE;
}

bool KTopLevelWidget::readProperties (bool global)
{
    // All comments by sven
    int xx, yy, ww, hh;
    
    QString entry;
    QStrList entryList;
    int n = 1; // Tolbar counter. toolbars are counted from 1,
               // in order they are in toolbar list
    int i = 0; // Number of entries in list
    
    KConfig *config = KApplication::getKApplication()->getConfig();

    // How do I check if there is group I want?
    
    if (global == FALSE) // i.e. if first specific ("file:/usr/local.." than global
    {
        if (caption() == 0)
            //return FALSE;
        config->setGroup(caption());
        if (config->hasKey("KTWGeometry") == FALSE) // no speciffic, try global
        {
            config->setGroup("WindowProperties");
            if (config->hasKey("KTWGeometry") == FALSE) // No global, return False
                return FALSE;
        }
    }
    else // try global only
    {
        config->setGroup("WindowProperties");
        if (config->hasKey("KTWGeometry") == FALSE) // No global, return False
            return FALSE;
    }

    // Group should hopefully be set now.
    // Why isn't there bool KConfig::hasGroup(groupname) ?

    i = config->readListEntry ("KTWGeometry", entryList, ';');
    if (i < 4)
    {
        //debug ("KTW readProperties: less than 4 numbers in geometry");
        return FALSE; // Or crash, whatever you wish.
    }
    entry = entryList.first();
    xx = entry.toInt();
    entry = entryList.next();
    yy = entry.toInt();
    entry = entryList.next();
    ww = entry.toInt();
    entry = entryList.next();
    hh = entry.toInt();

    // check for some stupidities

    if ((ww <= 0) || (hh <=0) || (xx<0) || (yy<0))
    {
        //debug ("KTW readProperties: geometry sux");
        return FALSE; // if you still live
    }

    setGeometry (xx, yy, ww, hh); // possibly 0,-128, 49000, -3 (my favourite)

    entryList.clear();

    if (kstatusbar)
    {
        entry = config->readEntry("StatusBar");
        if (entry == "Enabled")
            enableStatusBar(KStatusBar::Show);
        else if (entry == "Disabled")
            enableStatusBar(KStatusBar::Hide);
        else
        {
            //debug ("KTWreadProps: bad statusbar status!");
            return FALSE;
        }
    }

    if (kmenubar)
    {
        i = config->readListEntry ("MenuBar", entryList, ';');
        if (i < 2)
        {
            //debug ("KTWreadProps: bad number of kmenubar args");
            return FALSE;
        }

        entry = entryList.first();
        if (entry=="Enabled")
            kmenubar->show();
        else if (entry=="Disabled")
            kmenubar->hide();
        else
        {
            //debug ("KTWRP: bad menubar status");
            return FALSE;
        }

        entry = entryList.next();
        if (entry == "Top")
            kmenubar->setMenuBarPos(KMenuBar::Top);
        else if (entry == "Bottom")
            kmenubar->setMenuBarPos(KMenuBar::Bottom);
        else if (entry == "Floating")
        {
            kmenubar->setMenuBarPos(KMenuBar::Floating);
            entry=entryList.next();
            xx=entry.toInt();
            entry=entryList.next();
            yy=entry.toInt();
            entry=entryList.next();
            ww=entry.toInt();
            entry=entryList.next();
            hh=entry.toInt();
            if ((ww <= 0) || (hh <=0) || (xx<0) || (yy<0))
            {
                //debug ("KTWRP: menubar geometry sux");
                return FALSE; // if you still live
            }
            kmenubar->setGeometry (xx, yy, ww, hh);
        }
        else
        {
            //debug ("KTWRP: bad menubar position");
            return FALSE;
        }
        entryList.clear();
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
        
        entry = entryList.first();
        if (entry=="Enabled")
            toolbar->enable(KToolBar::Show);
        else if (entry=="Disabled")
            toolbar->enable(KToolBar::Hide);
        else
        {
            //debug ("KTWRP: bad toolbar status");
            return FALSE;
        }

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
            xx=entry.toInt();
            entry=entryList.next();
            yy=entry.toInt();
            entry=entryList.next();
            ww=entry.toInt();
            entry=entryList.next();
            hh=entry.toInt();
            if ((ww <= 0) || (hh <=0) || (xx<0) || (yy<0))
            {
                //debug ("KTWRP: toolbar geometry sux");
                return FALSE; // if you still live
            }
            toolbar->setGeometry (xx, yy, ww, hh);
            toolbar->updateRects(TRUE);
        }
        else
        {
            //debug ("KTWRP: bad toolbar position");
            return FALSE;
        }
        n++; // next toolbar
        entryList.clear();
    }
    return TRUE;
}

void KTopLevelWidget::setFrameBorderWidth(int size){

  borderwidth = size;

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
  return kstatusbar;
}

KToolBar *KTopLevelWidget::toolBar( int ID )
{
  return toolbars.at( ID );
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


