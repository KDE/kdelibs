/*
    This file is part of the KDE libraries
    Copyright (C) 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
              
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
#include <qapp.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ktoolboxmgr.h>
#include <kwm.h>

 // $Id$
 // $Log$
 // Revision 1.1  1998/04/28 09:16:18  radej
 // Initial checkin
 //


KToolBoxManager::KToolBoxManager (QWidget *_widget, bool) : QObject ()
{
  XGCValues gv;

  working=false;
  noLast=true;
  widget = _widget;
  geometryChanged = false;
  blockme = true;
  mode = Nothing;
  
  //transparent = true; //unused
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

void KToolBoxManager::removeHotSpot(int index)
{
  hotspots.remove (index);
}

void KToolBoxManager::doMove (bool hot_static, bool _dynamic, bool dontmove)
{
  if (working)
    return;

  debug("Doing move...");

  working=true;
  mode = Moving;
  dynamic = _dynamic;
  dontmoveres=dontmove;
  hotspot_static = hot_static;
  
  //QPoint p(widget->x(), widget->y());
  
  QRect rr = KWM::geometry(widget->winId(), true);
  QPoint p(rr.topLeft());

  //if (widget->parentWidget() != 0)
    //p=widget->parentWidget()->mapToGlobal(p);

  offX = QCursor::pos().x() - p.x();
  offY = QCursor::pos().y() - p.y();
  
  xp = p.x();
  yp = p.y();
  w = rr.width();
  h = rr.height();

  orig_w = w;
  orig_h = h;
  
  XChangeActivePointerGrab( qt_xdisplay(), 
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask ,
                            sizeAllCursor.handle(), 0);

  rx = sx = QCursor::pos().x();
  ry = sy = QCursor::pos().y();
  
  XGrabServer(qt_xdisplay());

  xp=sx-offX;
  yp=sy-offY;

  connect (timer, SIGNAL(timeout()), this, SLOT (doMoveInternal()));
  drawRectangle(xp, yp, w, h);
  timer->start(0);
  if (blockme)
    qApp->enter_loop();
}

void KToolBoxManager::doMoveInternal()
{
  bool onspot=false;
  bool changed=false;
  
  XMaskEvent(qt_xdisplay(),
             ButtonPressMask|ButtonReleaseMask|PointerMotionMask, &ev);

    if (ev.type == MotionNotify)
    {
      rx = ev.xmotion.x_root;
      ry = ev.xmotion.y_root;
    }
    else
    {
      stop();
      return;
    }

    if (rx == sx && ry == sy)
      return;

    if (geometryChanged)
    {
      offX = rx - xp;
      offY = ry - yp;
      geometryChanged = false;
    }
    
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

    deleteLastRectangle();
    drawRectangle(xp, yp, w, h);
    XFlush(qt_xdisplay());
    if (dynamic)
      emit posChanged(xp, yp);
    
}

void KToolBoxManager::doResize (bool dontresize, bool _dynamic)
{
  if (working)
    return;

  debug("Doing resize...");

  working=true;
  dynamic = _dynamic;
  dontmoveres=dontresize;
  mode = Resizing;
  
  //QPoint p(widget->x(), widget->y());
  QRect rr = KWM::geometry(widget->winId(), true);
  QPoint p(rr.topLeft());


  
  //if (widget->parentWidget() != 0)
    //p=widget->parentWidget()->mapToGlobal(p);

  offX = QCursor::pos().x() - p.x();
  offY = QCursor::pos().y() - p.y();
  
  xp = p.x();
  yp = p.y();
  w = rr.width();
  h = rr.height();

  orig_w = w;
  orig_h = h;
  
  XChangeActivePointerGrab( qt_xdisplay(), 
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask ,
                            sizeAllCursor.handle(), 0);

  rx = sx = QCursor::pos().x();
  ry = sy = QCursor::pos().y();
  
  XGrabServer(qt_xdisplay());

  connect (timer, SIGNAL(timeout()), this, SLOT (doResizeInternal()));
  
  drawRectangle(xp, yp, w, h);
  timer->start(0);
  if (blockme)
    qApp->enter_loop();
}

void KToolBoxManager::doResizeInternal ()
{
  
  XMaskEvent(qt_xdisplay(),
             ButtonPressMask|ButtonReleaseMask|PointerMotionMask, &ev);

  if (ev.type == MotionNotify)
  {
    rx = ev.xmotion.x_root;
    ry = ev.xmotion.y_root;
  }
  else
  {
    stop();
    return;
  }

  if (rx == sx && ry == sy)
    return;

  
  w += rx-sx;
  h += ry-sy;

  sx=rx;
  sy=ry;
  
  deleteLastRectangle();
  drawRectangle(xp, yp, w, h);
  XFlush(qt_xdisplay());
  if (dynamic)
    emit sizeChanged(w, h);
}


void KToolBoxManager::stop ()
{
  if (!working)
    return;
  
  timer->stop();
  disconnect (timer, SIGNAL(timeout()));
  
  deleteLastRectangle();
  XFlush(qt_xdisplay());
  
  XUngrabServer(qt_xdisplay());
  XAllowEvents(qt_xdisplay(), AsyncPointer, CurrentTime);
  XSync(qt_xdisplay(), False);

  if (widget->parentWidget() == 0)
    if (!dontmoveres)
      if (mode==Moving)
        widget->move(xp, yp);
      else if (mode == Resizing)
        widget->resize(w, h);
  
  working = false;
  mode=Nothing;
  if (blockme)
    qApp->exit_loop();
  debug ("stopped");
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
  deleteLastRectangle();
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
  noLast = true;
}

#include "ktoolboxmgr.moc"