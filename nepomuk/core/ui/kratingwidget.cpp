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
        : max(10),
          onlyFullSteps(false),
          rating(0),
          hoverRating(0),
          hovering(false),
          spacing(2),
          pixSize( KIconLoader::Small ),
          pixmapForced( false ) {
    }

    void setUsedPixmap( const QPixmap& _pix );

    const QPixmap& pixmap();
    const QPixmap& smallPixmap();
    const QPixmap& hoverSmallPixmap();
    const QPixmap& grayPixmap();
    const QPixmap& hoverPixmap();

    int numUsedPix() const {
        return ( onlyFullSteps ? max : max/2 );
    }

    unsigned int max;
    bool onlyFullSteps;

    unsigned int rating;
    unsigned int hoverRating;
    bool hovering;

    // GUI
    int spacing;

    int pixSize;
    QPixmap origPix;
    QPixmap pix;
    QPixmap smallPix;
    QPixmap hoverSmallPix;
    QPixmap grayPix;
    QPixmap hoverPix;

    bool pixmapForced;
};


void KRatingWidget::Private::setUsedPixmap( const QPixmap& _pix )
{
    pix = _pix;
    smallPix = QPixmap();
    hoverSmallPix = QPixmap();
    grayPix = QPixmap();
    hoverPix = QPixmap();
}


const QPixmap& KRatingWidget::Private::pixmap()
{
        // FIXME: maybe resize according to widget size
        //        maybe even use SVG?
    if( pix.isNull() )
        pix = origPix;
    return pix;
}


const QPixmap& KRatingWidget::Private::smallPixmap()
{
    if( smallPix.isNull() )
        smallPix = pixmap().scaled( pixmap().height()/2, pixmap().width()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    return smallPix;
}


const QPixmap& KRatingWidget::Private::hoverSmallPixmap()
{
    if( hoverSmallPix.isNull() ) {
        hoverSmallPix = pixmap().scaled( pixmap().height()/2, pixmap().width()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        QImage image = hoverSmallPix.toImage();
        KIconEffect::toGray( image, 0.5 ); // maybe Blitz::grayscale() would be better
        hoverSmallPix = QPixmap::fromImage( image );
    }
    return hoverSmallPix;
}

const QPixmap& KRatingWidget::Private::grayPixmap()
{
    if( grayPix.isNull() ) {
        QImage image = pixmap().toImage();
        KIconEffect::toGray( image, 1.0 ); // maybe Blitz::grayscale() would be better
        grayPix = QPixmap::fromImage( image );
    }
    return grayPix;
}


const QPixmap& KRatingWidget::Private::hoverPixmap()
{
    if( hoverPix.isNull() ) {
        QImage image = pixmap().toImage();
        KIconEffect::toGray( image, 0.5 );
        hoverPix = QPixmap::fromImage( image );
    }
    return hoverPix;
}


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
    d->origPix = pix;
    d->pixmapForced = true;
    d->setUsedPixmap( pix );
    update();
}


void KRatingWidget::setPixmapSize( int size )
{
    if ( !d->pixmapForced ) {
        // try to load the new size
        QPixmap pix = KIconLoader::global()->loadIcon( "rating", KIconLoader::NoGroup, size, KIconLoader::DefaultState, QStringList(), 0L, true );
        if ( !pix.isNull() ) {
            d->setUsedPixmap( pix );
            update();
            return;
        }
        else {
            // fallback icon
            d->origPix.load( KStandardDirs::locate( "data", "nepomuk/pics/rating.png" ) );
        }
    }

    // resize the pix
    d->setUsedPixmap( d->origPix.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
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
    d->max = max;
    if( max%2 )
        d->onlyFullSteps = true;
    if( d->rating > max )
        d->rating = max;
    update();
}


void KRatingWidget::setOnlyPaintFullSteps( bool fs )
{
    if( !(d->max%2) )
        d->onlyFullSteps = fs;
    update();
}


void KRatingWidget::mousePressEvent( QMouseEvent* e )
{
    setRating( posToRating( e->pos().x() ) );
    emit ratingChanged( rating() );
    update();
}


void KRatingWidget::mouseMoveEvent( QMouseEvent* e )
{
    // when moving the mouse we show the user what the result of clicking will be
    d->hovering = true;
    d->hoverRating = posToRating( e->pos().x() );
    update();
}


void KRatingWidget::leaveEvent( QEvent* )
{
    d->hovering = false;
    update();
}


void KRatingWidget::paintEvent( QPaintEvent* )
{
    const int rating = d->rating;
    const int hoverRating = d->hovering ? d->hoverRating : 0;

    QPainter p( this );

    if( rating > 0 ) {
        if( d->onlyFullSteps )
            drawRatingPixmaps( &p, rating, hoverRating, d->numUsedPix(), false );
        else
            drawRatingPixmaps( &p, (rating+1)/2, (hoverRating+1)/2, d->numUsedPix(), rating%2 );
    }
    else {
        drawRatingPixmaps( &p, 0, 0, d->numUsedPix(), false );
        p.drawText( contentsRect(), Qt::AlignCenter, i18n("Not yet rated") );
    }
}


// inspired by Amarok's playlistitem.cpp:
// (C) 2005 by Gav Wood
// (C) 2005 by Alexandre Oliveira
void KRatingWidget::drawRatingPixmaps( QPainter* p, int pix, int hoverPix, int grayPix, bool half )
{
    bool drawHoverSmallPix = false;
    if ( ( hoverPix > 0 ) && ( hoverPix < pix ) ) {
        int temp = pix;
        pix = hoverPix;
        hoverPix = temp;
        if ( half) {
            ++pix;
            drawHoverSmallPix = true;
        }
    }

    int w = contentsRect().width();
    w -= d->numUsedPix()*d->pixmap().width();
    int usedSpacing = w / (d->numUsedPix()-1);

    int i = 1, x = 1;
    int xInc = d->pixmap().width() + usedSpacing;
    if ( layoutDirection() == Qt::RightToLeft ) {
        x = contentsRect().width() - d->pixmap().width() - x;
        xInc = -xInc;
    }

    const int y = contentsRect().height() / 2 - d->pixmap().height() / 2;
    if( half )
        i++;
    for(; i <= pix; ++i ) {
        p->drawPixmap( x, y, d->pixmap() );
        x += xInc;
    }
    if( half && !drawHoverSmallPix ) {
        const QPixmap& pix = d->smallPixmap();
        p->drawPixmap( x + ( d->pixmap().width() - pix.width() ) / 2,
                       y + ( d->pixmap().height() - pix.height() ) / 2, pix );
        x += xInc;
    }
    for (; i <= hoverPix; ++i ) {
        p->drawPixmap( x, y, d->hoverPixmap() );
        x += xInc;
    }
    if( half && drawHoverSmallPix) {
        const QPixmap& pix = d->hoverSmallPixmap();
        p->drawPixmap( x + ( d->pixmap().width() - pix.width() ) / 2,
                       y + ( d->pixmap().height() - pix.height() ) / 2, pix );
        x += xInc;
    }
    for(; i <= grayPix; ++i ) {
        p->drawPixmap( x, y, d->grayPixmap() );
        x += xInc;
    }
}

unsigned int KRatingWidget::posToRating( int pos ) const
{
    int w = contentsRect().width();
    if ( layoutDirection() == Qt::RightToLeft )
        pos = w - pos;

    int usedSpacing = (w - d->numUsedPix()*d->pixmap().width()) / (d->numUsedPix()-1);

    //  int pixW = ( w - ((d->numUsedPix()-1)*spacing()) ) / d->numUsedPix();
    int pixW = d->pixmap().width();

    unsigned int r = 0;
    while( pos > 0 ) {
        if( r > 0 )
            pos -= usedSpacing/2;
        pos -= pixW;
        pos -= usedSpacing/2;
        ++r;
    }

    if( !d->onlyFullSteps ) {
        r*=2;

        // we always return the rating that results from the next mouse
        // click on this position. Thus, if the rating is r the next
        // mouse click will change it to a half step
        if( rating() == r )
            r-=1;
    }

    return r;
}


QSize KRatingWidget::minimumSizeHint() const
{
    int numPix = d->max;
    if( !d->onlyFullSteps )
        numPix /= 2;

    return QSize( d->pixmap().width()*numPix + spacing()*(numPix-1) + frameWidth()*2,
                  d->pixmap().height() + frameWidth()*2 );
}


void KRatingWidget::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );
}

#include "kratingwidget.moc"
