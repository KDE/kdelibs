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

#include <qapplication.h>
#include <qstyle.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include "ktabwidget.h"
#include "ktabbar.h"

class KTabWidgetPrivate {
public:
    bool m_automaticResizeTabs;
    int m_maxLength;
    int m_minLength;
    unsigned int m_CurrentMaxLength;

    //holds the full names of the tab, otherwise all we
    //know about is the shortened name
    QStringList m_tabNames;

    KTabWidgetPrivate() {
        m_automaticResizeTabs = false;
        m_maxLength = KGlobal::config()->readNumEntry("MaximumTabLength", 30);
        m_minLength = KGlobal::config()->readNumEntry("MinimumTabLength", 3);
        m_CurrentMaxLength = m_minLength;
        QStringList m_tabNames;
    }
};

KTabWidget::KTabWidget( QWidget *parent, const char *name, WFlags f )
    : QTabWidget( parent, name, f )
{
    d = new KTabWidgetPrivate;
    setTabBar( new KTabBar(this, "tabbar") );
    setAcceptDrops( true );

    connect(tabBar(), SIGNAL(contextMenu( int, const QPoint & )), SLOT(contextMenu( int, const QPoint & )));
    connect(tabBar(), SIGNAL(mouseDoubleClick( int )), SLOT(mouseDoubleClick( int )));
    connect(tabBar(), SIGNAL(mouseMiddleClick( int )), SLOT(mouseMiddleClick( int )));
    connect(tabBar(), SIGNAL(initiateDrag( int )), SLOT(initiateDrag( int )));
    connect(tabBar(), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )), SIGNAL(testCanDecode(const QDragMoveEvent *, bool & )));
    connect(tabBar(), SIGNAL(receivedDropEvent( int, QDropEvent * )), SLOT(receivedDropEvent( int, QDropEvent * )));
    connect(tabBar(), SIGNAL(moveTab( int, int )), SLOT(moveTab( int, int )));
    connect(tabBar(), SIGNAL(closeRequest( int )), SLOT(closeRequest( int )));
#ifndef QT_NO_WHEELEVENT
    connect(tabBar(), SIGNAL(wheelDelta( int )), SLOT(wheelDelta( int )));
#endif
}

KTabWidget::~KTabWidget()
{
    delete d;
}

void KTabWidget::setTabColor( QWidget *w, const QColor& color )
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        static_cast<KTabBar*>(tabBar())->setTabColor( t->identifier(), color );
    }
}

QColor KTabWidget::tabColor( QWidget *w ) const
{
    QTab *t = tabBar()->tabAt( indexOf( w ) );
    if (t) {
        return static_cast<KTabBar*>(tabBar())->tabColor( t->identifier() );
    } else {
        return QColor();
    }
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

unsigned int KTabWidget::tabBarWidthForMaxChars( uint maxLength )
{
    int hframe, overlap;
    hframe  = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabHSpace, this );
    overlap = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabOverlap, this );

    QFontMetrics fm = tabBar()->fontMetrics();
    int x = 0;
    for( int i=0; i < count(); ++i ) {
	QString newTitle;
	if ( d->m_tabNames.count() >= (uint)i ) {
	    newTitle = d->m_tabNames[i];
        } else {
	    newTitle = tabLabel( page(i) );
	}

        newTitle = KStringHandler::rsqueeze( newTitle, maxLength ).leftJustify( d->m_minLength, ' ' );

        QTab* tab = tabBar()->tabAt( i );
        int lw = fm.width( newTitle );
        int iw = 0;
        if ( tab->iconSet() )
          iw = tab->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
        x += ( tabBar()->style().sizeFromContents( QStyle::CT_TabBarTab, this,
		   QSize( QMAX( lw + hframe + iw, QApplication::globalStrut().width() ), 0 ),
                   QStyleOption( tab ) ) ).width();
    }
    return x;
}

void KTabWidget::setTabLabel( QWidget* sender, const QString &title )
{
    uint tabIndex = indexOf(sender);
    if ( d->m_tabNames.count() <= tabIndex ) {
	d->m_tabNames.append(title);
    } else {
	d->m_tabNames[tabIndex] = title;
    }

    QString newTitle = title;
    uint newMaxLength=d->m_maxLength;
    if ( automaticResizeTabs() ) {
        removeTabToolTip( sender );

        uint lcw=0, rcw=0;

        int tabBarHeight = tabBar()->sizeHint().height();
        if ( cornerWidget( TopLeft ) && cornerWidget( TopLeft )->isVisible() )
          lcw = QMAX( cornerWidget( TopLeft )->width(), tabBarHeight );
        if ( cornerWidget( TopRight ) && cornerWidget( TopRight )->isVisible() )
          rcw = QMAX( cornerWidget( TopRight )->width(), tabBarHeight );

        uint maxTabBarWidth = width() - lcw - rcw;

        for ( ; newMaxLength > (uint)d->m_minLength; newMaxLength-- ) {
          if ( tabBarWidthForMaxChars( newMaxLength ) < maxTabBarWidth )
            break;
        }

        if ( newTitle.length() > newMaxLength )
            setTabToolTip( sender, newTitle );

        newTitle = KStringHandler::rsqueeze( newTitle, newMaxLength ).leftJustify( d->m_minLength, ' ' );
        newTitle.replace( '&', "&&" );
    }

    if ( tabLabel( sender ) != newTitle )
        QTabWidget::setTabLabel(sender, newTitle );

    if ( automaticResizeTabs() ) {
        if( newMaxLength != d->m_CurrentMaxLength ) {
            for( int i = 0; i < count(); ++i) {
	        if ( d->m_tabNames.count() >= (uint)i ) {
                    newTitle = d->m_tabNames[i];
                } else {
	            newTitle = tabLabel( page(i) );
	        }

                removeTabToolTip( page( i ) );
                if ( newTitle.length() > newMaxLength )
                    setTabToolTip( page( i ), newTitle );

                newTitle = KStringHandler::rsqueeze( newTitle, newMaxLength ).leftJustify( d->m_minLength, ' ' );

                newTitle.replace( '&', "&&" );
                if ( newTitle != tabLabel(page(i)) ) {
                    QTabWidget::setTabLabel( page(i), newTitle );
                }
            }
            d->m_CurrentMaxLength = newMaxLength;
	}
    }
}

void KTabWidget::dragMoveEvent( QDragMoveEvent *e )
{
    if ( isEmptyTabbarSpace( e->pos() ) ) {
        bool accept = false;
        // The receivers of the testCanDecode() signal has to adjust
        // 'accept' accordingly.
        emit testCanDecode( e, accept);
        e->accept( accept );
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

#ifndef QT_NO_WHEELEVENT
void KTabWidget::wheelEvent( QWheelEvent *e )
{
    if ( e->orientation() == Horizontal )
        return;

    if ( isEmptyTabbarSpace( e->pos() ) )
        wheelDelta( e->delta() );
    else
        e->ignore();
}

void KTabWidget::wheelDelta( int delta )
{
   if ( count() < 2 )
       return;

   int page = currentPageIndex();
   if ( delta < 0 )
       page = (page + 1) % count();
   else {
       page--;
       if ( page < 0 )
           page = count() - 1;
   }
   setCurrentPage( page );
}
#endif

void KTabWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    if( e->button() != LeftButton )
        return;

    if ( isEmptyTabbarSpace( e->pos() ) ) {
        emit( mouseDoubleClick() );
        return;
    }
    QTabWidget::mouseDoubleClickEvent( e );
}

void KTabWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( contextMenu( mapToGlobal( e->pos() ) ) );
            return;
        }
    } else if ( e->button() == MidButton ) {
        if ( isEmptyTabbarSpace( e->pos() ) ) {
            emit( mouseMiddleClick() );
            return;
        }
    }
    QTabWidget::mousePressEvent( e );
}

void KTabWidget::receivedDropEvent( int index, QDropEvent *e )
{
    emit( receivedDropEvent( page( index ), e ) );
}

void KTabWidget::initiateDrag( int index )
{
    emit( initiateDrag( page( index ) ) );
}

void KTabWidget::contextMenu( int index, const QPoint &p )
{
    emit( contextMenu( page( index ), p ) );
}

void KTabWidget::mouseDoubleClick( int index )
{
    emit( mouseDoubleClick( page( index ) ) );
}

void KTabWidget::mouseMiddleClick( int index )
{
    emit( mouseMiddleClick( page( index ) ) );
}

void KTabWidget::moveTab( int from, int to )
{
    QString tablabel = label( from );
    QWidget *w = page( from );
    QColor color = tabColor( w );
    QIconSet tabiconset = tabIconSet( w );
    QString tabtooltip = tabToolTip( w );
    bool current = ( w == currentPage() );
    bool enabled = isTabEnabled( w );
    blockSignals(true);
    removePage( w );

    // Work-around kmdi brain damage which calls showPage() in insertTab()
    QTab * t = new QTab();
    t->setText(tablabel);
    QTabWidget::insertTab( w, t, to );

    w = page( to );
    changeTab( w, tabiconset, tablabel );
    setTabToolTip( w, tabtooltip );
    setTabColor( w, color );
    if ( current )
        showPage( w );
    setTabEnabled( w, enabled );
    blockSignals(false);

    emit ( movedTab( from, to ) );
}

void KTabWidget::removePage ( QWidget * w ) {
    if ( automaticResizeTabs() && count() && !d->m_tabNames[0].isEmpty() ) {
	int i = indexOf( w );
	d->m_tabNames.remove( d->m_tabNames.at(i) );
    }
    QTabWidget::removePage( w );
}


bool KTabWidget::isEmptyTabbarSpace( const QPoint &p ) const
{
    QPoint point( p );
    QSize size( tabBar()->sizeHint() );
    if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
        // QTabWidget::cornerWidget isn't const even it doesn't write any data ;(
        KTabWidget *that = const_cast<KTabWidget*>(this);
        QWidget *rightcorner = that->cornerWidget( TopRight );
        if ( rightcorner ) {
            if ( point.x()>=width()-rightcorner->width() )
                return false;
        }
        QWidget *leftcorner = that->cornerWidget( TopLeft );
        if ( leftcorner ) {
            if ( point.x()<=leftcorner->width() )
                return false;
            point.setX( point.x()-size.height() );
        }
        if ( tabPosition()==Bottom )
            point.setY( point.y()-( height()-size.height() ) );
        QTab *tab = tabBar()->selectTab( point);
        if( !tab )
            return true;
    }
    return false;
}

void KTabWidget::setHoverCloseButton( bool button )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButton( button );
}

bool KTabWidget::hoverCloseButton() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButton();
}

void KTabWidget::setHoverCloseButtonDelayed( bool delayed )
{
    static_cast<KTabBar*>(tabBar())->setHoverCloseButtonDelayed( delayed );
}

bool KTabWidget::hoverCloseButtonDelayed() const
{
    return static_cast<KTabBar*>(tabBar())->hoverCloseButtonDelayed();
}

void KTabWidget::setAutomaticResizeTabs( bool enabled )
{
    d->m_automaticResizeTabs = enabled;
}

bool KTabWidget::automaticResizeTabs() const
{
    return d->m_automaticResizeTabs;
}

void KTabWidget::closeRequest( int index )
{
    emit( closeRequest( page( index ) ) );
}

void KTabWidget::resizeEvent( QResizeEvent *e )
{
    QTabWidget::resizeEvent( e );
    if ( automaticResizeTabs() && count() && !d->m_tabNames[0].isEmpty() ) {
	setTabLabel( page(0), d->m_tabNames[0] );
    }
}

#include "ktabwidget.moc"
