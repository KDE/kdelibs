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

#include <kiconloader.h>

#include "ktabwidget.h"
#include "ktabbar.h"

KTabWidget::KTabWidget( QWidget *parent, const char *name, WFlags f )
    : QTabWidget( parent, name, f )
{
    setTabBar( new KTabBar(this, "tabbar") );
    setAcceptDrops( true );

    connect(tabBar(), SIGNAL(contextMenu( const int, const QPoint & )), SLOT(contextMenu( const int, const QPoint & )));
    connect(tabBar(), SIGNAL(mouseDoubleClick( const int )), SLOT(mouseDoubleClick( const int )));
    connect(tabBar(), SIGNAL(mouseMiddleClick( const int )), SLOT(mouseMiddleClick( const int )));
    connect(tabBar(), SIGNAL(dragInitiated( const int )), SLOT(dragInitiated( const int )));
    connect(tabBar(), SIGNAL(receivedDropEvent( const int, QDropEvent * )), SLOT(receivedDropEvent( const int, QDropEvent * )));
    connect(tabBar(), SIGNAL(moveTab( const int, const int )), SLOT(moveTab( const int, const int )));
}

void KTabWidget::setTabColor( QWidget *w, const QColor& color )
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    ( (KTabBar*)tabBar() )->setTabColor( t->identifier(), color );
}

QColor KTabWidget::tabColor( QWidget *w ) const
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    return ( (KTabBar*)tabBar() )->tabColor( t->identifier() );
}

void KTabWidget::setTabReorderingEnabled( bool on)
{
    ( (KTabBar*)tabBar() )->setTabReorderingEnabled( on );
}

bool KTabWidget::isTabReorderingEnabled() const
{
    return ( (KTabBar*)tabBar() )->isTabReorderingEnabled();
}

void KTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        e->accept( true );  // How to make it conditional?
        return;
    }
    e->accept( false );
    QTabWidget::dragMoveEvent( e );
}

void KTabWidget::dropEvent( QDropEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        emit ( receivedDropEvent( e ) );
        return;
    }
    QTabWidget::dropEvent( e );
}

void KTabWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( tabbarContextMenu( mapToGlobal( e->pos() ) ) );
            return;
        }
    }
    QTabWidget::mousePressEvent( e );
}

void KTabWidget::receivedDropEvent( const int index, QDropEvent *e )
{
    emit( receivedDropEvent( page( index ), e ) );
}

void KTabWidget::dragInitiated( const int index )
{
    emit( dragInitiated( page( index ) ) );
}

void KTabWidget::contextMenu( const int index, const QPoint &p )
{
    emit( contextMenu( page( index ), p ) );
}

void KTabWidget::mouseDoubleClick( const int index )
{
    emit( mouseDoubleClick( page( index ) ) );
}

void KTabWidget::mouseMiddleClick( const int index )
{
    emit( mouseMiddleClick( page( index ) ) );
}

void KTabWidget::moveTab( const int from, const int to )
{
    QString tablabel = label( from );
    QWidget *w = page( from );
    QColor color = tabColor( w );
    QIconSet tabiconset = tabIconSet( w );
    QString tabtooltip = tabToolTip( w );
    bool current = ( w == currentPage() );
    removePage( w );

    insertTab( w, tablabel, to );
    w = page( to );
    changeTab( w, tabiconset, tablabel );
    setTabToolTip( w, tabtooltip );
    setTabColor( w, color );
    if ( current )
        showPage( w );

    emit ( movedTab( from, to ) );
}

bool KTabWidget::isEmptyTabbarSpace( const QPoint &p )
{
    QPoint point( p );
    QSize size( tabBar()->sizeHint() );
    if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
        if ( cornerWidget( TopLeft ) )
            point.setX( point.x()-size.height() );
        if ( tabPosition()==Bottom )
            point.setY( point.y()-( height()-size.height() ) );
        QTab *tab = tabBar()->selectTab( point);
        if( tab== 0L )
            return true;
    }
    return false;
}

#include "ktabwidget.moc"
