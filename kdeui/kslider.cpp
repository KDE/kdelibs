/* This file is part of the KDE libraries
    Copyright (C) 1997-1998 Christian Esken (esken@kde.org)
              (C) 1997      Martin Jones    (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KSlider control V2.0
// KSlider now maintained by Christian Esken (esken@kde.org)
// Revision information.
// 1.0 KSlider by Martin R. Jones
// 1.1 All changes now by Christian Esken: sanity checks ( checkWidth() ).
// 1.2 Implemented rangeChange()
// 1.3 Reworked drawArrow(). For avoiding paint problems with some servers, I
//     am now painting the arrow with "precise" lines.
// 2.0 Now KSlider is a derivation of QSlider
// 2.1 Cleanups. Replacing eraseRect() by fillRect(). I would have thought,
//     eraseRect() would use BackgroundColor, but it doesn't.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpainter.h>
#include <qdrawutil.h>

#include "kslider.h"
#include "kslider.h"

#define ARROW_LENGTH	13

KSlider::KSlider( QWidget *parent, const char *name )
  : QSlider( parent, name )
{
  QSlider::setTickmarks(Below);
  isFocussed = false;
  update();
}

KSlider::KSlider( Orientation o, QWidget *parent, const char *name )
  : QSlider( o, parent, name )
{
  QSlider::setTickmarks(Below);
  isFocussed = false;
  update();
}

KSlider::KSlider( int _minValue, int _maxValue, int _Step, int _value,
		  Orientation o, QWidget *parent, const char *name )
  : QSlider( _minValue, _maxValue, _Step, _value, o,
             parent, name )
{
  // We always have TickMarks
  if ( orientation() == QSlider::Vertical)
    QSlider::setTickmarks(Right);
  else
    QSlider::setTickmarks(Below);
  isFocussed = false;
  update();
}



void KSlider::drawShadeLine( QPainter *painter )
{
  if ( orientation() == QSlider::Vertical )
    qDrawShadeLine(painter, 5, 3, 5, height()-3, colorGroup(), true, 1, 2);
  else
    qDrawShadeLine(painter, 3, 5, width()-3, 5, colorGroup(), true, 1, 2);
}


void KSlider::drawFocusBar(QPainter *painter, const QRect & )
{
/*
   if ( isFocussed )
     painter->setPen(colorGroup().dark() );
   else
     painter->setPen(colorGroup().background() );
*/
   if ( isFocussed )
     qDrawPlainRect(painter,0,0,width(),height(), colorGroup().dark(),1,0);
   else
     qDrawPlainRect(painter,0,0,width(),height(), colorGroup().background(),1,0);
}


void KSlider::paintSlider(QPainter *painter, const QRect &re )
{
  QPoint pos;

  // erase old arrow
  pos = calcArrowPos( prevValue() );
  drawArrow( painter, false, pos );

  // show, if focussed
  drawFocusBar(painter, re);
  drawShadeLine(painter);

  // draw new arrow
  pos = calcArrowPos( value() );
  drawArrow( painter, true, pos );
}

void KSlider::drawArrow( QPainter *painter, bool show, const QPoint &pos )
{
  QPen        arrowPen;
  QPointArray array(5);

  // Select Horizontal or Vertical Polygon Array
  if ( orientation() == QSlider::Vertical ) {
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()-4, pos.y()+4 );
    array.setPoint( 2, pos.x()-ARROW_LENGTH, pos.y()+4 );
    array.setPoint( 3, pos.x()-ARROW_LENGTH, pos.y()-4 );
    array.setPoint( 4, pos.x()-4, pos.y()-4 );
  }
  else {
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()+4, pos.y()-4 );
    array.setPoint( 2, pos.x()+4, pos.y()-ARROW_LENGTH );
    array.setPoint( 3, pos.x()-4, pos.y()-ARROW_LENGTH );
    array.setPoint( 4, pos.x()-4, pos.y()-4 );
  }

  // Select a base pen, then change parameters
  if ( show )
    arrowPen = QPen( colorGroup().light() );
  else
    arrowPen = QPen( colorGroup().background() );

  arrowPen.setWidth(1);		// Yup, we REALLY want width 1, not 0 here !!
  painter->setPen(arrowPen);
  painter->setBrush( colorGroup().background() );

  painter->drawPolygon( array );

  if ( show )
    {
      arrowPen = QPen( colorGroup().dark() );
      arrowPen.setWidth(1);	// Yup, we REALLY want width 1, not 0, here !!
      painter->setPen(arrowPen);
      painter->drawPolyline( array, 0, 3);

      // !!! This fixes a problem with a missing point! Qt Bug?
      // !!! I will wait for Qt1.3 to see the results
      // painter->drawPoint(array[1]);
    }
}


QSize KSlider::sizeHint() const
{
  QSize size;

  if ( orientation() == QSlider::Vertical ) {
    size.setWidth( ARROW_LENGTH + 5 + 1 );
    size.setHeight( -1 );
  }
  else {
    size.setWidth( -1 );
    size.setHeight( ARROW_LENGTH + 5 + 1 );
  }
  return size;
}



QPoint KSlider::calcArrowPos( int val )
{
  QPoint p;
  int diffMaxMin = checkWidth();	// sanity check, Christian Esken

  if ( orientation() == QSlider::Vertical ) {
    p.setY( height() - ( ((height()-10) * ( val - minValue() ))
				  / diffMaxMin ) + 5 );
    p.setX( ARROW_LENGTH );
  }
  else {
    p.setX( ( ((width()-10) * ( val - minValue() ))
	      / diffMaxMin) + 5  );
    p.setY( ARROW_LENGTH );
  }
  return p;
}


void KSlider::drawTickMarks(QPainter *painter)
{
  QPen tickPen = QPen( colorGroup().dark() );
  tickPen.setWidth(1);	// Yup, we REALLY want width 1, not 0 here !!
  painter->setPen(tickPen);

  int i;
  int diffMaxMin = checkWidth();	// sanity check
  if ( orientation() == QSlider::Vertical ) {
    // first clear the tickmark area
    painter->fillRect(ARROW_LENGTH+1,0, ARROW_LENGTH + 6, height()-1, colorGroup().background()  );

    // draw ruler marks
    for ( i = 0; i <= maxValue() - minValue(); i += lineStep() ) {
      int pos = (height()-10) * i / diffMaxMin + 5;
      painter->drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 4, pos );
    }
    for ( i = 0; i <= maxValue() - minValue(); i += pageStep() ) {
      int pos = (height()-10) * i / diffMaxMin + 5;
      painter->drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 6, pos );
    }
  }
  else {
    // first clear the tickmark area
    painter->fillRect(0, ARROW_LENGTH+1, width()-1, ARROW_LENGTH + 6, colorGroup().background() );
    // draw ruler marks
    for ( i = 0; i <= maxValue() - minValue(); i += lineStep() ) {
      int pos = (width()-10) * i / diffMaxMin + 5;
      painter->drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 4 );
    }
    for ( i = 0; i <= maxValue() - minValue(); i += pageStep() ) {
      int pos = (width()-10) * i / diffMaxMin + 5;
      painter->drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 6 );
    }
  }
}

void KSlider::drawTicks ( QPainter * p, int , int , int )
{
  drawTickMarks(p);
}

void KSlider::drawWinGroove (QPainter *, QCOORD)
{
  // Do nothing
}

void KSlider::paintEvent( QPaintEvent *qpe )
{
  QPainter painter;

  painter.begin( this );
  // build a rect for the paint event
  QRect rect(x(),y(),width(),height());
  // Clear widget area, because there might be "pixel dirt" around.
  // Especially then, when rangeChange(), resizes, such things happen.

  painter.fillRect(qpe->rect().x(),qpe->rect().y(),qpe->rect().width(),qpe->rect().height(), colorGroup().background() );
  // painter.eraseRect(qpe->rect().x(),qpe->rect().y(),qpe->rect().width(),qpe->rect().height());
  paintSlider(&painter, rect);

  if ( orientation() == QSlider::Vertical ) {
    QRect TickRect(ARROW_LENGTH+1,0,width(),height());
    if (qpe->rect().intersects(TickRect))
       drawTickMarks(&painter);
  }
  else {
    QRect TickRect(0, ARROW_LENGTH+1,width(),height());
    if (qpe->rect().intersects(TickRect)) {
      drawTickMarks(&painter);
    }
  }
  painter.end();
}


void KSlider::valueChange()
{
  QSlider::valueChange();
  if ( orientation() == QSlider::Vertical )
    repaint(0,0,ARROW_LENGTH,height());
  else
    repaint(0,0,width(),ARROW_LENGTH);

//  emit valueChanged( value() );
}


void KSlider::rangeChange()
{
  QSlider::rangeChange();
  // when range changes, everything must be repainted  
  update();
}

int KSlider::checkWidth()
{
  int diff = maxValue() - minValue();
  if ( diff == 0)
    // If (max - min) has no "secure" value, set it to lineStep().
    diff=lineStep();
  return diff;
}

void KSlider::paletteChange(const QPalette &)
{
  update();
}

void KSlider::backgroundColorChange(const QPalette &)
{
  update();
}

void KSlider::focusInEvent( QFocusEvent * )
{
  QPainter painter;
  QRect rect(x(),y(),width(),height());
  painter.begin( this );
  isFocussed = true;
  paintSlider(&painter,rect);
  painter.end();
}

void KSlider::focusOutEvent( QFocusEvent * )
{
  QPainter painter;
  QRect rect(x(),y(),width(),height());
  painter.begin( this );
  isFocussed = false;
  paintSlider(&painter,rect);
  painter.end();
}
#include "kslider.moc"

