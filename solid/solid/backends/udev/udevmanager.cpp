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

using namespace Solid::Backends::UDev;
using namespace Solid::Backends::Shared;

class UDevManager::Private
{
public:
    Private();
    ~Private();

    QStringList deviceListFromEnumerate(udev_enumerate_ *en) const;

    udev_ *udev;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};

UDevManager::Private::Private()
    : udev(udev_new())
{
}

UDevManager::Private::~Private()
{
    udev_unref(udev);
}

QStringList UDevManager::Private::deviceListFromEnumerate(udev_enumerate_ *en) const
{
    QStringList ret;
    udev_list_entry_ *list, *entry;

    udev_enumerate_scan_devices(en);
    list = udev_enumerate_get_list_entry(en);
    udev_list_entry_foreach (entry, list) {
        udev_device_ *const device = udev_device_new_from_syspath(udev,
                                                                  udev_list_entry_get_name(entry));

        const char *const subsystem = udev_device_get_subsystem(device);

        if (!device) {
            continue;
        }
        ret << QString(UDEV_UDI_PREFIX) + udev_device_get_syspath(device);

        udev_device_unref(device);
    }

    udev_enumerate_unref(en);

    return ret;
}

UDevManager::UDevManager(QObject *parent)
    : d(new Private)
{
    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::Button;
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
    return d->supportedInterfaces;
}

QStringList UDevManager::allDevices()
{
    udev_enumerate_ *const devices = udev_enumerate_new(d->udev);
    return d->deviceListFromEnumerate(devices);
}

QStringList UDevManager::devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type)
{
    // TODO: actually use the query parameters
    return allDevices();
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
    const QString udi = udi_.right(udi_.size() - QString(UDEV_UDI_PREFIX).size());
    return new UDevDevice(udev_device_new_from_syspath(d->udev, udi.toLatin1().data()));
}
