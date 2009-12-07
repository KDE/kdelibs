/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include "wmidvbinterface.h"

#include "wmidevice.h"

using namespace Solid::Backends::Wmi;

DvbInterface::DvbInterface(WmiDevice *device)
    : DeviceInterface(device)
{

}

DvbInterface::~DvbInterface()
{

}

QString DvbInterface::device() const
{
    return m_device->property("dvb.device").toString();
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

#include "backends/wmi/wmidvbinterface.moc"
