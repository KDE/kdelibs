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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifdef KDE_USE_FINAL
#ifdef KeyRelease
#undef KeyRelease
#endif
#endif

#include <kcursor.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qtimer.h>
#include <qwidget.h>

#include <kglobal.h>
#include <kconfig.h>
#include <q3scrollview.h>

#include "kcursor_private.h"

KCursor::KCursor()
{
}

QCursor KCursor::handCursor()
{
        static QCursor *hand_cursor = 0;

        if (!hand_cursor)
        {
                KConfigGroup cg( KGlobal::config(), "General" );

#ifndef Q_WS_WIN // this mask doesn't work too well on win32
                if ( cg.readEntry("handCursorStyle", "Windows") == "Windows" )
                {
                        static const unsigned char HAND_BITS[] = {
                                0x80, 0x01, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02,
                                0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x1e, 0x00, 0x40,
                                0xf2, 0x00, 0x40, 0x92, 0x01, 0x70, 0x92, 0x02, 0x50, 0x92, 0x04,
                                0x48, 0x80, 0x04, 0x48, 0x00, 0x04, 0x48, 0x00, 0x04, 0x08, 0x00,
                                0x04, 0x08, 0x00, 0x04, 0x10, 0x00, 0x04, 0x10, 0x00, 0x04, 0x20,
                                0x00, 0x02, 0x40, 0x00, 0x02, 0x40, 0x00, 0x01, 0xc0, 0xff, 0x01};
                        static const unsigned char HAND_MASK_BITS[] = {
                                0x80, 0x01, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03,
                                0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x1f, 0x00, 0xc0,
                                0xff, 0x00, 0xc0, 0xff, 0x01, 0xf0, 0xff, 0x03, 0xf0, 0xff, 0x07,
                                0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff,
                                0x07, 0xf8, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xe0,
                                0xff, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0xff, 0x01, 0xc0, 0xff, 0x01};
                        QBitmap hand_bitmap(22, 22, HAND_BITS, true);
                        QBitmap hand_mask(22, 22, HAND_MASK_BITS, true);
                        hand_cursor = new QCursor(hand_bitmap, hand_mask, 7, 0);
                        // Hack to force QCursor to call XCreatePixmapCursor() immediately
                        // so the bitmaps don't get pushed out of the Xcursor LRU cache.
                        hand_cursor->handle();
                }
                else
#endif //! Q_WS_WIN
                        hand_cursor = new QCursor(Qt::PointingHandCursor);
        }

        Q_CHECK_PTR(hand_cursor);
        return *hand_cursor;
}

/* XPM */
static const char * const working_cursor_xpm[]={
"32 32 3 1",
"# c None",
"a c #000000",
". c #ffffff",
"..##############################",
".a.##########.aaaa.#############",
".aa.#########.aaaa.#############",
".aaa.#######.aaaaaa.############",
".aaaa.#####.a...a..a..##########",
".aaaaa.####a....a...aa##########",
".aaaaaa.###a...aa...aa##########",
".aaaaaaa.##a..a.....aa##########",
".aaaaaaaa.#.aa.....a..##########",
".aaaaa....##.aaaaaa.############",
".aa.aa.######.aaaa.#############",
".a.#.aa.#####.aaaa.#############",
"..##.aa.########################",
"#####.aa.#######################",
"#####.aa.#######################",
"######..########################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################",
"################################"};


QCursor KCursor::workingCursor()
{
        static QCursor *working_cursor = 0;

        if (!working_cursor)
        {
            QPixmap pm( const_cast< const char** >( working_cursor_xpm ));
            working_cursor = new QCursor( pm, 1, 1 );
            // Hack to force QCursor to call XCreatePixmapCursor() immediately
            // so the bitmaps don't get pushed out of the Xcursor LRU cache.
            working_cursor->handle();
        }

        Q_CHECK_PTR(working_cursor);
        return *working_cursor;
}

/**
 * All of the follow functions will return the Qt default for now regardless
 * of the style.  This will change at some later date
 */
QCursor KCursor::arrowCursor()
{
    return Qt::ArrowCursor;
}


QCursor KCursor::upArrowCursor()
{
    return Qt::UpArrowCursor;
}


QCursor KCursor::crossCursor()
{
    return Qt::CrossCursor;
}


QCursor KCursor::waitCursor()
{
    return Qt::WaitCursor;
}


QCursor KCursor::ibeamCursor()
{
    return Qt::IBeamCursor;
}


QCursor KCursor::sizeVerCursor()
{
    return Qt::SizeVerCursor;
}


QCursor KCursor::sizeHorCursor()
{
    return Qt::SizeHorCursor;
}


QCursor KCursor::sizeBDiagCursor()
{
    return Qt::SizeBDiagCursor;
}


QCursor KCursor::sizeFDiagCursor()
{
    return Qt::SizeFDiagCursor;
}


QCursor KCursor::sizeAllCursor()
{
    return Qt::SizeAllCursor;
}


QCursor KCursor::blankCursor()
{
    return Qt::BlankCursor;
}

QCursor KCursor::whatsThisCursor()
{
    return Qt::WhatsThisCursor;
}

// auto-hide cursor stuff

void KCursor::setAutoHideCursor( QWidget *w, bool enable )
{
    setAutoHideCursor( w, enable, false );
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

    m_isOwnCursor = w->ownCursor();
    if ( m_isOwnCursor )
        m_oldCursor = w->cursor();

    w->setCursor( KCursor::blankCursor() );
}

void KCursorPrivateAutoHideEventFilter::unhideCursor()
{
    m_autoHideTimer.stop();

    if ( !m_isCursorHidden )
        return;

    m_isCursorHidden = false;

    QWidget* w = actualWidget();

    if ( m_isOwnCursor )
        w->setCursor( m_oldCursor );
    else
        w->unsetCursor();
}

QWidget* KCursorPrivateAutoHideEventFilter::actualWidget() const
{
    QWidget* w = m_widget;

    // Is w a scrollview ? Call setCursor on the viewport in that case.
    Q3ScrollView * sv = dynamic_cast<Q3ScrollView *>( w );
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
            m_autoHideTimer.start( KCursorPrivate::self()->hideCursorDelay, true );
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
    // WABA: We never delete KCursorPrivate. Don't change.

    return s_self;
}

KCursorPrivate::KCursorPrivate()
{
    hideCursorDelay = 5000; // 5s default value

    KConfigGroup cg( KGlobal::config(), QLatin1String("KDE") );
    enabled = cg.readBoolEntry(
		  QLatin1String("Autohiding cursor enabled"), true );
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
        if ( m_eventFilters.find( w ) != NULL )
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
        if ( filter == NULL )
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

    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.find( o );

    Q_ASSERT( filter != NULL );
    if ( filter == NULL )
        return false;

    return filter->eventFilter( o, e );
}

void KCursorPrivate::slotWidgetDestroyed( QObject* o )
{
    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( o );

    Q_ASSERT( filter != NULL );

    filter->resetWidget(); // so that dtor doesn't access it
    delete filter;
}

#include "kcursor_private.moc"
