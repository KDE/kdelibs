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

#include <qapplication.h>
#include <qstyle.h>
#include <qstylesheet.h>

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
        KConfigGroupSaver groupsaver(KGlobal::config(), "General");
        m_maxLength = KGlobal::config()->readNumEntry("MaximumTabLength", 30);
        m_minLength = KGlobal::config()->readNumEntry("MinimumTabLength", 3);
        m_CurrentMaxLength = m_minLength;
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

void KTabWidget::insertTab( QWidget *child, const QString &label, int index )
{
    QTabWidget::insertTab( child, label, index );
}

void KTabWidget::insertTab( QWidget *child, const QIconSet& iconset, const QString &label, int index )
{
    QTabWidget::insertTab( child, iconset, label, index );
}

void KTabWidget::insertTab( QWidget *child, QTab *tab, int index )
{
    QTabWidget::insertTab( child, tab, index);
    if ( d->m_automaticResizeTabs ) {
        if ( index < 0 || index >= count() ) {
            d->m_tabNames.append( tab->text() );
            resizeTabs( d->m_tabNames.count()-1 );
        }
        else {
            d->m_tabNames.insert( d->m_tabNames.at( index ), tab->text() );
            resizeTabs( index );
        }
    }
}

void KTabWidget::setTabBarHidden( bool hide )
{
    QWidget *rightcorner = this->cornerWidget( TopRight );
    QWidget *leftcorner = this->cornerWidget( TopLeft );

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
    hframe  = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabHSpace, tabBar() );
    overlap = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabOverlap, tabBar() );

    QFontMetrics fm = tabBar()->fontMetrics();
    int x = 0;
    for( int i=0; i < count(); ++i ) {
        QString newTitle = d->m_tabNames[ i ];
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

void KTabWidget::changeTab( QWidget *w, const QString &label )
{
    QTabWidget::changeTab( w, label );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = label;
            resizeTabs( index );
        }
    }
}

void KTabWidget::changeTab( QWidget *w, const QIconSet &iconset, const QString &label )
{
    QTabWidget::changeTab( w, iconset, label );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = label;
            resizeTabs( index );
        }
    }
}

QString KTabWidget::label( int index ) const
{
    if ( d->m_automaticResizeTabs ) {
        if ( index >= 0 && index < count() )
            return d->m_tabNames[ index ];
        else
            return QString::null;
    }
    else
        return QTabWidget::label( index );
}

QString KTabWidget::tabLabel( QWidget * w ) const
{
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index == -1 )
            return QString::null;
        else
            return d->m_tabNames[ index ];
    }
    else
        return QTabWidget::tabLabel( w );
}

void KTabWidget::setTabLabel( QWidget *w, const QString &l )
{
    QTabWidget::setTabLabel( w, l );
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 ) {
            d->m_tabNames[ index ] = l;
            resizeTabs( index );
        }
  }
}

void KTabWidget::resizeTabs( int changeTabIndex )
{
    uint newMaxLength;
    if ( d->m_automaticResizeTabs ) {
        // Calculate new max length
        newMaxLength=d->m_maxLength;
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
    }
    else
        newMaxLength = 4711;

    // Update hinted or all tabs
    if ( d->m_CurrentMaxLength != newMaxLength ) {
        d->m_CurrentMaxLength = newMaxLength;
        for( int i = 0; i < count(); ++i )
            updateTab( i );
    }
    else if ( changeTabIndex != -1 )
        updateTab( changeTabIndex );
}

void KTabWidget::updateTab( int index )
{
    QString title = d->m_automaticResizeTabs ? d->m_tabNames[ index ] : QTabWidget::label( index );
    removeTabToolTip( page( index ) );
    if ( title.length() > d->m_CurrentMaxLength ) {
        if ( QStyleSheet::mightBeRichText( title ) )
            setTabToolTip( page( index ), QStyleSheet::escape(title) );
        else
            setTabToolTip( page( index ), title );
    }

    title = KStringHandler::rsqueeze( title, d->m_CurrentMaxLength ).leftJustify( d->m_minLength, ' ' );
    title.replace( '&', "&&" );

    if ( QTabWidget::label( index ) != title )
        QTabWidget::setTabLabel( page( index ), title );
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
    if ( d->m_automaticResizeTabs ) {
        if ( to < 0 || to >= count() )
            d->m_tabNames.append( QString::null );
        else
            d->m_tabNames.insert( d->m_tabNames.at( to ), QString::null );
    }

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

void KTabWidget::removePage( QWidget * w ) {
    if ( d->m_automaticResizeTabs ) {
        int index = indexOf( w );
        if ( index != -1 )
            d->m_tabNames.remove( d->m_tabNames.at( index ) );
    }
    QTabWidget::removePage( w );
    if ( d->m_automaticResizeTabs )
        resizeTabs();
}


bool KTabWidget::isEmptyTabbarSpace( const QPoint &p ) const
{
    QPoint point( p );
    QSize size( tabBar()->sizeHint() );
    if ( ( tabPosition()==Top && point.y()< size.height() ) || ( tabPosition()==Bottom && point.y()>(height()-size.height() ) ) ) {
        QWidget *rightcorner = cornerWidget( TopRight );
        if ( rightcorner ) {
            if ( point.x()>=width()-rightcorner->width() )
                return false;
        }
        QWidget *leftcorner = cornerWidget( TopLeft );
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
    if ( d->m_automaticResizeTabs==enabled )
        return;

    d->m_automaticResizeTabs = enabled;
    if ( enabled ) {
        d->m_tabNames.clear();
        for( int i = 0; i < count(); ++i )
            d->m_tabNames.append( tabBar()->tabAt( i )->text() );
    }
    else
        for( int i = 0; i < count(); ++i )
            tabBar()->tabAt( i )->setText( d->m_tabNames[ i ] );
    resizeTabs();
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
    resizeTabs();
}

#include "ktabwidget.moc"
