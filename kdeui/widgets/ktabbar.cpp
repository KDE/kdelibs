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
#include <QtGui/QPushButton>
#include <QtGui/QStyle>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

class KTabBar::Private
{
  public:
    Private()
      : mReorderStartTab( -1 ),
        mReorderPreviousTab( -1 ),
        // mHoverCloseButtonTab( 0 ),
        mDragSwitchTab( -1 ),
        // mHoverCloseButton( 0 ),
        // mHoverCloseButtonEnabled( false ),
        // mHoverCloseButtonDelayed( true ),
        mTabReorderingEnabled( false ),
        mTabCloseActivatePrevious( false )
    {
    }

    QPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    int mDragSwitchTab;
    //QTab *mHoverCloseButtonTab, *mDragSwitchTab;
    //QPushButton *mHoverCloseButton;
    //QTimer mEnableCloseButtonTimer;
    QTimer *mActivateDragSwitchTabTimer;

    //bool mHoverCloseButtonEnabled;
    //bool mHoverCloseButtonDelayed;
    bool mTabReorderingEnabled : 1;
    bool mTabCloseActivatePrevious : 1;

};

KTabBar::KTabBar( QWidget *parent )
    : QTabBar( parent ),
      d( new Private )
{
  setAcceptDrops( true );
  setMouseTracking( true );

  //d->mEnableCloseButtonTimer = new QTimer( this );
  //connect( d->mEnableCloseButtonTimer, SIGNAL( timeout() ), SLOT( enableCloseButton() ) );

  d->mActivateDragSwitchTabTimer = new QTimer( this );
  d->mActivateDragSwitchTabTimer->setSingleShot( true );
  connect( d->mActivateDragSwitchTabTimer, SIGNAL( timeout() ), SLOT( activateDragSwitchTab() ) );

  //connect( this, SIGNAL( layoutChanged() ), SLOT( onLayoutChange() ) );
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
    emit newTabRequest();
  } else {
    emit mouseDoubleClick( tab ); //deprecated
    emit tabDoubleClicked( tab );
  }
}

void KTabBar::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton ) {
    //d->mEnableCloseButtonTimer->stop();
    d->mDragStart = event->pos();
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

  /*if ( d->mHoverCloseButtonEnabled && d->mReorderStartTab == -1 ) {
      QTab *t = selectTab( event->pos() );
      if ( t && t->iconSet() && t->isEnabled() ) {
        QPixmap pixmap = t->iconSet()->pixmap( QIcon::Small, QIcon::Normal );
        QRect rect( 0, 0, pixmap.width() + 4, pixmap.height() +4);

        int xoff = 0, yoff = 0;
        // The additional offsets were found by try and error, TODO: find the rational behind them
        if ( t == tab( currentTab() ) ) {
            xoff = style().pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, this ) + 3;
            yoff = style().pixelMetric( QStyle::PM_TabBarTabShiftVertical, this ) - 4;
        } else {
            xoff = 7;
            yoff = 0;
        }

        rect.moveLeft( t->rect().left() + 2 + xoff );
        rect.moveTop( t->rect().center().y() - pixmap.height() / 2 + yoff );
        if ( rect.contains( event->pos() ) ) {
          if ( d->mHoverCloseButton ) {
            if ( d->mHoverCloseButtonTab == t )
              return;
            d->mEnableCloseButtonTimer->stop();
            d->mHoverCloseButton->deleteLater();
            d->mHoverCloseButton = 0;
          }

          d->mHoverCloseButton = new QPushButton( this );
          d->mHoverCloseButton->setIcon( KGlobal::iconLoader()->loadIconSet( "window-close", KIconLoader::Toolbar, KIconLoader::SizeSmall ) );
          d->mHoverCloseButton->setGeometry( rect );
          d->mHoverCloseButton->setToolTip( i18n( "Close this tab" ) );
          d->mHoverCloseButton->setFlat( true );
          d->mHoverCloseButton->show();
          if ( d->mHoverCloseButtonDelayed ) {
            d->mHoverCloseButton->setEnabled( false );
            d->mEnableCloseButtonTimer->start( QApplication::doubleClickInterval(), true );
          }

          d->mHoverCloseButtonTab = t;
          connect( d->mHoverCloseButton, SIGNAL( clicked() ), SLOT( closeButtonClicked() ) );
          return;
        }
      }

      if ( d->mHoverCloseButton ) {
        d->mEnableCloseButtonTimer->stop();
        d->mHoverCloseButton->deleteLater();
        d->mHoverCloseButton = 0;
      }
  }*/

  QTabBar::mouseMoveEvent( event );
}

/*
void KTabBar::enableCloseButton()
{
  d->mHoverCloseButton->setEnabled( true );
}
*/

void KTabBar::activateDragSwitchTab()
{
  int tab = selectTab( mapFromGlobal( QCursor::pos() ) );
  if ( tab != -1 && d->mDragSwitchTab == tab )
    setCurrentIndex( d->mDragSwitchTab );

  d->mDragSwitchTab = 0;
}

void KTabBar::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::MidButton ) {
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
  }

  QTabBar::mouseReleaseEvent( event );
}

void KTabBar::dragEnterEvent( QDragEnterEvent *event )
{
  event->setAccepted( true );
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

  event->setAccepted( false );
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

/*
void KTabBar::closeButtonClicked()
{
  emit closeRequest( indexOf( d->mHoverCloseButtonTab->identifier() ) );
}
*/

void KTabBar::setHoverCloseButton( bool /*button*/ )
{
  // KDE4 porting: disabled
  return;

/*
  d->mHoverCloseButtonEnabled = button;
  if ( !button )
    tabLayoutChange();
*/
}

bool KTabBar::hoverCloseButton() const
{
  // KDE4 porting: disabled
  return false;

  //return d->mHoverCloseButtonEnabled;
}

void KTabBar::setHoverCloseButtonDelayed( bool /*delayed*/ )
{
  // KDE4 porting: disabled
  return;

  //d->mHoverCloseButtonDelayed = delayed;
}

bool KTabBar::hoverCloseButtonDelayed() const
{
  // KDE4 porting: disabled
  return false;

  //return d->mHoverCloseButtonDelayed;
}

void KTabBar::tabLayoutChange()
{
  //d->mEnableCloseButtonTimer->stop();
  //delete d->mHoverCloseButton;
  //d->mHoverCloseButton = 0;
  //d->mHoverCloseButtonTab = 0;
  d->mActivateDragSwitchTabTimer->stop();
  d->mDragSwitchTab = 0;
}

int KTabBar::selectTab( const QPoint &pos ) const
{
  for ( int i = 0; i < count(); ++i )
    if ( tabRect( i ).contains( pos ) )
      return i;

  return -1;
}

#include "ktabbar.moc"
