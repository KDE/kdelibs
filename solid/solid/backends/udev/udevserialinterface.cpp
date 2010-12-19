/*
    Copyright 2009 Harald Fernengel <harry@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which sudevl
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "udevserialinterface.h"

#include "udevdevice.h"

#include <stdio.h>

#include <QString>

using namespace Solid::Backends::UDev;

SerialInterface::SerialInterface(UDevDevice *device)
    : DeviceInterface(device)
{
    m_portnum = -1;
    m_type = Solid::SerialInterface::Unknown;

    QString path = m_device->deviceName();

    int lastSlash = path.length() - path.lastIndexOf(QLatin1String("/")) -1;
    QByteArray lastElement = path.right(lastSlash).toAscii();

    const char *lastElementAscii = lastElement.constData();

    if (sscanf (lastElementAscii, "ttyS%d", &m_portnum) == 1) {
        m_type = Solid::SerialInterface::Platform;
    } else if (sscanf (lastElementAscii, "ttyUSB%d", &m_portnum) == 1) {
        m_type = Solid::SerialInterface::Usb;
    }
}

SerialInterface::~SerialInterface()
{

}

QVariant SerialInterface::driverHandle() const
{
    return m_device->property("DEVNAME");
}

Solid::SerialInterface::SerialType SerialInterface::serialType() const
{
    return m_type;
}

int SerialInterface::port() const
{
    return m_portnum;
}

#include "backends/udev/udevserialinterface.moc"
