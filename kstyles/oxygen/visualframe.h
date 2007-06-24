/***************************************************************************
 *   Copyright (C) 2006-2007 by Thomas L�bking                             *
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
#ifndef VISUALFRAME_H
#define VISUALFRAME_H

class QFrame;
class QMouseEvent;
class QWheelEvent;
class QPaintEvent;

#include <QWidget>
#include <QPoint>

class VisualFrame : public QWidget {
   Q_OBJECT
public:
   enum Side { North, South, East, West };
   VisualFrame(QFrame *parent, Side side,
               uint thickness = 0, uint off1 = 0, uint off2 = 0);
   bool eventFilter ( QObject * o, QEvent * ev );
   void paintEvent ( QPaintEvent * event );
protected:
//    void enterEvent ( QEvent * event ) { passDownEvent(event, event->globalPos()); }
//    void leaveEvent ( QEvent * event ) { passDownEvent(event, event->globalPos()); }
   void mouseDoubleClickEvent ( QMouseEvent * event );
   void mouseMoveEvent ( QMouseEvent * event );
   void mousePressEvent ( QMouseEvent * event );
   void mouseReleaseEvent ( QMouseEvent * event );
   void wheelEvent ( QWheelEvent * event );
private:
   void passDownEvent(QEvent *ev, const QPoint &gMousePos);
   int _thickness;
   int _off[2];
   Side _side;
};

#endif //VISUALFRAME_H
