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

#include <QPainter>
#include <kstaticdeleter.h>
#include <kimageeffect.h>
#include "khuesaturationselect.h"


static QVector<QColor> *s_standardPalette = 0;
static KStaticDeleter<QVector<QColor> > spd;

// Shared with KColorValueSelector
KDEUI_EXPORT QVector<QColor> kdeui_standardPalette()
{
    if ( !s_standardPalette ) {
        spd.setObject(s_standardPalette, new QVector<QColor>);

        int i = 0;
#ifndef STANDARD_PAL_SIZE
#define STANDARD_PAL_SIZE 17
#endif
        s_standardPalette->resize( STANDARD_PAL_SIZE );

        (*s_standardPalette)[i++] = Qt::red;
        (*s_standardPalette)[i++] = Qt::green;
        (*s_standardPalette)[i++] = Qt::blue;
        (*s_standardPalette)[i++] = Qt::cyan;
        (*s_standardPalette)[i++] = Qt::magenta;
        (*s_standardPalette)[i++] = Qt::yellow;
        (*s_standardPalette)[i++] = Qt::darkRed;
        (*s_standardPalette)[i++] = Qt::darkGreen;
        (*s_standardPalette)[i++] = Qt::darkBlue;
        (*s_standardPalette)[i++] = Qt::darkCyan;
        (*s_standardPalette)[i++] = Qt::darkMagenta;
        (*s_standardPalette)[i++] = Qt::darkYellow;
        (*s_standardPalette)[i++] = Qt::white;
        (*s_standardPalette)[i++] = Qt::lightGray;
        (*s_standardPalette)[i++] = Qt::gray;
        (*s_standardPalette)[i++] = Qt::darkGray;
        (*s_standardPalette)[i++] = Qt::black;
    }
    return *s_standardPalette;
}


class KHueSaturationSelector::Private
{
public:
  Private(KHueSaturationSelector *q): q(q) {}
  
  KHueSaturationSelector *q;
  QPixmap pixmap;
};

KHueSaturationSelector::KHueSaturationSelector( QWidget *parent )
	: KXYSelector( parent ), d(new Private(this))
{
	setRange( 0, 0, 359, 255 );
	setChooserMode(ChooserClassic);
}

void KHueSaturationSelector::setChooserMode( KColorChooserMode c )
{
	int x;
	int y = 255;
	
	switch (c) {
		case ChooserSaturation:
		case ChooserValue:
			x = 359;
			break;
		default:
			x = 255;
			break;
	}
	
	setRange( 0, 0, x, y );
	_mode = c;
}

KHueSaturationSelector::~KHueSaturationSelector()
{
	delete d;
}

void KHueSaturationSelector::updateContents()
{
	drawPalette(&d->pixmap);
}

void KHueSaturationSelector::resizeEvent( QResizeEvent * )
{
	updateContents();
}

void KHueSaturationSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), d->pixmap );
}

void KHueSaturationSelector::drawPalette( QPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( QSize(xSize, ySize), QImage::Format_RGB32 );
	QColor col;
	int h, s;
	uint *p;

	col.setHsv(hue(), saturation(), colorValue());

	int _h, _s, _v, _r, _g, _b;
	col.getHsv(&_h, &_s, &_v);
	col.getRgb(&_r, &_g, &_b);

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
			switch (chooserMode()) {
				case ChooserClassic:
				default:
					col.setHsv( 359*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1), 192 );
					break;
				case ChooserHue:
					col.setHsv( _h, 255*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1));
					break;
				case ChooserSaturation:
					col.setHsv( 255*h/(xSize-1), _s, 255*s/((ySize == 1) ? 1 : ySize-1));
					break;
				case ChooserValue:
					col.setHsv( 255*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1), _v);
					break;
				case ChooserRed:
					col.setRgb( _r, 255*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1));
					break;
				case ChooserGreen:
					col.setRgb( 255*h/(xSize-1), _g, 255*s/((ySize == 1) ? 1 : ySize-1));
					break;
				case ChooserBlue:
					col.setRgb( 255*s/((ySize == 1) ? 1 : ySize-1), 255*h/(xSize-1), _b);
					break;
			}
			
			*p = col.rgb();
			p++;
		}
	}

	if ( pixmap->depth() <= 8 )
	{
          const QVector<QColor> standardPalette = kdeui_standardPalette();
		  KImageEffect::dither( image, standardPalette.data(), standardPalette.size() );
	}
	*pixmap=QPixmap::fromImage( image );
}

#include "khuesaturationselect.moc"
