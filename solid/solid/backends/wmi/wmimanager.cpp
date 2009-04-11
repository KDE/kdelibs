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

#include <QtCore/QDebug>

#include "wmimanager.h"
#include "wmidevice.h"
#include "wmideviceinterface.h"
#include "wmiquery.h"

#include "kdebug.h"

#ifdef _DEBUG
# pragma comment(lib, "comsuppwd.lib")
#else
# pragma comment(lib, "comsuppw.lib")
#endif
# pragma comment(lib, "wbemuuid.lib")

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")


using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiManagerPrivate
{
public:
    WmiManagerPrivate()
        : m_query()
    {
    }

    ~WmiManagerPrivate() 
    {
    }
    
    WmiQuery::ItemList sendQuery( const QString &wql )
    {
        return m_query.sendQuery( wql );
    }
 
    WmiQuery m_query; 
};


WmiManager::WmiManager(QObject *parent)
    : DeviceManager(parent),  d(new WmiManagerPrivate())
{
    
}

WmiManager::~WmiManager()
{
    delete d;
}

QStringList WmiManager::allDevices()
{
    QStringList deviceUdiList;

    QStringList aList = findDeviceByDeviceInterface(Solid::DeviceInterface::OpticalDrive);
    foreach(QString udi, aList)
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
    kDebug() << parentUdi << type;
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
    kDebug() << "has to be implemented" << key << value;
    QStringList result;

    kDebug() << result;
    return result;
}

QStringList WmiManager::findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    kDebug() << type;
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

    kDebug() << result;
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
