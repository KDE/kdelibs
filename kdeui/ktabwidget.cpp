/* This file is part of the KDE libraries

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

KTabWidget::KTabWidget( QWidget *parent, const char *name, WFlags f )
    : QTabWidget( parent, name, f )
{
  m_pTabBar = new KTabBar(this, "tabbar");
  setTabBar(m_pTabBar);
  setAcceptDrops(TRUE);

  connect(m_pTabBar, SIGNAL(contextMenu( QWidget *, const QPoint & )), this, SIGNAL(contextMenu( QWidget *, const QPoint & )));
  connect(m_pTabBar, SIGNAL(mouseDoubleClick( QWidget * )), this, SIGNAL(mouseDoubleClick( QWidget * )));
  connect(m_pTabBar, SIGNAL(mouseMiddleClick( QWidget * )), this, SIGNAL(mouseMiddleClick( QWidget * )));
  connect(m_pTabBar, SIGNAL(dragInitiated( QWidget * )), this, SIGNAL(dragInitiated( QWidget * )));
  connect(m_pTabBar, SIGNAL(receivedDropEvent( QWidget *, QDropEvent * )), this, SIGNAL(receivedDropEvent( QWidget *, QDropEvent * )));
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

bool KTabWidget::isEmptyTabbarSpace( const QPoint &p )
{
  QPoint point(p);
  QSize size( m_pTabBar->sizeHint() );
  if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
    if ( isLeftButton() )
      point.setX( point.x()-size.height() );
    if ( tabPosition()==Bottom )
      point.setY( point.y()-( height()-size.height() ) );
    QTab *tab = m_pTabBar->selectTab( point);
    if( tab== 0L )
      return true;
  }
  return false;
}

#include "ktabwidget.moc"
