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
#include <kdebug.h>

#include <QBitmap>
#include <QCursor>
#include <QEvent>
#include <QAbstractScrollArea>
#include <QTimer>
#include <QWidget>
#include <QFile>

#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <config.h>

#ifdef Q_WS_X11
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
#endif

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif

#include <fixx11h.h>


namespace
{
    // Borrowed from xc/lib/Xcursor/library.c
    static const char * const standard_names[] = {
        /* 0 */
        "X_cursor",         "arrow",            "based_arrow_down",     "based_arrow_up",
        "boat",             "bogosity",         "bottom_left_corner",   "bottom_right_corner",
        "bottom_side",      "bottom_tee",       "box_spiral",           "center_ptr",
        "circle",           "clock",            "coffee_mug",           "cross",

        /* 32 */
        "cross_reverse",    "crosshair",        "diamond_cross",        "dot",
        "dotbox",           "double_arrow",     "draft_large",          "draft_small",
        "draped_box",       "exchange",         "fleur",                "gobbler",
        "gumby",            "hand1",            "hand2",                "heart",

        /* 64 */
        "icon",             "iron_cross",       "left_ptr",             "left_side",
        "left_tee",         "leftbutton",       "ll_angle",             "lr_angle",
        "man",              "middlebutton",     "mouse",                "pencil",
        "pirate",           "plus",             "question_arrow",       "right_ptr",

        /* 96 */
        "right_side",       "right_tee",        "rightbutton",          "rtl_logo",
        "sailboat",         "sb_down_arrow",    "sb_h_double_arrow",    "sb_left_arrow",
        "sb_right_arrow",   "sb_up_arrow",      "sb_v_double_arrow",    "shuttle",
        "sizing",           "spider",           "spraycan",             "star",

        /* 128 */
        "target",           "tcross",           "top_left_arrow",       "top_left_corner",
        "top_right_corner", "top_side",         "top_tee",              "trek",
        "ul_angle",         "umbrella",         "ur_angle",             "watch",
        "xterm",
    };

    static Qt::HANDLE x11LoadXcursor(const QString &name)
    {
#ifdef HAVE_XCURSOR
        return XcursorLibraryLoadCursor(QX11Info::display(), QFile::encodeName(name));
#else
        return 0;
#endif
    }

    static int x11CursorShape(const QString &name)
    {
        static QHash<QString, int> shapes;

        // A font cursor is created from two glyphs; a shape glyph and a mask glyph
        // stored in pairs in the font, with the shape glyph first. There's only one
        // name for each pair. This function always returns the index for the
        // shape glyph.
        if (shapes.isEmpty())
        {
            int num = XC_num_glyphs / 2;
            shapes.reserve(num + 5);

            for (int i = 0; i < num; ++i)
                shapes.insert(standard_names[i], i << 1);

            // Qt uses alternative names for some core cursors
            shapes.insert("size_all",      XC_fleur);
            shapes.insert("up_arrow",      XC_center_ptr);
            shapes.insert("ibeam",         XC_xterm);
            shapes.insert("wait",          XC_watch);
            shapes.insert("pointing_hand", XC_hand2);
        }

        return shapes.value(name, -1);
    }

    static Qt::HANDLE x11LoadFontCursor(const QString &name)
    {
        int shape = x11CursorShape(name);

        if (shape != -1)
            return XCreateFontCursor(QX11Info::display(), shape);

        return 0;
    }

    bool x11HaveXfixes()
    {
        bool result = false;

#ifdef HAVE_XFIXES
        int event_base, error_base;
        if (XFixesQueryExtension(QX11Info::display(), &event_base, &error_base))
        {
            int major, minor;
            XFixesQueryVersion(QX11Info::display(), &major, &minor);
            result = (major >= 2);
        }
#endif
        return result;
    }

    static void x11SetCursorName(Qt::HANDLE handle, const QString &name)
    {
#ifdef HAVE_XFIXES
        static bool haveXfixes = x11HaveXfixes();

        if (haveXfixes)
            XFixesSetCursorName(QX11Info::display(), handle, QFile::encodeName(name));
#endif
    }
}
#endif // Q_WS_X11


KCursor::KCursor( const QString& name, Qt::CursorShape fallback )
    : QCursor( fallback ),
      d( 0 )
{
#ifdef Q_WS_X11
    Qt::HANDLE handle = x11LoadXcursor(name);

    if (!handle)
        handle = x11LoadFontCursor(name);

    // Unfortunately QCursor doesn't have a setHandle()
    if (handle)
        *this = KCursor(handle);

    x11SetCursorName(QCursor::handle(), name);
#else
    Q_UNUSED( name )
#endif
}


KCursor::KCursor( const QCursor &cursor )
    : QCursor( cursor ), d( 0 )
{
}

KCursor &KCursor::operator=( const KCursor &cursor )
{
    QCursor::operator=( cursor );
    return *this;
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
    mouseWidget()->setMouseTracking( true );
    connect( &m_autoHideTimer, SIGNAL( timeout() ),
             this, SLOT( hideCursor() ) );
}

KCursorPrivateAutoHideEventFilter::~KCursorPrivateAutoHideEventFilter()
{
    if( m_widget != NULL )
        mouseWidget()->setMouseTracking( m_wasMouseTracking );
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

    QWidget* w = mouseWidget();

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

    QWidget* w = mouseWidget();

    if ( w->cursor().shape() != Qt::BlankCursor ) // someone messed with the cursor already
	return;

    if ( m_isOwnCursor )
        w->setCursor( m_oldCursor );
    else
        w->unsetCursor();
}

// The widget which gets mouse events, and that shows the cursor
// (that is the viewport, for a QAbstractScrollArea)
QWidget* KCursorPrivateAutoHideEventFilter::mouseWidget() const
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
    Q_UNUSED(o);
    // o is m_widget or its viewport
    //Q_ASSERT( o == m_widget );

    switch ( e->type() )
    {
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

    QWidget* viewport = 0;
    QAbstractScrollArea * sv = qobject_cast<QAbstractScrollArea *>( w );
    if ( sv )
        viewport = sv->viewport();

    if ( enable )
    {
        if ( m_eventFilters.contains( w ) )
            return;
        KCursorPrivateAutoHideEventFilter* filter = new KCursorPrivateAutoHideEventFilter( w );
        m_eventFilters.insert( w, filter );
        if (viewport) {
            m_eventFilters.insert( viewport, filter );
            connect(viewport, SIGNAL(destroyed(QObject *)), this, SLOT(slotViewportDestroyed(QObject *)));
        }
        if ( !customEventFilter ) {
            w->installEventFilter( filter ); // for key events
            if (viewport)
                viewport->installEventFilter( filter ); // for mouse events
        }
        connect( w, SIGNAL( destroyed(QObject*) ),
                 this, SLOT( slotWidgetDestroyed(QObject*) ) );
    }
    else
    {
        KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( w );
        if ( filter == 0 )
            return;
        w->removeEventFilter( filter );
        if (viewport) {
            m_eventFilters.remove( viewport );
            disconnect(viewport, SIGNAL(destroyed(QObject *)), this, SLOT(slotViewportDestroyed(QObject *)));
            viewport->removeEventFilter( filter );
        }
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

void KCursorPrivate::slotViewportDestroyed(QObject *o)
{
    m_eventFilters.remove(o);
}

void KCursorPrivate::slotWidgetDestroyed( QObject* o )
{
    KCursorPrivateAutoHideEventFilter* filter = m_eventFilters.take( o );

    Q_ASSERT( filter != 0 );

    filter->resetWidget(); // so that dtor doesn't access it
    delete filter;
}

#include "kcursor_p.moc"
