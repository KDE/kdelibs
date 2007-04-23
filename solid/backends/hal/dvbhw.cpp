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

#include "dvbhw.h"

#include "haldevice.h"

DvbHw::DvbHw(HalDevice *device)
    : DeviceInterface(device)
{

}

DvbHw::~DvbHw()
{

}

QString DvbHw::device() const
{
    return m_device->property("dvb.device").toString();
}

int DvbHw::deviceAdapter() const
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

    if (!string.startsWith("adapter"))
        return -1;
    string = string.mid(7);

    bool ok;
    int adapter = string.toInt(&ok, 10);
    if (ok)
        return adapter;
    else
        return -1;
}

Solid::DvbHw::DeviceType DvbHw::deviceType() const
{
    Solid::DvbHw::DeviceType type;
    int index;

    if (parseTypeIndex(&type, &index))
        return type;
    else
        return Solid::DvbHw::DvbUnknown;
}

int DvbHw::deviceIndex() const
{
    Solid::DvbHw::DeviceType type;
    int index;

    if (parseTypeIndex(&type, &index))
        return index;
    else
        return -1;
}

bool DvbHw::parseTypeIndex(Solid::DvbHw::DeviceType *type, int *index) const
{
    QString string = device();
    int pos = string.lastIndexOf('/');
    if (pos < 0)
        return false;
    string = string.mid(pos + 1);

    if (string.startsWith("audio")) {
        *type = Solid::DvbHw::DvbAudio;
        string = string.mid(5);
    } else if (string.startsWith("ca")) {
        *type = Solid::DvbHw::DvbCa;
        string = string.mid(2);
    } else if (string.startsWith("demux")) {
        *type = Solid::DvbHw::DvbDemux;
        string = string.mid(5);
    } else if (string.startsWith("dvr")) {
        *type = Solid::DvbHw::DvbDvr;
        string = string.mid(3);
    } else if (string.startsWith("frontend")) {
        *type = Solid::DvbHw::DvbFrontend;
        string = string.mid(8);
    } else if (string.startsWith("net")) {
        *type = Solid::DvbHw::DvbNet;
        string = string.mid(3);
    } else if (string.startsWith("osd")) {
        *type = Solid::DvbHw::DvbOsd;
        string = string.mid(3);
    } else if (string.startsWith("sec")) {
        *type = Solid::DvbHw::DvbSec;
        string = string.mid(3);
    } else if (string.startsWith("video")) {
        *type = Solid::DvbHw::DvbVideo;
        string = string.mid(5);
    } else
        return false;

    bool ok;
    *index = string.toInt(&ok, 10);

    return ok;
}

#include "dvbhw.moc"
