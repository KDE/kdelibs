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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//-----------------------------------------------------------------------------
// Selector widgets for KDE Color Selector, but probably useful for other
// stuff also.

#include <stdlib.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <dither.h>
#include "kselect.h"
#include "kselect.moc"

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
	store.optimize( TRUE );
	store.resize( STORE_W2, STORE_W2 );
}

void KXYSelector::setRange( int _minX, int _minY, int _maxX, int _maxY )
{
	px = 2;
	py = 2;
	minX = _minX;
	minY = _minY;
	maxX = _maxX;
	maxY = _maxY;
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

QRect KXYSelector::contentsRect()
{
	return QRect( 2, 2, width()-4, height()-4 );
}

void KXYSelector::paintEvent( QPaintEvent * )
{
	QPainter painter;

	painter.begin( this );

	QBrush brush;
	qDrawShadePanel( &painter, 0, 0, width(), height(), colorGroup(),
			TRUE, 2, &brush );

	drawContents( &painter );
	bitBlt( &store, 0, 0, this, px - STORE_W, py - STORE_W,
		STORE_W2, STORE_W2, CopyROP );
	drawCursor( painter, px, py );

	painter.end();
}

void KXYSelector::mousePressEvent( QMouseEvent *e )
{
	setPosition( e->pos().x() - 2, e->pos().y() - 2 );

	emit valueChanged( xPos, yPos );
}

void KXYSelector::mouseMoveEvent( QMouseEvent *e )
{
	setPosition( e->pos().x() - 2, e->pos().y() - 2 );

	emit valueChanged( xPos, yPos );
}

void KXYSelector::setPosition( int xp, int yp )
{
	QPainter painter;

	if ( xp < 2 )
		xp = 2;
	else if ( xp > width() - 2 )
		xp = width() - 2;

	if ( yp < 2 )
		yp = 2;
	else if ( yp > height() - 2 )
		yp = height() - 2;

	painter.begin( this );

	bitBlt( this, px - STORE_W, py - STORE_W, &store, 0, 0,
			STORE_W2, STORE_W2, CopyROP );
	bitBlt( &store, 0, 0, this, xp - STORE_W, yp - STORE_W,
			STORE_W2, STORE_W2, CopyROP );
	drawCursor( painter, xp, yp );
	px = xp;
	py = yp;

	painter.end();

	xPos = ( (maxX-minX) * (xp-2) ) / ( width()-4 );
	yPos = maxY - ( ( (maxY-minY) * (yp-2) ) / ( height()-4 ) );
	
	if ( xPos > maxX )
		xPos = maxX;
	else if ( xPos < minX )
		xPos = minX;
	
	if ( yPos > maxY )
		yPos = maxY;
	else if ( yPos < minY )
		yPos = minY;
}

void KXYSelector::drawCursor( QPainter &painter, int xp, int yp )
{
	painter.setPen( QPen( white ) );

	painter.setClipRect( contentsRect() );
	painter.setClipping( TRUE );
	painter.drawLine( xp - 6, yp - 6, xp - 2, yp - 2 );
	painter.drawLine( xp - 6, yp + 6, xp - 2, yp + 2 );
	painter.drawLine( xp + 6, yp - 6, xp + 2, yp - 2 );
	painter.drawLine( xp + 6, yp + 6, xp + 2, yp + 2 );
	painter.setClipping( FALSE );
}

//-----------------------------------------------------------------------------
/*
 * 1D value selector with contents drawn by derived class.
 * See KColorDialog for example.
 */


KSelector::KSelector( Orientation o, QWidget *parent, const char *name )
	: QWidget( parent, name ), QRangeControl()
{
	_orientation = o;
	_indent = TRUE;
}

QRect KSelector::contentsRect()
{
	if ( orientation() == Vertical )
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
		if ( orientation() == Vertical )
			qDrawShadePanel( &painter, 0, 3, width()-5, height()-6,
				colorGroup(), TRUE, 2, &brush );
		else
			qDrawShadePanel( &painter, 3, 0, width()-6, height()-5,
				colorGroup(), TRUE, 2, &brush );
	}

	QPoint pos = calcArrowPos( value() );
	drawArrow( painter, TRUE, pos );   

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

void KSelector::valueChange()
{
	QPainter painter;
	QPoint pos;

	painter.begin( this );

	pos = calcArrowPos( prevValue() );
	drawArrow( painter, FALSE, pos );   

	pos = calcArrowPos( value() );
	drawArrow( painter, TRUE, pos );   

	painter.end();
}

void KSelector::moveArrow( const QPoint &pos )
{
	int val;

	if ( orientation() == Vertical )
		val = ( maxValue() - minValue() ) * (height()-pos.y()-3)
				/ (height()-10) + minValue();
	else
		val = ( maxValue() - minValue() ) * (width()-pos.x()-3)
				/ (width()-10) + minValue();

	if ( val > maxValue() )
		val = maxValue();
	if ( val < minValue() )
		val = minValue();

	emit valueChanged( val );
	setValue( val );
}

QPoint KSelector::calcArrowPos( int val )
{
	QPoint p;

	if ( orientation() == Vertical )
	{
		p.setY( height() - ( (height()-10) * val
				/ ( maxValue() - minValue() ) + 5 ) );
		p.setX( width() - 5 );
	}
	else
	{
		p.setX( width() - ( (width()-10) * val
				/ ( maxValue() - minValue() ) + 5 ) );
		p.setY( height() - 5 );
	}

	return p;
}

void KSelector::drawArrow( QPainter &painter, bool show, const QPoint &pos )
{
	QPointArray array(3);

	if ( show )
	{
		painter.setPen( QPen() );
		painter.setBrush( QBrush( black ) );
	}
	else
	{
		painter.setPen( QPen( backgroundColor() ) );
		painter.setBrush( backgroundColor() );
	}

	if ( orientation() == Vertical )
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

	painter.drawPolygon( array );
}

//----------------------------------------------------------------------------

KGradientSelector::KGradientSelector( Orientation o, QWidget *parent,
		const char *name )
	: KSelector( o, parent, name )
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

	kFSDither dither( ditherPalette, 8 );
	QImage dImage = dither.dither( image );

	QPixmap p;
	p.convertFromImage( dImage );

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


