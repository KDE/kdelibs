/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>
    Copyright 2007 Christoph Pfister <christophpfister@gmail.com>
    Copyright 2010 Pino Toscano <pino@kde.org>

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

#include "udevdvbinterface.h"

#include "udevdevice.h"

using namespace Solid::Backends::UDev;

DvbInterface::DvbInterface(UDevDevice *device)
    : DeviceInterface(device)
{

}

DvbInterface::~DvbInterface()
{

}

QString DvbInterface::device() const
{
    return m_device->property("DEVNAME").toString();
}

int DvbInterface::deviceAdapter() const
{
    bool ok;
    int adapter = m_device->property("DVB_ADAPTER_NUM").toString().toInt(&ok, 10);
    if (ok)
        return adapter;
    else
        return -1;
}

Solid::DvbInterface::DeviceType DvbInterface::deviceType() const
{
    Solid::DvbInterface::DeviceType type = Solid::DvbInterface::DvbUnknown;
    const QString typeString = m_device->property("DVB_DEVICE_TYPE").toString();

    if (typeString == QLatin1String("audio")) {
        type = Solid::DvbInterface::DvbAudio;
    } else if (typeString == QLatin1String("ca")) {
        type = Solid::DvbInterface::DvbCa;
    } else if (typeString == QLatin1String("demux")) {
        type = Solid::DvbInterface::DvbDemux;
    } else if (typeString == QLatin1String("dvr")) {
        type = Solid::DvbInterface::DvbDvr;
    } else if (typeString == QLatin1String("frontend")) {
        type = Solid::DvbInterface::DvbFrontend;
    } else if (typeString == QLatin1String("net")) {
        type = Solid::DvbInterface::DvbNet;
    } else if (typeString == QLatin1String("osd")) {
        type = Solid::DvbInterface::DvbOsd;
    } else if (typeString == QLatin1String("sec")) {
        type = Solid::DvbInterface::DvbSec;
    } else if (typeString == QLatin1String("video")) {
        type = Solid::DvbInterface::DvbVideo;
    }

    return type;
}

int DvbInterface::deviceIndex() const
{
    bool ok;
    int index = m_device->property("DVB_DEVICE_NUM").toString().toInt(&ok, 10);
    if (ok)
        return index;
    else
        return -1;
}

#include "backends/udev/udevdvbinterface.moc"
