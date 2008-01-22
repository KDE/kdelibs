/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kratingwidget.h"
#include "ratingpainter.h"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QKeyEvent>
#include <QtGui/QImage>

#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

class KRatingWidget::Private
{
public:
    Private()
        : rating(0),
          hoverRating(-1),
          spacing(2),
          pixSize( KIconLoader::Small ) {
    }

    unsigned int rating;
    int hoverRating;

    // GUI
    int spacing;

    int pixSize;

    Nepomuk::RatingPainter ratingPainter;
};



KRatingWidget::KRatingWidget( QWidget* parent )
    : QFrame( parent ),
      d( new Private() )
{
    setPixmapSize( 16 );
    setMouseTracking( true );
}


KRatingWidget::~KRatingWidget()
{
    delete d;
}


void KRatingWidget::setPixmap( const QPixmap& pix )
{
    d->ratingPainter.setCustomPixmap( pix );
    update();
}


void KRatingWidget::setPixmapSize( int size )
{
    d->pixSize = size;
    update();
}


int KRatingWidget::spacing() const
{
    return d->spacing;
}


void KRatingWidget::setSpacing( int s )
{
    d->spacing = s;
    update();
}


unsigned int KRatingWidget::rating() const
{
    return d->rating;
}


void KRatingWidget::setRating( unsigned int rating )
{
    d->rating = rating;
    d->hoverRating = rating;
    update();
}


void KRatingWidget::setMaxRating( unsigned int max )
{
    d->ratingPainter.setMaxRating( max );
    update();
}


void KRatingWidget::setOnlyPaintFullSteps( bool fs )
{
    d->ratingPainter.setHalfStepsEnabled( !fs );
    update();
}


void KRatingWidget::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton ) {
        d->hoverRating = d->rating = d->ratingPainter.fromPosition( contentsRect(), e->pos() );
        update();
        emit ratingChanged( d->rating );
    }
}


void KRatingWidget::mouseMoveEvent( QMouseEvent* e )
{
    // when moving the mouse we show the user what the result of clicking will be
    d->hoverRating = d->ratingPainter.fromPosition( contentsRect(), e->pos() );
    if ( d->hoverRating >= 0 && e->buttons() & Qt::LeftButton ) {
        d->rating = d->hoverRating;
        emit ratingChanged( d->rating );
    }
    update();
}


void KRatingWidget::leaveEvent( QEvent* )
{
    d->hoverRating = -1;
    update();
}


void KRatingWidget::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    d->ratingPainter.draw( &p, contentsRect(), d->rating, d->hoverRating );
}


QSize KRatingWidget::minimumSizeHint() const
{
    int numPix = d->ratingPainter.maxRating();
    if( d->ratingPainter.halfStepsEnabled() )
        numPix /= 2;

    QSize pixSize( d->pixSize, d->pixSize );
    if ( !d->ratingPainter.customPixmap().isNull() ) {
        pixSize = d->ratingPainter.customPixmap().size();
    }

    return QSize( pixSize.width()*numPix + spacing()*(numPix-1) + frameWidth()*2,
                  pixSize.height() + frameWidth()*2 );
}


void KRatingWidget::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );
}

#include "kratingwidget.moc"
