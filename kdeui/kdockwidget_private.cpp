/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

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
#include "kdockwidget.h"
#include "kdockwidget_private.h"

#include <qcursor.h>
#include <qapplication.h>
#include <qwindowdefs.h>
#include <qtimer.h>
#include <qrect.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpainter.h>

KDockMoveManager::KDockMoveManager(QWidget *_widget) : QObject ()
{
  XGCValues gv;

  working=false;
  noLast=true;
  widget = _widget;

  scr = qt_xscreen();
  root = qt_xrootwin();

  gv.function = GXxor;
  gv.line_width = 0;
  gv.foreground = WhitePixel(qt_xdisplay(), scr)^BlackPixel(qt_xdisplay(), scr);
  gv.subwindow_mode = IncludeInferiors;
  long mask = GCForeground | GCFunction | GCLineWidth | GCSubwindowMode;
  rootgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);

  timer = new QTimer(this);
}

KDockMoveManager::~KDockMoveManager()
{
  stop();
}

void KDockMoveManager::doMove()
{
  if (working) return;

  Window wroot, wchild;
  int trash;
  
  working=true;

  QRect rr = QRect( widget->mapToGlobal(QPoint(0,0)), widget->size() );
  QPoint p(rr.topLeft());

  offX = QCursor::pos().x() - p.x();
  offY = QCursor::pos().y() - p.y();
  
  xp = p.x();
  yp = p.y();
  w = rr.width();
  h = rr.height();

  orig_x = p.x();
  orig_y = p.y();
  orig_w = w;
  orig_h = h;

  unsigned int active_button;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &sx, &sy, &trash, &trash, &active_button);
  
  rx = sx;
  ry = sy;
  
  xp=sx-offX;
  yp=sy-offY;

  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
  connect (timer, SIGNAL(timeout()), this, SLOT (doMoveInternal()));

  drawRectangle(xp, yp, w, h);

  pauseMove = false;
  timer->start(0);
}

void KDockMoveManager::doMoveInternal()
{
  if ( pauseMove || !working ) return;

  Window wroot, wchild;
  int trash;
  unsigned int buttons;

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &rx, &ry, &trash, &trash, &buttons );
  
  if (rx == sx && ry == sy) return;

  sx=rx;
  sy=ry;

  xp=rx-offX;
  yp=ry-offY;

  deleteLastRectangle();
  drawRectangle(xp, yp, w, h);

  XFlush(qt_xdisplay());
  XSync(qt_xdisplay(), False);
}

void KDockMoveManager::stop ()
{
  if (!working) return;
  
  timer->stop();
  disconnect (timer, SIGNAL(timeout()));
  QApplication::restoreOverrideCursor();

  deleteLastRectangle();
  XFlush(qt_xdisplay());
  QApplication::restoreOverrideCursor();

  QPoint p(xp, yp);
  if (widget->parent() != 0) p = widget->parentWidget()->mapFromGlobal(p);

  working = false;
}

void KDockMoveManager::setGeometry (int _x, int _y, int _w, int _h)
{
  if (!working) return;
  xp=_x;
  yp=_y;
  w=_w;
  h=_h;
  deleteLastRectangle();
  drawRectangle( _x, _y, _w, _h);
}

void KDockMoveManager::drawRectangle(int _x, int _y, int _w, int _h)
{
  ox = _x;
  oy = _y;
  ow = _w;
  oh = _h;

  XDrawRectangle(qt_xdisplay(), root, rootgc, _x, _y, _w, _h);
  if (_w > 2)
    _w -= 2;
  if (_h > 2)
    _h -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, _x+1, _y+1, _w, _h);
  if (_w > 2)
    _w -= 2;
  if (_h > 2)
    _h -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, _x+2, _y+2, _w, _h);
  
  noLast = false;
}

void KDockMoveManager::deleteLastRectangle()
{
  if (noLast) return;
  
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox, oy, ow, oh);
  if (ow > 2)
    ow -= 2;
  if (oh > 2)
    oh -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox+1, oy+1, ow, oh);
  if (ow > 2)
    ow -= 2;
  if (oh > 2)
    oh -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox+2, oy+2, ow, oh);

  noLast = true;
}

KDockSplitter::KDockSplitter(QWidget *parent, const char *name, Orientation orient, int pos)
: QWidget(parent, name)
{
  divider = 0L;
  child0 = 0L;
  child1 = 0L;
  orientation = orient;
  setSeparatorPos( pos, false );
  initialised = false;
}

void KDockSplitter::activate(QWidget *c0, QWidget *c1)
{
  if ( c0 ) child0 = c0;
  if ( c1 ) child1 = c1;

  setupMinMaxSize();

  if (divider) delete divider;
  divider = new QFrame(this, "pannerdivider");
  divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
  divider->setLineWidth(1);
  divider->raise();

  if (orientation == Horizontal)
    divider->setCursor(QCursor(sizeVerCursor));
  else
    divider->setCursor(QCursor(sizeHorCursor));

  divider->installEventFilter(this);

  initialised= true;

  updateName();

  divider->show();
  resizeEvent(0);
}

void KDockSplitter::setupMinMaxSize()
{
  // Set the minimum and maximum sizes
  int minx, maxx, miny, maxy;
  if (orientation == Horizontal) {
    miny = child0->minimumSize().height() + child1->minimumSize().height()+4;
    maxy = child0->maximumSize().height() + child1->maximumSize().height()+4;
    minx = (child0->minimumSize().width() > child1->minimumSize().width()) ? child0->minimumSize().width() : child1->minimumSize().width();
    maxx = (child0->maximumSize().width() > child1->maximumSize().width()) ? child0->maximumSize().width() : child1->maximumSize().width();

    miny = (miny > 4) ? miny : 4;
    maxy = (maxy < 2000) ? maxy : 2000;
    minx = (minx > 2) ? minx : 2;
    maxx = (maxx < 2000) ? maxx : 2000;
  } else {
    minx = child0->minimumSize().width() + child1->minimumSize().width()+4;
    maxx = child0->maximumSize().width() + child1->maximumSize().width()+4;
    miny = (child0->minimumSize().height() > child1->minimumSize().height()) ? child0->minimumSize().height() : child1->minimumSize().height();
    maxy = (child0->maximumSize().height() > child1->maximumSize().height()) ? child0->maximumSize().height() : child1->maximumSize().height();

    minx = (minx > 4) ? minx : 4;
    maxx = (maxx < 2000) ? maxx : 2000;
    miny = (miny > 2) ? miny : 2;
    maxy = (maxy < 2000) ? maxy : 2000;
  }
	setMinimumSize(minx, miny);
	setMaximumSize(maxx, maxy);
}

void KDockSplitter::deactivate()
{
  if (divider) delete divider;
  divider = 0L;
  initialised= false;
}

void KDockSplitter::setSeparatorPos(int pos, bool do_resize)
{
  xpos = pos;
  if (do_resize)
    resizeEvent(0);
}

int KDockSplitter::separatorPos()
{
  return xpos;
}

void KDockSplitter::resizeEvent(QResizeEvent*)
{
  if (initialised){
    int position = checkValue( (orientation == Vertical ? width() : height()) * xpos/100 );
    if (orientation == Horizontal){
      child0->setGeometry(0, 0, width(), position);
      child1->setGeometry(0, position+4, width(), height()-position-4);
      divider->setGeometry(0, position, width(), 4);
    } else {
      child0->setGeometry(0, 0, position, height());
      child1->setGeometry(position+4, 0, width()-position-4, height());
      divider->setGeometry(position, 0, 4, height());
    }
  }
}

int KDockSplitter::checkValue( int position )
{
  if (initialised){
    if (orientation == Vertical){
      if (position < (child0->minimumSize().width()))
        position = child0->minimumSize().width();
      if ((width()-4-position) < (child1->minimumSize().width()))
        position = width() - (child1->minimumSize().width()) -4;
    } else {
      if (position < (child0->minimumSize().height()))
        position = (child0->minimumSize().height());
      if ((height()-4-position) < (child1->minimumSize().height()))
        position = height() - (child1->minimumSize().height()) -4;
    }
  }
   
  if (position < 0) position = 0;

  if ((orientation == Vertical) && (position > width()))
    position = width();
  if ((orientation == Horizontal) && (position > height()))
    position = height();

  return position;
}

bool KDockSplitter::eventFilter(QObject *o, QEvent *e)
{
  QMouseEvent *mev;
  bool handled = false;

  switch (e->type()) {
    case QEvent::MouseMove:
      mev= (QMouseEvent*)e;
      child0->setUpdatesEnabled(false);
      child1->setUpdatesEnabled(false);
      if (orientation == Horizontal) {
        int position = checkValue( divider->mapToParent(mev->pos()).y() );
	divider->move( 0, position );
      } else {
        int position = checkValue( mapFromGlobal(QCursor::pos()).x() );
	divider->move( position, 0 );
      }
      handled= true;
      break;
    case QEvent::MouseButtonRelease:
      child0->setUpdatesEnabled(true);
      child1->setUpdatesEnabled(true);
      mev= (QMouseEvent*)e;
      if (orientation == Horizontal){
        xpos = 100* checkValue( divider->mapToParent(mev->pos()).y() ) / height();
        resizeEvent(0);
        divider->repaint(true);
      } else {
        xpos = 100* checkValue( divider->mapToParent(mev->pos()).x() ) / width();
        resizeEvent(0);
        divider->repaint(true);
      }
      handled= true;
      break;
    default:
      break;
  }
  return (handled) ? true : QWidget::eventFilter( o, e );
}

bool KDockSplitter::event( QEvent* e )
{
  if ( e->type() == QEvent::LayoutHint ){
    // change children min/max size
    setupMinMaxSize();
    setSeparatorPos(xpos);
  }
  return QWidget::event(e);
}

QWidget* KDockSplitter::getAnother( QWidget* w )
{
  return ( w == child0 ) ? child1 : child0;
}

void KDockSplitter::updateName()
{
  if ( !initialised ) return;

  QString new_name = QString( child0->name() ) + "," + child1->name();
  parentWidget()->setName( new_name.latin1() );
  parentWidget()->setCaption( child0->caption() + "," + child1->caption() );
  parentWidget()->repaint( false );

  ((KDockWidget*)parentWidget())->firstName = child0->name();
  ((KDockWidget*)parentWidget())->lastName = child1->name();
  ((KDockWidget*)parentWidget())->splitterOrientation = orientation;

  QWidget* p = parentWidget()->parentWidget();
  if ( p != 0L && p->inherits("KDockSplitter" ) )
    ((KDockSplitter*)p)->updateName();
}

/*************************************************************************/
KDockButton_Private::KDockButton_Private( QWidget *parent, const char * name )
:QPushButton( parent, name )
{
  moveMouse = false;
  setFocusPolicy( NoFocus );
}

KDockButton_Private::~KDockButton_Private()
{
}

void KDockButton_Private::drawButton( QPainter* p )
{
  p->fillRect( 0,0, width(), height(), QBrush(colorGroup().brush(QColorGroup::Background)) );
  p->drawPixmap( (width() - pixmap()->width()) / 2, (height() - pixmap()->height()) / 2, *pixmap() );
  if ( moveMouse && !isDown() ){
    p->setPen( white );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( colorGroup().dark() );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
  if ( isOn() || isDown() ){
    p->setPen( colorGroup().dark() );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( white );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
}

void KDockButton_Private::enterEvent( QEvent * )
{
  moveMouse = true;
  repaint();
}

void KDockButton_Private::leaveEvent( QEvent * )
{
  moveMouse = false;
  repaint();
}

/*************************************************************************/
KDockWidgetPrivate::KDockWidgetPrivate()
  : QObject()
  ,index(-1)
  ,splitPosInPercent(50)
{
}

KDockWidgetPrivate::~KDockWidgetPrivate()
{
}

#ifndef NO_KDE2 // for Qt-only projects, because tmake doesn't take this name
#include "kdockwidget_private.moc"
#endif
