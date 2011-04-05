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

#ifndef SOLID_BACKENDS_FAKE_NETWORKSHARE_H
#define SOLID_BACKENDS_FAKE_NETWORKSHARE_H

#include <solid/ifaces/networkshare.h>
#include "fakedeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
    class FakeNetworkShare : public FakeDeviceInterface, public Solid::Ifaces::NetworkShare
    {
        Q_OBJECT
        Q_INTERFACES(Solid::Ifaces::NetworkShare)

        public:
            explicit FakeNetworkShare(FakeDevice *device);

            virtual ~FakeNetworkShare();

            virtual Solid::NetworkShare::ShareType type() const;

            virtual QUrl url() const;
    };

}
}
}

#endif // SOLID_BACKENDS_FAKE_NETWORKSHARE_H
