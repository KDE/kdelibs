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
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qstring.h>
#include <qframe.h>

#include "qobjectlist.h"

#include "ktopwidget.h"

#include "kmenubar.moc"

#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
#include <ktoolboxmgr.h>
#include <kwm.h>

#define CONTEXT_TOP 1
#define CONTEXT_BOTTOM 2
#define CONTEXT_FLOAT 3

// $Id$
// $Log$
//
// Revision 1.59  1998/12/16 01:40:22  ettrich
// yet another bugfix (don't worry, didn't effect any yet-existing program)
//
// Revision 1.58  1998/12/16 01:27:12  ettrich
// fixed slightly broken macstyle removal
//
// Revision 1.57  1998/12/16 00:01:38  ettrich
// small fix for menubars
//
// Revision 1.56  1998/12/09 14:11:58  radej
// sven: Bad geometry (vanishing last item) fixed. Debug output commented out.
//
// Revision 1.55  1998/12/07 21:47:26  tibirna
// Endorsed by: Sven Radej <sven@kde.org>
//
// CT: 1)         ~/.kderc#[Menubar]\TopOfScreen=yes|no
//        becomes ~/.kderc#[KDE]\macStyle=on|off
//     2) GUI config for the macStyle
//     3) krootwm supports "hot" switching of macStyle (still small problems,
// 	a restart of krootwm fixes them)
//
// Revision 1.54  1998/12/02 16:22:21  radej
// sven: Flat menubar looks like flat toolbars in Motif style
//
// Revision 1.53  1998/11/23 18:32:04  radej
// sven: MACmode: runtime change works,
//
// Revision 1.52  1998/11/23 15:34:04  radej
// sven: Nicer sysmenu button
//
// Revision 1.51  1998/11/23 11:57:17  radej
// sven: Mac: Force show, if on top y= -2, take icon in showEvent.
//       Still doesn't gets hidden when it's window loses focus.
//       ToolBar does.
//
// Revision 1.50  1998/11/23 09:57:49  ettrich
// small improvement, hope Sven likes it
//
// Revision 1.49  1998/11/23 00:12:27  radej
// sven: MACMenu: doesn't show if app doesn't want it to (kvt) + icon size fix
//
// Revision 1.48  1998/11/22 13:35:46  radej
// sven: IMPROVED Mac menubar: Accelerators, SystemMenu, look...
//
// Revision 1.47  1998/11/21 20:28:39  ettrich
// yet another small fix
//
// Revision 1.46  1998/11/21 20:25:41  ettrich
// small fix
//
// Revision 1.45  1998/11/18 01:00:03  radej
// sven: set*BarPos(Flat) works now (I hope)
//
// Revision 1.44  1998/11/11 14:32:10  radej
// sven: *Bars can be made flat by MMB (Like in Netscape, but this works)
//
// Revision 1.43  1998/11/10 14:12:47  radej
// sven: windows-style handle smaller
//
// Revision 1.42  1998/11/07 22:39:13  radej
// sven: Fixed QAccel too (unrepaired what repairEventFilter messed)
//
// Revision 1.41  1998/11/07 17:13:57  radej
// sven: Fixed KAccel, for now
//
// Revision 1.40  1998/11/06 17:59:55  radej
// sven: fixed dynamic style change (QMenuBar is buggy)
//
// Revision 1.39  1998/11/06 16:48:21  radej
// sven: nicer docking, some bugfixes
//
// Revision 1.38  1998/11/06 15:08:49  radej
// sven: finished handles. Comments?
//
// Revision 1.37  1998/11/06 12:55:53  radej
// sven: handle changed again (still not perfect)
//
// Revision 1.36  1998/11/05 18:23:32  radej
// sven: new look for *Bar handles (unfinished)
//
// Revision 1.35  1998/10/05 15:09:52  kulow
// purify (and me) likes initialized members, so I choose one (like the compiler
// would do :)
//
// Revision 1.34  1998/09/07 18:44:29  ettrich
// Matthias: preparation for new features
//
// Revision 1.33  1998/09/07 13:46:58  ettrich
// Matthias: removed some debug output...
//
// Revision 1.32  1998/09/07 13:45:19  ettrich
// Matthias: removed old qt-1.2 compatibility hack
//
// Revision 1.31  1998/09/01 20:22:03  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.30  1998/08/31 00:52:11  torben
// Torben: One new function and made others virtual
// => binary incompatible. Sorry. Please use virtual whenever it is
// not a performance problem.
//
// Revision 1.29  1998/07/23 09:43:38  radej
// sven: Improved look under diff styles. Winlook is beetter now.
//
// Revision 1.28  1998/06/18 08:58:14  radej
// sven: removed debug output
//
// Revision 1.27  1998/05/28 21:49:39  kulow
// I thought, a little sync between my different acinclude.m4.ins. I've done much
// to much to know, which version is where ;)
//
// Revision 1.26  1998/05/19 14:10:23  radej
// Bugfixes: Unhighlighting a handle and catching the fast click

// Revision 1.25  1998/05/07 23:13:09  radej
// Moving with KToolBoxManager
//

_menuBar::_menuBar (QWidget *parent, const char *name)
  : QMenuBar (parent, name)
{
   // Menubar is raised in motif style
   //setFrameStyle(NoFrame);
	
   //MD (17-9-97)
  setLineWidth(1);
 }

_menuBar::~_menuBar ()
 {
 }

static bool standalone_menubar = FALSE;

static QPixmap* miniGo = 0;

/*************************************************************/

  title = 0;
KMenuBar::KMenuBar(QWidget *parent, const char *name)
  
{
  Parent = parent;        // our father
  oldWFlags = getWFlags();
  menu = new _menuBar (frame);
  frame = new QFrame (this);
  frame->setFrameStyle(NoFrame);
  menu = new QMenuBar (frame);
  menu->setLineWidth( 1 );
  oldMenuFrameStyle = menu->frameStyle();

  connect (menu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  connect (menu, SIGNAL(highlighted(int)), this, SLOT(slotHighlighted(int)));
  handle = new QFrame (this);
  handle->setMouseTracking( TRUE );
  handle->installEventFilter(this);
  handle->show();
  handle->raise();
  buttonDownOnHandle = FALSE;
  init();
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
      break;
    case CONTEXT_FLAT:
        setFlat (position != Flat);
	break;
   }

  handle->repaint (false);
}
  context->insertItem( klocale->translate("Top"),  CONTEXT_TOP );
  context->insertItem( klocale->translate("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( klocale->translate("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Top"),  CONTEXT_TOP );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );

  position = Top;
  moving = TRUE;
  highlight = false;
  transparent = false;

  setLineWidth( 0 );

  resize( Parent->width(), menu->height());
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

  KConfig *config = kapp->getConfig();
  QString group = config->group();
  config->setGroup("Toolbar style");
  _highlight =config->readNumEntry("Highlighting", 1);
  _transparent = config->readBoolEntry("TransparentMoving", true);

  if (_highlight != highlight)
    highlight = _highlight;

  if (_transparent != transparent)
    menu->setStyle(style()); //Uh!

  if (style() == MotifStyle)
  {
    // menu->setStyle(style()); TODO: port to real Styles
    menu->setMouseTracking(false);
    if (position != Floating || position == FloatingSystem)
    menu->setStyle(style()); //Uh!
  }
  else
  {
    // menu->setStyle(style()); TODO: port to real Styles
    menu->setMouseTracking(true);
    if (position != Floating && position != FloatingSystem)
      menu->setFrameStyle(NoFrame);
  }

  config->setGroup("KDE");//CT as Sven asked
  if (!standalone_menubar && macmode) //was not and now is
  if (config->readEntry("macStyle") == "on") //CT as Sven asked
    macmode = true;

  if ( menuBarPos() != FloatingSystem && macmode) //was not and now is
  {
      standalone_menubar = TRUE;
      if (Parent->isVisible())
	  setMenuBarPos( FloatingSystem );
  else if (standalone_menubar && !macmode) //was and now is not
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
     context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
   {
     position = lastPosition;
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
     context->changeItem (i18n("Float"), CONTEXT_FLOAT);
     emit moved (position);
     e->ignore();
     return;
   }
  e->accept();
}

void KMenuBar::leaveEvent (QEvent *e){
  QApplication::sendEvent(menu, e);
}

  if (ob == Parent && ev->type() == Event_Show && standalone_menubar)
bool KMenuBar::eventFilter(QObject *ob, QEvent *ev){


  if (ob == Parent && ev->type() == QEvent::Show && standalone_menubar)
  {
    //bool aha = isVisible(); // did app enable show?
      setMenuBarPos(FloatingSystem);
      Parent->removeEventFilter(this); //One time only
      return false;
  }

  if (mgr)
    if (ev->type() == Event_MouseButtonPress)


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

        //  KWM::sendKWMCommand(QString("krootwm:go")+x+y);
        return false; //or true? Bah...
      buttonDownOnHandle = TRUE;
      if ( moving && ((QMouseEvent*)ev)->button() == RightButton)
	{
	  buttonDownOnHandle = FALSE;
	  context->popup( handle->mapToGlobal(((QMouseEvent*)ev)->pos()), 0 );
	  ContextCallback(0);
      else if (position != Flat)
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
        {
          mgr->doMove(true, false, false);
        }
	}
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
      return TRUE;
		//debug ("KMenuBar: moving done");
	    }
    if (ev->type() == Event_MouseButtonRelease)
	return TRUE;
	if (mgr)
	  mgr->stop();
	return TRUE;
	      mgr->stop();
	  if ( position != Floating)
    if ((ev->type() == Event_Paint)||(ev->type() == Event_Enter)||(ev->type() == Event_Leave) ){
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

      if (ev->type() == Event_Enter && highlight) // highlight? - sven
        b = kapp->selectColor; // this is much more logical then

      //else
      //  b = QWidget::backgroundColor();
      if (ev->type() == QEvent::Enter && highlight) // highlight? - sven
        b = colorGroup().highlight();  // this is much more logical then
                                             colorGroup(), true);

          return(true);
      }

      if (style() == MotifStyle) //Motif style handle
      {
        if (position == Flat)
        {
          qDrawShadePanel( &paint, 0, 0, w, 9,
			   g , FALSE, 1, &b );
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
        qDrawShadePanel( &paint, 0, 0, 9, h,
                           g , FALSE, 1, &b );


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
      else //windows style handle
      {
        if (position == Flat)
        {
          qDrawPlainRect ( &paint, 0, 0, handle->width(), 9,
                           g.mid(), 0, &b);

          h = 16;
          paint.setClipRect(2, 0, w-4, 6);
          paint.setPen( g.light() );
          int a = 0-h;
          while (a <= w+h)
          {
            paint.drawLine(w-a, h, w-a+h, 0);
            paint.drawLine(w-a+1, h, w-a+1+h, 0);
            a +=6;
          }
          a = 0-h;
          paint.setPen( g.dark() );
          while (a <= w+h)
          {
            paint.drawLine(w-a+2, h, w-a+2+h, 0);
            paint.drawLine(w-a+3, h, w-a+3+h, 0);
            a +=6;
          }
         return true;
        }

        qDrawPlainRect ( &paint, 0, 0, 6, handle->height(),
                         g.mid(), 0, &b);
        w=6;
        paint.setClipRect(0, 2, w, h-4);

        paint.setPen( g.light() );
        int a=0-w;
        while (a <= h+5)
        {
          paint.drawLine(0, h-a, h, 0-a);
          paint.drawLine(0, h-a+1, h, 0-a+1);
          a +=6;
        }
        a=0-w;
        paint.setPen( g.dark() );
        while (a <= h+5)
        {
          paint.drawLine(0, h-a+2, h, 0-a+2);
          paint.drawLine(0, h-a+3, h, 0-a+3);
          a +=6;
        }
        return TRUE;
      }
    }
  }
  return FALSE;
}

void KMenuBar::enableMoving(bool flag)
{
  moving = flag;
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
	  if ( mpos == FloatingSystem) 
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
	  recreate(0, 0,
		   p, FALSE);
	  XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
	  if (mpos == FloatingSystem)
	      KWM::setDecoration(winId(), KWM::noDecoration | KWM::standaloneMenuBar | KWM::noFocus);
	  if (title){
	      KWM::setDecoration(winId(), KWM::tinyDecoration | KWM::noFocus);
	  }
	  else {
	  setCaption(""); // this triggers a qt bug
	  if (!title.isNull()){
	      setCaption(title);
	  } else {
	      QString s = Parent->caption();
	      s.append(" [menu]");
	      setCaption(s);
	  }
	  setFrameStyle( NoFrame);
	  if (mpos == FloatingSystem)
	      {
		  if (style() == MotifStyle)
		      menu->setFrameStyle(Panel | Raised);
		  else
	  context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
	  else
	      menu->setFrameStyle( NoFrame) ;
	  context->changeItem (i18n("UnFloat"), CONTEXT_FLOAT);
	  context->setItemEnabled (CONTEXT_FLAT, FALSE);
	
	

	if (mpos == FloatingSystem) {
	    QRect r =  KWM::getWindowRegion(KWM::currentDesktop());
	    setGeometry(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(), // check panel top
			heightForWidth(r.width()));
	    int dim = fontMetrics().height();
	    if (!miniGo)
		miniGo = new QPixmap(kapp->kde_datadir() + "/kpanel/pics/mini/go.xpm");
	
	    QPixmap px(KWM::miniIcon(Parent->winId(), dim, dim));
	    if (!px.isNull())
		*miniGo = px;
	    show();
	}

        emit moved (mpos);
//        if (style() == MotifStyle)
//          menu->setMouseTracking(false);
//        else
//          menu->setMouseTracking(true);
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
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
	menu->setFrameStyle(oldMenuFrameStyle);
        recreate(Parent, oldWFlags, QPoint(oldX, oldY), TRUE);
        context->changeItem (i18n("Float"), CONTEXT_FLOAT);
        context->setItemEnabled (CONTEXT_FLAT, TRUE);
        emit moved (mpos);
        if (style() == MotifStyle)
        {
//          menu->setMouseTracking(false);
          menu->setFrameStyle(Panel | Raised);
        }
        else
        {
//          menu->setMouseTracking(true);
          menu->setFrameStyle(NoFrame);
        }

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

/*******************************************************/

uint KMenuBar::count()
int KMenuBar::insertItem(const char *text,
  return menu->count();
}

int KMenuBar::insertItem(const QString& text,
               const QObject *receiver, const char *member,
               int accel)
int KMenuBar::insertItem( const char *text, int id, int index)
  return menu->insertItem(text, receiver, member, accel);
}

int KMenuBar::insertItem( const char *text, QPopupMenu *popup,
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
const char *KMenuBar::text( int id )
  menu->setAccel(key, id);
}

QString KMenuBar::text( int id )
void KMenuBar::changeItem( const char *text, int id )
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
  if (flag && ((position == Floating  || position == FloatingSystem) && position == Flat))

  if (!flag && (position != Flat))

  if (position == Floating  || position == FloatingSystem)

    return;
  if ( flag == (position == Flat))

  if (flag) //flat
  {
    context->changeItem (i18n("UnFlat"), CONTEXT_FLAT);
    lastPosition = position; // test float. I did and it works by miracle!?
    //debug ("Flat");
    position = Flat;
    resize(30, 10);
    handle->resize(30, 10);
    frame->move(100, 100); // move menubar out of sight
    enableFloating(false);
  }
  else //unflat
  {
    context->changeItem (i18n("Flat"), CONTEXT_FLAT);
    //debug ("Unflat");
    setMenuBarPos(lastPosition);

    emit moved (position); // KTM will call this->updateRects
  }
}
#include "kmenubar.moc"
