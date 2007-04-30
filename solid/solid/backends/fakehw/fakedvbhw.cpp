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

#include "backends/fakehw/fakedvbhw.h"

FakeDvbHw::FakeDvbHw(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeDvbHw::~FakeDvbHw()
{

}

QString FakeDvbHw::device() const
{
    return fakeDevice()->property("device").toString();
}

int FakeDvbHw::deviceAdapter() const
{
    return fakeDevice()->property("deviceAdapter").toInt();
}

Solid::DvbHw::DeviceType FakeDvbHw::deviceType() const
{
    QString string = fakeDevice()->property("deviceType").toString();

    if (string == "audio")
        return Solid::DvbHw::DvbAudio;
    if (string == "ca")
        return Solid::DvbHw::DvbCa;
    if (string == "demux")
        return Solid::DvbHw::DvbDemux;
    if (string == "dvr")
        return Solid::DvbHw::DvbDvr;
    if (string == "frontend")
        return Solid::DvbHw::DvbFrontend;
    if (string == "net")
        return Solid::DvbHw::DvbNet;
    if (string == "osd")
        return Solid::DvbHw::DvbOsd;
    if (string == "sec")
        return Solid::DvbHw::DvbSec;
    if (string == "video")
        return Solid::DvbHw::DvbVideo;

    return Solid::DvbHw::DvbUnknown;
}

int FakeDvbHw::deviceIndex() const
{
    return fakeDevice()->property("deviceIndex").toInt();
}

#include "backends/fakehw/fakedvbhw.moc"
