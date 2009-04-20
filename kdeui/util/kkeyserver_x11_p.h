#ifndef KKEYSERVER_X11_P_H
#define KKEYSERVER_X11_P_H
/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kapplication.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace KKeyServer {

bool initializeMods();

/**
 * A little helper widget to install an eventFilter on kapp
 */
class EventFilter : public QObject
    {
    Q_OBJECT

public:

    EventFilter()
        : QObject()
        {
        // We need to react to KeyboardLayoutChange events
        kapp->installEventFilter(this);
        }

    // Needed for the vtable
    virtual ~EventFilter();

    virtual bool eventFilter(QObject *object, QEvent *event);

    };


} // namespace KKeyServer

#endif /* KKEYSERVER_X11_P_H */
