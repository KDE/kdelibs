/*
    Copyright (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
    Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)

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

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include "qobjectlist.h"
#include "ktmainwindow.h"
#include "kmenubar.h"
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qstring.h>
#include <qframe.h>
#include <qmenudata.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kconfig.h>
#include <ktoolboxmgr.h>
#include <kwm.h>
#include <kstyle.h>

#define CONTEXT_TOP 1
#define CONTEXT_BOTTOM 2
#define CONTEXT_FLOAT 3
#define CONTEXT_FLAT 4

static bool standalone_menubar = FALSE;

static QPixmap* miniGo = 0;

/*************************************************************/

KMenuBar::KMenuBar(QWidget *parent, const char *name)
  : QFrame( parent, name )
{
  Parent = parent;        // our father
  oldWFlags = getWFlags();

  standalone_menubar = FALSE;
  frame = new QFrame (this);
  frame->setFrameStyle(NoFrame);
  menu = new KStyleMenuBarInternal (frame);
  menu->setLineWidth( 1 );
  oldMenuFrameStyle = menu->frameStyle();

  connect (menu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  connect (menu, SIGNAL(highlighted(int)), this, SLOT(slotHighlighted(int)));
  handle = new QFrame (this);
  handle->setMouseTracking( TRUE );
  handle->setFrameStyle(NoFrame);
  handle->installEventFilter(this);
  handle->show();
  handle->raise();
  buttonDownOnHandle = FALSE;
  init();
}

void KMenuBar::show() {
  /* TODO: it seems to be necessary to have an extra show() call for
   * the QMenuBar, otherwise not all menu items are redrawn correctly
   * Recheck this after the next Qt-Beta (Beta 4?)
   */
  QFrame::show();
  menu->show();
}

int KMenuBar::idAt( int index )
{
  return menu->idAt( index );
}

int KMenuBar::heightForWidth ( int max_width ) const
{
  return menu->heightForWidth( max_width - 9);
}

void KMenuBar::resizeEvent (QResizeEvent *)
{
  if (position == Flat)
    return;
  int hwidth = 9;
  if (standalone_menubar)
    hwidth = 20;

  frame->setGeometry(hwidth , 0, width()-hwidth,
                     menu->heightForWidth(width()-hwidth));
  menu->resize(frame->width(), frame->height());
  handle->setGeometry(0,0,hwidth,height());
  if (height() != heightForWidth(width()))
  {
    resize(width(), heightForWidth(width()));
	// set fixed height so KTMainWindow geometry management will work
	setFixedHeight(heightForWidth(width()));
    return;
  }
}

void KMenuBar::ContextCallback( int )
{
  int i;
  i = context->exec();
  switch (i)
   {
    case CONTEXT_TOP:
      setMenuBarPos( Top );
      break;
    case CONTEXT_BOTTOM:
      setMenuBarPos( Bottom );
      break;
    case CONTEXT_FLOAT:
      if (position == Floating || position == FloatingSystem){
        setMenuBarPos (lastPosition);
      }
      else {
        setMenuBarPos( Floating );
	move(QCursor::pos());
	show();
      }
      break;
    case CONTEXT_FLAT:
        setFlat (position != Flat);
	break;
   }

  handle->repaint (false);
}

void KMenuBar::init()
{
  context = new QPopupMenu( 0, "context" );
  context->insertItem( i18n("Top"),  CONTEXT_TOP );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );

  position = Top;
  moving = TRUE;
  highlight = false;
  transparent = false;

  setLineWidth( 0 );

//  resize( Parent->width(), menu->height());
  // set fixed height so KTMainWindow geometry management will work
  setFixedHeight(menu->height());

  enableFloating (TRUE);
  connect (kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));
  slotReadConfig();

  mgr =0;

}


KMenuBar::~KMenuBar()
{
  if (!QApplication::closingDown())
    delete context;
}

void KMenuBar::mousePressEvent ( QMouseEvent *e )
{
  QApplication::sendEvent(menu, e);
}

void KMenuBar::slotReadConfig ()
{
  int _highlight;
  bool _transparent;

  KConfig *config = KGlobal::config();
  QString group = config->group();
  config->setGroup("Toolbar style");
  _highlight =config->readNumEntry("Highlighting", 1);
  _transparent = config->readBoolEntry("TransparentMoving", true);

  if (_highlight != highlight)
    highlight = _highlight;

  if (_transparent != transparent)
    transparent= _transparent;

  menu->setMouseTracking(false);
  if (position != Floating || position == FloatingSystem)
    menu->setFrameStyle(Panel | Raised);

  config->setGroup("KDE");//CT as Sven asked
  bool macmode = false;
  if (config->readEntry("macStyle") == "on") //CT as Sven asked
    macmode = true;

  if ( menuBarPos() != FloatingSystem && macmode) //was not and now is
  {
      standalone_menubar = TRUE;
      if (Parent->isVisible())
	  setMenuBarPos( FloatingSystem );
      else {
	  Parent->installEventFilter(this); // to show menubar
      }
  }
  else if (menuBarPos() == FloatingSystem && !macmode) //was and now is not
  {
    standalone_menubar = FALSE;
    setMenuBarPos (lastPosition);
  }
  //else if was and now is - nothing;
  //else if was not and now is not - nothing;
  config->setGroup(group);
}

void KMenuBar::slotHotSpot (int hs)
{
  if (mgr == 0)
    return;
  if (!transparent) // opaque
  {
    switch (hs)
    {
      case 0: //top
        setMenuBarPos(Top);
        break;

      case 1: //bottom
        setMenuBarPos(Bottom);
        break;

      case -1: // left all
        setMenuBarPos(Floating);
        break;
    }
    if (position != Floating)
    {
      QPoint p(Parent->mapToGlobal(pos())); // OH GOOOOODDDD!!!!!
      mgr->setGeometry(p.x(), p.y(), width(), height());
    }
    if (!isVisible())
      show();
  }
  else // transparent
  {
    switch (hs)
    {
      case 0: //top
        mgr->setGeometry(0);
        movePosition=Top;
        break;

      case 1: //bottom
        mgr->setGeometry(1);
        movePosition=Bottom;
        break;

      case -1: // left all
        mgr->setGeometry(mgr->mouseX(), mgr->mouseY(), width(), height());
        movePosition=Floating;
        break;
    }
  }
}


void KMenuBar::paintEvent(QPaintEvent *)
{
  //QApplication::sendEvent(menu, e);
  menu->repaint();
}

void KMenuBar::closeEvent (QCloseEvent *e)
{
  if (position == Floating)
   {
     position = lastPosition;
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
     emit moved (position);
     context->changeItem (i18n("Float"), CONTEXT_FLOAT);
     e->ignore();
     return;
   }
  e->accept();
}

void KMenuBar::leaveEvent (QEvent *e){
  QApplication::sendEvent(menu, e);
}


bool KMenuBar::eventFilter(QObject *ob, QEvent *ev){


  if (ob == Parent && ev->type() == QEvent::Show && standalone_menubar)
  {
    //bool aha = isVisible(); // did app enable show?
      setMenuBarPos(FloatingSystem);
      Parent->removeEventFilter(this); //One time only
      return false;
  }

  if (mgr)
    return true;


  if (ob == handle){
    if (ev->type() == QEvent::MouseButtonPress)
    {
      if (standalone_menubar)
      {
        //Dunno but krootwm does this; without it menu stays...
        XUngrabPointer(qt_xdisplay(), CurrentTime);
        XSync(qt_xdisplay(), False);

        QString x,y;
        x.setNum(pos().x());
        y.setNum(pos().y()+height());
        while (x.length()<4)
          x.prepend("0");
        while (y.length()<4)
          y.prepend("0");
        //if (((QMouseEvent*)ev)->button() == LeftButton)
          KWM::sendKWMCommand(QString("kpanel:go")+x+y);
        //else
        //  KWM::sendKWMCommand(QString("krootwm:go")+x+y);
        return false; //or true? Bah...
      }
      buttonDownOnHandle = TRUE;
      if ( moving && ((QMouseEvent*)ev)->button() == RightButton)
	{
	  buttonDownOnHandle = FALSE;
	  context->popup( handle->mapToGlobal(((QMouseEvent*)ev)->pos()), 0 );
	  ContextCallback(0);
	}

      // too confusing/buggy, imho. --nu
      //else if (((QMouseEvent*)ev)->button() == MidButton &&
      //         position != Floating)
      //  setFlat (position != Flat);

    }
    
    if (ev->type() == QEvent::MouseMove && buttonDownOnHandle ){

	if (position != Flat)
	    {
		//Move now
		QRect rr(Parent->geometry());
		int ox = rr.x();
		int oy = rr.y();
		int ow = rr.width();
		int oh = rr.height();
		
		int  fat = 25; //ness
		
		mgr = new KToolBoxManager(this, transparent);
		
		mgr->addHotSpot(ox, oy, ow, fat);           // top
		mgr->addHotSpot(ox, oy+oh-fat, ow, fat);    // bottom
		
		movePosition = position;
		connect (mgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
		if (transparent)
		    mgr->doMove(true, false, true);
		else
		    mgr->doMove(true, false, false);

		if (transparent)
		    {
			setMenuBarPos (movePosition);
			
			if (movePosition == Floating)
			    move (mgr->x(), mgr->y());
			if (!isVisible())
			    show();
		    }
		delete mgr;
		mgr=0;
		handle->repaint(false);
		//debug ("KMenuBar: moving done");
	    }
	buttonDownOnHandle = FALSE;
	return TRUE;
    }

    if (ev->type() == QEvent::MouseButtonRelease &&  handle->rect().contains(( (QMouseEvent*)ev)->pos() ))
      {
	  buttonDownOnHandle = FALSE;
	  if (mgr)
	      mgr->stop();
	  if ( position != Floating)
	      setFlat (position != Flat);
	  return TRUE;
      }

    if ((ev->type() == QEvent::Paint)||(ev->type() == QEvent::Enter)||(ev->type() == QEvent::Leave) ){

      QColorGroup g = QWidget::colorGroup();
      QPainter paint(handle);
      QBrush b = QWidget::backgroundColor();

      if (standalone_menubar)
      {
        if ( style() == WindowsStyle )
          qDrawWinButton( &paint, 0, 0, handle->width(), handle->height(),
                          g, false );
        else
          qDrawShadePanel( &paint, 0, 0, handle->width(), handle->height(),
                           g, false, 1, 0L );

        if (miniGo)
        {
          int dx = ( handle->width() - miniGo->width() ) / 2;
          int dy = ( handle->height() - miniGo->height() ) / 2;
          paint.drawPixmap( dx, dy, *miniGo);
        }

        return true;
      }

      int stipple_height;
      if (ev->type() == QEvent::Enter && highlight) // highlight? - sven
        b = colorGroup().highlight();  // this is much more logical then
                               // the hardwired value used before!!

      int h = handle->height();
      int w = handle->width();

      if(kapp->kstyle()){
          if(position == Flat)
              kapp->kstyle()->drawKBarHandle(&paint, 0, 0, w, 9,
                                             colorGroup(), false);
          else
              kapp->kstyle()->drawKBarHandle(&paint, 0, 0, 9, h,
                                             colorGroup(), true);

          return(true);
      }

      if (position == Flat)
      {
        qDrawShadePanel( &paint, 0, 0, w, 9, g , FALSE, 1, &b );
        paint.setPen( g.light() );
        stipple_height = 3;
        while ( stipple_height < w-4 ) {
          paint.drawPoint( stipple_height+1, 1);
          paint.drawPoint( stipple_height, 4 );
          stipple_height+=3;
        }
        paint.setPen( g.dark() );
        stipple_height = 4;
        while ( stipple_height < w-4 ) {
          paint.drawPoint( stipple_height+1, 2 );
          paint.drawPoint( stipple_height, 5);
          stipple_height+=3;
        }
        paint.drawLine( 1, 9, w, 9);
        return true;
      }
      qDrawShadePanel( &paint, 0, 0, 9, h, g , FALSE, 1, &b );

      paint.setPen( g.light() );
      stipple_height = 3;
      while ( stipple_height < h-4 ) {
        paint.drawPoint( 1, stipple_height+1);
        paint.drawPoint( 4, stipple_height);
        stipple_height+=3;
      }
      paint.setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < h-4 ) {
        paint.drawPoint( 2, stipple_height+1);
        paint.drawPoint( 5, stipple_height);
        stipple_height+=3;
      }
      return TRUE;
    }
  }
  return FALSE;
}

void KMenuBar::enableMoving(bool flag)
{
  moving = flag;
}

QSize 
KMenuBar::sizeHint() const
{
	if (position == Flat)
		return (QSize(30, 10));

	return (size());
}

void KMenuBar::setMenuBarPos(menuPosition mpos)
{
    if (position == FloatingSystem && standalone_menubar == true) {
	return; // Ignore positioning of Mac menubar
    }

  if (position != mpos)
   {
     if (mpos == Floating || mpos == FloatingSystem)
      {
	  lastPosition = position;
	  position = mpos;
	  oldX = x();
	  oldY = y();
	  if ( mpos == FloatingSystem && position == Floating)
	      lastPosition = Top;
	  else
	      oldWFlags = getWFlags();
	  QPoint p = mapToGlobal(QPoint(0,0));
	  parentOffset = pos();
	  hide();
	  emit moved (mpos);
	  recreate(0, 0,
		   p, FALSE);
	  XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
	  if (mpos == FloatingSystem)
	      KWM::setDecoration(winId(), KWM::noDecoration | KWM::standaloneMenuBar | KWM::noFocus);
	  else
	      KWM::setDecoration(winId(), KWM::tinyDecoration | KWM::noFocus);
	  KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
	  setCaption(""); // this triggers a qt bug
	  if (!title.isNull()){
	      setCaption(title);
	  } else {
	      QString s = Parent->caption();
	      s.append(" [menu]");
	      setCaption(s);
	  }
	  setFrameStyle( NoFrame);
      menu->setFrameStyle(Panel | Raised);
	  context->changeItem (i18n("UnFloat"), CONTEXT_FLOAT);
	  context->setItemEnabled (CONTEXT_FLAT, FALSE);
	
	

	if (mpos == FloatingSystem) {
	    QRect r =  KWM::windowRegion(KWM::currentDesktop());
	    setGeometry(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(), // check panel top
			heightForWidth(r.width()));
	    int dim = fontMetrics().height();
	    if (!miniGo)
		miniGo = new QPixmap(locate("data", "kpanel/pics/mini/go.xpm"));
	
	    QPixmap px(KWM::miniIcon(Parent->winId(), dim, dim));
	    if (!px.isNull())
		*miniGo = px;
	    show();
	}

//----------------------------------------------------------------------------
        // Repair repaired Accelerators (Eh, those Trolls...)
        QObjectList	*accelerators = queryList( "QAccel" );
        QObjectListIt it( *accelerators );
        QObject *obj;
        while ( (obj=it.current()) != 0 )
        {
          ++it;
          this->removeEventFilter(obj); //Man...
          disconnect( this, SIGNAL(destroyed()), obj, SLOT(tlwDestroyed()));

          Parent->installEventFilter(obj);
          connect( Parent, SIGNAL(destroyed()), obj, SLOT(tlwDestroyed()));
        }
//----------------------------------------------------------------------------

        return;
      }
     else if (position == Floating || position == FloatingSystem) // was floating
      {
        position = mpos;
        hide();
		setFrameStyle(NoFrame);
		menu->setFrameStyle(oldMenuFrameStyle);
        recreate(Parent, oldWFlags, QPoint(oldX, oldY), TRUE);
        emit moved (mpos);
        context->changeItem (i18n("Float"), CONTEXT_FLOAT);
        context->setItemEnabled (CONTEXT_FLAT, TRUE);
        menu->setFrameStyle(Panel | Raised);

        return;
      }
     else
      {
        if (mpos == Flat)
        {
          setFlat (true);
          return;
        }
        enableFloating (true);
        position = mpos;
        emit moved ( mpos );
        return;
      }
   }
}

void KMenuBar::enableFloating (bool arrrrrrgh)
{
  context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}

bool KMenuBar::enable( BarStatus stat )
{
  bool mystat = isVisible();
  if ( (stat == Toggle && mystat) || stat == Hide )
    hide();
  else
    show();
  emit moved (position); // force KTM::updateRects (David)
  return ( isVisible() == mystat );
}

/*******************************************************/

uint KMenuBar::count()
{
  return menu->count();
}

int KMenuBar::insertItem(const QString& text,
               const QObject *receiver, const char *member,
               int accel)
{
	return menu->insertItem(text, receiver, member, accel);
}

int KMenuBar::insertItem( const QString& text, int id, int index)
{
	return menu->insertItem(text, id, index);
}
int KMenuBar::insertItem( const QString& text, QPopupMenu *popup,
                          int id, int index)
{
	return menu->insertItem(text, popup, id, index);
}
/* Later - should be virtual and I can't do it right now - sven
int KMenuBar::insertItem (const QPixmap &pixmap, const QObject *receiver,
                          const char *member, int accel)
{
  return->menu->insertItem (pixmap, receiver, member, accel);
}
*/
void KMenuBar::insertSeparator(int index)
{
  menu->insertSeparator(index);
  /*
   * This show() seems to be necessary, otherwise the only the first
   * submenu is visible. It should probably be fixed somewhere else but this
   * solves the problem for the time beeing. CS
   */
  show();
}

void KMenuBar::removeItem( int id )
{
  menu->removeItem(id);
}
void KMenuBar::removeItemAt( int index )
{
  menu->removeItemAt(index);
}
void KMenuBar::clear()
{
  menu->clear();
}

int KMenuBar::accel( int id )
{
  return menu->accel(id);
}
void KMenuBar::setAccel( int key, int id )
{
  menu->setAccel(key, id);
}

QString KMenuBar::text( int id )
{
  return menu->text(id);
}

void KMenuBar::changeItem( const QString& text, int id )
{
  menu->changeItem(text, id);
}

void KMenuBar::setItemChecked(int id , bool flag)
{
  menu->setItemChecked(id , flag);
}

void KMenuBar::setItemEnabled(int id, bool flag)
{
  menu->setItemEnabled(id, flag);
}


void KMenuBar::slotActivated (int id)
{
  emit activated(id);
}

void KMenuBar::slotHighlighted (int id)
{
  emit highlighted (id);
}

void KMenuBar::setFlat (bool flag)
{

#define also

  if (position == Floating  || position == FloatingSystem)
    return;
  if ( flag == (position == Flat))
    also return;

  if (flag) //flat
  {
    context->changeItem (i18n("UnFlat"), CONTEXT_FLAT);
    lastPosition = position; // test float. I did and it works by miracle!?
    //debug ("Flat");
    position = Flat;
    resize(30, 10);
	// set fixed height so KTMainWindow geometry management will work
	setFixedHeight(10);
    handle->resize(30, 10);
    frame->move(100, 100); // move menubar out of sight
    enableFloating(false);
    emit moved(Flat); // KTM will call this->updateRects
  }
  else //unflat
  {
    context->changeItem (i18n("Flat"), CONTEXT_FLAT);
    //debug ("Unflat");
    setMenuBarPos(lastPosition);
	// set fixed height so KTMainWindow geometry management will work
	setFixedHeight(menu->height());
    enableFloating(true);
    emit moved (position); // KTM will call this->updateRects
  }
}

// From Qt's spacing
static const int motifBarFrame          = 2;    // menu bar frame width

static const int motifBarHMargin        = 2;    // menu bar hor margin to item
#ifndef KTHEMESTYLE_CONSTANTS
static const int motifBarVMargin        = 1;    // menu bar ver margin to item
static const int motifItemFrame         = 2;    // menu item frame width

static const int motifItemHMargin       = 5;    // menu item hor text marginstatic const int motifItemVMargin       = 4;    // menu item ver text margin                      
static const int motifItemVMargin       = 4;    // menu item ver text margin

#define KTHEMESTYLE_CONSTANTS
#endif

void KStyleMenuBarInternal::drawContents(QPainter *p)
{

    KStyle *stylePtr = kapp->kstyle();
    if(!stylePtr)
        QMenuBar::drawContents(p);
    else{
        int i, x, y, nlitems;
        bool popupshown;
        QFontMetrics fm = fontMetrics();
        stylePtr->drawKMenuBar(p, 0, 0, width(), height(), colorGroup(),
                               NULL);

        for(i=0, nlitems=0, x=2, y=2; i < (int)mitems->count(); ++i, ++nlitems)
        {
            int h=0;
            int w=0;
            QMenuItem *mi = mitems->at(i);
            if(mi->pixmap()){
                w = mi->pixmap()->width();
                h = mi->pixmap()->height();
            }
            else if(!mi->text().isEmpty()){
                QString s = mi->text();
                w = fm.boundingRect(s).width() + 2*motifItemHMargin;
                w -= s.contains('&')*fm.width('&');
                w += s.contains("&&")*fm.width('&');
                h = fm.height() + motifItemVMargin;
            }

            if (!mi->isSeparator()){
                if (x + w + motifBarFrame - width() > 0 && nlitems > 0 ){
                    nlitems = 0;
                    x = motifBarFrame + motifBarHMargin;
                    y += h + motifBarHMargin;
                }
            }
            popupshown = mi->popup() ? mi->popup()->isVisible() : false;
            stylePtr->drawKMenuItem(p, x, y, w, h, mi->isEnabled()  ?
                                    palette().normal() : palette().disabled(),
                                    i == actItem && (hasFocus() || mouseActive
                                                     || popupshown),
                                    mi, NULL);
            x += w;
        }
    }
}

void KStyleMenuBarInternal::enterEvent(QEvent *ev)
{
    mouseActive = true;
    QMenuBar::enterEvent(ev);
}

void KStyleMenuBarInternal::leaveEvent(QEvent *ev)
{
    mouseActive = false;
    QMenuBar::leaveEvent(ev);
}

#include "kmenubar.moc"

