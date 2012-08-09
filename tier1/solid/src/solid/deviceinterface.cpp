/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "deviceinterface.h"
#include "deviceinterface_p.h"

#include <solid/ifaces/deviceinterface.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaEnum>


Solid::DeviceInterface::DeviceInterface(DeviceInterfacePrivate &dd, QObject *backendObject)
    : d_ptr(&dd)
{
    Q_D(DeviceInterface);

    d->setBackendObject(backendObject);
}


Solid::DeviceInterface::~DeviceInterface()
{
    delete d_ptr;
    d_ptr = 0;
}

bool Solid::DeviceInterface::isValid() const
{
    Q_D(const DeviceInterface);
    return d->backendObject()!=0;
}

QString Solid::DeviceInterface::typeToString(Type type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return QString(metaEnum.valueToKey((int)type));
}

Solid::DeviceInterface::Type Solid::DeviceInterface::stringToType(const QString &type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return (Type)metaEnum.keyToValue(type.toUtf8());
}

QString Solid::DeviceInterface::typeDescription(Type type)
{
    switch (type)
    {
    case Unknown:
        return QCoreApplication::translate("", "Unknown", "Unknown device type");
    case GenericInterface:
        return QCoreApplication::translate("", "Generic Interface", "Generic Interface device type");
    case Processor:
        return QCoreApplication::translate("", "Processor", "Processor device type");
    case Block:
        return QCoreApplication::translate("", "Block", "Block device type");
    case StorageAccess:
        return QCoreApplication::translate("", "Storage Access", "Storage Access device type");
    case StorageDrive:
        return QCoreApplication::translate("", "Storage Drive", "Storage Drive device type");
    case OpticalDrive:
        return QCoreApplication::translate("", "Optical Drive", "Optical Drive device type");
    case StorageVolume:
        return QCoreApplication::translate("", "Storage Volume", "Storage Volume device type");
    case OpticalDisc:
        return QCoreApplication::translate("", "Optical Disc", "Optical Disc device type");
    case Camera:
        return QCoreApplication::translate("", "Camera", "Camera device type");
    case PortableMediaPlayer:
        return QCoreApplication::translate("", "Portable Media Player", "Portable Media Player device type");
    case NetworkInterface:
        return QCoreApplication::translate("", "Network Interface", "Network Interface device type");
    case AcAdapter:
        return QCoreApplication::translate("", "Ac Adapter", "Ac Adapter device type");
    case Battery:
        return QCoreApplication::translate("", "Battery", "Battery device type");
    case Button:
        return QCoreApplication::translate("", "Button", "Button device type");
    case AudioInterface:
        return QCoreApplication::translate("", "Audio Interface", "Audio Interface device type");
    case DvbInterface:
        return QCoreApplication::translate("", "Dvb Interface", "Dvb Interface device type");
    case Video:
        return QCoreApplication::translate("", "Video", "Video device type");
    case SerialInterface:
        return QCoreApplication::translate("", "Serial Interface", "Serial Interface device type");
    case SmartCardReader:
        return QCoreApplication::translate("", "Smart Card Reader", "Smart Card Reader device type");
    case InternetGateway:
        return QCoreApplication::translate("", "Internet Gateway Device", "Internet Gateway device type");
    case NetworkShare:
        return QCoreApplication::translate("", "Network Share", "Network Share device type");
    case Last:
        return QString();
    }
    return QString();
}

Solid::DeviceInterfacePrivate::DeviceInterfacePrivate()
    : m_devicePrivate(0)
{

}

Solid::DeviceInterfacePrivate::~DeviceInterfacePrivate()
{

}

QObject *Solid::DeviceInterfacePrivate::backendObject() const
{
    return m_backendObject.data();
}

void Solid::DeviceInterfacePrivate::setBackendObject(QObject *object)
{
    m_backendObject = object;
}

Solid::DevicePrivate* Solid::DeviceInterfacePrivate::devicePrivate() const
{
    return m_devicePrivate;
}

void Solid::DeviceInterfacePrivate::setDevicePrivate(DevicePrivate *devicePrivate)
{
    m_devicePrivate = devicePrivate;
}

