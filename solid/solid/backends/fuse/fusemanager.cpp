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

#include "fusemanager.h"
#include "fusedevice.h"
#include "fusehandling.h"
#include "../shared/rootdevice.h"
#include "fuseservice.h"
#include "fusewatcher.h"

#include <QDebug>

using namespace Solid::Backends::Fuse;
using namespace Solid::Backends::Shared;

FuseManager::FuseManager(QObject *parent)
  : Solid::Ifaces::DeviceManager(parent)
{
    m_supportedInterfaces << Solid::DeviceInterface::StorageAccess;

    m_deviceList = FuseHandling::deviceList();

    connect(FuseWatcher::instance(), SIGNAL(mtabChanged()), this, SLOT(onMtabChanged()));
}

QString FuseManager::udiPrefix() const
{
    return QString::fromLatin1(FUSE_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> FuseManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList FuseManager::allDevices()
{
    QStringList result;

    result << udiPrefix();
    foreach (const QString &device, m_deviceList) {
        result << udiPrefix() + device;
    }

    return result;
}

QStringList FuseManager::devicesFromQuery( const QString &parentUdi,
                                             Solid::DeviceInterface::Type type)
{
    qDebug() << "devicesFromQuery" << parentUdi << type;

    if (type == Solid::DeviceInterface::StorageAccess
            || type == Solid::DeviceInterface::StorageVolume
            || type == Solid::DeviceInterface::StorageDrive) {

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

QObject *FuseManager::createDevice(const QString &udi)
{
    qDebug() << "createDevice" << udi;

    if (udi == udiPrefix()) {
        qDebug() << "returning a root device";
        RootDevice *root = new RootDevice(FUSE_UDI_PREFIX);

        root->setProduct(tr("FUSE mounts"));
        root->setDescription(tr("FUSE mounts in your system"));
        root->setIcon("folder-remote");

        return root;

    } else {
        // global device manager makes sure udi starts with udi prefix + '/'
        QString internalName = udi.mid( udiPrefix().length(), -1 );
        qDebug() << "internalName" << internalName;

        if (!m_deviceList.contains(internalName))
            return 0;

        qDebug() << "Returning a FuseDevice for" << udi;
        QObject* device = new FuseDevice(udi);
        connect (this, SIGNAL(mtabChanged(QString)), device, SLOT(onMtabChanged(QString)));
        return device;

    }
}

void FuseManager::_k_updateDeviceList()
{
    QStringList deviceList = FuseHandling::deviceList();
    QSet <QString> newlist = deviceList.toSet();
    QSet <QString> oldlist = m_deviceList.toSet();

    qDebug() << "old list" << oldlist << "new list" << newlist;
    m_deviceList = deviceList;

    foreach (const QString &device, newlist) {
        if ( !oldlist.contains(device) ) {
            emit deviceAdded(udiPrefix() + device);
        }
    }

    foreach (const QString &device, oldlist) {
        if ( !newlist.contains(device) ) {
            emit deviceRemoved(udiPrefix() + device);
        }
    }
}

void FuseManager::onMtabChanged()
{
    FuseHandling::flushMtabCache();

    _k_updateDeviceList();

    foreach (const QString &device, m_deviceList) {
        // notify storageaccess objects via device ...
        emit mtabChanged(device);
    }
}

FuseManager::~FuseManager()
{
}
