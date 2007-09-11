/* This file is part of the KDE libraries
   Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
          pixSize( K3Icon::Small ),
          pixmapForced( false ) {
    }

    void setUsedPixmap( const QPixmap& _pix ) {
        pix = _pix;
        smallPix = QPixmap();
        grayPix = QPixmap();
    }

    const QPixmap& pixmap() {
        // FIXME: maybe resize according to widget size
        //        maybe even use SVG?
        if( pix.isNull() )
            pix = origPix;
        return pix;
    }

    const QPixmap& smallPixmap() {
        if( smallPix.isNull() )
            smallPix = pixmap().scaled( pixmap().height()/2, pixmap().width()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        return smallPix;
    }

    const QPixmap& grayPixmap() {
        if( grayPix.isNull() ) {
            QImage image = pixmap().toImage();
            KIconEffect::toGray( image, 1.0 ); // maybe Blitz::grayscale() would be better
            grayPix = QPixmap::fromImage( image );
        }
        return grayPix;
    }

    int numUsedPix() {
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
    QPixmap grayPix;

    bool pixmapForced;
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
    d->origPix = pix;
    d->pixmapForced = true;
    d->setUsedPixmap( pix );
    update();
}


void KRatingWidget::setPixmapSize( int size )
{
    if ( !d->pixmapForced ) {
        // try to load the new size
        QPixmap pix = KIconLoader::global()->loadIcon( "rating", K3Icon::NoGroup, size, K3Icon::DefaultState, QStringList(), 0L, true );
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
    int r = ( d->hovering ? d->hoverRating : d->rating );

    QPainter p( this );

    if( r > 0 ) {
        if( !d->onlyFullSteps )
            drawRatingPixmaps( &p, (r+1)/2, d->numUsedPix(), r%2 );
        else
            drawRatingPixmaps( &p, r, d->numUsedPix(), false );
    }
    else {
        drawRatingPixmaps( &p, 0, d->numUsedPix(), false );
        p.drawText( contentsRect(), Qt::AlignCenter, i18n("Not yet rated") );
    }
}


// inspired by Amarok's playlistitem.cpp:
// (C) 2005 by Gav Wood
// (C) 2005 by Alexandre Oliveira
void KRatingWidget::drawRatingPixmaps( QPainter* p, int pix, int grayPix, bool half )
{
    int w = contentsRect().width();
    w -= d->numUsedPix()*d->pixmap().width();
    int usedSpacing = w / (d->numUsedPix()-1);

    int i = 1, x = 1;
    const int y = contentsRect().height() / 2 - d->pixmap().height() / 2;
    if( half )
        i++;
    for(; i <= pix; ++i ) {
        //bitBlt( p->device(), x, y, star() );
        p->drawPixmap( x, y, d->pixmap() );
        x += d->pixmap().width() + usedSpacing;
    }
    if( half ) {
        //bitBlt( p->device(), x, y, smallStar() );
        p->drawPixmap( x + ( d->pixmap().width() - d->smallPixmap().width() ) / 2,
                       y + ( d->pixmap().height() - d->smallPixmap().height() ) / 2, d->smallPixmap() );
        x += d->pixmap().width() + usedSpacing;
    }
    for(; i <= grayPix; ++i ) {
        //      bitBlt( p->device(), x, y, grayedStar() );
        p->drawPixmap( x, y, d->grayPixmap() );
        x += d->pixmap().width() + usedSpacing;
    }
}


unsigned int KRatingWidget::posToRating( int pos ) const
{
    int w = contentsRect().width();

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
