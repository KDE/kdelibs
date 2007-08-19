/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "fakenetworkinterface.h"

using namespace Solid::Backends::Fake;

FakeNetworkInterface::FakeNetworkInterface(FakeDevice *device)
 : FakeDeviceInterface(device)
{

}

FakeNetworkInterface::~FakeNetworkInterface()
{
}

QString FakeNetworkInterface::ifaceName() const
{
    return fakeDevice()->property("ifaceName").toString();
}

bool FakeNetworkInterface::isWireless() const
{
    return fakeDevice()->property("wireless").toBool();
}

QString FakeNetworkInterface::hwAddress() const
{
    return fakeDevice()->property("hwAddress").toString();
}

qulonglong FakeNetworkInterface::macAddress() const
{
    return fakeDevice()->property("macAddress").toULongLong();
}

#include "backends/fakehw/fakenetworkinterface.moc"
