/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>

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

#ifndef UDISKS2DEVICEINTERFACE_H
#define UDISKS2DEVICEINTERFACE_H

#include <ifaces/deviceinterface.h>
#include "udisksdevice.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{

class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(Device *device);
    virtual ~DeviceInterface();

protected:
    Device *m_device;

public:
    inline static QStringList toStringList(Solid::DeviceInterface::Type type)
    {
        QStringList list;

        switch(type)
        {
        case Solid::DeviceInterface::GenericInterface:
            list << "generic";
            break;
        case Solid::DeviceInterface::Block:
            list << "block";
            break;
        case Solid::DeviceInterface::StorageAccess:
            list << "volume";
            break;
        case Solid::DeviceInterface::StorageDrive:
            list << "storage";
            break;
        case Solid::DeviceInterface::OpticalDrive:
            list << "storage.cdrom";
            break;
        case Solid::DeviceInterface::StorageVolume:
            list << "volume";
            break;
        case Solid::DeviceInterface::OpticalDisc:
            list << "volume.disc";
            break;
        case Solid::DeviceInterface::Processor:
        case Solid::DeviceInterface::Camera:
        case Solid::DeviceInterface::PortableMediaPlayer:
        case Solid::DeviceInterface::NetworkInterface:
        case Solid::DeviceInterface::AcAdapter:
        case Solid::DeviceInterface::Battery:
        case Solid::DeviceInterface::Button:
        case Solid::DeviceInterface::AudioInterface:
        case Solid::DeviceInterface::DvbInterface:
        case Solid::DeviceInterface::Video:
        case Solid::DeviceInterface::SerialInterface:
        case Solid::DeviceInterface::SmartCardReader:
        case Solid::DeviceInterface::InternetGateway:
        case Solid::DeviceInterface::NetworkShare:
        case Solid::DeviceInterface::Keyboard:
        case Solid::DeviceInterface::PointingDevice:
            // These do not exist with UDisks
            break;
        case Solid::DeviceInterface::Unknown:
            break;
        case Solid::DeviceInterface::Last:
            break;
        }

        return list;
    }

    inline static Solid::DeviceInterface::Type fromString(const QString &capability)
    {
        if (capability == "generic")
            return Solid::DeviceInterface::GenericInterface;
        else if (capability == "block")
            return Solid::DeviceInterface::Block;
        else if (capability == "storage")
            return Solid::DeviceInterface::StorageDrive;
        else if (capability == "storage.cdrom")
            return Solid::DeviceInterface::OpticalDrive;
        else if (capability == "volume")
            return Solid::DeviceInterface::StorageVolume;
        else if (capability == "volume.disc")
            return Solid::DeviceInterface::OpticalDisc;
        else
            return Solid::DeviceInterface::Unknown;
    }
};

}
}
}

#endif // UDISKS2DEVICEINTERFACE_H
