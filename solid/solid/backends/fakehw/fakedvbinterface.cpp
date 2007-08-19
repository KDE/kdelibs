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
