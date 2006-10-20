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

#ifndef SOLID_NETWORKHW_H
#define SOLID_NETWORKHW_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    /**
     * This capability is available on network interfaces.
     */
    class SOLID_EXPORT NetworkHw : public Capability
    {
        Q_OBJECT
        Q_PROPERTY( QString ifaceName READ ifaceName )
        Q_PROPERTY( bool wireless READ isWireless )
        Q_PROPERTY( QString hwAddress READ hwAddress )
        Q_PROPERTY( qulonglong macAddress READ macAddress )

    public:
        /**
         * Creates a new NetworkHw object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit NetworkHw( QObject *backendObject );

        /**
         * Destroys a NetworkHw object.
         */
        virtual ~NetworkHw();


        /**
         * Get the Solid::Capability::Type of the NetworkHw capability.
         *
         * @return the NetworkHw capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::NetworkHw; }


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
