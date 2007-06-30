/***************************************************************************
 *   Copyright (C) 2006-2007 by Thomas Luebking                            *
 *   thomas.luebking@web.de                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "visualframe.h"
#include <QBitmap>
#include <QCoreApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStyle>
#include <QWheelEvent>

#include <QtDebug>

static QRegion corner[4];

VisualFrame::VisualFrame(QWidget *parent, QFrame *frame, Side side, uint t,
                         int e, uint o1, uint o2, uint o3, uint o4) :
QWidget(parent) {
   if (!(frame && t)) {
      deleteLater(); return;
   }
   _frame = frame;
   _side = side;
   _ext = e;
   if (corner[0].isEmpty()) {
      int $5 = 4;
      QBitmap bm(2*$5, 2*$5);
      bm.fill(Qt::black);
      QPainter p(&bm);
      p.setPen(Qt::NoPen);
      p.setBrush(Qt::white);
      p.drawEllipse(0,0,2*$5,2*$5);
      p.end();
      QRegion circle(bm);
      corner[0] = circle & QRegion(0,0,$5,$5); // tl
      corner[1] = circle & QRegion($5,0,$5,$5); // tr
      corner[1].translate(-corner[1].boundingRect().left(), 0);
      corner[2] = circle & QRegion(0,$5,$5,$5); // bl
      corner[2].translate(0, -corner[2].boundingRect().top());
      corner[3] = circle & QRegion($5,$5,$5,$5); // br
      corner[3].translate(-corner[3].boundingRect().topLeft());
   }
   _thickness = t;
   _off[0] = o1; _off[1] = o2; _off[2] = o3; _off[3] = o4;
   connect(frame, SIGNAL(destroyed(QObject*)), this, SLOT(hide()));
   connect(frame, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
   if (frame->isVisible()) show(); else hide();
   frame->installEventFilter(this);
   this->installEventFilter(this);
//    setMouseTracking ( true );
//    setAcceptDrops(true);
}

void VisualFrame::rise() {
   QWidgetList widgets = parentWidget()->findChildren<QWidget*>();
   QWidget *up = 0; int cnt = widgets.size()-1;
   for (int i = 0; i < cnt; ++i)
      if (widgets.at(i) == _frame) {
         up = widgets.at(i+1);
         break;
      }
   if (up)
      stackUnder(up);
   else
      raise();
}

#include <QtDebug>

void VisualFrame::paintEvent ( QPaintEvent * event ) {
   QPainter p(this);
   p.setClipRegion(event->region(), Qt::IntersectClip);
   QStyleOption opt;
   if (_frame->frameShadow() == QFrame::Raised)
      opt.state |= QStyle::State_Raised;
   else if (_frame->frameShadow() == QFrame::Sunken)
      opt.state |= QStyle::State_Sunken;
   if (_frame->hasFocus())
      opt.state |= QStyle::State_HasFocus;
   if (_frame->isEnabled())
      opt.state |= QStyle::State_Enabled;
   opt.rect = _frame->frameRect();
   switch (_side) {
   case North:
      opt.rect.setWidth(opt.rect.width()+_off[0]+_off[1]);
      opt.rect.setHeight(opt.rect.height()+_ext);
      opt.rect.moveTopLeft(rect().topLeft());
      break;
   case South:
      opt.rect.setWidth(opt.rect.width()+_off[0]+_off[1]);
      opt.rect.setHeight(opt.rect.height()+_ext);
      opt.rect.moveBottomLeft(rect().bottomLeft());
      break;
   case West:
      opt.rect.setWidth(opt.rect.width()+_ext);
      opt.rect.setHeight(opt.rect.height()+_off[2]+_off[3]);
      opt.rect.moveTopLeft(QPoint(0, -_off[2]));
      break;
   case East:
      opt.rect.setWidth(opt.rect.width()+_ext);
      opt.rect.setHeight(opt.rect.height()+_off[2]+_off[3]);
      opt.rect.moveTopRight(QPoint(width()-1, -_off[2]));
      break;
   }
   style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
   p.end();
}

void VisualFrame::passDownEvent(QEvent *ev, const QPoint &gMousePos) {
   // the raised frames don't look like you could click in, we'll see if this should be changed...
   if (_frame->frameShadow() == QFrame::Raised)
      return;
   QList<QWidget *> candidates = _frame->findChildren<QWidget *>();
   QList<QWidget *>::const_iterator i = candidates.constEnd();
   QWidget *match = 0;
   while (i != candidates.constBegin()) {
      --i;
      if (*i == this)
         continue;
      if ((*i)->rect().contains((*i)->mapFromGlobal(gMousePos))) {
         match = *i;
         break;
      }
   }
   if (!match) match = _frame;
   QCoreApplication::sendEvent( match, ev );
}

#define HANDLE_EVENT(_EV_) \
void VisualFrame::_EV_ ( QMouseEvent * event ) {\
   passDownEvent((QEvent *)event, event->globalPos());\
}

HANDLE_EVENT(mouseDoubleClickEvent)
HANDLE_EVENT(mouseMoveEvent)
HANDLE_EVENT(mousePressEvent)
HANDLE_EVENT(mouseReleaseEvent)

void VisualFrame::wheelEvent ( QWheelEvent * event ) {
   passDownEvent((QEvent *)event, event->globalPos());
}

#undef HANDLE_EVENT

bool VisualFrame::eventFilter ( QObject * o, QEvent * ev ) {
   if (o == this) {
      if (ev->type() == QEvent::ZOrderChange)
         rise();
      return false;
   }
   if (o != _frame) {
      o->removeEventFilter(this);
      return false;
   }
   if (ev->type() == QEvent::Resize || ev->type() == QEvent::Show) {
      QRect rect = _frame->frameRect();
      rect.translate(_frame->mapTo(parentWidget(), QPoint(0,0)));
      int offs = _off[0]+_off[1];
      switch (_side) {
      case North: {
         
         const int x = _frame->frameRect().x();
         const int y = _frame->frameRect().y();
         const int w = _frame->frameRect().right()+1;
         const int h = _frame->frameRect().bottom()+1;
         QRegion mask(_frame->rect());
         mask -= corner[0].translated(x, y); // tl
         QRect br = corner[1].boundingRect();
         mask -= corner[1].translated(w-br.width(), y); // tr
         br = corner[2].boundingRect();
         mask -= corner[2].translated(x, h-br.height()); // bl
         br = corner[3].boundingRect();
         mask -= corner[3].translated(w-br.width(), h-br.height()); // br
         _frame->setMask(mask);
         
         resize(rect.width()+offs, _thickness);
         move(rect.x()-_off[0], rect.y()-_ext);
         break;
      }
      case South:
         resize(rect.width()+offs, _thickness);
         move(rect.x()-_off[0], rect.bottom()+1+_ext-_thickness);
         break;
      case West:
         resize(_thickness, rect.height()-offs);
         move(rect.x()-_ext, rect.y()+_off[0]);
         break;
      case East:
         resize(_thickness, rect.height()-offs);
         move(rect.right()+1-_thickness+_ext, rect.y()+_off[0]);
         break;
      }
      if (ev->type() == QEvent::Show)
         show();
      return false;
   }
   if (ev->type() == QEvent::Hide) {
      hide();
      return false;
   }
   if (ev->type() == QEvent::FocusIn ||
       ev->type() == QEvent::FocusOut) {
      update();
      return false;
   }
   if (ev->type() == QEvent::ParentChange) {
      qWarning("parent changed?");
      _frame->parentWidget() ?
         setParent(_frame->parentWidget() ) :
         setParent(_frame );
      rise();
   }
   return false;
}
