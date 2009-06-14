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

#include "iokitserialinterface.h"
#include "iokitdevice.h"

#include <QtCore/qdebug.h>

using namespace Solid::Backends::IOKit;

SerialInterface::SerialInterface(IOKitDevice *device)
    : DeviceInterface(device)
{
}

SerialInterface::~SerialInterface()
{
}

QVariant SerialInterface::driverHandle() const
{
    return m_device->property(QLatin1String("IODialinDevice"));
}

Solid::SerialInterface::SerialType SerialInterface::serialType() const
{
    return Solid::SerialInterface::Unknown;
}

int SerialInterface::port() const
{
    return -1; // TODO
}

#include "backends/iokit/iokitserialinterface.moc"



