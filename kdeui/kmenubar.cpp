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

// Moving with KToolBoxManager
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
    return menu->heightForWidth( max_width - 9);

int KMenuBar::heightForWidth ( int max_width ) const
{
  return menu->heightForWidth( max_width - 9);
     frame->setGeometry( 9, 0, width()-9, menu->heightForWidth(width()));
     menu->resize(frame->width(), frame->height());
     handle->setGeometry(0,0,9,height());
     if (height() != heightForWidth(width())) {
	 resize(width(), heightForWidth(width()));
	 return;
      }
  {
    resize(width(), heightForWidth(width()));
void KMenuBar::ContextCallback( int index )
  }
  int i = index; // to shut the -Wall up

void KMenuBar::ContextCallback( int )
{
  int i;
  i = context->exec();
  switch (i)
   {
    case CONTEXT_TOP:
      setMenuBarPos( Top );
      break;
      if (position == Floating){
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
//   connect( context, SIGNAL( activated( int ) ), this,
// 	   SLOT( ContextCallback( int ) ) );
  context->insertItem( i18n("Top"),  CONTEXT_TOP );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );

  position = Top;
  moving = TRUE;
  highlight = false;
  transparent = false;


  {
      KConfig* config = kapp->getConfig();
      KConfigGroupSaver saver(config, "Menubar");
      if (config->readEntry("position") == "TopOfScreen") {
	  int verticalOffset = config->readNumEntry("verticalOffset", 0);
	  setMenuBarPos(Floating);
	  QRect r =  KWM::getWindowRegion(KWM::currentDesktop());
	  setGeometry(r.x(),r.y()-3+verticalOffset, r.width()-6, heightForWidth(r.width()));
      }
  }
 

  resize( Parent->width(), menu->height());
  enableFloating (TRUE);
  
  slotReadConfig();

  mgr =0;

}
   // what is that?! we do not need to recreate before
  // destroying.... (Matthias)
//  if (position == Floating)
//      recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);



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
  config->setGroup(group);
  QString group = config->group();
  config->setGroup("Toolbar style");
  _highlight =config->readNumEntry("Highlighting", 1);
  _transparent = config->readBoolEntry("TransparentMoving", true);

  if (_highlight != highlight)
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
      Parent->removeEventFilter(this); //One time only
      return false;
  
  if (mgr)
    if (ev->type() == Event_MouseButtonPress)

      //pointerOffset = mapFromGlobal(handle->mapToGlobal(((QMouseEvent*)ev)->pos()));
      if ( moving && ((QMouseEvent*)ev)->button() != LeftButton)
        return false; //or true? Bah...
      buttonDownOnHandle = TRUE;
      if ( moving && ((QMouseEvent*)ev)->button() == RightButton)
	{
      else
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
      }


      QBrush b;
      if (ev->type() == Event_Enter && highlight) // highlight? - sven
        b = kapp->selectColor; // this is much more logical then
      // the hardwired value used before!!
      else
        b = QWidget::backgroundColor();
      if (ev->type() == QEvent::Enter && highlight) // highlight? - sven
        b = colorGroup().highlight();  // this is much more logical then
      

          return(true);
	  }
                         g , FALSE, 1, &b );
          return true;
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
        qDrawPlainRect ( &paint, 0, 0, 9, handle->height(),
         return true;

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
    if (position == FloatingSystem && standalone_menubar == true) {
	return; // Ignore positioning of Mac menubar
     if (mpos == Floating)

        lastPosition = position;
        position = mpos;
        oldX = x();
        oldY = y();
        oldWFlags = getWFlags();
	QPoint p = mapToGlobal(QPoint(0,0));
	parentOffset = pos();
        hide();
        recreate(0, 0,
                 p, FALSE);
 	XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
	KWM::setDecoration(winId(), 2);
	KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
	setCaption(""); // this triggers a qt bug
	if (title){
	  setCaption(title);
	}
	else {
	  QString s = Parent->caption();
	  s.append(" [menu]");
	  setCaption(s);
	}
	setFrameStyle( NoFrame);
	menu->setFrameStyle( NoFrame) ;
        context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
		*miniGo = px;
          connect( Parent, SIGNAL(destroyed()), obj, SLOT(tlwDestroyed()));
        }
     else if (position == Floating) // was floating

        return;
      }
     else if (position == Floating || position == FloatingSystem) // was floating
      {
        position = mpos;
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
	menu->setFrameStyle(oldMenuFrameStyle);
//          menu->setMouseTracking(true);
          menu->setFrameStyle(NoFrame);
        }

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

    emit moved (position); // KTM will call this->updateRects
  }
}
#include "kmenubar.moc"
