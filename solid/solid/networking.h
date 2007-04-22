/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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
         * Retrieves the status of networking (as a whole) in the system.
         * This is distinct from whether the system's networking is online or offline.
         * To check that, see @ref NetworkStatus.
         *
         * @return true if this networking is enabled, false otherwise
         */
        SOLID_EXPORT bool isNetworkingEnabled();

        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        SOLID_EXPORT void setNetworkingEnabled(bool enabled);

        class Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
        };

        SOLID_EXPORT Notifier *notifier();
    }

} // Solid

#endif
