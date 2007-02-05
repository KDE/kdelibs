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

#include "kcolorvalueselector.h"
#include <kimageeffect.h>
#include <QPainter>
#include <kstaticdeleter.h>

class KColorValueSelector::Private
{
public:
  Private(KColorValueSelector *q): q(q), _hue(0), _sat(0) {}
  
  KColorValueSelector *q;
  int _hue;
  int _sat;
  QPixmap pixmap;
};

KColorValueSelector::KColorValueSelector( QWidget *parent )
	: KSelector( Qt::Vertical, parent ), d(new Private(this))
{
	setRange( 0, 255 );
}

KColorValueSelector::KColorValueSelector(Qt::Orientation o, QWidget *parent )
	: KSelector( o, parent ), d(new Private(this))
{
	setRange( 0, 255 );
}

KColorValueSelector::~KColorValueSelector()
{
  delete d;
}

int KColorValueSelector::hue() const
{
  return d->_hue;
}

void KColorValueSelector::setHue( int h )
{
  d->_hue = h;
}

int KColorValueSelector::saturation() const
{
  return d->_sat;
}

void KColorValueSelector::setSaturation( int s )
{
  d->_sat = s;
}

void KColorValueSelector::updateContents()
{
	drawPalette(&d->pixmap);
}

void KColorValueSelector::resizeEvent( QResizeEvent * )
{
	updateContents();
}

void KColorValueSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), d->pixmap );
}

void KColorValueSelector::drawPalette( QPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( QSize(xSize, ySize), QImage::Format_RGB32 );
	QColor col;
	uint *p;
	QRgb rgb;

	if ( orientation() == Qt::Horizontal )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );

			for( int x = 0; x < xSize; x++ )
			{
				col.setHsv( d->_hue, d->_sat, 255*x/((xSize == 1) ? 1 : xSize-1) );
				rgb = col.rgb();
				*p++ = rgb;
			}
		}
	}

	if( orientation() == Qt::Vertical )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );
			col.setHsv( d->_hue, d->_sat, 255*v/((ySize == 1) ? 1 : ySize-1) );
			rgb = col.rgb();
			for ( int i = 0; i < xSize; i++ )
				*p++ = rgb;
		}
	}

	if ( pixmap->depth() <= 8 )
	{
		extern QVector<QColor> kdeui_standardPalette();
                const QVector<QColor> standardPalette = kdeui_standardPalette();
		KImageEffect::dither( image, standardPalette.data(), standardPalette.size() );
	}
	*pixmap=QPixmap::fromImage( image );
}

#include "kcolorvalueselector.moc"
