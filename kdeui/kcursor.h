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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
/*
 * $Id$
 *
 */
#ifndef _KCURSOR_H
#define _KCURSOR_H

#include <kapp.h>

class QCursor;
class QWidget;

/**
 * A wrapper around @ref QCursor that allows for "themed" cursors.
 *
 * Currently, the only themed cursor is a hand shaped cursor.
 *
 * A typical usage would be
 * <PRE>
 * 	setCursor(KCursor::handCursor());
 * </PRE>
 *
 * @short A QCursor wrapper allowing "themed" cursors and auto-hiding cursors.
 * @author Kurt Granroth <granroth@kde.org>
 */
class KCursor
{
public:
    /**
     * Constructor.
     *
     * Does not do anything so far.
     **/
    KCursor();

    /**
     * Retrieve the proper hand cursor according to
     * the current GUI style (static function).
     */
    static QCursor handCursor();

    /**
     * Retrieve the proper arrow cursor according to
     * the current GUI style (static function).
     */
    static QCursor arrowCursor();

    /**
     * Retrieve the proper up arrow cursor according to
     * the current GUI style (static function).
     */
    static QCursor upArrowCursor();

    /**
     * Retrieve the proper cross-hair cursor according to
     * the current GUI style (static function).
     */
    static QCursor crossCursor();

    /**
     * Retrieve the proper hourglass cursor according to
     * the current GUI style (static function).
     */
    static QCursor waitCursor();

    /**
     * Retrieve the proper text cursor according to
     * the current GUI style (static function).
     */
    static QCursor ibeamCursor();

    /**
     * Retrieve the proper vertical resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeVerCursor();

    /**
     * Retrieve the proper horizontal resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeHorCursor();

    /**
     * Retrieve the proper diagonal resize (/) cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeBDiagCursor();

    /**
     * Retrieve the proper diagonal resize (\) cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeFDiagCursor();

    /**
     * Retrieve the proper all-directions resize cursor
     * according to the current GUI style (static function).
     */
    static QCursor sizeAllCursor();

    /**
     * Retrieve a blank or invisible cursor (static function).
     */
    static QCursor blankCursor();

    /**
     * Sets auto-hiding the cursor for widget @p w. Enabling it will result in
     * the cursor being hidden when
     * @li a key-event happens
     * @li there are no key-events for a configured time-frame (see
     *     @ref setHideCursorDelay())
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
     */
    static void setAutoHideCursor( QWidget *w, bool enable );

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

};


#endif // _KCURSOR_H
