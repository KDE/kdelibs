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
#include <QtGui/QStyleOption>

#include <kglobalsettings.h>
#include <kicon.h>
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
        mHoverCloseButtonTab( -1 ),
        mHoverCloseButton( 0 ),
        mHoverCloseButtonEnabled( false ),
        mHoverCloseButtonDelayed( true ),
        mTabReorderingEnabled( false ),
        mTabCloseActivatePrevious( false )
    {
    }

    QPoint mDragStart;
    int mReorderStartTab;
    int mReorderPreviousTab;
    int mDragSwitchTab;
    int mHoverCloseButtonTab;
    //QTab *mDragSwitchTab;
    QPushButton *mHoverCloseButton;
    QTimer *mEnableCloseButtonTimer;
    QTimer *mActivateDragSwitchTabTimer;

    bool mHoverCloseButtonEnabled;
    bool mHoverCloseButtonDelayed;
    bool mTabReorderingEnabled : 1;
    bool mTabCloseActivatePrevious : 1;

};

KTabBar::KTabBar( QWidget *parent )
    : QTabBar( parent ),
      d( new Private )
{
  setAcceptDrops( true );
  setMouseTracking( true );

  d->mEnableCloseButtonTimer = new QTimer( this );
  d->mEnableCloseButtonTimer->setSingleShot( true );
  connect( d->mEnableCloseButtonTimer, SIGNAL( timeout() ), SLOT( enableCloseButton() ) );

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
    d->mEnableCloseButtonTimer->stop();
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

  if ( d->mHoverCloseButtonEnabled && d->mReorderStartTab == -1 ) {
      int t = selectTab( event->pos() );
      if ( t != -1 && !tabIcon(t).isNull() && isTabEnabled(t) ) {
        QPixmap pixmap = tabIcon(t).pixmap(  KIconLoader::SizeSmall, QIcon::Normal );
        QRect rect( 0, 0, pixmap.width() + 4, pixmap.height() + 4 );

        QStyleOption option;
        option.initFrom(this);
        QStyle* st = style();
        int xoff = 0, yoff = 0;
        // The place for the icon was found by try and error, by trying to have good results with
        // all styles
        // TODO: find the rational behind them
        if ( t == currentIndex() ) {
            xoff = st->pixelMetric( QStyle::PM_TabBarTabShiftHorizontal, &option, this );
            yoff = st->pixelMetric( QStyle::PM_TabBarTabShiftVertical, &option, this ) - 1;
        }

        int xPos = 0;
        if (qobject_cast<KStyle *>(style()) && layoutDirection() == Qt::RightToLeft) {
            //KStyle paints the tab icon on the right in RTL mode
            xPos = st->pixelMetric( QStyle::PM_TabBarBaseOverlap, &option, this ) + st->pixelMetric( QStyle::PM_TabBarTabOverlap, &option, this ) + 4 + rect.width();
            rect.moveLeft( tabRect(t).right() - xPos );
        } else
        {
            xPos = st->pixelMetric( QStyle::PM_TabBarBaseOverlap, &option, this ) - st->pixelMetric( QStyle::PM_TabBarTabOverlap, &option, this ) + 5;
            rect.moveLeft( tabRect(t).left() + xPos );
        }
        int yPos = st->pixelMetric( QStyle::PM_TabBarBaseHeight, &option, this ) - st->pixelMetric( QStyle::PM_TabBarTabVSpace, &option, this ) + 1;

        rect.moveTop( tabRect(t).center().y() - pixmap.height() / 2 - yoff - yPos );


        if ( rect.contains( event->pos() ) ) {
          if ( d->mHoverCloseButton ) {
            if ( d->mHoverCloseButtonTab == t )
              return;
            d->mEnableCloseButtonTimer->stop();
            d->mHoverCloseButton->deleteLater();
            d->mHoverCloseButton = 0;
          }

          d->mHoverCloseButton = new QPushButton( this );
          d->mHoverCloseButton->setIcon( KIcon( "window-close" ) );
          d->mHoverCloseButton->setGeometry( rect );
          d->mHoverCloseButton->setToolTip( i18n( "Close this tab" ) );
          d->mHoverCloseButton->setFlat( true );
          d->mHoverCloseButton->show();
          if ( d->mHoverCloseButtonDelayed ) {
            d->mHoverCloseButton->setEnabled( false );
            d->mEnableCloseButtonTimer->start( QApplication::doubleClickInterval() );
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
  }

  QTabBar::mouseMoveEvent( event );
}


void KTabBar::enableCloseButton()
{
  d->mHoverCloseButton->setEnabled( true );
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


void KTabBar::closeButtonClicked()
{
  emit closeRequest( d->mHoverCloseButtonTab );
}

void KTabBar::setHoverCloseButton( bool button )
{
  d->mHoverCloseButtonEnabled = button;
  if ( !button )
    tabLayoutChange();
}

bool KTabBar::hoverCloseButton() const
{
  return d->mHoverCloseButtonEnabled;
}

void KTabBar::setHoverCloseButtonDelayed( bool delayed )
{
  d->mHoverCloseButtonDelayed = delayed;
}

bool KTabBar::hoverCloseButtonDelayed() const
{
  return d->mHoverCloseButtonDelayed;
}

void KTabBar::setCloseButtonEnabled( bool enable )
{
    // TODO
    Q_UNUSED( enable );
}

bool KTabBar::isCloseButtonEnabled() const
{
    // TODO
    return false;
}

void KTabBar::tabLayoutChange()
{
  d->mEnableCloseButtonTimer->stop();
  delete d->mHoverCloseButton;
  d->mHoverCloseButton = 0;
  d->mHoverCloseButtonTab = 0;
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
