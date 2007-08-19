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

#include "halgenericinterface.h"

#include "haldevice.h"

using namespace Solid::Backends::Hal;

GenericInterface::GenericInterface(HalDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
            this, SIGNAL(propertyChanged(const QMap<QString,int> &)));
    connect(device, SIGNAL(conditionRaised(const QString &, const QString &)),
            this, SIGNAL(conditionRaised(const QString &, const QString &)));
}

GenericInterface::~GenericInterface()
{

}

QVariant GenericInterface::property(const QString &key) const
{
    return m_device->property(key);
}

QMap<QString, QVariant> GenericInterface::allProperties() const
{
    return m_device->allProperties();
}

bool GenericInterface::propertyExists(const QString &key) const
{
    return m_device->propertyExists(key);
}

#include "backends/hal/halgenericinterface.moc"
