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
#include <kapplication.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qevent.h>
#include <qtimer.h>
#include <qwidget.h>

#include <kglobal.h>
#include <kconfig.h>
#include <qscrollview.h>

#include "kcursor_private.h"

KCursor::KCursor()
{
}

QCursor KCursor::handCursor()
{
        static QCursor *hand_cursor = 0;

        if (hand_cursor == 0)
        {
                KConfig *config = KGlobal::config();
                KConfigGroupSaver saver( config, "General" );

                if ( config->readEntry("handCursorStyle", "Windows") == "Windows" )
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
                        hand_cursor = new QCursor(PointingHandCursor);
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

        if (working_cursor == 0)
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
    return Qt::arrowCursor;
}


QCursor KCursor::upArrowCursor()
{
    return Qt::upArrowCursor;
}


QCursor KCursor::crossCursor()
{
    return Qt::crossCursor;
}


QCursor KCursor::waitCursor()
{
    return Qt::waitCursor;
}


QCursor KCursor::ibeamCursor()
{
    return Qt::ibeamCursor;
}


QCursor KCursor::sizeVerCursor()
{
    return Qt::sizeVerCursor;
}


QCursor KCursor::sizeHorCursor()
{
    return Qt::sizeHorCursor;
}


QCursor KCursor::sizeBDiagCursor()
{
    return Qt::sizeBDiagCursor;
}


QCursor KCursor::sizeFDiagCursor()
{
    return Qt::sizeFDiagCursor;
}


QCursor KCursor::sizeAllCursor()
{
    return Qt::sizeAllCursor;
}


QCursor KCursor::blankCursor()
{
    return Qt::blankCursor;
}

QCursor KCursor::whatsThisCursor()
{
    return Qt::whatsThisCursor;
}

// auto-hide cursor stuff

void KCursor::setAutoHideCursor( QWidget *w, bool enable )
{
    setAutoHideCursor( w, enable, false );
}

void KCursor::setAutoHideCursor( QWidget *w, bool enable,
				 bool customEventFilter )
{
    if ( !w )
        return;

    KCursorPrivate *kp = KCursorPrivate::self();
    if ( !kp->enabled )
	return;

    if ( enable ) {
        kp->start();
        w->setMouseTracking( true );

	if ( !customEventFilter )
	    w->installEventFilter( kp );
	else // safety
	    w->removeEventFilter( kp );
    }

    else {
        w->setMouseTracking( false );
        w->removeEventFilter( kp );
        kp->stop();
    }
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
// **************************************************************************

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
    count = 0;
    autoHideTimer = 0;
    hideCursorDelay = 5000; // 5s default value
    isCursorHidden = false;
    isOwnCursor = false;
    hideWidget = 0L;

    KConfig *kc = KGlobal::config();
    KConfigGroupSaver ks( kc, QString::fromLatin1("KDE") );
    enabled = kc->readBoolEntry(
		  QString::fromLatin1("Autohiding cursor enabled"), true );
}

KCursorPrivate::~KCursorPrivate()
{
}

void KCursorPrivate::start()
{
    if ( !autoHideTimer ) {
        autoHideTimer = new QTimer(qApp);
        QObject::connect( autoHideTimer, SIGNAL( timeout() ),
                          this, SLOT( slotHideCursor() ));
    }
    count++;
}

void KCursorPrivate::stop()
{
    count--;
    if ( count <= 0 ) {
        delete autoHideTimer;
        autoHideTimer = 0L;
        count = 0;
    }
}

void KCursorPrivate::hideCursor( QWidget *w )
{
    if ( !w )
        return;
    // Is w a scrollview ? Call setCursor on the viewport in that case.
    QScrollView * sv = dynamic_cast<QScrollView *>( w );
    if ( sv )
        w = sv->viewport();

    isCursorHidden = true;
    isOwnCursor = w->ownCursor();
    if ( isOwnCursor )
        oldCursor = w->cursor();

    w->setCursor( KCursor::blankCursor() );
}

void KCursorPrivate::unhideCursor( QWidget *w )
{
    isCursorHidden = false;

    if ( w ) {
        QScrollView * sv = dynamic_cast<QScrollView *>( w );
        if ( sv )
            w = sv->viewport();

        if ( isOwnCursor )
            w->setCursor( oldCursor );
        else
            w->unsetCursor();
    }
}


// what a mess :-/
// no kidding!
bool KCursorPrivate::eventFilter( QObject *o, QEvent *e )
{
    if ( !enabled || !o->isWidgetType() )
        return false;

    int t = e->type();
    
    // If it is not one of the events we respond to, ignore it.
    if( ! ( (t >= QEvent::MouseButtonPress && t <= QEvent::Leave) ||
            (t >= QEvent::Destroy && t <= QEvent::Hide) ||
             t == QEvent::AccelOverride ||
             t == QEvent::Wheel ) )
      return false;
    
    // disconnect() and connect() on events for a new widget
    if ( o != hideWidget ) {
        if ( hideWidget ) {
            disconnect( hideWidget, SIGNAL( destroyed() ),
                        this, SLOT( slotWidgetDestroyed()));
        }
        connect( o, SIGNAL( destroyed() ),
                 this, SLOT( slotWidgetDestroyed()));
    }

    QWidget *w = static_cast<QWidget *>( o );
    hideWidget = w;

    if ( t == QEvent::Leave || t == QEvent::FocusOut || t == QEvent::Destroy) {
	if ( autoHideTimer )
            autoHideTimer->stop();

        if ( isCursorHidden && t != QEvent::Destroy )
            unhideCursor( w );

        return false;
    }

    if ( t == QEvent::Create ) // Qt steals mouseTracking on create()
        w->setMouseTracking( true );

    // don't process events not coming from the focus-window
    // or from widgets that accept focus, but don't have it.
    if ( !w->isActiveWindow() || (w->isFocusEnabled() && !w->hasFocus()) ) {
        disconnect( hideWidget, SIGNAL( destroyed() ),
                    this, SLOT( slotWidgetDestroyed() ));
 	hideWidget = 0L;
 	return false;
    }

    else if ( t == QEvent::Enter ) {
        if ( isCursorHidden )
            unhideCursor( w );
        if ( autoHideTimer )
            autoHideTimer->start( hideCursorDelay, true );
    }

    else { // other than enter/leave/focus events
        
        if ( isCursorHidden ) {
            if ( t == QEvent::MouseButtonPress ||
                 t == QEvent::MouseButtonRelease ||
                 t == QEvent::MouseButtonDblClick || t == QEvent::MouseMove ||
                 t == QEvent::Show || t == QEvent::Hide ) {
                unhideCursor( w );
		autoHideTimer->start( hideCursorDelay, true );
	    }
	}

        else { // cursor not hidden yet
            if ( t == QEvent::KeyPress || t == QEvent::AccelOverride ) { //t == QEvent::KeyRelease ) {
                if ( insideWidget( QCursor::pos(), w )) {
                    hideCursor( w );
                    autoHideTimer->stop();
		}
            }

	    // restart the timer on user-input
            else if ( (t >= QEvent::MouseButtonPress &&
		       t <= QEvent::KeyRelease) ||
		      t == QEvent::Wheel || t == QEvent::AccelOverride ) {

                if ( insideWidget( QCursor::pos(), w ) && autoHideTimer )
                    autoHideTimer->start( hideCursorDelay, true );
            }
        }
    }
    return false;
}

void KCursorPrivate::slotHideCursor()
{
    if ( !isCursorHidden )
        hideCursor( hideWidget );
}

void KCursorPrivate::slotWidgetDestroyed()
{
    isCursorHidden = false;
    hideWidget = 0L;
}

bool KCursorPrivate::insideWidget( const QPoint &p, QWidget *w )
{
    return w->rect().contains( w->mapFromGlobal( p ) );
}

#include "kcursor_private.moc"
