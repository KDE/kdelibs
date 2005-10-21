/*
 *   Copyright (C) 1997  Michael Roth <mroth@wirlweb.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "kseparator.h"
#include <QStyle>
#include <QStyleOption>
#include <QPainter>

KSeparator::KSeparator(QWidget* parent, Qt::WFlags f) : QFrame(parent, f)
{
   setLineWidth(1);
   setMidLineWidth(0);
   setOrientation( Qt::Horizontal );
}


KSeparator::KSeparator(Qt::Orientation orientation, QWidget* parent, Qt::WFlags f)
   : QFrame(parent, f)
{
   setLineWidth(1);
   setMidLineWidth(0);
   setOrientation( orientation );
}


void KSeparator::setOrientation(Qt::Orientation orientation)
{
   if (orientation == Qt::Vertical) {
      setFrameShape ( QFrame::VLine );
      setFrameShadow( QFrame::Sunken );
      setMinimumSize(2, 0);
   }
   else {
      setFrameShape ( QFrame::HLine );
      setFrameShadow( QFrame::Sunken );
      setMinimumSize(0, 2);
   }
}


Qt::Orientation KSeparator::orientation() const
{
   return ( frameStyle() & VLine ) ? Qt::Vertical : Qt::Horizontal;
}


void KSeparator::paintEvent(QPaintEvent*)
{
   QPainter p(this);

   QStyleOption opt;
   opt.init(this);

   QPoint	p1, p2;
   QRect	r     = frameRect();
   if ( frameStyle() & HLine ) {
      p1 = QPoint( r.x(), r.height()/2 );
      p2 = QPoint( r.x()+r.width(), p1.y() );
   }
   else {
      p1 = QPoint( r.x()+r.width()/2, 0 );
      p2 = QPoint( p1.x(), r.height() );
   }

   opt.rect = QRect(p1, p2);

   style()->drawPrimitive( QStyle::PE_Q3Separator, &opt, &p);
}


QSize KSeparator::sizeHint() const
{
   return ( frameStyle() & VLine ) ? QSize(2, 0) : QSize(0, 2);
}


void KSeparator::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kseparator.moc"
