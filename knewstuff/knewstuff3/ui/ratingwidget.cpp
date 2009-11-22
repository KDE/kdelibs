/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 * Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>
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

#include "ratingwidget.h"
#include "ratingpainter.h"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QKeyEvent>
#include <QtGui/QImage>
#include <QtGui/QIcon>

#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

using namespace KNS3;

class RatingWidget::Private
{
public:
    Private()
        : rating(0),
          hoverRating(-1),
          pixSize( 16 ),
          editable(true) {
    }

    int rating;
    int hoverRating;
    int pixSize;
    bool editable;

    RatingPainter ratingPainter;
};



RatingWidget::RatingWidget( QWidget* parent )
    : QFrame( parent ),
      d( new Private() )
{
    setMouseTracking( true );
}


RatingWidget::~RatingWidget()
{
    delete d;
}


void RatingWidget::setPixmap( const QPixmap& pix )
{
    setCustomPixmap( pix );
}


void RatingWidget::setCustomPixmap( const QPixmap& pix )
{
    d->ratingPainter.setCustomPixmap( pix );
    update();
}


void RatingWidget::setIcon( const QIcon& icon )
{
    d->ratingPainter.setIcon( icon );
    update();
}


void RatingWidget::setPixmapSize( int size )
{
    d->pixSize = size;
    updateGeometry();
}


int RatingWidget::spacing() const
{
    return d->ratingPainter.spacing();
}


QIcon RatingWidget::icon() const
{
    return d->ratingPainter.icon();
}


void RatingWidget::setSpacing( int s )
{
    d->ratingPainter.setSpacing( s );
    update();
}


Qt::Alignment RatingWidget::alignment() const
{
    return d->ratingPainter.alignment();
}


void RatingWidget::setAlignment( Qt::Alignment align )
{
    d->ratingPainter.setAlignment( align );
    update();
}


Qt::LayoutDirection RatingWidget::layoutDirection() const
{
    return d->ratingPainter.layoutDirection();
}


void RatingWidget::setLayoutDirection( Qt::LayoutDirection direction )
{
    d->ratingPainter.setLayoutDirection( direction );
    update();
}


unsigned int RatingWidget::rating() const
{
    return d->rating;
}


int RatingWidget::maxRating() const
{
    return d->ratingPainter.maxRating();
}


bool RatingWidget::halfStepsEnabled() const
{
    return d->ratingPainter.halfStepsEnabled();
}


void RatingWidget::setRating( unsigned int rating )
{
    setRating( (int)rating );
}


void RatingWidget::setRating( int rating )
{
    d->rating = rating;
    d->hoverRating = rating;
    update();
}


void RatingWidget::setMaxRating( unsigned int max )
{
    setMaxRating( (int)max );
}


void RatingWidget::setMaxRating( int max )
{
    d->ratingPainter.setMaxRating( max );
    update();
}


void RatingWidget::setHalfStepsEnabled( bool enabled )
{
    d->ratingPainter.setHalfStepsEnabled( enabled );
    update();
}


void RatingWidget::setOnlyPaintFullSteps( bool fs )
{
    setHalfStepsEnabled( !fs );
}

void RatingWidget::setEditable(bool editable)
{
    d->editable = editable;
    setMouseTracking( editable );
}

bool RatingWidget::isEditable()
{
    return d->editable;
}

void RatingWidget::mousePressEvent( QMouseEvent* e )
{
    if (!d->editable) {
        return;
    }
    if ( e->button() == Qt::LeftButton ) {
        d->hoverRating = d->rating = d->ratingPainter.ratingFromPosition( contentsRect(), e->pos() );
        update();
        emit ratingChanged( d->rating );
        emit ratingChanged( (unsigned int)d->rating );
    }
}


void RatingWidget::mouseMoveEvent( QMouseEvent* e )
{
    if ( !d->editable ) {
        return;
    }
    // when moving the mouse we show the user what the result of clicking will be
    d->hoverRating = d->ratingPainter.ratingFromPosition( contentsRect(), e->pos() );
    if ( d->hoverRating >= 0 && e->buttons() & Qt::LeftButton ) {
        d->rating = d->hoverRating;
        emit ratingChanged( d->rating );
        emit ratingChanged( (unsigned int)d->rating );
    }
    update();
}


void RatingWidget::leaveEvent( QEvent* )
{
    d->hoverRating = -1;
    update();
}


void RatingWidget::paintEvent( QPaintEvent* e )
{
    QFrame::paintEvent( e );
    QPainter p( this );
    d->ratingPainter.setEnabled( isEnabled() );
    d->ratingPainter.paint( &p, contentsRect(), d->rating, d->hoverRating );
}


QSize RatingWidget::sizeHint() const
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


void RatingWidget::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );
}

#include "ratingwidget.moc"
