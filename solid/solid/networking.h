/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_NETWORKING
#define SOLID_NETWORKING

#include <QtCore/QObject>

#include <solid/solid_export.h>

namespace Solid
{
    /**
     * This class allow to query the underlying system to discover the available
     * network interfaces and reachable network.It has also the
     * responsibility to notify when a network interface or a network appear or disappear.
     *
     * It's the unique entry point for network management. Applications should use
     * it to find network interfaces, or to be notified about network related changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace Networking
    {
        /**
         * Describes the state of the networking system
         * - Unknown : the networking system is not active or unable to report its status - proceed
         *   with caution
         * - Disconnected : the system is not connected to any network
         * - Connecting : the system is in the process of making a connection
         * - Connected : the system is currently connected to a network
         * - Disconnecting : the system is breaking the connection
         */
        enum Status { Unknown, Unconnected, Connecting, Connected, Disconnecting };

        /**
         * Get the current networking status
         */
        SOLID_EXPORT Status status();

        /**
         * This object emits signals, for use if your application requires notification
         * of changes to networking.
         */
        class Notifier : public QObject
        {
        Q_OBJECT
        Q_SIGNALS:
            void statusChanged( Status );
        };

        SOLID_EXPORT Notifier *notifier();
    }

} // Solid

#endif
