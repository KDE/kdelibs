/*
    This file is part of the KDE project

    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

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

#include "rootdevice.h"
#include <QtCore/QStringList>

using namespace Solid::Backends::Fstab;

RootDevice::RootDevice(const QString &udi, const QString &product, const QString &description) :
    Solid::Ifaces::Device(),
    m_udi(udi),
    m_product(product),
    m_description(description)
{
}

RootDevice::~RootDevice()
{
}

QString RootDevice::udi() const
{
    return m_udi;
}

QString RootDevice::parentUdi() const
{
    return QString();
}

QString RootDevice::vendor() const
{
    return QString();
}

QString RootDevice::product() const
{
    return m_product;
}

QString RootDevice::icon() const
{
    return QString();
}

QStringList RootDevice::emblems() const
{
    QStringList res;
    return QStringList();
}

QString RootDevice::description() const
{
    return m_description;
}

bool RootDevice::queryDeviceInterface(const Solid::DeviceInterface::Type&) const
{
    return false;
}

QObject* RootDevice::createDeviceInterface(const Solid::DeviceInterface::Type&)
{
    return 0;
}
