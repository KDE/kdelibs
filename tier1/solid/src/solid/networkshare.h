/*
    Copyright 2011 Mario Bensi <mbensi@ipsquad.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_NETWORKSHARE_H
#define SOLID_NETWORKSHARE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

namespace Solid
{
    class Device;
    class NetworkSharePrivate;

    /**
     * NetworkShare interface.
     *
     * a NetworkShare interface is used to determine the type of
     * network access.
     * @since 4.7
     */
    class SOLID_EXPORT NetworkShare : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(ShareType)
        Q_PROPERTY(ShareType type READ type)
        Q_PROPERTY(QUrl url READ url)
        Q_DECLARE_PRIVATE(NetworkShare)
        friend class Device;

    private:
        /**
         * Creates a new NetworkShare object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit NetworkShare(QObject *backendObject);

    public:
        /**
         * Destroys a NetworkShare object.
         */
        virtual ~NetworkShare();

        /**
         * This enum type defines the type of networkShare device can be.
         *
         * - Unknown : a unsupported network protocol
         * - Nfs : nfs protocol
         * - Cifs : samba protocol
         * - Upnp : Upnp protocol
         */

        enum ShareType { Unknown, Nfs, Cifs, Upnp };

        /**
         * Get the Solid::DeviceInterface::Type of the NetworkShare device interface.
         *
         * @return the NetworkShare device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::NetworkShare; }

        /**
         * Retrieves the type of network share
         *
         * @return the type of network share
         */
        ShareType type() const;

        /**
         * Retrieves the url of network share
         *
         * @return the url of network share
         */
        QUrl url() const;

    };
}

#endif
