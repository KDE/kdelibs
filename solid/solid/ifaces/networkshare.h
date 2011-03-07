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

#ifndef SOLID_IFACES_NETWORKSHARE_H
#define SOLID_IFACES_NETWORKSHARE_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/networkshare.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * NetworkShare interface.
     *
     * a NetworkShare interface is used to determine the type of
     * network access.
     */
    class NetworkShare : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a NetworkShare object.
         */
        virtual ~NetworkShare();


        /**
         * Retrieves the type of network
         *
         * @return the type of network
         */
        virtual Solid::NetworkShare::ShareType type() const = 0;

        /**
         * Retrieves the url of network share
         *
         * @return the url of network share
         */
        virtual QUrl url() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::NetworkShare, "org.kde.Solid.Ifaces.NetworkShare/0.1")

#endif
