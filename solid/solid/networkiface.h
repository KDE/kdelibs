/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_NETWORKIFACE_H
#define SOLID_NETWORKIFACE_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    /**
     * This capability is available on network interfaces.
     */
    class KDE_EXPORT NetworkIface : public Capability
    {
        Q_OBJECT
    public:
        /**
         * Creates a new NetworkIface object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        NetworkIface( QObject *backendObject );

        /**
         * Destroys a NetworkIface object.
         */
        virtual ~NetworkIface();


        /**
         * Get the Solid::Capability::Type of the NetworkIface capability.
         *
         * @return the NetworkIface capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::NetworkIface; }


        /**
         * Retrieves the name of the interface in the system.
         * This name is system dependent, it allows to identify the interface
         * in the system. For example it can be of the form "eth0" under Linux.
         *
         * @return the interface name
         */
        QString ifaceName() const;

        /**
         * Indicates if this interface is wireless.
         *
         * @return true if the interface is wireless, false otherwise
         */
        bool isWireless() const;


        /**
         * Retrieves the hardware address of the interface.
         *
         * @return the hardware address as a string
         */
        QString hwAddress() const;

        /**
         * Retrieves the MAC address of the interface.
         *
         * @return the MAC address
         */
        qulonglong macAddress() const;
    };
}

#endif
