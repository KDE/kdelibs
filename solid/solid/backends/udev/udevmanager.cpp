/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#include "udevmanager.h"

#include "udev.h"
#include "udevdevice.h"
#include "../shared/rootdevice.h"

#include <QtCore/QSet>
#include <QtCore/QDebug>

using namespace Solid::Backends::UDev;
using namespace Solid::Backends::Shared;

class UDevManager::Private
{
public:
    Private();
    ~Private();

    bool isOfInterest(const UdevQt::Device &device);

    UdevQt::Client *const m_client;
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
};

UDevManager::Private::Private()
    : m_client(new UdevQt::Client)
{
}

UDevManager::Private::~Private()
{
    delete m_client;
}

bool UDevManager::Private::isOfInterest(const UdevQt::Device &device)
{
    return device.driver() == "processor" ||
           device.driver() == "video";
}

UDevManager::UDevManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      d(new Private)
{
    connect(d->m_client, SIGNAL(deviceAdded(UdevQt::Device)), this, SLOT(slotDeviceAdded(UdevQt::Device)));
    connect(d->m_client, SIGNAL(deviceRemoved(UdevQt::Device)), this, SLOT(slotDeviceRemoved(UdevQt::Device)));

    d->m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                             << Solid::DeviceInterface::Processor
                             << Solid::DeviceInterface::AudioInterface
                             << Solid::DeviceInterface::Camera
                             << Solid::DeviceInterface::PortableMediaPlayer
                             << Solid::DeviceInterface::Video;
}

UDevManager::~UDevManager()
{
    delete d;
}

QString UDevManager::udiPrefix() const
{
    return QString::fromLatin1(UDEV_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> UDevManager::supportedInterfaces() const
{
    return d->m_supportedInterfaces;
}

QStringList UDevManager::allDevices()
{
    QStringList res;
    const UdevQt::DeviceList deviceList = d->m_client->allDevices();
    foreach (const UdevQt::Device &device, deviceList) {
        if (d->isOfInterest(device)) {
            res << udiPrefix() + device.sysfsPath();
        }
    }
    return res;
}

QStringList UDevManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    Q_UNUSED(parentUdi)
    Q_UNUSED(type)
    // TODO: actually use the query parameters
    return QStringList();
}

QObject *UDevManager::createDevice(const QString &udi_)
{
    if (udi_ == udiPrefix()) {
        RootDevice *const device = new RootDevice(UDEV_UDI_PREFIX);
        device->setProduct(tr("Devices"));
        device->setDescription(tr("Devices declared in your system"));
        device->setIcon("folder-remote"); // TODO: CHANGE ICON
        return device;
    }
    const QString udi = udi_.right(udi_.size() - udiPrefix().size());
    UdevQt::Device device = d->m_client->deviceBySysfsPath(udi);
    if (d->isOfInterest(device)) {
        return new UDevDevice(device);
    }
    return 0;
}

void UDevManager::slotDeviceAdded(const UdevQt::Device &device)
{
    emit deviceAdded(udiPrefix() + device.sysfsPath());
}

void UDevManager::slotDeviceRemoved(const UdevQt::Device &device)
{
    emit deviceRemoved(udiPrefix() + device.sysfsPath());
}
