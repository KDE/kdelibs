/**
 * Copyright 2009, 2010  Michael Leupold <lemma@confuego.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef KSYSTEMEVENTFILTER_H
#define KSYSTEMEVENTFILTER_H

#include <kwindowsystem_export.h>

class QWidget;

/**
 * Members of this namespace allow installing and removing global event-filters
 * that will catch all window system events the application receives.
 *
 * Qt5 porting note: you must port your x11Event()/winEvent()/macEvent() reimplementations to nativeEvent!
 *
 * @author Michael Leupold <lemma@confuego.org>
 * @deprecated use QCoreApplication::instance()->installNativeEventFilter directly.
 */
namespace KSystemEventFilter
{
    /**
     * Installs a widget filter as a global native event filter.
     *
     * The widget filter receives all events in its standard nativeEvent() event handler function.
     * When the filter widget is destroyed, it is automatically removed from the list of known filters.
     * Each filter widget can be added (and will be called) only once.
     * The function doesn't transfer the widget filter's ownership, so the code installing
     * the event filter is responsible for freeing it if it's no longer needed.
     *
     * @param filter the filter widget to install
     *
     * @remarks The order in which installed event filters are called is arbitrary.
     *          Processing the events will stop as soon as a filter "consumes" an
     *          event (ie. the nativeEvent() method returns true).
     *
     * @warning Only do this when absolutely necessary. An installed event filter
     *          can slow things down.
     */
    void KWINDOWSYSTEM_DEPRECATED installEventFilter(QWidget *filter);

    /**
     * Removes a global widget filter.
     */
    void removeEventFilter(const QWidget *filter);
}

#endif
