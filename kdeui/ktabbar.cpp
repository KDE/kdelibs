/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>

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

#include "ktabbar.h"
#include "ktabwidget.h"
#include <kglobalsettings.h>

KTabBar::KTabBar( QWidget * parent, const char *name )
    : QTabBar( parent, name ), reorderStartTab( -1 ), previousTabIndex( -1 )
{
  setAcceptDrops(TRUE);
}

void KTabBar::mouseDoubleClickEvent(QMouseEvent *e)
{
  QTab *tab = selectTab( e->pos() );
  if( tab!= 0L ) {
    QWidget *page = ((KTabWidget*)parent())->page( indexOf( tab->identifier() ) );
    emit( mouseDoubleClick( page ) );
    return;
  }
  QTabBar::mouseDoubleClickEvent(e);
}

void KTabBar::mousePressEvent(QMouseEvent *e)
{
  if(e->button() == LeftButton || e->button() == LeftButton) {
    mDragStart = e->pos();
  }
  else if(e->button() == RightButton) {
    QTab *tab = selectTab(e->pos() );
    if( tab!= 0L ) {
      QWidget *page = ((KTabWidget*)parent())->page( indexOf( tab->identifier() ) );
      emit( contextMenu( page, mapToGlobal( e->pos() ) ) );
      return;
    }
  }
  QTabBar::mousePressEvent(e);
}

void KTabBar::mouseMoveEvent(QMouseEvent *e)
{
  if (e->state() == LeftButton) {
    int delay = KGlobalSettings::dndEventDelay();
    QPoint newPos = e->pos();
    if(newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
       newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay)
    {
      QTab *tab = selectTab(e->pos() );
      if( tab!= 0L ) {
        QWidget *page = ((KTabWidget*)parent())->page( indexOf( tab->identifier() ) );
        emit( dragInitiated( page ) );
        return;
      }
    }
  }
  else if (e->state() == MidButton) {
    if (reorderStartTab==-1) {
      int delay = KGlobalSettings::dndEventDelay();
      QPoint newPos = e->pos();
      if(newPos.x() > mDragStart.x()+delay || newPos.x() < mDragStart.x()-delay ||
         newPos.y() > mDragStart.y()+delay || newPos.y() < mDragStart.y()-delay)
      {
        QTab *tab = selectTab(e->pos() );
        if( tab!= 0L ) {
          reorderStartTab = indexOf( tab->identifier() );
          setMouseTracking(true);
          grabMouse(sizeAllCursor);
          return;
        }
      }
    }
    else {
      QTab *tab = selectTab(e->pos() );
      if( tab!= 0L ) {
        int reorderStopTab = indexOf( tab->identifier() );
        if (reorderStartTab!=reorderStopTab && previousTabIndex!=reorderStopTab) {
          emit( movedTab( reorderStartTab, reorderStopTab ) );
          previousTabIndex=reorderStartTab;
          reorderStartTab=reorderStopTab;
          return;
        }
      }
    }
  }
  QTabBar::mouseMoveEvent(e);
}

void KTabBar::mouseReleaseEvent(QMouseEvent *e)
{
  if(e->button() == MidButton) {
    QTab *tab = selectTab(e->pos() );
    if (reorderStartTab==-1) {
      if( tab!= 0L ) {
        QWidget *page = ((KTabWidget*)parent())->page( indexOf( tab->identifier() ) );
        emit( mouseMiddleClick( page ) );
        return;
      }
    }
    else {
      releaseMouse();
      setCursor(arrowCursor);
      setMouseTracking(false);
      reorderStartTab=-1;
      previousTabIndex=-1;
    }
  }
  QTabBar::mouseReleaseEvent(e);
}

void KTabBar::dragMoveEvent( QDragMoveEvent *e )
{
  QTab *tab = selectTab(e->pos() );
  if( tab!= 0L ) {
    e->accept(true);  // How to make it conditional?
    return;
  }
  e->accept(false);
  QTabBar::dragMoveEvent( e );
}

void KTabBar::dropEvent( QDropEvent *e )
{
  QTab *tab = selectTab(e->pos() );
  if( tab!= 0L ) {
    QWidget *page = ((KTabWidget*)parent())->page( indexOf( tab->identifier() ) );
    emit( receivedDropEvent( page, e ) );
    return;
  }
  QTabBar::dropEvent( e );
}

#include "ktabbar.moc"
