/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    
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

#include "windevice.h"
#include <solid/deviceinterface.h>

#include "wingenericinterface.h"
#include "winstoragevolume.h"
#include "winblock.h"
#include "winstorageaccess.h"
#include "winstoragedrive.h"

using namespace Solid::Backends::Win;

QMap<QString,QString> WinDevice::m_driveLetters = QMap<QString,QString>();

WinDevice::WinDevice(const QString &udi) :
    Device(),
    m_udi(udi),
    m_type(Solid::DeviceInterface::Unknown)
{
    /*
     /org/kde/solid/win/volume/disk #%1, partition #%2
     /org/kde/solid/win/storage.cdrom/disk #%0
    */
    QStringList data = udi.split("/");
    QString parentName = data[6].split(",")[0].trimmed();
    QString type = data[5];

    if(type == "storage")
        m_type = Solid::DeviceInterface::StorageDrive;
    else if(type == "volume")
        m_type = Solid::DeviceInterface::StorageVolume;
    else if (type == "storage.cdrom")
        m_type = Solid::DeviceInterface::OpticalDrive;



    switch(m_type)
    {
    case Solid::DeviceInterface::StorageVolume:
    {
        m_parentUdi = QString("/org/kde/solid/win/storage/").append(parentName);
    }
        break;
    default:
        m_parentUdi = QString("/org/kde/solid/win/").append(type);
    }

    QString dev;
    if(m_type == Solid::DeviceInterface::StorageDrive)
    {
          dev = QString("PhysicalDrive%1").arg(WinBlock(this).deviceMajor());
    }
    else
    {
        dev = driveLetter();
    }

    STORAGE_PROPERTY_QUERY query;
    query.PropertyId = StorageDeviceProperty;
    query.QueryType =  PropertyStandardQuery;

    char buff[1024];
    WinDeviceManager::getDeviceInfo(dev,IOCTL_STORAGE_QUERY_PROPERTY,buff,1024,&query);
    STORAGE_DEVICE_DESCRIPTOR *info = ((STORAGE_DEVICE_DESCRIPTOR*)buff);
    m_vendor = QString((char*)buff+ info->ProductIdOffset).trimmed();


}

QString WinDevice::udi() const
{
    return m_udi;
}

QString WinDevice::parentUdi() const
{
    return m_parentUdi;
}

QString WinDevice::vendor() const
{
    return m_vendor;
}

QString WinDevice::product() const
{
    switch(m_type)
    {
    case Solid::DeviceInterface::StorageVolume:
    {
        WinDevice wDev(udi());
        WinStorageVolume dev(&wDev);
        return dev.label();
    }
    default:
        return QString("Not implemented");
    }
}

QString WinDevice::icon() const
{
    QString icon;
    switch(type()){

//    case Solid::DeviceInterface::OpticalDisc:
//    {
////        WmiDevice dev(udi());
////        OpticalDisc disk(&dev);
////        if(disk.availableContent() | Solid::OpticalDisc::Audio)//no other are recognized yet
////            propertyName = "media-optical-audio";
////        else
////            propertyName = "drive-optical";

////        break;
//    }
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageVolume:
    {
        WinStorageDrive storage(const_cast<WinDevice*>(this));
        if(storage.bus() == Solid::StorageDrive::Usb)
            icon = "drive-removable-media-usb-pendrive";
        else
            icon = "drive-harddisk";
    }
        break;
    default:
        break;
    }
    return icon;
}

QStringList WinDevice::emblems() const
{
    //TODO:implement
    return QStringList("Not implemented");
}

QString WinDevice::description() const
{
    return product();
}


bool WinDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    if (type==Solid::DeviceInterface::GenericInterface) {
        return true;
    }

    QList<Solid::DeviceInterface::Type> interfaceList;
    interfaceList<<type;

    switch (type)
    {
    case Solid::DeviceInterface::GenericInterface:
        break;
    case Solid::DeviceInterface::Block:
        break;
    case Solid::DeviceInterface::StorageAccess:
        break;
    case Solid::DeviceInterface::StorageDrive:
        break;
    case Solid::DeviceInterface::OpticalDrive:
        interfaceList <<Solid::DeviceInterface::Block<<Solid::DeviceInterface::StorageDrive;
        break;
    case Solid::DeviceInterface::StorageVolume:
        interfaceList <<Solid::DeviceInterface::Block<<Solid::DeviceInterface::StorageAccess;
        break;
    case Solid::DeviceInterface::OpticalDisc:
        interfaceList <<Solid::DeviceInterface::Block<<Solid::DeviceInterface::StorageVolume;
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
    default:
        break;
    }

    if (interfaceList.size() == 0)
        qWarning() << "no interface found for type" << type;
    return interfaceList.contains(type);
}

QObject *WinDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }
    WinInterface *iface = 0;


    switch (type)
    {
    case Solid::DeviceInterface::GenericInterface:
        iface = new WinGenericInterface(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new WinBlock(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new WinStorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new WinStorageDrive(this);
        break;
        //      case Solid::DeviceInterface::OpticalDrive:
        //          iface = new Cdrom(this);
        //          break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new WinStorageVolume(this);
        break;
        //      case Solid::DeviceInterface::OpticalDisc:
        //          iface = new OpticalDisc(this);
        //          break;
        //      case Solid::DeviceInterface::PortableMediaPlayer:
        //          iface = new PortableMediaPlayer(this);
        //          break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
        break;
    default:
        break;
    }

    return iface;
}

Solid::DeviceInterface::Type WinDevice::type() const
{
    return m_type;
}

QString WinDevice::driveLetter() const
{
    return m_driveLetters[udi()];
}

#include "windevice.moc"
