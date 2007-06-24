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
#include <QCoreApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStyle>
#include <QWheelEvent>

VisualFrame::VisualFrame(QFrame *parent, Side side, uint t, uint o1, uint o2) :
QWidget(parent) {
   if (!(parent && t)) {
      deleteLater(); return;
   }
   _side = side;
   _thickness = t;
   _off[0] = o1; _off[1] = o2;
   connect(parent, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
   parent->installEventFilter(this);
   this->installEventFilter(this);
//    setMouseTracking ( true );
//    setAcceptDrops(true);
   show();
}

void VisualFrame::paintEvent ( QPaintEvent * event ) {
   if (!parent()) {
      deleteLater(); return;
   }
   QFrame *frame = static_cast<QFrame*>(parent());
   QPainter p(this);
   p.setClipRegion(event->region(), Qt::IntersectClip);
   QStyleOption opt;
   if (frame->frameShadow() == QFrame::Raised)
      opt.state |= QStyle::State_Raised;
   else if (frame->frameShadow() == QFrame::Sunken)
      opt.state |= QStyle::State_Sunken;
   if (frame->hasFocus())
      opt.state |= QStyle::State_HasFocus;
   if (frame->isEnabled())
      opt.state |= QStyle::State_Enabled;
   opt.rect = frame->frameRect();
   switch (_side) {
   case North:
      opt.rect.moveTopLeft(rect().topLeft());
      break;
   case South:
      opt.rect.moveBottomLeft(rect().bottomLeft());
      break;
   case West:
      opt.rect.moveTopLeft(QPoint(0, -_off[0]));
      break;
   case East:
      opt.rect.moveTopRight(QPoint(width(), -_off[0]));
      break;
   }
   style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
   p.end();
}

void VisualFrame::passDownEvent(QEvent *ev, const QPoint &gMousePos) {
   if (!parent()) {
      deleteLater(); return;
   }
   QFrame *frame = static_cast<QFrame*>(parentWidget());
   // the raised frames don't look like you could click in, we'll see if this should be changed...
   if (frame->frameShadow() == QFrame::Raised)
      return;
   QList<QWidget *> candidates = frame->findChildren<QWidget *>();
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
   if (!match) match = frame;
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
         this->raise();
      return false;
   }
   if (o != parent()) {
      o->removeEventFilter(this);
      return false;
   }
   if (ev->type() == QEvent::Resize) {
      const QRect &rect = static_cast<QFrame*>(o)->frameRect();
      switch (_side) {
      case North:
         resize(rect.width(), _thickness);
         move(rect.topLeft());
         break;
      case South:
         resize(rect.width(), _thickness);
         move(rect.x(), rect.bottom()+1-_thickness);
         break;
      case West:
         resize(_thickness, rect.height()-_off[0]-_off[1]);
         move(rect.x(), rect.y()+_off[0]);
         break;
      case East:
         resize(_thickness, rect.height()-_off[0]-_off[1]);
         move(rect.right()+1-_thickness, rect.y()+_off[0]);
         break;
      }
      return false;
   }
   if (ev->type() == QEvent::FocusIn ||
       ev->type() == QEvent::FocusOut) {
      update();
      return false;
   }
   return false;
}
