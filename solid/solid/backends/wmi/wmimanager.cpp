 /*  This file is part of the KDE project
    Copyright (C) 2005,2006 Kevin Ottens <ervin@kde.org>

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

    QStringList aList = findDeviceByDeviceInterface(Solid::DeviceInterface::OpticalDrive);
    foreach(const QString &udi, aList)
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
    qDebug() << parentUdi << type;
    if (!parentUdi.isEmpty())
    {
        QStringList result = findDeviceStringMatch("info.parent", parentUdi);

        if (type!=Solid::DeviceInterface::Unknown) {
            QStringList::Iterator it = result.begin();
            QStringList::ConstIterator end = result.end();

            for (; it!=end; ++it)
            {
                WmiDevice device(*it);

                if (!device.queryDeviceInterface(type)) {
                    result.erase(it);
                }
            }
        }

        return result;

    } else if (type!=Solid::DeviceInterface::Unknown) {
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

    qDebug() << result;
    return result;
}

QStringList WmiManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    qDebug() << type;
    QStringList result;
    WmiQuery::ItemList list;

    switch (type)
    {
    case Solid::DeviceInterface::GenericInterface:
        break;
    case Solid::DeviceInterface::Processor:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::Block:
        break;
    case Solid::DeviceInterface::StorageAccess:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::StorageDrive:
        break;
    case Solid::DeviceInterface::OpticalDrive:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::StorageVolume:
        break;
    case Solid::DeviceInterface::OpticalDisc:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::Camera:
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        break;
    case Solid::DeviceInterface::NetworkInterface:
        break;
    case Solid::DeviceInterface::AcAdapter:
        break;
    case Solid::DeviceInterface::Battery:
        result << WmiDevice::generateUDIList(type);
        break;
    case Solid::DeviceInterface::Button:
        break;
    case Solid::DeviceInterface::AudioInterface:
        break;
    case Solid::DeviceInterface::DvbInterface:
        break;
    case Solid::DeviceInterface::Video:
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
        break;
    }

    qDebug() << result;
    return result;
}

void WmiManager::slotDeviceAdded(const QString &udi)
{
    emit deviceAdded(udi);
}

void WmiManager::slotDeviceRemoved(const QString &udi)
{
    emit deviceRemoved(udi);
}

#include "backends/wmi/wmimanager.moc"
