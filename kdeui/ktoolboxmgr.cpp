/*
    This file is part of the KDE libraries
    Copyright (C) 1998 Sven Radej <radej@kde.org>
              
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


#include <qcursor.h>
#include <qobject.h>
#include <qapplication.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ktoolboxmgr.h>
#include <kwm.h>

 // $Id$
 // $Log$
 // Revision 1.12  1998/09/01 20:22:25  kulow
 // I renamed all old qt header files to the new versions. I think, this looks
 // nicer (and gives the change in configure a sense :)
 //
 // Revision 1.11  1998/08/10 13:33:54  radej
 // sven: Added X-only and Y-only resizing.
 //
 // Revision 1.10  1998/07/29 12:48:30  ssk
 // Removed more warnings, possible portability problems and ANSI violations.
 //
 // Revision 1.9  1998/06/18 09:15:01  radej
 // sven: made transparent resizer's lines thicker  (3.pts)
 //
 // Revision 1.8  1998/06/18 08:54:31  radej
 // sven: removed debug outpot before 1.0
 //
 // Revision 1.7  1998/05/07 16:50:06  radej
 // I caught that mouseRelease without blocking (Yeah!)
 //
 // Revision 1.6  1998/05/05 16:52:45  radej
 // Bugs...
 //
 // Revision 1.5  1998/05/05 10:03:50  radej
 // Improvement for opaque moving (steeling releaseEvent from widget)
 //
 // Revision 1.4  1998/05/04 16:39:12  radej
 // No more server locking + opaque sizing/moving
 //
 // Revision 1.3  1998/05/03 10:56:23  radej
 // Fixing the locked server bug; all grab ungrab is in one function now.
 // Screen is not locked any more across different QTimer steps
 //
 // Revision 1.2  1998/05/02 18:30:13  radej
 // Switched to KWM::geometry instead x() and y()
 //
 // Revision 1.1  1998/04/28 09:16:18  radej
 // Initial checkin
 //


KToolBoxManager::KToolBoxManager (QWidget *_widget, bool _transparent) : QObject ()
{
  XGCValues gv;

  working=false;
  noLast=true;
  widget = _widget;
  geometryChanged = false;
  mode = Nothing;
  
  transparent = _transparent;
  scr = qt_xscreen();
  root = qt_xrootwin();

  gv.function = GXxor;
  gv.line_width = 0;
  gv.foreground = WhitePixel(qt_xdisplay(), scr)^BlackPixel(qt_xdisplay(), scr);
  gv.subwindow_mode = IncludeInferiors;
  long mask = GCForeground | GCFunction | GCLineWidth | GCSubwindowMode;
  rootgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);

  hotspots.setAutoDelete(true);
  
  //driver for mover and resizer
  timer = new QTimer(this);

  yOnly = false;
  xOnly = false;

}

KToolBoxManager::~KToolBoxManager ()
{
  stop();
}

int KToolBoxManager::addHotSpot(int rx, int ry, int rw, int rh)
{
  QRect *r = new QRect (rx, ry, rw, rh);
  hotspots.append(r);
  return hotspots.at();
}

int KToolBoxManager::addHotSpot (const QRect& _r, bool mapToGlobal)
{
  QRect *r = new QRect (_r.x(), _r.y(), _r.width(), _r.height());
  if (mapToGlobal)
    r->moveTopLeft(widget->parentWidget()->mapToGlobal(r->topLeft()));
  hotspots.append(r);
  return hotspots.at();
}


void KToolBoxManager::removeHotSpot(int index)
{
  hotspots.remove (index);
}

void KToolBoxManager::doMove (bool hot_static, bool _dynamic, bool dontmove)
{
  if (working)
    return;

  Window wroot, wchild;
  int trash;
  
  //debug("Doing move...");

  working=true;
  mode = Moving;
  dynamic = _dynamic;
  dontmoveres=dontmove;
  hotspot_static = hot_static;
  
  QRect rr = KWM::geometry(widget->winId(), true);
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

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &sx, &sy, &trash, &trash, &active_button);
  
  rx = sx;
  ry = sy;
  
  xp=sx-offX;
  yp=sy-offY;

  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
  
  connect (timer, SIGNAL(timeout()), this, SLOT (doMoveInternal()));
  if (transparent)
    drawRectangle(xp, yp, w, h);

  timer->start(0);
  qApp->enter_loop();
}

void KToolBoxManager::doMoveInternal()
{
  bool onspot=false;
  bool changed=false;
  Window wroot, wchild;
  int trash;
  unsigned int buttons;

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &rx, &ry, &trash, &trash, &buttons );
  
  if (buttons != active_button)
  {
    /*bool b = */ XCheckMaskEvent(qt_xdisplay(), ButtonReleaseMask, &ev);
    // if (b) debug ("KTBmgr: Got and removed mouseRelease");
    stop();  
    return;
  }
  
  if (rx == sx && ry == sy)
    return;
  /*
  if (geometryChanged)
  {
    offX = rx - xp;
    offY = ry - yp;
    geometryChanged = false;
  }
  */
  sx=rx;
  sy=ry;

  xp=rx-offX;
  yp=ry-offY;

  for (QRect *hsp = hotspots.first(); hsp; hsp = hotspots.next())
  {
    if (hsp->contains(QPoint(rx,ry)))
    {
      if (hsp != last_hsp)
      {
        last_hsp = hsp;
        emit onHotSpot (hotspots.at());
        changed=true;
      }
      onspot=true;
      deepSpace=false;
      break;
    }
  }
  // we re out of all hotspots;
  if (!deepSpace && !onspot)
  {
    emit onHotSpot (-1);
    deepSpace = true;
    last_hsp=0;
  }

  if (onspot && !changed && hotspot_static)
  {
    geometryChanged = true;
    return;
  }

  if (transparent)
  {
    deleteLastRectangle();
    drawRectangle(xp, yp, w, h);

    XFlush(qt_xdisplay());
  }
  else
  {
    QPoint p(xp, yp);
    if (widget->parentWidget() != 0)
      p=widget->parentWidget()->mapFromGlobal(p);

    XMoveWindow(qt_xdisplay(), widget->winId(), p.x(), p.y());
    //widget->move(p);
  }
  XSync(qt_xdisplay(), False);

  if (dynamic)
    emit posChanged(xp, yp);
}

void KToolBoxManager::doXResize (bool _dontresize, bool _dynamic)
{
  if (working)
    return;

  yOnly = false;
  xOnly = true;

  doResize(_dontresize, _dynamic);

  yOnly = false;
  xOnly = false;

}

void KToolBoxManager::doYResize (bool _dontresize, bool _dynamic)
{
  if (working)
    return;

  yOnly = true;
  xOnly = false;

  doResize(_dontresize, _dynamic);

  yOnly = false;
  xOnly = false;

}

void KToolBoxManager::doResize (bool dontresize, bool _dynamic)
{
  if (working)
    return;


  Window wroot, wchild;
  int trash;
  
  //debug("Doing resize...");

  working=true;
  dynamic = _dynamic;
  dontmoveres=dontresize;
  mode = Resizing;
  
  QRect rr = KWM::geometry(widget->winId(), true);
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

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &sx, &sy, &trash, &trash, &active_button);
  
  rx = sx;
  ry = sy;

  QApplication::setOverrideCursor(QCursor(sizeFDiagCursor));
  
  connect (timer, SIGNAL(timeout()), this, SLOT (doResizeInternal()));

  if (transparent)
    drawRectangle(xp, yp, w, h);
  
  timer->start(0);
  qApp->enter_loop();
}

void KToolBoxManager::doResizeInternal ()
{
  Window wroot, wchild;
  int trash;
  unsigned int buttons;

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &rx, &ry, &trash, &trash, &buttons );
  
  if (buttons != active_button)
  {
    /*bool b = */ XCheckMaskEvent(qt_xdisplay(), ButtonReleaseMask, &ev);
    // if (b) debug ("KTBmgr: Got and removed mouseRelease");
    stop();
    return;
  }


  if (xOnly)
    ry=sy;
  else if (yOnly)
    rx=sx;

  if (rx == sx && ry == sy) return;
  
  w += rx-sx;
  h += ry-sy;

  sx=rx;
  sy=ry;

  if (transparent)
  {
    deleteLastRectangle();
    drawRectangle(xp, yp, w, h);
    XFlush(qt_xdisplay());
  }
  else
    widget->resize(w, h);
//    XResizeWindow (qt_xdisplay(), widget->winId(), w, h);

  XSync(qt_xdisplay(), False);

  if (dynamic)
    emit sizeChanged(w, h);
}


void KToolBoxManager::stop ()
{
  if (!working)
    return;
  
  timer->stop();
  disconnect (timer, SIGNAL(timeout()));

  QApplication::restoreOverrideCursor();

  if (transparent)
    deleteLastRectangle();

  XFlush(qt_xdisplay());

  if (dontmoveres) // do not move or resize caller'll do it himself
  {
    if (!transparent)
      if (mode==Moving)
      {
        QPoint p(orig_x, orig_y);
        if (widget->parent() != 0)
          p=widget->parentWidget()->mapFromGlobal(p);

        widget->move(p);
      }
      else if (mode == Resizing)
        widget->resize(orig_w, orig_h);
  }
  else // do move or resize, even children
  {
    if (transparent) // else if opaque: already moved/sized
      if (mode==Moving)
      {
        QPoint p(xp, yp);
        if (widget->parent() != 0)
          p=widget->parentWidget()->mapFromGlobal(p);

        widget->move(p);
      }
      else if (mode == Resizing)
        widget->resize(w, h);
  }
  
  working = false;
  mode=Nothing;

  qApp->exit_loop();
  //debug ("stopped");
}

void KToolBoxManager::setGeometry (int index)
{
  if (index == -1)
    return;
  QRect *r = hotspots.at(index);
  if (r)
    setGeometry(r->x(), r->y(), r->width(), r->height());
}

void KToolBoxManager::setGeometry (int _x, int _y, int _w, int _h)
{
  if (!working)
    return;

  xp=_x;
  yp=_y;
  w=_w;
  h=_h;
  if (transparent)
    deleteLastRectangle();
  else
    widget->resize(w, h);
  geometryChanged=true;
}


void KToolBoxManager::drawRectangle(int _x, int _y, int _w, int _h)
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

void KToolBoxManager::deleteLastRectangle()
{
  if (noLast)
    return;
  
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

#include "ktoolboxmgr.moc"

