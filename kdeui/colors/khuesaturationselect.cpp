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

#include "khuesaturationselect.h"

#include <QtGui/QPainter>

#include "kcolorchoosermode_p.h"

using namespace KDEPrivate;

class KHueSaturationSelector::Private
{
public:
    Private(KHueSaturationSelector *q): q(q) {}

    KHueSaturationSelector *q;
    QPixmap pixmap;

    /**
     * Stores the chooser mode
     */
    KColorChooserMode _mode;

    /**
     * Stores the values for hue, saturation and lumniousity
     */
    int _hue, _sat, _colorValue;
};



KHueSaturationSelector::KHueSaturationSelector( QWidget *parent )
        : KXYSelector( parent ), d( new Private( this ) )
{
    setChooserMode( ChooserClassic );
}

KColorChooserMode KHueSaturationSelector::chooserMode() const
{
    return d->_mode;
}

void KHueSaturationSelector::setChooserMode( KColorChooserMode chooserMode )
{
    int x;
    int y = 255;

    switch ( chooserMode ) {
    case ChooserSaturation:
    case ChooserValue:
        x = 359;
        break;
    default:
        x = 255;
        break;
    }

    setRange( 0, 0, x, y );
    d->_mode = chooserMode;
}

int KHueSaturationSelector::hue () const
{
    return d->_hue;
}

void KHueSaturationSelector::setHue ( int hue )
{
    d->_hue = hue;
}

int KHueSaturationSelector::saturation () const

{
    return d->_sat;
}

void KHueSaturationSelector::setSaturation( int saturation )
{
    d->_sat = saturation;
}

int KHueSaturationSelector::colorValue() const
{
    return d->_colorValue;
}

void KHueSaturationSelector::setColorValue( int colorValue )
{
    d->_colorValue = colorValue;
}

KHueSaturationSelector::~KHueSaturationSelector()
{
    delete d;
}

void KHueSaturationSelector::updateContents()
{
    drawPalette( &d->pixmap );
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
    int xSteps = componentXSteps(chooserMode());
    int ySteps = componentYSteps(chooserMode());

    QColor color;
    color.setHsv(hue(), saturation(), chooserMode() == ChooserClassic ? 192 : colorValue());

    QImage image(QSize(xSteps + 1, ySteps + 1), QImage::Format_RGB32);
    for (int y = 0; y <= ySteps; ++y) {
        setComponentY(color, chooserMode(), y * (1.0 / ySteps));
        for (int x = 0; x <= xSteps; ++x) {
            setComponentX(color, chooserMode(), x * (1.0 / xSteps));
            image.setPixel(x, ySteps - y, color.rgb());
        }
    }

    QPixmap pix(contentsRect().size());
    QPainter painter(&pix);
    // Bilinear filtering
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QRectF srcRect(0.5, 0.5, xSteps, ySteps);
    QRectF destRect(QPointF(0, 0), contentsRect().size());
    painter.drawImage(destRect, image, srcRect);
    painter.end();

    *pixmap = pix;
}

#include "khuesaturationselect.moc"
