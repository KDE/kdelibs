/*
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>
    Copyright 2012 Ivan Cukic <ivan.cukic@kde.org>

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

#include "fusedevice.h"
#include "fusehandling.h"
#include "fusestorageaccess.h"
#include "fusestoragedrive.h"
#include "fusestoragevolume.h"
#include "fuseservice.h"
#include <QtCore/QStringList>
#include <QDebug>

using namespace Solid::Backends::Fuse;

FuseDevice::FuseDevice(QString udi) :
    Solid::Ifaces::Device(),
    m_udi(udi)
{
    qDebug() << "UDI:" << udi;
    m_device = m_udi.mid(QString::fromLatin1(FUSE_UDI_PREFIX).length(), -1);

    if (m_device.endsWith(":media")) {
        m_device.chop(6);
    }

    m_product = m_device;
    m_vendor  = "FUSE";

    m_description = m_vendor + " on " + m_product;
    qDebug() << "Info:" << m_device << m_product << m_vendor << m_description;
}

FuseDevice::~FuseDevice()
{
}

QString FuseDevice::udi() const
{
    return m_udi;
}

QString FuseDevice::parentUdi() const
{
    return udi();
    // return QString::fromLatin1(FUSE_UDI_PREFIX);
}

QString FuseDevice::vendor() const
{
    return m_vendor;
}

QString FuseDevice::product() const
{
    return m_product;
}

QString FuseDevice::icon() const
{
    return QString::fromLatin1("drive-removable-media");
}

QStringList FuseDevice::emblems() const
{
    QStringList res;
    const FuseStorageAccess accessIface(const_cast<FuseDevice *>(this));
    if (accessIface.isAccessible()) {
        res << "emblem-mounted";
    } else {
        res << "emblem-unmounted";
    }

    return res;
}

QString FuseDevice::description() const
{
    return m_description;
}

bool FuseDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        case Solid::DeviceInterface::StorageAccess:
        case Solid::DeviceInterface::StorageVolume:
        case Solid::DeviceInterface::StorageDrive:
            return true;

        default:
            return false;
    }
}

QObject* FuseDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    switch (type) {
        case Solid::DeviceInterface::StorageAccess:
            return new FuseStorageAccess(this);

        case Solid::DeviceInterface::StorageVolume:
            return new FuseStorageVolume(this);

        case Solid::DeviceInterface::StorageDrive:
            return new FuseStorageDrive(this);

        default:
            return 0;
    }
}

QString FuseDevice::device() const
{
    return m_device;
}

void FuseDevice::onMtabChanged(const QString& device)
{
    if (m_device == device)
        emit mtabChanged(device);
}
