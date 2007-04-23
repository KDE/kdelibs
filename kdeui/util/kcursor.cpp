/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifdef KDE_USE_FINAL
#ifdef KeyRelease
#undef KeyRelease
#endif
#endif

#include "kcursor.h"
#include "kcursor_p.h"

#include <QBitmap>
#include <QCursor>
#include <QEvent>
#include <QAbstractScrollArea>
#include <QTimer>
#include <QWidget>

#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

KCursor::KCursor( const QString& name, Qt::CursorShape fallback )
    : QCursor( fallback ),
      d( 0 )
{
    Q_UNUSED( name )
    //FIXME: actually try and load cursor name. frederikh has this task.
}

void KCursor::setAutoHideCursor( QWidget *w, bool enable,
                                 bool customEventFilter )
{
    KCursorPrivate::self()->setAutoHideCursor( w, enable, customEventFilter );
}

void KCursor::autoHideEventFilter( QObject *o, QEvent *e )
{
    KCursorPrivate::self()->eventFilter( o, e );
}

void KCursor::setHideCursorDelay( int ms )
{
    KCursorPrivate::self()->hideCursorDelay = ms;
}

int KCursor::hideCursorDelay()
{
    return KCursorPrivate::self()->hideCursorDelay;
}

// **************************************************************************

KCursorPrivateAutoHideEventFilter::KCursorPrivateAutoHideEventFilter( QWidget* widget )
    : m_widget( widget )
    , m_wasMouseTracking( m_widget->hasMouseTracking() )
    , m_isCursorHidden( false )
    , m_isOwnCursor( false )
{
    m_widget->setMouseTracking( true );
    connect( &m_autoHideTimer, SIGNAL( timeout() ),
             this, SLOT( hideCursor() ) );
}

KCursorPrivateAutoHideEventFilter::~KCursorPrivateAutoHideEventFilter()
{
    if( m_widget != NULL )
        m_widget->setMouseTracking( m_wasMouseTracking );
}

void KCursorPrivateAutoHideEventFilter::resetWidget()
{
    m_widget = NULL;
}

void KCursorPrivateAutoHideEventFilter::hideCursor()
{
    m_autoHideTimer.stop();

    if ( m_isCursorHidden )
        return;

    m_isCursorHidden = true;

    QWidget* w = actualWidget();

    m_isOwnCursor = w->testAttribute(Qt::WA_SetCursor);
    if ( m_isOwnCursor )
        m_oldCursor = w->cursor();

    w->setCursor( QCursor( Qt::BlankCursor ) );
}

void KCursorPrivateAutoHideEventFilter::unhideCursor()
{
    m_autoHideTimer.stop();

    if ( !m_isCursorHidden )
        return;

    m_isCursorHidden = false;

    QWidget* w = actualWidget();

    if ( w->cursor().shape() != Qt::BlankCursor ) // someone messed with the cursor already
	return;

    if ( m_isOwnCursor )
        w->setCursor( m_oldCursor );
    else
        w->unsetCursor();
}

QWidget* KCursorPrivateAutoHideEventFilter::actualWidget() const
{
    QWidget* w = m_widget;

    // Is w a QAbstractScrollArea ? Call setCursor on the viewport in that case.
    QAbstractScrollArea * sv = qobject_cast<QAbstractScrollArea *>( w );
    if ( sv )
        w = sv->viewport();

    return w;
}

bool KCursorPrivateAutoHideEventFilter::eventFilter( QObject *o, QEvent *e )
{
    Q_ASSERT( o == m_widget );

    switch ( e->type() )
    {
    case QEvent::Create:
        // Qt steals mouseTracking on create()
        m_widget->setMouseTracking( true );
        break;
    case QEvent::Leave:
    case QEvent::FocusOut:
    case QEvent::WindowDeactivate:
        unhideCursor();
        break;
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride:
        hideCursor();
        break;
    case QEvent::Enter:
    case QEvent::FocusIn:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Show:
    case QEvent::Hide:
    case QEvent::Wheel:
        unhideCursor();
        if ( m_widget->hasFocus() )
        {
            m_autoHideTimer.setSingleShot( true );
            m_autoHideTimer.start( KCursorPrivate::self()->hideCursorDelay );
        }
        break;
    default:
        break;
    }

    return false;
}

KCursorPrivate * KCursorPrivate::s_self = 0L;

KCursorPrivate * KCursorPrivate::self()
{
    if ( !s_self )
        s_self = new KCursorPrivate;
    // WABA: Don't delete KCursorPrivate, it serves no real purpose.
    // Even worse it causes crashes because it seems to get deleted
    // during ~QApplication and ~QApplication doesn't seem to like it
    // when we delete a QCursor. No idea if that is a bug itself.

    return s_self;
}

KCursorPrivate::KCursorPrivate()
{
    hideCursorDelay = 5000; // 5s default value

    KConfigGroup cg( KGlobal::config(), QLatin1String("KDE") );
    enabled = cg.readEntry( QLatin1String("Autohiding cursor enabled"), true);
}

KCursorPrivate::~KCursorPrivate()
{
}

void KCursorPrivate::setAutoHideCursor( QWidget *w, bool enable, bool customEventFilter )
{
    if ( !w || !enabled )
        return;

    if ( enable )
    {
        if ( m_eventFilters.contains( w ) )
            return;
        KCursorPrivateAutoHideEventFilter* filter = new KCursorPrivateAutoHideEventFilter( w );
        m_eventFilters.insert( w, filter );
        if ( !customEventFilter )
            w->installEventFilter( filter );
        connect( w, SIGNAL( destroyed(QObject*) ),
                 this, SLOT( slotWidgetDestroyed(QObject*) ) );
    }
    else
    {
        KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( w );
        if ( filter == 0 )
            return;
        w->removeEventFilter( filter );
        delete filter;
        disconnect( w, SIGNAL( destroyed(QObject*) ),
                    this, SLOT( slotWidgetDestroyed(QObject*) ) );
    }
}

bool KCursorPrivate::eventFilter( QObject *o, QEvent *e )
{
    if ( !enabled )
        return false;

    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.value( o );

    Q_ASSERT( filter != 0 );
    if ( filter == 0 )
        return false;

    return filter->eventFilter( o, e );
}

void KCursorPrivate::slotWidgetDestroyed( QObject* o )
{
    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( o );

    Q_ASSERT( filter != 0 );

    filter->resetWidget(); // so that dtor doesn't access it
    delete filter;
}

#include "kcursor_p.moc"
