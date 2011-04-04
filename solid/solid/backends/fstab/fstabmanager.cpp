/*
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

#include "fstabmanager.h"
#include "fstabdevice.h"
#include "fstabhandling.h"
#include "../shared/rootdevice.h"
#include "fstabservice.h"
#include "fstabwatcher.h"

using namespace Solid::Backends::Fstab;
using namespace Solid::Backends::Shared;

FstabManager::FstabManager(QObject *parent)
  : Solid::Ifaces::DeviceManager(parent)
{
    m_supportedInterfaces << Solid::DeviceInterface::StorageAccess;
    m_supportedInterfaces << Solid::DeviceInterface::NetworkShare;

    m_deviceList = FstabHandling::deviceList();

    connect(FstabWatcher::instance(), SIGNAL(fstabChanged()), this, SLOT(onFstabChanged()));
}

QString FstabManager::udiPrefix() const
{
    return QString::fromLatin1(FSTAB_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> FstabManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList FstabManager::allDevices()
{
    QStringList result;

    result << udiPrefix();
    foreach (const QString &device, m_deviceList) {
        result << udiPrefix() + "/" + device;
    }

    return result;
}

QStringList FstabManager::devicesFromQuery( const QString &parentUdi,
                                             Solid::DeviceInterface::Type type)
{
    if (type == Solid::DeviceInterface::StorageAccess
     || type == Solid::DeviceInterface::NetworkShare) {
        if (parentUdi.isEmpty() || parentUdi == udiPrefix()) {
            QStringList list = allDevices();
            list.removeFirst();
            return list;
        } else {
            QStringList list;
            list << parentUdi;
            return list;
        }
    }
    return QStringList();
}

QObject *FstabManager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        RootDevice *root = new RootDevice(FSTAB_UDI_PREFIX);

        root->setProduct(tr("Network Shares"));
        root->setDescription(tr("NFS and SMB shares declared in your system"));
        root->setIcon("folder-remote");

        return root;
    } else {
        return new FstabDevice(udi);
    }
}

void FstabManager::onFstabChanged()
{
    QStringList deviceList = FstabHandling::deviceList();
    if (deviceList.count() > m_deviceList.count()) {
        //new device
        foreach (const QString &device, deviceList) {
            if (!m_deviceList.contains(device)) {
                emit deviceAdded(udiPrefix() + "/" + device);
            }
        }
    } else {
        //device has been removed
        foreach (const QString &device, m_deviceList) {
            if (!deviceList.contains(device)) {
                emit deviceRemoved(udiPrefix() + "/" + device);
            }
        }
    }
    m_deviceList = deviceList;
}

FstabManager::~FstabManager()
{
}
