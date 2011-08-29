/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

#include "fakedvbinterface.h"

using namespace Solid::Backends::Fake;

FakeDvbInterface::FakeDvbInterface(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeDvbInterface::~FakeDvbInterface()
{

}

QString FakeDvbInterface::device() const
{
    return fakeDevice()->property("device").toString();
}

int FakeDvbInterface::deviceAdapter() const
{
    return fakeDevice()->property("deviceAdapter").toInt();
}

Solid::DvbInterface::DeviceType FakeDvbInterface::deviceType() const
{
    QString string = fakeDevice()->property("deviceType").toString();

    if (string == "audio")
        return Solid::DvbInterface::DvbAudio;
    if (string == "ca")
        return Solid::DvbInterface::DvbCa;
    if (string == "demux")
        return Solid::DvbInterface::DvbDemux;
    if (string == "dvr")
        return Solid::DvbInterface::DvbDvr;
    if (string == "frontend")
        return Solid::DvbInterface::DvbFrontend;
    if (string == "net")
        return Solid::DvbInterface::DvbNet;
    if (string == "osd")
        return Solid::DvbInterface::DvbOsd;
    if (string == "sec")
        return Solid::DvbInterface::DvbSec;
    if (string == "video")
        return Solid::DvbInterface::DvbVideo;

    return Solid::DvbInterface::DvbUnknown;
}

int FakeDvbInterface::deviceIndex() const
{
    return fakeDevice()->property("deviceIndex").toInt();
}

#include "backends/fakehw/fakedvbinterface.moc"
