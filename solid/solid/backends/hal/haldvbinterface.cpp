/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>
    Copyright 2007 Christoph Pfister <christophpfister@gmail.com>

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

#include "haldvbinterface.h"

#include "haldevice.h"

using namespace Solid::Backends::Hal;

DvbInterface::DvbInterface(HalDevice *device)
    : DeviceInterface(device)
{

}

DvbInterface::~DvbInterface()
{

}

QString DvbInterface::device() const
{
    return m_device->prop("dvb.device").toString();
}

int DvbInterface::deviceAdapter() const
{
    QString string = device();
    int pos = string.lastIndexOf('/');
    if (pos < 0)
        return -1;
    string = string.left(pos);

    pos = string.lastIndexOf('/');
    if (pos < 0)
        return -1;
    string = string.mid(pos + 1);

    if (!string.startsWith(QLatin1String("adapter")))
        return -1;
    string = string.mid(7);

    bool ok;
    int adapter = string.toInt(&ok, 10);
    if (ok)
        return adapter;
    else
        return -1;
}

Solid::DvbInterface::DeviceType DvbInterface::deviceType() const
{
    Solid::DvbInterface::DeviceType type;
    int index;

    if (parseTypeIndex(&type, &index))
        return type;
    else
        return Solid::DvbInterface::DvbUnknown;
}

int DvbInterface::deviceIndex() const
{
    Solid::DvbInterface::DeviceType type;
    int index;

    if (parseTypeIndex(&type, &index))
        return index;
    else
        return -1;
}

bool DvbInterface::parseTypeIndex(Solid::DvbInterface::DeviceType *type, int *index) const
{
    QString string = device();
    int pos = string.lastIndexOf('/');
    if (pos < 0)
        return false;
    string = string.mid(pos + 1);

    if (string.startsWith(QLatin1String("audio"))) {
        *type = Solid::DvbInterface::DvbAudio;
        string = string.mid(5);
    } else if (string.startsWith(QLatin1String("ca"))) {
        *type = Solid::DvbInterface::DvbCa;
        string = string.mid(2);
    } else if (string.startsWith(QLatin1String("demux"))) {
        *type = Solid::DvbInterface::DvbDemux;
        string = string.mid(5);
    } else if (string.startsWith(QLatin1String("dvr"))) {
        *type = Solid::DvbInterface::DvbDvr;
        string = string.mid(3);
    } else if (string.startsWith(QLatin1String("frontend"))) {
        *type = Solid::DvbInterface::DvbFrontend;
        string = string.mid(8);
    } else if (string.startsWith(QLatin1String("net"))) {
        *type = Solid::DvbInterface::DvbNet;
        string = string.mid(3);
    } else if (string.startsWith(QLatin1String("osd"))) {
        *type = Solid::DvbInterface::DvbOsd;
        string = string.mid(3);
    } else if (string.startsWith(QLatin1String("sec"))) {
        *type = Solid::DvbInterface::DvbSec;
        string = string.mid(3);
    } else if (string.startsWith(QLatin1String("video"))) {
        *type = Solid::DvbInterface::DvbVideo;
        string = string.mid(5);
    } else
        return false;

    bool ok;
    *index = string.toInt(&ok, 10);

    return ok;
}

#include "backends/hal/haldvbinterface.moc"
