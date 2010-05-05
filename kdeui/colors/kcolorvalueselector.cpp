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

#include <QtGui/QPainter>

#include "kcolorchoosermode_p.h"

using namespace KDEPrivate;

class KColorValueSelector::Private
{
public:
    Private(KColorValueSelector *q): q(q), _hue(0), _sat(0), _colorValue(0), _mode(ChooserClassic) {}

    KColorValueSelector *q;
    int _hue;
    int _sat;
    int _colorValue;
    KColorChooserMode _mode;
    QPixmap pixmap;
};

KColorValueSelector::KColorValueSelector( QWidget *parent )
        : KSelector( Qt::Vertical, parent ), d( new Private( this ) )
{
    setRange( 0, 255 );
}

KColorValueSelector::KColorValueSelector( Qt::Orientation o, QWidget *parent )
        : KSelector( o, parent ), d( new Private( this ) )
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

void KColorValueSelector::setHue( int hue )
{
    d->_hue = hue;
}

int KColorValueSelector::saturation() const
{
    return d->_sat;
}

void KColorValueSelector::setSaturation( int saturation )
{
    d->_sat = saturation;
}

int KColorValueSelector::colorValue () const
{
    return d->_colorValue;
}

void KColorValueSelector::setColorValue ( int colorValue )
{
    d->_colorValue = colorValue;
}



void KColorValueSelector::updateContents()
{
    drawPalette( &d->pixmap );
}

void KColorValueSelector::resizeEvent( QResizeEvent * )
{
    updateContents();
}

void KColorValueSelector::drawContents( QPainter *painter )
{
    painter->drawPixmap( contentsRect().x(), contentsRect().y(), d->pixmap );
}

void KColorValueSelector::setChooserMode( KColorChooserMode c )
{
    if ( c == ChooserHue ) {
        setRange( 0, 360 );
    } else {
        setRange( 0, 255 );
    }
    d->_mode = c;

    //really needed?
    //emit modeChanged();
}

KColorChooserMode KColorValueSelector::chooserMode () const
{
    return d->_mode;
}

void KColorValueSelector::drawPalette( QPixmap *pixmap )
{
    QColor color;
    if (chooserMode() == ChooserHue) {
        color.setHsv(hue(), 255, 255);
    } else {
        color.setHsv(hue(), saturation(), colorValue());
    }

    QLinearGradient gradient;
    if (orientation() == Qt::Vertical) {
        gradient.setStart(0, contentsRect().height());
        gradient.setFinalStop(0, 0);
    } else {
        gradient.setStart(0, 0);
        gradient.setFinalStop(contentsRect().width(), 0);
    }

    const int steps = componentValueSteps(chooserMode());
    for (int v = 0; v <= steps; ++v) {
        setComponentValue(color, chooserMode(), v * (1.0 / steps));
        gradient.setColorAt(v * (1.0 / steps), color);
    }

    *pixmap = QPixmap(contentsRect().size());
    QPainter painter(pixmap);
    painter.fillRect(pixmap->rect(), gradient);
}


#include "kcolorvalueselector.moc"
