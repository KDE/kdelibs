/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef _KCURSOR_H
#define _KCURSOR_H

#include <qcursor.h>
#include <kdelibs_export.h>

class QEvent;
class QWidget;

/**
 * @short A QCursor wrapper allowing "themed" cursors and auto-hiding cursors.
 *
 * A wrapper around QCursor that allows for "themed" cursors.
 *
 * Currently, the only themed cursor is a hand shaped cursor.
 *
 * A typical usage would be
 * \code
 * setCursor(KCursor::handCursor());
 * \endcode
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KCursor 
{
public:
    /**
     * Constructor.
     *
     * Does not do anything so far.
     **/
    KCursor();

    /**
     * Returns the proper hand cursor according to
     * the current GUI style (static function).
     */
    static QCursor handCursor();

    /**
     * Returns the proper arrow+hourglass cursor according to
     * the current GUI style (static function).
     */
    static QCursor workingCursor();

    /**
     * Returns the proper arrow cursor according to
     * the current GUI style (static function).
     */
    static QCursor arrowCursor();

    /**
     * Returns the proper up arrow cursor according to
     * the current GUI style (static function).
     */
    static QCursor upArrowCursor();

    /**
     * Returns the proper cross-hair cursor according to
     * the current GUI style (static function).
     */
    static QCursor crossCursor();

    /**
     * Returns the proper hourglass cursor according to
     * the current GUI style (static function).
     */
    static QCursor waitCursor();

    /**
     * Returns the proper text cursor according to
     * the current GUI style (static function).
     */
    static QCursor ibeamCursor();

    /**
     * Returns the proper vertical resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeVerCursor();

    /**
     * Returns the proper horizontal resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeHorCursor();

    /**
     * Returns the proper diagonal resize (/) cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeBDiagCursor();

    /**
     * Returns the proper diagonal resize (\) cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeFDiagCursor();

    /**
     * Returns the proper all-directions resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeAllCursor();

    /**
     * Returns a blank or invisible cursor (static function).
     */
    static QCursor blankCursor();

    /**
     * Returns a WhatsThis cursor (static function).
     */
    static QCursor whatsThisCursor();

    /**
     * Sets auto-hiding the cursor for widget @p w. Enabling it will result in
     * the cursor being hidden when
     * @li a key-event happens
     * @li there are no key-events for a configured time-frame (see
     * setHideCursorDelay())
     *
     * The cursor will be shown again when the focus is lost or a mouse-event
     * happens.
     *
     * Side effect: when enabling auto-hide, mouseTracking is enabled for the
     * specified widget, because it's needed to get mouse-move-events. So
     * don't disable mouseTracking for a widget while using auto-hide for it.
     *
     * When disabling auto-hide, mouseTracking will be disabled, so if you need
     * mouseTracking after disabling auto-hide, you have to reenable
     * mouseTracking.
     *
     * If you want to use auto-hiding for widgets that don't take focus, e.g.
     * a QCanvasView, then you have to pass all key-events that should trigger
     * auto-hiding to autoHideEventFilter().
     */
    static void setAutoHideCursor( QWidget *w, bool enable );

    /**
     * Overloaded method for the case where you have an event-filter installed
     * on the widget you want to enable auto-cursor-hiding.
     *
     * In this case set @p customEventFilter to true and call
     * autoHideEventFilter() from the beginning of your eventFilter().
     *
     * @see autoHideEventFilter
     */
    static void setAutoHideCursor( QWidget *w, bool enable,
				   bool customEventFilter );

    /**
     * Sets the delay time in milliseconds for auto-hiding. When no keyboard
     * events arrive for that time-frame, the cursor will be hidden.
     *
     * Default is 5000, i.e. 5 seconds.
     */
    static void setHideCursorDelay( int ms );

    /**
     * @returns the current auto-hide delay time.
     *
     * Default is 5000, i.e. 5 seconds.
     */
    static int hideCursorDelay();

    /**
     * KCursor has to install an eventFilter over the widget you want to
     * auto-hide. If you have an own eventFilter() on that widget and stop
     * some events by returning true, you might break auto-hiding, because
     * KCursor doesn't get those events.
     *
     * In this case, you need to call setAutoHideCursor( widget, true, true );
     * to tell KCursor not to install an eventFilter. Then you call this method
     * from the beginning of your eventFilter, for example:
     * \code
     * edit = new KEdit( this, "some edit widget" );
     * edit->installEventFilter( this );
     * KCursor::setAutoHideCursor( edit, true, true );
     *
     * [...]
     *
     * bool YourClass::eventFilter( QObject *o, QEvent *e )
     * {
     *     if ( o == edit ) // only that widget where you enabled auto-hide!
     *         KCursor::autoHideEventFilter( o, e );
     *
     *     // now you can do your own event-processing
     *     [...]
     * }
     * \endcode
     *
     * Note that you must not call KCursor::autoHideEventFilter() when you
     * didn't enable or after disabling auto-hiding.
     */
    static void autoHideEventFilter( QObject *, QEvent * );

private:
    static QCursor *s_handCursor;
};


#endif // _KCURSOR_H
