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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ktabbar.h"
#include "ktabwidget.h"

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstyle.h>

class KTabBar::Private
{
  public:
    Private()
      : mReorderStartTab( -1 ),
        mReorderPreviousTab( -1 ),
        mDragSwitchTab( -1 ),
        mHoveredCloseIconIndex( -1 ),
        mActivateDragSwitchTabTimer( 0 ),
        mHoveredCloseIcon( 0 ),
        mInactiveCloseIcon( 0 ),
        mActiveCloseIcon( 0 ),
        mTabReorderingEnabled( false ),
        mTabCloseActivatePrevious( false ),
        mTabCloseButtonClicked( false )
    {
    }

    QPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    int mDragSwitchTab;
    int mHoveredCloseIconIndex;
    //QTab *mDragSwitchTab;
    QTimer *mActivateDragSwitchTabTimer;
    QPixmap *mHoveredCloseIcon;
    QPixmap *mInactiveCloseIcon;
    QPixmap *mActiveCloseIcon;

    bool mTabReorderingEnabled : 1;
    bool mTabCloseActivatePrevious : 1;
    bool mTabCloseButtonClicked : 1;

};

KTabBar::KTabBar( QWidget *parent )
    : QTabBar( parent ),
      d( new Private )
{
  setAcceptDrops( true );
  setMouseTracking( true );

  d->mActivateDragSwitchTabTimer = new QTimer( this );
  d->mActivateDragSwitchTabTimer->setSingleShot( true );
  connect( d->mActivateDragSwitchTabTimer, SIGNAL( timeout() ), SLOT( activateDragSwitchTab() ) );

  //connect( this, SIGNAL( layoutChanged() ), SLOT( onLayoutChange() ) );
}

KTabBar::~KTabBar()
{
  delete d->mHoveredCloseIcon;
  d->mHoveredCloseIcon = 0;

  delete d->mInactiveCloseIcon;
  d->mInactiveCloseIcon = 0;

  delete d->mActiveCloseIcon;
  d->mActiveCloseIcon = 0;

  delete d;
}

void KTabBar::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  int tab = selectTab( event->pos() );

  if(tab == -1) {
    emit newTabRequest();
  } else {
    emit mouseDoubleClick( tab ); //deprecated
    emit tabDoubleClicked( tab );
  }
}

void KTabBar::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton ) {
    d->mDragStart = event->pos();
    if ( isCloseButtonEnabled() ) {
      const QPoint pos = event->pos();
      const int tabIndex = tabAt( pos );
      if (closeButtonRect( tabIndex ).contains( pos )) {
        // the close button is clicked - prevent that the tab gets activated
        d->mTabCloseButtonClicked = true;
        return;
      }
    }
  } else if( event->button() == Qt::RightButton ) {
    int tab = selectTab( event->pos() );
    if ( tab != -1 ) {
      emit contextMenu( tab, mapToGlobal( event->pos() ) );
    } else {
      emit emptyAreaContextMenu( mapToGlobal( event->pos() ) );
    }
    return;
  }

  QTabBar::mousePressEvent( event );
}

void KTabBar::mouseMoveEvent( QMouseEvent *event )
{
  if ( isCloseButtonEnabled() ) {
    if ( d->mHoveredCloseIconIndex >= 0) {
      // reset previously hovered close button
      update( closeButtonRect( d->mHoveredCloseIconIndex ));
      d->mHoveredCloseIconIndex = -1;
    }

    const QPoint pos = event->pos();
    const int tabCount = count();
    for ( int i = 0; i < tabCount; ++i ) {
      const QRect rect = closeButtonRect( i );
      if (rect.contains( pos )) {
        // update currently hovered close button
        d->mHoveredCloseIconIndex = i;
        update( rect );
        break;
      }
    }
  }

  if ( event->buttons() == Qt::LeftButton ) {
    int tab = selectTab( event->pos() );
    if ( d->mDragSwitchTab && tab != d->mDragSwitchTab ) {
      d->mActivateDragSwitchTabTimer->stop();
      d->mDragSwitchTab = 0;
    }

    int delay = KGlobalSettings::dndEventDelay();
    QPoint newPos = event->pos();
    if ( newPos.x() > d->mDragStart.x() + delay || newPos.x() < d->mDragStart.x() - delay ||
         newPos.y() > d->mDragStart.y() + delay || newPos.y() < d->mDragStart.y() - delay ) {
      if ( tab != -1 ) {
        emit initiateDrag( tab );
        return;
      }
    }
  } else if ( event->buttons() == Qt::MidButton ) {
    if ( d->mReorderStartTab == -1 ) {
      int delay = KGlobalSettings::dndEventDelay();
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
          emit moveTab( d->mReorderStartTab, reorderStopTab );

          d->mReorderPreviousTab = d->mReorderStartTab;
          d->mReorderStartTab = reorderStopTab;

          return;
        }
      }
    }
  }

  QTabBar::mouseMoveEvent( event );
}


void KTabBar::closeButtonClicked()
{
  // deprecated
}


void KTabBar::enableCloseButton()
{
  // deprecated
}


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
    if ( isCloseButtonEnabled() ) {
      update(closeButtonRect( d->mHoveredCloseIconIndex ));

      const QPoint pos = event->pos();
      const int tabIndex = tabAt( pos );
      if (d->mTabCloseButtonClicked && closeButtonRect( tabIndex ).contains( pos )) {
        d->mHoveredCloseIconIndex = -1;
        emit closeRequest( tabIndex );
      }
    }
    break;

  case Qt::MidButton:
    if ( d->mReorderStartTab == -1 ) {
      int tab = selectTab( event->pos() );
      if ( tab != -1 ) {
        emit mouseMiddleClick( tab );
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

  d->mTabCloseButtonClicked = false;
  QTabBar::mouseReleaseEvent( event );
}

void KTabBar::dragEnterEvent( QDragEnterEvent *event )
{
  int tab = selectTab( event->pos() );
  if ( tab != -1 ) {
    bool accept = false;
    // The receivers of the testCanDecode() signal has to adjust
    // 'accept' accordingly.
    emit testCanDecode( event, accept );
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
    emit testCanDecode( event, accept );
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
    emit receivedDropEvent( tab , event );
    return;
  }

  QTabBar::dropEvent( event );
}

void KTabBar::paintEvent( QPaintEvent *event )
{
  QTabBar::paintEvent( event );

  if ( isCloseButtonEnabled() ) {
    QPainter painter( this );
    const int tabCount = count();
    for ( int i = 0; i < tabCount; ++i ) {
      QPixmap icon;
      if ( i == d->mHoveredCloseIconIndex)
        icon = *d->mActiveCloseIcon;
      else if ( i == currentIndex() )
        icon = *d->mHoveredCloseIcon;
      else
        icon = *d->mInactiveCloseIcon;

      painter.drawPixmap( closeButtonPos( i ), icon );
    }
  }
}

void KTabBar::leaveEvent( QEvent *event )
{
  QTabBar::leaveEvent( event );
  if ( d->mHoveredCloseIconIndex >= 0 ) {
    update( closeButtonRect( d->mHoveredCloseIconIndex ));
    d->mHoveredCloseIconIndex = -1;
  }
}

QSize KTabBar::tabSizeHint( int index ) const
{
  QSize size = QTabBar::tabSizeHint( index );
  if ( isCloseButtonEnabled() )
      size.rwidth() += KIconLoader::SizeSmall * 3 / 2;

  return size;
}

#ifndef QT_NO_WHEELEVENT
void KTabBar::wheelEvent( QWheelEvent *event )
{
  if ( event->orientation() == Qt::Horizontal )
    return;

  emit( wheelDelta( event->delta() ) );
}
#endif

bool KTabBar::isTabReorderingEnabled() const
{
  return d->mTabReorderingEnabled;
}

void KTabBar::setTabReorderingEnabled( bool on )
{
  d->mTabReorderingEnabled = on;
}

bool KTabBar::tabCloseActivatePrevious() const
{
  return d->mTabCloseActivatePrevious;
}

void KTabBar::setTabCloseActivatePrevious( bool on )
{
  d->mTabCloseActivatePrevious = on;
}


void KTabBar::setHoverCloseButton( bool button )
{
  // deprecated
  setCloseButtonEnabled( button );
}

bool KTabBar::hoverCloseButton() const
{
  // deprecated
  return isCloseButtonEnabled();
}

void KTabBar::setHoverCloseButtonDelayed( bool delayed )
{
  // deprecated
  Q_UNUSED( delayed );
}

bool KTabBar::hoverCloseButtonDelayed() const
{
  // deprecated
  return false;
}

void KTabBar::setCloseButtonEnabled( bool enable )
{
  if ( enable == isCloseButtonEnabled() )
    return;

  d->mHoveredCloseIconIndex = -1;
  if ( enable ) {
    Q_ASSERT( d->mHoveredCloseIcon == 0 );
    Q_ASSERT( d->mInactiveCloseIcon == 0 );
    Q_ASSERT( d->mActiveCloseIcon == 0 );

    const QPixmap icon = KIconLoader::global()->loadIcon( "dialog-close", KIconLoader::Small );

    d->mHoveredCloseIcon = new QPixmap( icon );
    KIconEffect::semiTransparent( *d->mHoveredCloseIcon );

    KIconEffect iconEffect;
    d->mInactiveCloseIcon = new QPixmap( iconEffect.apply( icon, KIconLoader::Small, KIconLoader::DisabledState ));

    KIconEffect::semiTransparent( *d->mInactiveCloseIcon );
    d->mActiveCloseIcon = new QPixmap( icon );
  } else {
    delete d->mHoveredCloseIcon;
    d->mHoveredCloseIcon = 0;

    delete d->mInactiveCloseIcon;
    d->mInactiveCloseIcon = 0;

    delete d->mActiveCloseIcon;
    d->mActiveCloseIcon = 0;
  }
}

bool KTabBar::isCloseButtonEnabled() const
{
  return d->mHoveredCloseIcon != 0;
}

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
  QPoint buttonPos;
  if ( tabIndex < 0 ) {
    return buttonPos;
  }

  int availableHeight = height();
  if ( tabIndex == currentIndex() ) {
    QStyleOption option;
    option.initFrom(this);
    availableHeight -= style()->pixelMetric( QStyle::PM_TabBarTabShiftVertical, &option, this );
  }

  const QRect tabBounds = tabRect( tabIndex );
  const int xInc = (height() - KIconLoader::SizeSmall) / 2;

  if ( layoutDirection() == Qt::RightToLeft ) {
    buttonPos = tabBounds.topLeft();
    buttonPos.rx() += xInc;
  } else {
    buttonPos = tabBounds.topRight();
    buttonPos.rx() -= KIconLoader::SizeSmall + xInc;
  }
  buttonPos.ry() += (availableHeight - KIconLoader::SizeSmall) / 2;

  return buttonPos;
}

QRect KTabBar::closeButtonRect( int tabIndex ) const
{
  QRect rect;
  if ( tabIndex >= 0 ) {
    rect.setTopLeft(closeButtonPos( tabIndex ));
    rect.setSize(QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall));
  }
  return rect;
}

#include "ktabbar.moc"
