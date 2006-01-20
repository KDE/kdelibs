/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>

#include <kimageeffect.h>

#include "kselect.h"

//-----------------------------------------------------------------------------
/*
 * 2D value selector.
 * The contents of the selector are drawn by derived class.
 */

KXYSelector::KXYSelector( QWidget *parent )
  : QWidget( parent )
{
  xPos = 0;
  yPos = 0;
  minX = 0;
  minY = 0;
  maxX = 100;
  maxY = 100;
}


KXYSelector::~KXYSelector()
{}


void KXYSelector::setRange( int _minX, int _minY, int _maxX, int _maxY )
{
  px = 2;
  py = 2;
  minX = _minX;
  minY = _minY;
  maxX = _maxX;
  maxY = _maxY;
}

void KXYSelector::setXValue( int _xPos )
{
  setValues(_xPos, yPos);
}

void KXYSelector::setYValue( int _yPos )
{
  setValues(xPos, _yPos);
}

void KXYSelector::setValues( int _xPos, int _yPos )
{
  xPos = _xPos;
  yPos = _yPos;

  if ( xPos > maxX )
    xPos = maxX;
  else if ( xPos < minX )
    xPos = minX;

  if ( yPos > maxY )
    yPos = maxY;
  else if ( yPos < minY )
    yPos = minY;

  int xp = 2 + (width() - 4) * xPos / (maxX - minX);
  int yp = height() - 2 - (height() - 4) * yPos / (maxY - minY);

  setPosition( xp, yp );
}

QRect KXYSelector::contentsRect() const
{
  return QRect( 2, 2, width()-4, height()-4 );
}

void KXYSelector::paintEvent( QPaintEvent *ev )
{
  QRect paintRect = ev->rect();

  QPainter painter;
  painter.begin( this );

  QBrush brush;
  qDrawShadePanel( &painter, 0, 0, width(), height(), colorGroup(),
      true, 2, &brush );

  drawContents( &painter );
  drawCursor( &painter, px, py );

  painter.end();
}

void KXYSelector::mousePressEvent( QMouseEvent *e )
{
  int xVal, yVal;
  valuesFromPosition( e->pos().x() - 2, e->pos().y() - 2, xVal, yVal );
  setValues( xVal, yVal );

  emit valueChanged( xPos, yPos );
}

void KXYSelector::mouseMoveEvent( QMouseEvent *e )
{
  int xVal, yVal;
  valuesFromPosition( e->pos().x() - 2, e->pos().y() - 2, xVal, yVal );
  setValues( xVal, yVal );

  emit valueChanged( xPos, yPos );
}

void KXYSelector::wheelEvent( QWheelEvent *e )
{
  if ( e->orientation() == Qt::Horizontal )
    setValues( xValue() + e->delta()/120, yValue() );
  else
    setValues( xValue(), yValue() + e->delta()/120 );

  emit valueChanged( xPos, yPos );
}

void KXYSelector::valuesFromPosition( int x, int y, int &xVal, int &yVal ) const
{
  xVal = ( (maxX-minX) * (x-2) ) / ( width()-4 );
  yVal = maxY - ( ( (maxY-minY) * (y-2) ) / ( height()-4 ) );

  if ( xVal > maxX )
    xVal = maxX;
  else if ( xVal < minX )
    xVal = minX;

  if ( yVal > maxY )
    yVal = maxY;
  else if ( yVal < minY )
    yVal = minY;
}

void KXYSelector::setPosition( int xp, int yp )
{
  if ( xp < 2 )
    xp = 2;
  else if ( xp > width() - 2 )
    xp = width() - 2;

  if ( yp < 2 )
    yp = 2;
  else if ( yp > height() - 2 )
    yp = height() - 2;

  px = xp;
  py = yp;


  update();
}

void KXYSelector::drawContents( QPainter * )
{}


void KXYSelector::drawCursor( QPainter *p, int xp, int yp )
{
  p->setPen( QPen( Qt::white ) );

  p->drawLine( xp - 6, yp - 6, xp - 2, yp - 2 );
  p->drawLine( xp - 6, yp + 6, xp - 2, yp + 2 );
  p->drawLine( xp + 6, yp - 6, xp + 2, yp - 2 );
  p->drawLine( xp + 6, yp + 6, xp + 2, yp + 2 );
}

//-----------------------------------------------------------------------------
/*
 * 1D value selector with contents drawn by derived class.
 * See KColorDialog for example.
 */


KSelector::KSelector( QWidget *parent )
  : QAbstractSlider( parent )
{
  _orientation = Qt::Horizontal;
  _indent = true;
  _previousPos = calcArrowPos( 0 );
}

KSelector::KSelector( Qt::Orientation o, QWidget *parent )
  : QAbstractSlider( parent )
{
  _orientation = o;
  _indent = true;
  _previousPos = calcArrowPos( 0 );
}


KSelector::~KSelector()
{}


QRect KSelector::contentsRect() const
{
  if ( orientation() == Qt::Vertical )
    return QRect( 2, 5, width()-9, height()-10 );
  else
    return QRect( 5, 2, width()-10, height()-9 );
}

void KSelector::paintEvent( QPaintEvent * )
{
  QPainter painter;

  painter.begin( this );

  drawContents( &painter );

  QBrush brush;

  if ( indent() )
  {
    if ( orientation() == Qt::Vertical )
      qDrawShadePanel( &painter, 0, 3, width()-5, height()-6,
        colorGroup(), true, 2, &brush );
    else
      qDrawShadePanel( &painter, 3, 0, width()-6, height()-5,
        colorGroup(), true, 2, &brush );
  }

  QPoint pos = calcArrowPos( value() );
  drawArrow( &painter, true, pos );

  painter.end();
}

void KSelector::mousePressEvent( QMouseEvent *e )
{
  moveArrow( e->pos() );
}

void KSelector::mouseMoveEvent( QMouseEvent *e )
{
  moveArrow( e->pos() );
}

void KSelector::wheelEvent( QWheelEvent *e )
{
  int val = value() + e->delta()/120;
  setValue( val );
}

void KSelector::sliderChange( SliderChange change )
{
  QPoint newPos = calcArrowPos( value() );
  update(QRect(_previousPos, newPos));
  _previousPos = newPos;

  if ( change == SliderValueChange )
    emit valueChanged( value() );
}

void KSelector::moveArrow( const QPoint &pos )
{
  int val;

  if ( orientation() == Qt::Vertical )
    val = ( maxValue() - minValue() ) * (height()-pos.y()-3)
        / (height()-10) + minValue();
  else
    val = ( maxValue() - minValue() ) * (width()-pos.x()-3)
        / (width()-10) + minValue();

  setValue( val );
}

QPoint KSelector::calcArrowPos( int val )
{
  QPoint p;

  if ( orientation() == Qt::Vertical )
  {
    p.setY( height() - ( (height()-10) * val
        / ( maximum() - minimum() ) + 5 ) );
    p.setX( width() - 5 );
  }
  else
  {
    p.setX( width() - ( (width()-10) * val
        / ( maximum() - minimum() ) + 5 ) );
    p.setY( height() - 5 );
  }

  return p;
}

void KSelector::drawContents( QPainter * )
{}

void KSelector::drawArrow( QPainter *painter, bool show, const QPoint &pos )
{
  if ( show )
  {
    QPolygon array(3);

    painter->setPen( QPen() );
    painter->setBrush( QBrush( colorGroup().buttonText() ) );
    if ( orientation() == Qt::Vertical )
    {
      array.setPoint( 0, pos.x()+0, pos.y()+0 );
      array.setPoint( 1, pos.x()+5, pos.y()+5 );
      array.setPoint( 2, pos.x()+5, pos.y()-5 );
    }
    else
    {
      array.setPoint( 0, pos.x()+0, pos.y()+0 );
      array.setPoint( 1, pos.x()+5, pos.y()+5 );
      array.setPoint( 2, pos.x()-5, pos.y()+5 );
    }

    painter->drawPolygon( array );
  }
  else
  {
    if ( orientation() == Qt::Vertical )
    {
       repaint(pos.x(), pos.y()-5, 6, 11, true);
    }
    else
    {
       repaint(pos.x()-5, pos.y(), 11, 6, true);
    }
  }
}

//----------------------------------------------------------------------------

KGradientSelector::KGradientSelector( QWidget *parent )
    : KSelector( parent )
{
    init();
}


KGradientSelector::KGradientSelector( Qt::Orientation o, QWidget *parent )
    : KSelector( o, parent )
{
    init();
}


KGradientSelector::~KGradientSelector()
{}


void KGradientSelector::init()
{
    color1.setRgb( 0, 0, 0 );
    color2.setRgb( 255, 255, 255 );

    text1 = text2 = "";
}


void KGradientSelector::drawContents( QPainter *painter )
{
  QImage image( contentsRect().width(), contentsRect().height(), 32 );

  QColor col;
  float scale;

  int redDiff   = color2.red() - color1.red();
  int greenDiff = color2.green() - color1.green();
  int blueDiff  = color2.blue() - color1.blue();

  if ( orientation() == Qt::Vertical )
  {
    for ( int y = 0; y < image.height(); y++ )
    {
      scale = 1.0 * y / image.height();
      col.setRgb( color1.red() + int(redDiff*scale),
            color1.green() + int(greenDiff*scale),
            color1.blue() + int(blueDiff*scale) );

      unsigned int *p = (uint *) image.scanLine( y );
      for ( int x = 0; x < image.width(); x++ )
        *p++ = col.rgb();
    }
  }
  else
  {
    unsigned int *p = (uint *) image.scanLine( 0 );

    for ( int x = 0; x < image.width(); x++ )
    {
      scale = 1.0 * x / image.width();
      col.setRgb( color1.red() + int(redDiff*scale),
            color1.green() + int(greenDiff*scale),
            color1.blue() + int(blueDiff*scale) );
      *p++ = col.rgb();
    }

    for ( int y = 1; y < image.height(); y++ )
      memcpy( image.scanLine( y ), image.scanLine( y - 1),
         sizeof( unsigned int ) * image.width() );
  }

  QColor ditherPalette[8];

  for ( int s = 0; s < 8; s++ )
    ditherPalette[s].setRgb( color1.red() + redDiff * s / 8,
                color1.green() + greenDiff * s / 8,
                color1.blue() + blueDiff * s / 8 );

  KImageEffect::dither( image, ditherPalette, 8 );

  QPixmap p;
  p.convertFromImage( image );

  painter->drawPixmap( contentsRect().x(), contentsRect().y(), p );

  if ( orientation() == Qt::Vertical )
  {
    int yPos = contentsRect().top() + painter->fontMetrics().ascent() + 2;
    int xPos = contentsRect().left() + (contentsRect().width() -
       painter->fontMetrics().width( text2 )) / 2;
    QPen pen( color2 );
    painter->setPen( pen );
    painter->drawText( xPos, yPos, text2 );

    yPos = contentsRect().bottom() - painter->fontMetrics().descent() - 2;
    xPos = contentsRect().left() + (contentsRect().width() -
      painter->fontMetrics().width( text1 )) / 2;
    pen.setColor( color1 );
    painter->setPen( pen );
    painter->drawText( xPos, yPos, text1 );
  }
  else
  {
    int yPos = contentsRect().bottom()-painter->fontMetrics().descent()-2;

    QPen pen( color2 );
    painter->setPen( pen );
    painter->drawText( contentsRect().left() + 2, yPos, text1 );

    pen.setColor( color1 );
    painter->setPen( pen );
    painter->drawText( contentsRect().right() -
       painter->fontMetrics().width( text2 ) - 2, yPos, text2 );
  }
}

//-----------------------------------------------------------------------------

void KXYSelector::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KSelector::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KGradientSelector::virtual_hook( int id, void* data )
{ KSelector::virtual_hook( id, data ); }

#include "kselect.moc"

