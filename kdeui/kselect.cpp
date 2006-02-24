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

#include <qimage.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <kimageeffect.h>
#include "kselect.h"

#define STORE_W 8
#define STORE_W2 STORE_W * 2

//-----------------------------------------------------------------------------
/*
 * 2D value selector.
 * The contents of the selector are drawn by derived class.
 */

KXYSelector::KXYSelector( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	xPos = 0;
	yPos = 0;
	minX = 0;
	minY = 0;
	maxX = 100;
	maxY = 100;
	store.setOptimization( QPixmap::BestOptim );
	store.resize( STORE_W2, STORE_W2 );
}


KXYSelector::~KXYSelector()
{}


void KXYSelector::setRange( int _minX, int _minY, int _maxX, int _maxY )
{
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	px = w;
	py = w;
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
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (w < 5) w = 5;

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

	int xp = w + (width() - 2 * w) * xPos / (maxX - minX);
	int yp = height() - w - (height() - 2 * w) * yPos / (maxY - minY);

	setPosition( xp, yp );
}

QRect KXYSelector::contentsRect() const
{
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (w < 5) {
		w = 5;
	}
	QRect contents(rect());
	contents.addCoords(w, w, -w, -w);
	return contents;
}

void KXYSelector::paintEvent( QPaintEvent *ev )
{
	QRect cursorRect( px - STORE_W, py - STORE_W, STORE_W2, STORE_W2);
	QRect paintRect = ev->rect();
	QRect borderRect = rect();

	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (w < 5) {
		w = 5 - w;
	}
	borderRect.addCoords(w, w, -w, -w);

	QPainter painter;
	painter.begin( this );

	style().drawPrimitive(QStyle::PE_Panel, &painter, 
			      borderRect, colorGroup(), 
			      QStyle::Style_Sunken);

	drawContents( &painter );
	if (paintRect.contains(cursorRect))
	{
	   bitBlt( &store, 0, 0, this, px - STORE_W, py - STORE_W,
		STORE_W2, STORE_W2, CopyROP );
	   drawCursor( &painter, px, py );
        }
        else if (paintRect.intersects(cursorRect))
        {
           repaint( cursorRect, false);
        }

	painter.end();
}

void KXYSelector::mousePressEvent( QMouseEvent *e )
{
	mouseMoveEvent(e);
}

void KXYSelector::mouseMoveEvent( QMouseEvent *e )
{
	int xVal, yVal;

	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	valuesFromPosition( e->pos().x() - w, e->pos().y() - w, xVal, yVal );
	
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
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (w < 5) w = 5;
	xVal = ( (maxX-minX) * (x-w) ) / ( width()-2*w );
	yVal = maxY - ( ( (maxY-minY) * (y-w) ) / ( height()-2*w ) );
	
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
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	if (w < 5) w = 5;
	if ( xp < w )
		xp = w;
	else if ( xp > width() - w )
		xp = width() - w;

	if ( yp < w )
		yp = w;
	else if ( yp > height() - w )
		yp = height() - w;

	QPainter painter;
	painter.begin( this );

	bitBlt( this, px - STORE_W, py - STORE_W, &store, 0, 0,
			STORE_W2, STORE_W2, CopyROP );
	bitBlt( &store, 0, 0, this, xp - STORE_W, yp - STORE_W,
			STORE_W2, STORE_W2, CopyROP );
	drawCursor( &painter, xp, yp );
	px = xp;
	py = yp;

	painter.end();
}

void KXYSelector::drawContents( QPainter * )
{}


void KXYSelector::drawCursor( QPainter *p, int xp, int yp )
{
	p->setPen( QPen( white ) );

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


KSelector::KSelector( QWidget *parent, const char *name )
	: QWidget( parent, name ), QRangeControl()
{
	_orientation = Horizontal;
	_indent = true;
}

KSelector::KSelector( Orientation o, QWidget *parent, const char *name )
	: QWidget( parent, name ), QRangeControl()
{
	_orientation = o;
	_indent = true;
}


KSelector::~KSelector()
{}


QRect KSelector::contentsRect() const
{
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	int iw = (w < 5) ? 5 : w;
	if ( orientation() == Vertical )
		return QRect( w, iw, width() - w * 2 - 5, height() - 2 * iw );
	else
		return QRect( iw, w, width() - 2 * iw, height() - w * 2 - 5 );
}

void KSelector::paintEvent( QPaintEvent * )
{
	QPainter painter;
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	int iw = (w < 5) ? 5 : w;

	painter.begin( this );

	drawContents( &painter );

	if ( indent() )
	{
		QRect r = rect();
		if ( orientation() == Vertical )
			r.addCoords(0, iw - w, -iw, w - iw);
		else
			r.addCoords(iw - w, 0, w - iw, -iw);
		style().drawPrimitive(QStyle::PE_Panel, &painter, 
			r, colorGroup(), 
			QStyle::Style_Sunken);
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

void KSelector::valueChange()
{
	QPainter painter;
	QPoint pos;

	painter.begin( this );

	pos = calcArrowPos( prevValue() );
	drawArrow( &painter, false, pos );   

	pos = calcArrowPos( value() );
	drawArrow( &painter, true, pos );   

	painter.end();

	emit valueChanged( value() );
}

void KSelector::moveArrow( const QPoint &pos )
{
	int val;
	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	int iw = (w < 5) ? 5 : w;

	if ( orientation() == Vertical )
		val = ( maxValue() - minValue() ) * (height()-pos.y()-5+w)
				/ (height()-iw*2) + minValue();
	else
		val = ( maxValue() - minValue() ) * (width()-pos.x()-5+w)
				/ (width()-iw*2) + minValue();

	setValue( val );
}

QPoint KSelector::calcArrowPos( int val )
{
	QPoint p;

	int w = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
	int iw = (w < 5) ? 5 : w;
	if ( orientation() == Vertical )
	{
		p.setY( height() - ( (height()-2*iw) * val
				/ ( maxValue() - minValue() ) + 5 ) );
		p.setX( width() - 5 );
	}
	else
	{
		p.setX( width() - ( (width()-2*iw) * val
				/ ( maxValue() - minValue() ) + 5 ) );
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
    QPointArray array(3);

    painter->setPen( QPen() );
    painter->setBrush( QBrush( colorGroup().buttonText() ) );
    array.setPoint( 0, pos.x()+0, pos.y()+0 );
    array.setPoint( 1, pos.x()+5, pos.y()+5 );
    if ( orientation() == Vertical )
    {
      array.setPoint( 2, pos.x()+5, pos.y()-5 );
    }
    else
    {
      array.setPoint( 2, pos.x()-5, pos.y()+5 );
    }

    painter->drawPolygon( array );
  } 
  else 
  {
    if ( orientation() == Vertical )
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

KGradientSelector::KGradientSelector( QWidget *parent, const char *name )
    : KSelector( parent, name )
{
    init();
}


KGradientSelector::KGradientSelector( Orientation o, QWidget *parent,
		const char *name )
	: KSelector( o, parent, name )
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

	if ( orientation() == Vertical )
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

	if ( orientation() == Vertical )
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

