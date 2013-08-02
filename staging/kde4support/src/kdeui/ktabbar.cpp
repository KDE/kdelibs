/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>
    Copyright (C) 2009 Urs Wolfer <uwolfer @ kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ktabbar.h"

#include <QtCore/QTimer>
#include <QApplication>
#include <QCursor>
#include <QMouseEvent>

class KTabBar::Private
{
  public:
    Private()
      : mReorderStartTab( -1 ),
        mReorderPreviousTab( -1 ),
        mDragSwitchTab( -1 ),
        mActivateDragSwitchTabTimer( 0 ),
        mTabReorderingEnabled( false ),
        mMiddleMouseTabMoveInProgress( false)
    {
    }

    QPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    int mDragSwitchTab;
    QTimer *mActivateDragSwitchTabTimer;

    bool mTabReorderingEnabled : 1;
    bool mMiddleMouseTabMoveInProgress : 1;

};

KTabBar::KTabBar( QWidget *parent )
    : QTabBar( parent ),
      d( new Private )
{
  setAcceptDrops( true );
  setMouseTracking( true );

  d->mActivateDragSwitchTabTimer = new QTimer( this );
  d->mActivateDragSwitchTabTimer->setSingleShot( true );
  connect( d->mActivateDragSwitchTabTimer, SIGNAL(timeout()), SLOT(activateDragSwitchTab()) );
#ifndef KDE_NO_DEPRECATED
  connect( this, SIGNAL(tabCloseRequested(int)), this, SIGNAL(closeRequest(int))); // just for backward compatibility, KDE5 remove
#endif

  //connect( this, SIGNAL(layoutChanged()), SLOT(onLayoutChange()) );
}

KTabBar::~KTabBar()
{
  delete d;
}

void KTabBar::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  int tab = selectTab( event->pos() );

  if(tab == -1) {
    Q_EMIT newTabRequest();
  } else {
#ifndef KDE_NO_DEPRECATED
    Q_EMIT mouseDoubleClick( tab ); //deprecated
#endif
    Q_EMIT tabDoubleClicked( tab );
  }

  QTabBar::mouseDoubleClickEvent( event );
}

void KTabBar::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton ) {
    d->mDragStart = event->pos();
  } else if( event->button() == Qt::RightButton ) {
    int tab = selectTab( event->pos() );
    if ( tab != -1 ) {
      Q_EMIT contextMenu( tab, mapToGlobal( event->pos() ) );
    } else {
      Q_EMIT emptyAreaContextMenu( mapToGlobal( event->pos() ) );
    }
    return;
  }

  QTabBar::mousePressEvent( event );
}

void KTabBar::mouseMoveEvent( QMouseEvent *event )
{
  if ( event->buttons() == Qt::LeftButton && !isMovable() ) {
    int tab = selectTab( event->pos() );
    if ( d->mDragSwitchTab && tab != d->mDragSwitchTab ) {
      d->mActivateDragSwitchTabTimer->stop();
      d->mDragSwitchTab = 0;
    }

    int delay = QApplication::startDragDistance();
    QPoint newPos = event->pos();
    if ( newPos.x() > d->mDragStart.x() + delay || newPos.x() < d->mDragStart.x() - delay ||
         newPos.y() > d->mDragStart.y() + delay || newPos.y() < d->mDragStart.y() - delay ) {
      if ( tab != -1 ) {
        Q_EMIT initiateDrag( tab );
        return;
      }
    }
  } else if ( event->buttons() == Qt::MidButton && !isMovable() ) {
    if ( d->mReorderStartTab == -1 ) {
      int delay = QApplication::startDragDistance();
      QPoint newPos = event->pos();

      if ( newPos.x() > d->mDragStart.x() + delay || newPos.x() < d->mDragStart.x() - delay ||
           newPos.y() > d->mDragStart.y() + delay || newPos.y() < d->mDragStart.y() - delay ) {
        int tab = selectTab( event->pos() );
        if ( tab != -1 && d->mTabReorderingEnabled ) {
          d->mReorderStartTab = tab;
          grabMouse( Qt::SizeAllCursor );
          return;
        }
      }
    } else {
      int tab = selectTab( event->pos() );
      if ( tab != -1 ) {
        int reorderStopTab = tab;
        if ( d->mReorderStartTab != reorderStopTab && d->mReorderPreviousTab != reorderStopTab ) {
          Q_EMIT moveTab( d->mReorderStartTab, reorderStopTab );

          d->mReorderPreviousTab = d->mReorderStartTab;
          d->mReorderStartTab = reorderStopTab;

          return;
        }
      }
    }
  } else if ( event->button() == Qt::NoButton && event->buttons() == Qt::MidButton && isMovable() ) {
    // compatibility feature for old middle mouse tab moving
    event->accept();
    if( d->mMiddleMouseTabMoveInProgress == false ) {
      QMouseEvent fakedMouseEvent(QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
      QCoreApplication::sendEvent(this, &fakedMouseEvent);
    }
    d->mMiddleMouseTabMoveInProgress = true;
    QMouseEvent fakedMouseEvent(event->type(), event->pos(), event->button(), Qt::LeftButton, event->modifiers());
    QCoreApplication::sendEvent(this, &fakedMouseEvent);
    return;
  }

  QTabBar::mouseMoveEvent( event );
}


#ifndef KDE_NO_DEPRECATED
void KTabBar::closeButtonClicked()
{
  // deprecated
}
#endif


#ifndef KDE_NO_DEPRECATED
void KTabBar::enableCloseButton()
{
  // deprecated
}
#endif


void KTabBar::activateDragSwitchTab()
{
  int tab = selectTab( mapFromGlobal( QCursor::pos() ) );
  if ( tab != -1 && d->mDragSwitchTab == tab )
    setCurrentIndex( d->mDragSwitchTab );

  d->mDragSwitchTab = 0;
}

void KTabBar::mouseReleaseEvent( QMouseEvent *event )
{
  switch ( event->button() ) {
  case Qt::LeftButton:
    break;

  case Qt::MidButton:
    if (d->mMiddleMouseTabMoveInProgress && QTabBar::isMovable()) {
      // compatibility feature for old middle mouse tab moving
      d->mMiddleMouseTabMoveInProgress = false;
      event->accept();
      QMouseEvent fakedMouseEvent(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
      QCoreApplication::sendEvent(this, &fakedMouseEvent);
      return;
    }
    if ( d->mReorderStartTab == -1 ) {
      int tab = selectTab( event->pos() );
      if ( tab != -1 ) {
        event->accept();
        if (QTabBar::isMovable()) {
          QMouseEvent fakedMouseEvent(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
          QCoreApplication::sendEvent(this, &fakedMouseEvent);
        }
        if ( tabsClosable() ) {
          Q_EMIT tabCloseRequested( tab );
        } else {
          Q_EMIT mouseMiddleClick( tab );
        }
        return;
      }
    } else {
      releaseMouse();
      setCursor( Qt::ArrowCursor );
      d->mReorderStartTab = -1;
      d->mReorderPreviousTab = -1;
    }
    break;

  default:
    break;
  }

  QTabBar::mouseReleaseEvent( event );
}

void KTabBar::dragEnterEvent( QDragEnterEvent *event )
{
  int tab = selectTab( event->pos() );
  if ( tab != -1 ) {
    bool accept = false;
    // The receivers of the testCanDecode() signal has to adjust
    // 'accept' accordingly.
    Q_EMIT testCanDecode( event, accept );
    if ( accept && tab != currentIndex() ) {
      d->mDragSwitchTab = tab;
      d->mActivateDragSwitchTabTimer->start( QApplication::doubleClickInterval() * 2 );
    }

    event->setAccepted( accept );
    return;
  }

  QTabBar::dragEnterEvent( event );
}

void KTabBar::dragMoveEvent( QDragMoveEvent *event )
{
  int tab = selectTab( event->pos() );
  if ( tab != -1 ) {
    bool accept = false;
    // The receivers of the testCanDecode() signal has to adjust
    // 'accept' accordingly.
    Q_EMIT testCanDecode( event, accept );
    if ( accept && tab != currentIndex() ) {
      d->mDragSwitchTab = tab;
      d->mActivateDragSwitchTabTimer->start( QApplication::doubleClickInterval() * 2 );
    }

    event->setAccepted( accept );
    return;
  }

  QTabBar::dragMoveEvent( event );
}

void KTabBar::dropEvent( QDropEvent *event )
{
  int tab = selectTab( event->pos() );
  if ( tab != -1 ) {
    d->mActivateDragSwitchTabTimer->stop();
    d->mDragSwitchTab = 0;
    Q_EMIT receivedDropEvent( tab , event );
    return;
  }

  QTabBar::dropEvent( event );
}

void KTabBar::paintEvent( QPaintEvent *event )
{
  QTabBar::paintEvent( event );
}

void KTabBar::leaveEvent( QEvent *event )
{
  QTabBar::leaveEvent( event );
}

QSize KTabBar::tabSizeHint( int index ) const
{
  QSize size = QTabBar::tabSizeHint( index );

  return size;
}

#ifndef QT_NO_WHEELEVENT
void KTabBar::wheelEvent( QWheelEvent *event )
{
  if ( !( event->orientation() == Qt::Horizontal ) ) {
    if ( receivers( SIGNAL(wheelDelta(int)) ) ) {
      Q_EMIT( wheelDelta( event->delta() ) );
      return;
    }
    int lastIndex = count() - 1;
    //Set an invalid index as base case
    int targetIndex = -1;
    bool forward = event->delta() < 0;
    if ( forward && lastIndex == currentIndex() ) {
      targetIndex = 0;
    }
    else if ( !forward && 0 == currentIndex() ) {
      targetIndex = lastIndex;
    }
    //Will not move when targetIndex is invalid
    setCurrentIndex( targetIndex );
    //If it has not moved yet (targetIndex == -1), or if it moved but current tab is disabled
    if ( targetIndex != currentIndex() || !isTabEnabled( targetIndex ) ) {
      QTabBar::wheelEvent( event );
    }
    event->accept();
  } else {
    event->ignore();
  }
}
#endif

#ifndef KDE_NO_DEPRECATED
bool KTabBar::isTabReorderingEnabled() const
{
  return d->mTabReorderingEnabled;
}
#endif

#ifndef KDE_NO_DEPRECATED
void KTabBar::setTabReorderingEnabled( bool on )
{
  d->mTabReorderingEnabled = on;
}
#endif

#ifndef KDE_NO_DEPRECATED
bool KTabBar::tabCloseActivatePrevious() const
{
  return selectionBehaviorOnRemove() == QTabBar::SelectPreviousTab;
}
#endif

#ifndef KDE_NO_DEPRECATED
void KTabBar::setTabCloseActivatePrevious( bool on )
{
  setSelectionBehaviorOnRemove(on ? QTabBar::SelectPreviousTab : QTabBar::SelectRightTab);
}
#endif


#ifndef KDE_NO_DEPRECATED
void KTabBar::setHoverCloseButton( bool button )
{
  // deprecated
  setTabsClosable(button);
}
#endif

#ifndef KDE_NO_DEPRECATED
bool KTabBar::hoverCloseButton() const
{
  // deprecated
  return tabsClosable();
}
#endif

#ifndef KDE_NO_DEPRECATED
void KTabBar::setHoverCloseButtonDelayed( bool delayed )
{
  // deprecated
  Q_UNUSED( delayed );
}
#endif

#ifndef KDE_NO_DEPRECATED
bool KTabBar::hoverCloseButtonDelayed() const
{
  // deprecated
  return false;
}
#endif

#ifndef KDE_NO_DEPRECATED
void KTabBar::setCloseButtonEnabled( bool enable )
{
  QTabBar::setTabsClosable(enable);
}
#endif

#ifndef KDE_NO_DEPRECATED
bool KTabBar::isCloseButtonEnabled() const
{
  return QTabBar::tabsClosable();
}
#endif

void KTabBar::tabLayoutChange()
{
  d->mActivateDragSwitchTabTimer->stop();
  d->mDragSwitchTab = 0;
}

int KTabBar::selectTab( const QPoint &pos ) const
{
  const int tabCount = count();
  for ( int i = 0; i < tabCount; ++i )
    if ( tabRect( i ).contains( pos ) )
      return i;

  return -1;
}

QPoint KTabBar::closeButtonPos( int tabIndex ) const
{
  Q_UNUSED(tabIndex);
  return QPoint();
}

QRect KTabBar::closeButtonRect( int tabIndex ) const
{
  Q_UNUSED(tabIndex);
  return QRect();
}

