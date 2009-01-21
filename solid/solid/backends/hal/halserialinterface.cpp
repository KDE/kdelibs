/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#include "halserialinterface.h"

#include "haldevice.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Hal;

SerialInterface::SerialInterface(HalDevice *device)
    : DeviceInterface(device)
{

}

SerialInterface::~SerialInterface()
{

}

QVariant SerialInterface::driverHandle() const
{
    return m_device->property("serial.device");
}

Solid::SerialInterface::SerialType SerialInterface::serialType() const
{
    QString type = m_device->property("serial.type").toString();
    if (type == QLatin1String("platform"))
        return Solid::SerialInterface::Platform;
    if (type == QLatin1String("usb"))
        return Solid::SerialInterface::Usb;
    return Solid::SerialInterface::Unknown;
}

int SerialInterface::port() const
{
    return m_device->property("serial.port").toInt();
}

#include "backends/hal/halserialinterface.moc"
