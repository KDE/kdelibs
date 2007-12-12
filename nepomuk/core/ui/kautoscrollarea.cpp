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

#include "kautoscrollarea.h"

#include <kdebug.h>

#include <QtCore/QTimer>
#include <QtGui/QSizePolicy>
#include <QtCore/QSize>
#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QPainter>

#include <math.h>


class KAutoScrollArea::Private
{
public:
    Private( KAutoScrollArea* _parent )
        : widget( 0 ),
          orientation( Qt::Horizontal ),
          fadeSize( 20 ),
          scrollSpeed( 0 ),
          parent( _parent ) {
    }

    void setWidget( QWidget* w ) {
        if ( w != widget ) {
            delete widget;
            widget = w;
            widget->setParent( parent );
            widget->installEventFilter( parent );

            // now grab all the paint events from all the children
            grabChildren( widget );
        }
    }

    void grabChildren( QObject* o ) {
        const QObjectList& children = o->children();
        for( QObjectList::const_iterator it = children.constBegin(); it != children.constEnd(); ++it ) {
            QObject* child = *it;
            if ( child->isWidgetType() ) {
                child->installEventFilter( parent );
                qobject_cast<QWidget*>( child )->setMouseTracking( true );
                grabChildren( child );
            }
        }
    }

    void setupWidget() {
        // update the size policy to fit the child widget
        // and the orientation
        QSizePolicy sp( widget->sizePolicy() );
        if ( orientation == Qt::Horizontal )
            sp.setHorizontalPolicy( QSizePolicy::Expanding );
        else
            sp.setVerticalPolicy( QSizePolicy::Expanding );
        parent->setSizePolicy( sp );

        // resize the child according to our policy
        // ie. fill at least the complete viewport
        QSize s( widget->sizeHint() );
        if ( orientation == Qt::Horizontal ) {
            s.setHeight( parent->height() );
            s.setWidth( qMax( s.width(), parent->width() ) );
        }
        else {
            s.setWidth( parent->width() );
            s.setHeight( qMax( s.height(), parent->height() ) );
        }
        widget->setGeometry( QRect( QPoint( 0, 0 ), s ) );

        // mainly for QLabel which disables mouse tracking itself
        widget->setMouseTracking( true );
    }

    void startAutoScroll() {
        if ( scrollSpeed != 0 ) {
            if ( !scrollTimer->isActive() )
                scrollTimer->start();
        }
        else
            stopAutoScroll();
    }

    void stopAutoScroll() {
        scrollTimer->stop();
    }

    QWidget* widget;
    Qt::Orientation orientation;
    int fadeSize;
    int scrollSpeed;
    QTimer* scrollTimer;

private:
    KAutoScrollArea* parent;
};


KAutoScrollArea::KAutoScrollArea( QWidget* parent )
    : QWidget( parent )
{
    d = new Private( this );
    d->scrollTimer = new QTimer( this );
    d->scrollTimer->setSingleShot( true );
    connect( d->scrollTimer,  SIGNAL( timeout() ), this, SLOT( slotScroll() ) );
    installEventFilter( this );
    setMouseTracking( true );
}


KAutoScrollArea::~KAutoScrollArea()
{
    delete d;
}


QSize KAutoScrollArea::minimumSizeHint() const
{
    if ( d->widget ) {
        if ( d->orientation == Qt::Horizontal )
            return QSize( d->fadeSize > 0 ? d->fadeSize*2 : 20, d->widget->minimumSizeHint().height() );
        else
            return QSize( d->widget->minimumSizeHint().width(), d->fadeSize > 0 ? d->fadeSize*2 : 20 );
    }
    else
        return QSize();
}


QSize KAutoScrollArea::sizeHint() const
{
    if ( d->widget )
        return d->widget->sizeHint();
    else
        return QSize();
}


void KAutoScrollArea::setWidget( QWidget* w )
{
    d->setWidget( w );
    d->setupWidget();
}


void KAutoScrollArea::setFadeWidth( int w )
{
    d->fadeSize = w;
}


void KAutoScrollArea::setOrientation( Qt::Orientation o )
{
    d->orientation = o;
    d->setupWidget();
}


bool KAutoScrollArea::eventFilter( QObject* o, QEvent* e )
{
    if ( e->type() == QEvent::MouseMove ) {
        QMouseEvent* me = static_cast<QMouseEvent*>( e );
        QWidget* w = qobject_cast<QWidget*>( o );

        QPoint pos = me->pos();
        if ( o != this )
            pos = w->mapTo( this, me->pos() );

        if ( me->buttons() ) {
            // TODO: scroll manually

        }
        else {
            // we define the max speed to be 1 step every 2ms
            // and the min to be 1 step every 100ms
            if ( d->orientation == Qt::Horizontal ) {
                if ( pos.x() < width()/2 )
                    d->scrollSpeed = -2 - 100*pos.x()/( width()/2 );
                else if ( pos.x() > width()/2 )
                    d->scrollSpeed = 2 + ( 100 - 100*( pos.x()-width()/2 )/( width()/2 ) );
            }
            else {
                if ( pos.y() < height()/2 )
                    d->scrollSpeed = -2 - 100*pos.y()/( height()/2 );
                else if ( pos.y() > height()/2 )
                    d->scrollSpeed = 2 + ( 100 - 100*( pos.y()-height()/2 )/( height()/2 ) );
            }

            d->startAutoScroll();
        }
    }

    else if ( e->type() == QEvent::MouseButtonPress ||
              e->type() == QEvent::Leave ) {
        d->stopAutoScroll();
    }

    else if ( e->type() == QEvent::MouseButtonRelease ) {
        d->startAutoScroll();
    }

    else if ( e->type() == QEvent::LayoutRequest ) {
        // the child widget changed and maybe also its children
        d->grabChildren( d->widget );
        d->setupWidget();
    }

    // Brute force: we just paint the fading effect over each and every child widget
    // This is the only solution I found since child widgets are painted over their parents
    else if ( e->type() == QEvent::Paint &&
              o != this ) {
        // paint the child widgets
        o->event( e );

        if ( d->fadeSize <= 0 )
            return true;

        // and now do some fancy fading out effect
        QColor bg( d->widget->palette().color( QPalette::Background ) );
        QColor ac( bg );
        ac.setAlpha( 0 );

        QWidget* w = qobject_cast<QWidget*>( o );

        QPainter p( w );

        if ( d->orientation == Qt::Horizontal ) {
            if ( d->widget->geometry().right() > rect().right() ) {
                QRect r( w->mapFrom( this, QPoint( width()-d->fadeSize, 0 ) ), QSize( d->fadeSize, height() ) );
                QLinearGradient g( QPoint( r.left(), height()/2 ), QPoint( r.right(), height()/2 ) );
                g.setColorAt( 0.0, ac );
                g.setColorAt( 1.0, bg );
                p.fillRect( r, g );
            }
            if ( d->widget->geometry().left() < rect().left() ) {
                QRect r( w->mapFrom( this, QPoint( 0, 0 ) ), QSize( d->fadeSize, height() ) );
                QLinearGradient g( QPoint( r.left(), height()/2 ), QPoint( r.right(), height()/2 ) );
                g.setColorAt( 0.0, bg );
                g.setColorAt( 1.0, ac );
                p.fillRect( r, g );
            }
        }
        else {
            if ( d->widget->geometry().bottom() > rect().bottom() ) {
                QRect r( w->mapFrom( this, QPoint( 0, height()-d->fadeSize ) ), QSize( width(), d->fadeSize ) );
                QLinearGradient g( QPoint( this->width()/2, r.top() ), QPoint( this->width()/2, r.bottom() ) );
                g.setColorAt( 0.0, ac );
                g.setColorAt( 1.0, bg );
                p.fillRect( r, g );
            }
            if ( d->widget->geometry().top() < rect().top() ) {
                QRect r( w->mapFrom( this, QPoint( 0, 0 ) ), QSize( width(), d->fadeSize ) );
                QLinearGradient g( QPoint( this->width()/2, r.top() ), QPoint( this->width()/2, r.bottom() ) );
                g.setColorAt( 0.0, bg );
                g.setColorAt( 1.0, ac );
                p.fillRect( r, g );
            }
        }

        return true;
    }

    return QWidget::eventFilter( o, e );
}


void KAutoScrollArea::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
    if ( d->widget )
        d->setupWidget();
}


void KAutoScrollArea::slotScroll()
{
    static const int step = 1;
    if ( d->orientation == Qt::Horizontal ) {
        if ( d->scrollSpeed > 0 &&
             d->widget->geometry().right() > rect().right() )
            d->widget->move( d->widget->geometry().left() - step, 0 );
        else if ( d->scrollSpeed < 0 &&
                  d->widget->geometry().left() < rect().left() )
            d->widget->move( d->widget->geometry().left() + step, 0 );
    }
    else {
        if ( d->scrollSpeed > 0 &&
             d->widget->geometry().bottom() > rect().bottom() )
            d->widget->move( 0, d->widget->geometry().top() - step );
        else if ( d->scrollSpeed < 0 &&
                  d->widget->geometry().top() < rect().top() )
            d->widget->move( 0, d->widget->geometry().top() + step );
    }

    d->scrollTimer->setInterval( abs( d->scrollSpeed ) );
    d->scrollTimer->start();
}

#include "kautoscrollarea.moc"
