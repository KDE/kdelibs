//-----------------------------------------------------------------------------
// KSlider control V1.3
//
// Copyright (C) Martin R. Jones 1997
//
// KSlider now maintained by Christian Esken (chris@magicon.prima.ruhr.de)
// Revision information.
// 1.0 KSlider by Martin R. Jones
// 1.1 All changes now by Christian Esken: sanity checks ( checkWidth() ).
// 1.2 Implemented rangeChange()

// 1.3 Reworked drawArrow(). For avoiding paint problems with some servers, I
// am now painting the arrow with "precise" lines.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpainter.h>
#include <qdrawutl.h>
#include <qevent.h>

#include "kslider.h"

#include "kslider.moc"

#define ARROW_LENGTH	13

KSlider::KSlider( Orientation o, QWidget *parent, const char *name )
  : QWidget( parent, name ), QRangeControl()
{
  _orientation = o;
}

KSlider::KSlider( int _minValue, int _maxValue, int _lineStep, int _pageStep,
		  Orientation o, QWidget *parent, const char *name )
  : QWidget( parent, name ), QRangeControl()
{
  _orientation = o;
  setRange( _minValue, _maxValue );
  setSteps( _lineStep, _pageStep );
}

void KSlider::rangeChange()
{
  // Necessary for range changes after creation time
  valueChange();
  repaint();
}

void KSlider::valueChange()
{
  QPainter painter;
  QPoint pos;

  painter.begin( this );

  pos = calcArrowPos( prevValue() );
  drawArrow( painter, FALSE, pos );

  if ( orientation() == Vertical )
    {
      qDrawShadeLine(&painter, 5, 3, 5, height()-3, colorGroup(), TRUE, 1, 2);
    }
  else
    {
      qDrawShadeLine(&painter, 3, 5, width()-3, 5, colorGroup(), TRUE, 1, 2);
    }

  pos = calcArrowPos( value() );
  drawArrow( painter, TRUE, pos );

  painter.end();
}


void KSlider::drawArrow( QPainter &painter, bool show, const QPoint &pos )
{
  QPen        arrowPen;
  QPointArray array(5);

  // Select Horizontal or Vertical Polygon Array
  if ( orientation() == Vertical )
    {
      array.setPoint( 0, pos.x()+0, pos.y()+0 );
      array.setPoint( 1, pos.x()-4, pos.y()+4 );
      array.setPoint( 2, pos.x()-ARROW_LENGTH, pos.y()+4 );
      array.setPoint( 3, pos.x()-ARROW_LENGTH, pos.y()-4 );
      array.setPoint( 4, pos.x()-4, pos.y()-4 );
    }
  else
    {
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
    arrowPen = QPen( backgroundColor() );

  arrowPen.setWidth(1);		// Yup, we REALLY want width 1, not 0 here !!
  painter.setPen(arrowPen);
  painter.setBrush( backgroundColor() );
  painter.setRasterOp ( CopyROP );

  painter.drawPolygon( array );

  if ( show )
    {
      arrowPen = QPen( colorGroup().dark() );
      arrowPen.setWidth(1);	// Yup, we REALLY want width 1, not 0, here !!
      painter.setPen(arrowPen);
      painter.drawPolyline( array, 0, 3);

      // !!! This fixes a problem with a missing point! Qt Bug? !!!
      painter.drawPoint(array[1]);
    }
}


QSize KSlider::sizeHint() const
{
  QSize size;

  if ( orientation() == Vertical )
    {
      size.setWidth( ARROW_LENGTH + 5 + 1 );
      size.setHeight( 50 );
    }
  else
    {
      size.setWidth( 50 );
      size.setHeight( ARROW_LENGTH + 5 + 1 );
    }

  return size;
}

QPoint KSlider::calcArrowPos( int val )
{
  QPoint p;
  int diffMaxMin = checkWidth();	// sanity check, Christian Esken

  if ( orientation() == Vertical )
    {
      p.setY( height() - ( (height()-10) * ( val - minValue() )
			   / diffMaxMin + 5 ) );
      p.setX( ARROW_LENGTH );
    }
  else
    {
      p.setX( ( (width()-10) * ( val - minValue() )
		/ diffMaxMin + 5 ) );
      p.setY( ARROW_LENGTH );
    }

  return p;
}

void KSlider::moveArrow( const QPoint &pos )
{
  int val;

  if ( orientation() == Vertical )
    val = ( maxValue() - minValue() ) * (height()-pos.y()-3)
      / (height()-10) + minValue();
  else
    val = ( maxValue() - minValue() ) * (pos.x()-3)
      / (width()-10) + minValue();

  if ( val > maxValue() )
    val = maxValue();
  if ( val < minValue() )
    val = minValue();

  emit valueChanged( val );
  setValue( val );
}

void KSlider::paintEvent( QPaintEvent * )
{
  QPainter painter;

  int i;
  int diffMaxMin = checkWidth();	// sanity check, Christian Esken
  painter.begin( this );

  if ( orientation() == Vertical )
    {
      qDrawShadeLine(&painter, 5, 3, 5, height()-3, colorGroup(), TRUE, 1, 2);

      // draw ruler marks
      for ( i = 0; i <= maxValue() - minValue(); i += lineStep() )
	{
	  int pos = (height()-10) * i / diffMaxMin + 5;
	  painter.drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 4, pos );
	}

      for ( i = 0; i <= maxValue() - minValue(); i += pageStep() )
	{
	  int pos = (height()-10) * i / diffMaxMin + 5;
	  painter.drawLine( ARROW_LENGTH+1, pos, ARROW_LENGTH + 6, pos );
	}
    }
  else
    {
      qDrawShadeLine(&painter, 3, 5, width()-3, 5, colorGroup(), TRUE, 1, 2);

      // draw ruler marks
      for ( i = 0; i <= maxValue() - minValue(); i += lineStep() )
	{
	  int pos = (width()-10) * i / diffMaxMin + 5;
	  painter.drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 4 );
	}

      for ( i = 0; i <= maxValue() - minValue(); i += pageStep() )
	{
	  int pos = (width()-10) * i / diffMaxMin + 5;
	  painter.drawLine( pos, ARROW_LENGTH+1, pos, ARROW_LENGTH + 6 );
	}
    }

  drawArrow( painter, TRUE, calcArrowPos( value() ) );


  painter.end();
}

void KSlider::mousePressEvent( QMouseEvent *e )
{
  moveArrow( e->pos() );
}

void KSlider::mouseMoveEvent( QMouseEvent *e )
{
  moveArrow( e->pos() );
}

int KSlider::checkWidth()
{
  int diff = maxValue() - minValue();
  if ( diff == 0)
    // If (max - min) has no "secure" value, set it to lineStep().  Christian Esken
    diff=lineStep();
  return diff;
}
