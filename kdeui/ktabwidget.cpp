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

#include "ktabwidget.h"
#include "ktabbar.h"
#include <kiconloader.h>

KTabWidget::KTabWidget( QWidget *parent, const char *name, WFlags f )
    : QTabWidget( parent, name, f ), mTabReordering( false )
{
  m_pTabBar = new KTabBar(this, "tabbar");
  setTabBar(m_pTabBar);
  setAcceptDrops(TRUE);

  connect(m_pTabBar, SIGNAL(contextMenu( QWidget *, const QPoint & )), this, SIGNAL(contextMenu( QWidget *, const QPoint & )));
  connect(m_pTabBar, SIGNAL(mouseDoubleClick( QWidget * )), this, SIGNAL(mouseDoubleClick( QWidget * )));
  connect(m_pTabBar, SIGNAL(mouseMiddleClick( QWidget * )), this, SIGNAL(mouseMiddleClick( QWidget * )));
  connect(m_pTabBar, SIGNAL(dragInitiated( QWidget * )), this, SIGNAL(dragInitiated( QWidget * )));
  connect(m_pTabBar, SIGNAL(receivedDropEvent( QWidget *, QDropEvent * )), this, SIGNAL(receivedDropEvent( QWidget *, QDropEvent * )));
  connect(m_pTabBar, SIGNAL(moveTab( int, int )), this, SLOT(moveTab( int, int )));
}

bool KTabWidget::isTabReorderingEnabled() const
{
  return mTabReordering;
}

void KTabWidget::setTabReorderingEnabled( bool on)
{
  mTabReordering = on;
}

void KTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
  if ( isEmptyTabbarSpace( e->pos() ) ) {
    e->accept(true);  // How to make it conditional?
    return;
  }
  e->accept(false);
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

void KTabWidget::moveTab( int from, int to )
{
  QString tablabel = label( from );
  QWidget* w = page( from );
  QColor color = tabColor( w );
  QIconSet tabiconset = tabIconSet( w );
  QString tabtooltip = tabToolTip( w );
  bool current = (w == currentPage() );
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
  QPoint point(p);
  QSize size( m_pTabBar->sizeHint() );
  if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
    if ( cornerWidget( TopLeft ) )
      point.setX( point.x()-size.height() );
    if ( tabPosition()==Bottom )
      point.setY( point.y()-( height()-size.height() ) );
    QTab *tab = m_pTabBar->selectTab( point);
    if( tab== 0L )
      return true;
  }
  return false;
}

void KTabWidget::setTabColor( QWidget *w, const QColor& color )
{
    QTab * t = m_pTabBar->tabAt( indexOf( w ) );

    setTabColor( t->identifier(), color );
}

void KTabWidget::setTabColor( int id, const QColor& color )
{
    if ( id < 0 || ! m_pTabBar->tab( id ) )
        return;

    m_pTabBar->setTabColor( id, color );
}

QColor KTabWidget::tabColor( QWidget * w ) const
{
    QTab * t = m_pTabBar->tabAt( indexOf( w ) );

    return m_pTabBar->tabColor( t->identifier() );
}

#include "ktabwidget.moc"
