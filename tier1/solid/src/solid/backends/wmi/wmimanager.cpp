/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    Copyright 2005,2006 Kevin Ottens <ervin@kde.org>

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

#include "wmimanager.h"

#include <QtCore/QDebug>

#include "wmidevice.h"
#include "wmideviceinterface.h"
#include "wmiquery.h"

using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiManagerPrivate
{
public:
    WmiManagerPrivate()
    {
    }

    ~WmiManagerPrivate()
    {
    }

    WmiQuery::ItemList sendQuery( const QString &wql )
    {
		return WmiQuery::instance().sendQuery( wql );
    }

    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};


WmiManager::WmiManager(QObject *parent)
    : DeviceManager(parent),  d(new WmiManagerPrivate())
{
    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Block
                           << Solid::DeviceInterface::StorageAccess
                           << Solid::DeviceInterface::StorageDrive
                           << Solid::DeviceInterface::OpticalDrive
                           << Solid::DeviceInterface::StorageVolume
                           << Solid::DeviceInterface::OpticalDisc
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::NetworkInterface
                           << Solid::DeviceInterface::AcAdapter
                           << Solid::DeviceInterface::Battery
                           << Solid::DeviceInterface::Button
                           << Solid::DeviceInterface::AudioInterface
                           << Solid::DeviceInterface::DvbInterface
                           << Solid::DeviceInterface::Video
                           << Solid::DeviceInterface::SerialInterface
                           << Solid::DeviceInterface::SmartCardReader;
}

WmiManager::~WmiManager()
{
    delete d;
}

QString WmiManager::udiPrefix() const
{
    return QString(); //FIXME: We should probably use a prefix there... has to be tested on Windows
}

QSet<Solid::DeviceInterface::Type> WmiManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList WmiManager::allDevices()
{
    QStringList deviceUdiList;

    QStringList aList;
    Q_FOREACH(const Solid::DeviceInterface::Type &dev, d->supportedInterfaces)
      aList<<findDeviceByDeviceInterface(dev);
    Q_FOREACH(const QString &udi, aList)
    {
        if (!deviceUdiList.contains(udi))
            deviceUdiList << udi;
    }

    return deviceUdiList;
}

bool WmiManager::deviceExists(const QString &udi)
{
    return WmiDevice::exists(udi);
}

QStringList WmiManager::devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type)
{
//    qDebug() <<"WmiManager::devicesFromQuery"<< parentUdi << type;
//    if (!parentUdi.isEmpty())
//    {
//        QStringList result = findDeviceStringMatch("info.parent", parentUdi);

//        if (type!=Solid::DeviceInterface::Unknown) {
//            QStringList::Iterator it = result.begin();
//            QStringList::ConstIterator end = result.end();

//            for (; it!=end; ++it)
//            {
//                WmiDevice device(*it);

//                if (!device.queryDeviceInterface(type)) {
//                    result.erase(it);
//                }
//            }
//        }

//        return result;

//    } else
    if (type!=Solid::DeviceInterface::Unknown) {
        return findDeviceByDeviceInterface(type);
    } else {
        return allDevices();
    }
}

QObject *WmiManager::createDevice(const QString &udi)
{
    if (deviceExists(udi)) {
        return new WmiDevice(udi);
    } else {
        return 0;
    }
}

QStringList WmiManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    qDebug() << "has to be implemented" << key << value;
    QStringList result;

//    qDebug() << result;
    return result;
}

QStringList WmiManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    QStringList result;

    switch (type)
    {
    case Solid::DeviceInterface::Processor:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::StorageAccess:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::StorageDrive:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::OpticalDrive:
//        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::StorageVolume:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::OpticalDisc:
//        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::Battery:
        result << WmiDevice::generateUDIList(type);
        break;
    }

    return result;
}

void WmiManager::slotDeviceAdded(const QString &udi)
{
    Q_EMIT deviceAdded(udi);
}

void WmiManager::slotDeviceRemoved(const QString &udi)
{
    Q_EMIT deviceRemoved(udi);
}

#include "backends/wmi/wmimanager.moc"
