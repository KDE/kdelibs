/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#include <qpainter.h>
#include <qstyle.h>
#include <qtoolbutton.h>

#include <kglobalsettings.h>
#include <kiconloader.h>

#include "ktabbar.h"
#include "ktabwidget.h"

KTabBar::KTabBar( QWidget *parent, const char *name )
    : QTabBar( parent, name ), reorderStartTab( -1 ), previousTabIndex( -1 ), b( 0 ), btab( 0 ), mHoverCloseButton( false )
{
    setAcceptDrops( true );
    setMouseTracking( true );

    connect(this, SIGNAL(layoutChanged()), SLOT(onLayoutChange()));
}

void KTabBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        emit( mouseDoubleClick( indexOf( tab->identifier() ) ) );
        return;
    }
    QTabBar::mouseDoubleClickEvent( e );
}

void KTabBar::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == LeftButton || e->button() == LeftButton )
        mDragStart = e->pos();
    else if( e->button() == RightButton ) {
        QTab *tab = selectTab( e->pos() );
        if( tab!= 0L ) {
            emit( contextMenu( indexOf( tab->identifier() ), mapToGlobal( e->pos() ) ) );
            return;
        }
    }
    QTabBar::mousePressEvent( e );
}

void KTabBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->state() == LeftButton ) {
        int delay = KGlobalSettings::dndEventDelay();
        QPoint newPos = e->pos();
        if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
            newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
         {
            QTab *tab = selectTab( e->pos() );
            if( tab!= 0L ) {
                emit( dragInitiated( indexOf( tab->identifier() ) ) );
                return;
           }
       }
    }
    else if ( e->state() == MidButton ) {
        if (reorderStartTab==-1) {
            int delay = KGlobalSettings::dndEventDelay();
            QPoint newPos = e->pos();
            if( newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
                newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay )
            {
                QTab *tab = selectTab( e->pos() );
                if( tab!= 0L && mTabReordering ) {
                    reorderStartTab = indexOf( tab->identifier() );
                    grabMouse( sizeAllCursor );
                    return;
                }
            }
        }
        else {
            QTab *tab = selectTab( e->pos() );
            if( tab!= 0L ) {
                int reorderStopTab = indexOf( tab->identifier() );
                if ( reorderStartTab!=reorderStopTab && previousTabIndex!=reorderStopTab ) {
                    emit( moveTab( reorderStartTab, reorderStopTab ) );
                    previousTabIndex=reorderStartTab;
                    reorderStartTab=reorderStopTab;
                    return;
                }
            }
        }
    }

    if ( mHoverCloseButton ) {
        QTab *t = selectTab( e->pos() );
        if( t && t->iconSet() ) {
            QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal );
            QRect rect( 0, 0, pixmap.width() + 4, pixmap.height() );

            int xoff = 0, yoff = 0;
            // The additional offsets were found by try and error, TODO: find the rational behind them
            if ( t == tab( currentTab() ) ) {
                xoff = style().pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, this ) + 3;
                yoff = style().pixelMetric( QStyle::PM_TabBarTabShiftVertical, this ) - 1;
            }
            else {
                xoff = 5;
                yoff = 1;
            }
            rect.moveLeft( t->rect().left() + 2 + xoff );
            rect.moveTop( t->rect().center().y()-pixmap.height()/2 + yoff );
            if ( rect.contains( e->pos() ) ) {
                if ( b ) {
                    if ( btab == t )
                        return;
                    delete b;
                }

                b = new QToolButton( this );
                b->setIconSet( SmallIcon( "fileclose" ) );
                b->setGeometry( rect );
                b->show();
                btab = t;
                connect( b, SIGNAL( clicked() ), SLOT( closeButtonClicked() ) );
                return;
            }
        }
        if ( b ) {
            delete b;
            b = 0;
        }
    }

    QTabBar::mouseMoveEvent( e );
}

void KTabBar::mouseReleaseEvent( QMouseEvent *e )
{
    if( e->button() == MidButton ) {
        QTab *tab = selectTab( e->pos() );
        if ( reorderStartTab==-1 ) {
            if( tab!= 0L ) {
                emit( mouseMiddleClick( indexOf( tab->identifier() ) ) );
                return;
            }
        }
        else {
            releaseMouse();
            setCursor( arrowCursor );
            reorderStartTab=-1;
            previousTabIndex=-1;
        }
    }
    QTabBar::mouseReleaseEvent( e );
}

void KTabBar::dragMoveEvent( QDragMoveEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        e->accept( true );  // How to make it conditional?
        return;
    }
    e->accept( false );
    QTabBar::dragMoveEvent( e );
}

void KTabBar::dropEvent( QDropEvent *e )
{
    QTab *tab = selectTab( e->pos() );
    if( tab!= 0L ) {
        emit( receivedDropEvent( indexOf( tab->identifier() ) , e ) );
        return;
    }
    QTabBar::dropEvent( e );
}

void KTabBar::setTabColor( int id, const QColor& color )
{
    QTab *t = tab( id );
    if ( t ) {
        tabColors.insert( id, color );
        repaint( t->rect() );
    }
}

QColor KTabBar::tabColor( int id  ) const
{
    if ( tabColors.contains( id ) )
        return tabColors[id];

    return colorGroup().foreground();
}

void KTabBar::paintLabel( QPainter *p, const QRect& br,
                          QTab *t, bool has_focus ) const
{
    QRect r = br;
    bool selected = currentTab() == t->identifier();
    if ( t->iconSet() ) {
        // the tab has an iconset, draw it in the right mode
        QIconSet::Mode mode = ( t->isEnabled() && isEnabled() )
                                 ? QIconSet::Normal : QIconSet::Disabled;
        if ( mode == QIconSet::Normal && has_focus )
            mode = QIconSet::Active;
        QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        r.setLeft( r.left() + pixw + 4 );
        r.setRight( r.right() + 2 );
        // ### the pixmap shift should probably not be hardcoded..
        p->drawPixmap( br.left() + 2 + ((selected == TRUE) ? 0 : 2),
                         br.center().y()-pixh/2 + ((selected == TRUE) ? 0 : 2),
                         pixmap );
    }

    QStyle::SFlags flags = QStyle::Style_Default;

    if ( isEnabled() && t->isEnabled() )
        flags |= QStyle::Style_Enabled;
    if ( has_focus )
        flags |= QStyle::Style_HasFocus;

    QColorGroup cg( colorGroup() );
    if ( tabColors.contains( t->identifier() ) )
        cg.setColor( QColorGroup::Foreground, tabColors[t->identifier()] );

    style().drawControl( QStyle::CE_TabBarLabel, p, this, r,
                             t->isEnabled() ? cg : palette().disabled(),
                             flags, QStyleOption(t) );
}

bool KTabBar::isTabReorderingEnabled() const
{
    return mTabReordering;
}

void KTabBar::setTabReorderingEnabled( bool on )
{
    mTabReordering = on;
}

void KTabBar::closeButtonClicked()
{
    emit closeRequest( indexOf( btab->identifier() ) );
}

void KTabBar::setHoverCloseButton( bool button )
{
    mHoverCloseButton = button;
    if ( !button )
        onLayoutChange();
}

bool KTabBar::hoverCloseButton() const
{
    return mHoverCloseButton;
}

void KTabBar::onLayoutChange()
{
    delete b;
    b = 0;
    btab = 0;
}

#include "ktabbar.moc"
