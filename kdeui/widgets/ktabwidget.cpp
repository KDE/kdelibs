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

#include "ktabwidget.h"

#include <QtGui/QApplication>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtCore/QList>

#include <ksharedconfig.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include <ktabbar.h>

#include <kconfiggroup.h>

class KTabWidget::Private
{
  public:
    Private( KTabWidget *parent )
      : m_parent( parent ),
        m_automaticResizeTabs( false )
    {

      KConfigGroup cg(KGlobal::config(), "General");
      m_maxLength = cg.readEntry("MaximumTabLength", 30);
      m_minLength = cg.readEntry("MinimumTabLength", 3);
      m_currentMaxLength = m_minLength;
    }

    KTabWidget *m_parent;
    bool m_automaticResizeTabs;
    int m_maxLength;
    int m_minLength;
    int m_currentMaxLength;

    //holds the full names of the tab, otherwise all we
    //know about is the shortened name
    QStringList m_tabNames;
    
    // Used when tabCloseActivatePrevious is enabled
    QList<QWidget*> m_previousTabList;

    bool isEmptyTabbarSpace( const QPoint & )  const;
    void resizeTabs( int changedTabIndex = -1 );
    void updateTab( int index );
    void removeTab( int index );
};

bool KTabWidget::Private::isEmptyTabbarSpace( const QPoint &point ) const
{
    if (m_parent->count() == 0) {
        return true;
    }
    if (m_parent->tabBar()->isHidden()) {
        return false;
    }
  QSize size( m_parent->tabBar()->sizeHint() );
  if ( ( m_parent->tabPosition() == QTabWidget::North && point.y() < size.height() ) ||
       ( m_parent->tabPosition() == QTabWidget::South && point.y() > (m_parent->height() - size.height() ) ) ) {

    QWidget *rightcorner = m_parent->cornerWidget( Qt::TopRightCorner );
    if ( rightcorner && rightcorner->isVisible() ) {
      if ( point.x() >= m_parent->width()-rightcorner->width() )
        return false;
    }

    QWidget *leftcorner = m_parent->cornerWidget( Qt::TopLeftCorner );
    if ( leftcorner && leftcorner->isVisible() ) {
      if ( point.x() <= leftcorner->width() )
        return false;
    }

    for ( int i = 0; i < m_parent->count(); ++i )
      if ( m_parent->tabBar()->tabRect( i ).contains( m_parent->tabBar()->mapFromParent( point ) ) )
        return false;

    return true;
  }

  return false;
}

void KTabWidget::Private::removeTab( int index )
{
  m_previousTabList.removeOne( m_parent->widget( index ) );
  
  // We need to get the widget for "activate previous" before calling to 
  // QTabWidget::removeTab() because that call changes currentIndex which calls
  // to currentChanged() and thus modifies m_previousTabList.first(). And we
  // store the widget and not the index of it because the index might be changed
  // by the removeTab call.
  QWidget *widget = 0;
  if( !m_previousTabList.isEmpty() && m_parent->tabCloseActivatePrevious() )
    widget = m_previousTabList.first();
  
  m_parent->QTabWidget::removeTab( index );
  
  if( widget )
    m_parent->setCurrentIndex( m_parent->indexOf( widget ) );
}

void KTabWidget::Private::resizeTabs( int changeTabIndex )
{
  int newMaxLength;
  if ( m_automaticResizeTabs ) {
    // Calculate new max length
    newMaxLength = m_maxLength;
    int lcw = 0, rcw = 0;

    int tabBarHeight = m_parent->tabBar()->sizeHint().height();
    if ( m_parent->cornerWidget( Qt::TopLeftCorner ) && m_parent->cornerWidget( Qt::TopLeftCorner )->isVisible() )
      lcw = qMax( m_parent->cornerWidget( Qt::TopLeftCorner )->width(), tabBarHeight );

    if ( m_parent->cornerWidget( Qt::TopRightCorner ) && m_parent->cornerWidget( Qt::TopRightCorner )->isVisible() )
      rcw = qMax( m_parent->cornerWidget( Qt::TopRightCorner )->width(), tabBarHeight );

    int maxTabBarWidth = m_parent->width() - lcw - rcw;

    for ( ; newMaxLength > m_minLength; newMaxLength-- ) {
      if ( m_parent->tabBarWidthForMaxChars( newMaxLength ) < maxTabBarWidth )
        break;
    }
  } else
    newMaxLength = 4711;

  // Update hinted or all tabs
  if ( m_currentMaxLength != newMaxLength ) {
    m_currentMaxLength = newMaxLength;
    for ( int i = 0; i < m_parent->count(); ++i )
      updateTab( i );
  } else if ( changeTabIndex != -1 )
    updateTab( changeTabIndex );
}

void KTabWidget::Private::updateTab( int index )
{
  QString title = m_automaticResizeTabs ? m_tabNames[ index ] : m_parent->QTabWidget::tabText( index );
  m_parent->setTabToolTip( index, QString() );

  if ( title.length() > m_currentMaxLength ) {
    if ( Qt::mightBeRichText( title ) )
      m_parent->setTabToolTip( index, Qt::escape( title ) );
    else
      m_parent->setTabToolTip( index, title );
  }

  title = KStringHandler::rsqueeze( title, m_currentMaxLength ).leftJustified( m_minLength, ' ' );

  if ( m_parent->QTabWidget::tabText( index ) != title )
    m_parent->QTabWidget::setTabText( index, title );
}

KTabWidget::KTabWidget( QWidget *parent, Qt::WFlags flags )
  : QTabWidget( parent ),
    d( new Private( this ) )
{
  setWindowFlags( flags );
  setTabBar( new KTabBar( this ) );
  setObjectName( "tabbar" );
  setAcceptDrops( true );

  connect(tabBar(), SIGNAL(contextMenu( int, const QPoint & )), SLOT(contextMenu( int, const QPoint & )));
  connect(tabBar(), SIGNAL(tabDoubleClicked( int )), SLOT(mouseDoubleClick( int )));
  connect(tabBar(), SIGNAL(mouseMiddleClick( int )), SLOT(mouseMiddleClick( int )));
  connect(tabBar(), SIGNAL(initiateDrag( int )), SLOT(initiateDrag( int )));
  connect(tabBar(), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )));
  connect(tabBar(), SIGNAL(receivedDropEvent( int, QDropEvent * )), SLOT(receivedDropEvent( int, QDropEvent * )));
  connect(tabBar(), SIGNAL(moveTab( int, int )), SLOT(moveTab( int, int )));
  connect(tabBar(), SIGNAL(closeRequest( int )), SLOT(closeRequest( int )));
  connect(tabBar(), SIGNAL(currentChanged( int )), SLOT(currentChanged( int )));
#ifndef QT_NO_WHEELEVENT
  connect(tabBar(), SIGNAL(wheelDelta( int )), SLOT(wheelDelta( int )));
#endif
}

KTabWidget::~KTabWidget()
{
  delete d;
}

/*void KTabWidget::insertTab( QWidget *child, const QString &label, int index )
{
  QTabWidget::insertTab( child, label, index );
}

void KTabWidget::insertTab( QWidget *child, const QIcon& iconset, const QString &label, int index )
{
  QTabWidget::insertTab( child, iconset, label, index );
}

void KTabWidget::insertTab( QWidget *child, QTab *tab, int index )
{
  QTabWidget::insertTab( child, tab, index);
  if ( d->m_automaticResizeTabs ) {
    if ( index < 0 || index >= count() ) {
      d->m_tabNames.append( tab->text() );
      d->resizeTabs( d->m_tabNames.count()-1 );
    }
    else {
      d->m_tabNames.insert( d->m_tabNames.at( index ), tab->text() );
      d->resizeTabs( index );
    }
  }
}*/

void KTabWidget::setTabBarHidden( bool hide )
{
  QWidget *rightcorner = cornerWidget( Qt::TopRightCorner );
  QWidget *leftcorner = cornerWidget( Qt::TopLeftCorner );

  if ( hide ) {
    if ( leftcorner ) leftcorner->hide();
    if ( rightcorner ) rightcorner->hide();
    tabBar()->hide();
  } else {
    tabBar()->show();
    if ( leftcorner ) leftcorner->show();
    if ( rightcorner ) rightcorner->show();
  }
}

bool KTabWidget::isTabBarHidden() const
{
  return !( tabBar()->isVisible() );
}

void KTabWidget::setTabTextColor( int index, const QColor& color )
{
  tabBar()->setTabTextColor( index, color );
}

QColor KTabWidget::tabTextColor( int index ) const
{
  return tabBar()->tabTextColor( index );
}

void KTabWidget::setTabReorderingEnabled( bool on)
{
  static_cast<KTabBar*>(tabBar())->setTabReorderingEnabled( on );
}

bool KTabWidget::isTabReorderingEnabled() const
{
  return static_cast<KTabBar*>(tabBar())->isTabReorderingEnabled();
}

void KTabWidget::setTabCloseActivatePrevious( bool previous)
{
  static_cast<KTabBar*>(tabBar())->setTabCloseActivatePrevious( previous );
}

bool KTabWidget::tabCloseActivatePrevious() const
{
  return static_cast<KTabBar*>(tabBar())->tabCloseActivatePrevious();
}

int KTabWidget::tabBarWidthForMaxChars( int maxLength )
{
  int hframe, overlap;
  hframe  = tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabHSpace, 0L, tabBar() );
  overlap = tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabOverlap, 0L, tabBar() );

  const QFontMetrics fm = tabBar()->fontMetrics();
  int x = 0;
  for ( int i = 0; i < count(); ++i ) {
    QString newTitle = d->m_tabNames[ i ];
    newTitle = KStringHandler::rsqueeze( newTitle, maxLength ).leftJustified( d->m_minLength, ' ' );

    int lw = fm.width( newTitle );
    int iw = 0;
    if ( !tabBar()->tabIcon( i ).isNull() ) {
      iw = tabBar()->tabIcon( i ).pixmap( style()->pixelMetric( QStyle::PM_SmallIconSize ), QIcon::Normal ).width() + 4;
    }
    if ( isCloseButtonEnabled() ) {
      // FIXME: how to get the size of the close button directly from the tabBar()?
      iw += KIconLoader::SizeSmall * 3 / 2;
    }
    x += ( tabBar()->style()->sizeFromContents( QStyle::CT_TabBarTab, 0L,
         QSize( qMax( lw + hframe + iw, QApplication::globalStrut().width() ), 0 ),
         this ) ).width();
  }

  return x;
}

QString KTabWidget::tabText( int index ) const
{
    if ( d->m_automaticResizeTabs ) {
        if (index >= 0 && index < count()) {
            if (index >= d->m_tabNames.count()) {
                // Ooops, the tab exists, but tabInserted wasn't called yet.
                // This can happen when inserting the first tab,
                // and calling tabText from slotCurrentChanged,
                // see KTabWidget_UnitTest.
                const_cast<KTabWidget*>(this)->tabInserted(index);
            }
            return d->m_tabNames[ index ];
        }
        else
            return QString();
    }
    else
        return QTabWidget::tabText( index );
}

void KTabWidget::setTabText( int index, const QString &text )
{
  if (text == tabText(index))
    return;

  if ( d->m_automaticResizeTabs ) {

    tabBar()->setUpdatesEnabled(false); //no flicker

    QTabWidget::setTabText( index, text );

    if ( index != -1 ) {
      d->m_tabNames[ index ] = text;
      d->resizeTabs( index );
    }

    tabBar()->setUpdatesEnabled(true);

  } else {
    QTabWidget::setTabText( index, text );
  }
}


void KTabWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if ( d->isEmptyTabbarSpace( event->pos() ) ) {
    bool accept = false;
    // The receivers of the testCanDecode() signal has to adjust
    // 'accept' accordingly.
    emit testCanDecode( event, accept);

    event->setAccepted( accept );
    return;
  }

  QTabWidget::dragEnterEvent( event );
}

void KTabWidget::dragMoveEvent( QDragMoveEvent *event )
{
  if ( d->isEmptyTabbarSpace( event->pos() ) ) {
    bool accept = false;
    // The receivers of the testCanDecode() signal has to adjust
    // 'accept' accordingly.
    emit testCanDecode( event, accept);

    event->setAccepted( accept );
    return;
  }

  QTabWidget::dragMoveEvent( event );
}

void KTabWidget::dropEvent( QDropEvent *event )
{
  if ( d->isEmptyTabbarSpace( event->pos() ) ) {
    emit ( receivedDropEvent( event ) );
    return;
  }

  QTabWidget::dropEvent( event );
}

#ifndef QT_NO_WHEELEVENT
void KTabWidget::wheelEvent( QWheelEvent *event )
{
  if ( event->orientation() == Qt::Horizontal )
    return;

  if ( d->isEmptyTabbarSpace( event->pos() ) )
    wheelDelta( event->delta() );
  else
    event->ignore();
}

void KTabWidget::wheelDelta( int delta )
{
  if ( count() < 2 )
    return;

  int page = currentIndex();
  if ( delta < 0 )
     page = (page + 1) % count();
  else {
    page--;
    if ( page < 0 )
      page = count() - 1;
  }
  setCurrentIndex( page );
}
#endif

void KTabWidget::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  if ( d->isEmptyTabbarSpace( event->pos() ) ) {
    emit( mouseDoubleClick() );
    return;
  }

  QTabWidget::mouseDoubleClickEvent( event );
}

void KTabWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::RightButton ) {
    if ( d->isEmptyTabbarSpace( event->pos() ) ) {
      emit( contextMenu( mapToGlobal( event->pos() ) ) );
      return;
    }
  } else if ( event->button() == Qt::MidButton ) {
    if ( d->isEmptyTabbarSpace( event->pos() ) ) {
      emit( mouseMiddleClick() );
      return;
    }
  }

  QTabWidget::mousePressEvent( event );
}

void KTabWidget::receivedDropEvent( int index, QDropEvent *event )
{
  emit( receivedDropEvent( widget( index ), event ) );
}

void KTabWidget::initiateDrag( int index )
{
  emit( initiateDrag( widget( index ) ) );
}

void KTabWidget::contextMenu( int index, const QPoint &point )
{
  emit( contextMenu( widget( index ), point ) );
}

void KTabWidget::mouseDoubleClick( int index )
{
  emit( mouseDoubleClick( widget( index ) ) );
}

void KTabWidget::mouseMiddleClick( int index )
{
  emit( mouseMiddleClick( widget( index ) ) );
}

void KTabWidget::moveTab( int from, int to )
{
  setUpdatesEnabled(false);

  const QString tablabel = tabText( from );
  QWidget *w = widget( from );
  const QColor color = tabTextColor( from );
  const QIcon tabiconset = tabIcon( from );
  const QString tabtooltip = tabToolTip( from );
  const bool current = ( from == currentIndex() );
  const bool enabled = isTabEnabled( from );

  const bool blocked = blockSignals( true );
  removeTab( from );
  insertTab( to, w, tablabel );

  setTabIcon( to, tabiconset );
  setTabText( to, tablabel );
  setTabToolTip( to, tabtooltip );
  setTabTextColor( to, color );
  if ( current )
    setCurrentIndex( to );
  setTabEnabled( to, enabled );
  if ( d->m_automaticResizeTabs ) {
    d->resizeTabs( to );
  }
  blockSignals( blocked );

  setUpdatesEnabled(true);

  emit ( movedTab( from, to ) );
}

void KTabWidget::removePage( QWidget *widget )
{
  if ( d->m_automaticResizeTabs ) {

    setUpdatesEnabled(false);

    d->removeTab( indexOf( widget ) );
    d->resizeTabs();

    setUpdatesEnabled(true);

  } else {
    d->removeTab( indexOf( widget ) );
  }
}

void KTabWidget::removeTab( int index )
{
  if ( d->m_automaticResizeTabs ) {

    setUpdatesEnabled(false);

    d->removeTab( index );
    d->resizeTabs();

    setUpdatesEnabled(true);

  } else {
    d->removeTab( index );
  }
}

void KTabWidget::setHoverCloseButton( bool button )
{
  // deprecated
  setCloseButtonEnabled( button );
}

bool KTabWidget::hoverCloseButton() const
{
  // deprecated
  return false;
}

void KTabWidget::setHoverCloseButtonDelayed( bool delayed )
{
  // deprecated
  Q_UNUSED( delayed );
}

bool KTabWidget::hoverCloseButtonDelayed() const
{
  // deprecated
  return isCloseButtonEnabled();
}

void KTabWidget::setCloseButtonEnabled( bool enable )
{
  static_cast<KTabBar*>( tabBar() )->setCloseButtonEnabled( enable );
}

bool KTabWidget::isCloseButtonEnabled() const
{
  return static_cast<KTabBar*>( tabBar() )->isCloseButtonEnabled();
}

void KTabWidget::setAutomaticResizeTabs( bool enabled )
{
  if ( d->m_automaticResizeTabs == enabled )
    return;

  setUpdatesEnabled(false);

  d->m_automaticResizeTabs = enabled;
  if ( enabled ) {
    d->m_tabNames.clear();
    for ( int i = 0; i < count(); ++i )
      d->m_tabNames.append( tabBar()->tabText( i ) );
  } else
    for ( int i = 0; i < count(); ++i )
      tabBar()->setTabText( i, d->m_tabNames[ i ] );

  d->resizeTabs();

  setUpdatesEnabled(true);
}

bool KTabWidget::automaticResizeTabs() const
{
  return d->m_automaticResizeTabs;
}

void KTabWidget::closeRequest( int index )
{
  emit( closeRequest( widget( index ) ) );
}

void KTabWidget::resizeEvent( QResizeEvent *event )
{
  QTabWidget::resizeEvent( event );
  d->resizeTabs();
}

void KTabWidget::tabInserted( int idx )
{
   d->m_tabNames.insert( idx, tabBar()->tabText( idx ) );
}

void KTabWidget::tabRemoved( int idx )
{
   d->m_tabNames.removeAt( idx );
}

void KTabWidget::currentChanged( int idx )
{
   // The tab should appear only once
   d->m_previousTabList.removeOne( widget(idx) );
   
   d->m_previousTabList.push_front( widget(idx) );
}

#include "ktabwidget.moc"
