/*
    This file is part of the KDE project

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "kupnprootdevice.h"

// Qt
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#define QI18N(x) x

namespace Solid
{
namespace Backends
{
namespace KUPnP
{

KUPnPRootDevice::KUPnPRootDevice()
  : Solid::Ifaces::Device()
{
}

KUPnPRootDevice::~KUPnPRootDevice()
{
}

QString KUPnPRootDevice::udi() const
{
    return QString::fromLatin1("/org/kde/KUPnP");
}

QString KUPnPRootDevice::parentUdi() const { return QString(); }
QString KUPnPRootDevice::vendor() const { return QString(); }

QString KUPnPRootDevice::product() const
{
    return QObject::tr("UPnP devices");
}

QString KUPnPRootDevice::icon() const
{
    return QString::fromLatin1("network-server");
}

QStringList KUPnPRootDevice::emblems() const
{
    return QStringList();
}

QString KUPnPRootDevice::description() const
{
    return QString();
}


bool KUPnPRootDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    Q_UNUSED( type );
    return false;
}

QObject* KUPnPRootDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    Q_UNUSED( type );
    return 0;
}

}
}
}
