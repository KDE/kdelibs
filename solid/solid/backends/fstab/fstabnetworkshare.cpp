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

#include "fstabnetworkshare.h"
#include <solid/backends/fstab/fstabdevice.h>

using namespace Solid::Backends::Fstab;

FstabNetworkShare::FstabNetworkShare(Solid::Backends::Fstab::FstabDevice *device) :
    QObject(device),
    m_fstabDevice(device)
{
    QString url;
    if (m_fstabDevice->device().startsWith("//")) {
        m_type = Solid::NetworkShare::Cifs;
        url = "smb:";
        url += m_fstabDevice->device();
    } else if (m_fstabDevice->device().contains(":/")) {
        m_type = Solid::NetworkShare::Nfs;
        url = "nfs://";
        url += m_fstabDevice->product();
        url += m_fstabDevice->vendor();
    } else {
        m_type = Solid::NetworkShare::Unknown;
    }
    m_url = QUrl(url);
}

FstabNetworkShare::~FstabNetworkShare()
{
}

Solid::NetworkShare::ShareType FstabNetworkShare::type() const
{
    return m_type;
}

QUrl FstabNetworkShare::url() const
{
    return m_url;
}


const Solid::Backends::Fstab::FstabDevice *FstabNetworkShare::fstabDevice() const
{
    return m_fstabDevice;
}
