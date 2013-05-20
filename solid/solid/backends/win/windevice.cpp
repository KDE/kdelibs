/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>
    
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
#include "winopticaldrive.h"
#include "winopticaldisc.h"
#include "windevice.h"
#include "winprocessor.h"
#include "winbattery.h"
#include "winacadapter.h"

#include <batclass.h>

using namespace Solid::Backends::Win;

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
    else if (type == "volume.cdrom")
        m_type = Solid::DeviceInterface::OpticalDisc;
    else if (type == "cpu")
        m_type = Solid::DeviceInterface::Processor;
    else if (type == "power.battery")
        m_type = Solid::DeviceInterface::Battery;
    else if(type == "power")
        m_type = Solid::DeviceInterface::AcAdapter;



    switch(m_type)
    {
    case Solid::DeviceInterface::StorageVolume:
    {
        m_parentUdi = QLatin1String("/org/kde/solid/win/storage/") + parentName;
    }
        break;
    case Solid::DeviceInterface::OpticalDisc:
    {
        m_parentUdi = QLatin1String("/org/kde/solid/win/storage.cdrom/") + parentName;
    }
        break;
    default:
        m_parentUdi = QLatin1String("/org/kde/solid/win/")+ type;
    }

    QString dev;

    if(m_type == Solid::DeviceInterface::Processor)
    {
        WinProcessor cpu(this);
        WinProcessor::ProcessorInfo info = WinProcessor::updateCache()[cpu.number()];
        m_vendor = info.vendor;
        m_product = info.produuct;
        m_description = info.name;
    }
    else if(m_type == Solid::DeviceInterface::Battery)
    {
        WinBattery::Battery battery = WinBattery::batteryInfoFromUdi(m_udi);
        BATTERY_QUERY_INFORMATION query;
        ZeroMemory(&query,sizeof(query));
        query.BatteryTag = battery.second;


        size_t size = 1024;
        wchar_t buff[size];

        query.InformationLevel = BatteryDeviceName;
        WinDeviceManager::getDeviceInfo<wchar_t,BATTERY_QUERY_INFORMATION>(battery.first,IOCTL_BATTERY_QUERY_INFORMATION,buff,size,&query);
        m_product = QString::fromWCharArray(buff);

        query.InformationLevel = BatteryManufactureName;
        WinDeviceManager::getDeviceInfo<wchar_t,BATTERY_QUERY_INFORMATION>(battery.first,IOCTL_BATTERY_QUERY_INFORMATION,buff,size,&query);
        m_vendor = QString::fromWCharArray(buff);

        query.InformationLevel = BatteryInformation;
        BATTERY_INFORMATION info = WinDeviceManager::getDeviceInfo<BATTERY_INFORMATION,BATTERY_QUERY_INFORMATION>(battery.first,IOCTL_BATTERY_QUERY_INFORMATION,&query);

        if(info.Chemistry != 0)
        {
            QString tech = QString::fromUtf8((const char*)info.Chemistry,4);

            m_description = QObject::tr("%1 Battery", "%1 is battery technology").arg(batteryTechnology(tech.toUpper()));
        }

    }
    else if(m_type ==Solid::DeviceInterface::AcAdapter )
    {
        m_description = QObject::tr("A/C Adapter");
    }
    else if(m_type == Solid::DeviceInterface::OpticalDrive)
    {
        dev = WinBlock::driveLetterFromUdi(udi);
    }
    else if(m_type == Solid::DeviceInterface::StorageDrive )
    {
        dev = QString("PhysicalDrive%1").arg(WinBlock(this).deviceMajor());
    }
    else if(queryDeviceInterface(Solid::DeviceInterface::StorageVolume))
    {
        m_description = WinStorageVolume(this).label();
        dev = WinBlock::driveLetterFromUdi(udi);
    }
    if(!dev.isNull())
    {
        STORAGE_PROPERTY_QUERY query;
        ZeroMemory(&query,sizeof(STORAGE_PROPERTY_QUERY));
        query.PropertyId = StorageDeviceProperty;
        query.QueryType =  PropertyStandardQuery;

        char buff[1024];
        WinDeviceManager::getDeviceInfo<char,STORAGE_PROPERTY_QUERY>(dev,IOCTL_STORAGE_QUERY_PROPERTY,buff,1024,&query);
        STORAGE_DEVICE_DESCRIPTOR *info = ((STORAGE_DEVICE_DESCRIPTOR*)buff);
        if(info->VendorIdOffset != 0)
        {
            m_vendor = QString((char*)buff+ info->VendorIdOffset).trimmed();
            if(info->ProductIdOffset != 0)
            {
                m_product = QString((char*)buff+ info->ProductIdOffset).trimmed();
            }
        }
        else if(info->ProductIdOffset != 0)//fallback doesnt work for all devices
        {
            QStringList tmp = QString((char*)buff+ info->ProductIdOffset).trimmed().split(" ");
            m_vendor = tmp.takeFirst();
            m_product = tmp.join(" ");
        }
    }


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
    return m_product;
}

QString WinDevice::description() const
{
    return m_description.isEmpty()?m_product:m_description;
}

QString WinDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
    }

    QString icon;
    switch(type()){
    case Solid::DeviceInterface::OpticalDrive:
        icon =  QLatin1String("drive-optical");
        break;
    case Solid::DeviceInterface::OpticalDisc:
    {
        WinOpticalDisc disk(const_cast<WinDevice*>(this));
        if(disk.availableContent() | Solid::OpticalDisc::Audio)
        {
            icon =  QLatin1String("media-optical-audio");
        }
        else
        {
            icon =  QLatin1String("drive-optical");
        }
        break;
    }
    case Solid::DeviceInterface::StorageDrive:
    case Solid::DeviceInterface::StorageAccess:
    case Solid::DeviceInterface::StorageVolume:
    {
        WinStorageDrive storage(const_cast<WinDevice*>(this));
        if(storage.bus() == Solid::StorageDrive::Usb)
            icon =  QLatin1String("drive-removable-media-usb-pendrive");
        else
            icon =  QLatin1String("drive-harddisk");
    }
        break;
    case Solid::DeviceInterface::Processor:
        icon = QLatin1String("cpu");
        break;
    case Solid::DeviceInterface::Battery:
        icon = QLatin1String("battery");
        break;
    case Solid::DeviceInterface::AcAdapter:
        icon = QLatin1String("preferences-system-power-management");
        break;
    default:
        break;
    }
    return icon;
}

QStringList WinDevice::emblems() const
{
    //TODO:implement
    return QStringList();
}

bool WinDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &queryType) const
{
    if (queryType == Solid::DeviceInterface::GenericInterface) {
        return true;
    }

    QList<Solid::DeviceInterface::Type> interfaceList;
    interfaceList << type();

    switch (type())
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
        interfaceList << Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageDrive;
        break;
    case Solid::DeviceInterface::StorageVolume:
        interfaceList <<Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageAccess;
        break;
    case Solid::DeviceInterface::OpticalDisc:
        interfaceList << Solid::DeviceInterface::Block << Solid::DeviceInterface::StorageVolume << Solid::DeviceInterface::StorageAccess;
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
    default:
        break;
    }

    if (interfaceList.size() == 0)
        qWarning() << "no interface found for type" << type();
    return interfaceList.contains(queryType);
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
    case Solid::DeviceInterface::Processor:
        iface = new WinProcessor(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new WinStorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new WinStorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new WinOpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new WinStorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new WinOpticalDisc(this);
        break;
        //      case Solid::DeviceInterface::PortableMediaPlayer:
        //          iface = new PortableMediaPlayer(this);
        //          break;
    case Solid::DeviceInterface::Battery:
        iface = new WinBattery(this);
        break;
    case Solid::DeviceInterface::AcAdapter:
        iface = new WinAcadapter(this);
        break;
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

QString WinDevice::batteryTechnology(QString tec) const
{

    if(tec == "LION" || tec == "LI-I")
    {
        return QObject::tr("Lithium Ion", "battery technology");
    }
    else if(tec == "PBAC")
    {
        return QObject::tr("Lead Acid", "battery technology");
    }
    else if(tec == "NICD")
    {
        return QObject::tr("Nickel Cadmium", "battery technology");
    }
    else if(tec == "NIMH")
    {
        return QObject::tr("Nickel Metal Hydride", "battery technology");
    }
    else if(tec == "NIZN")
    {
        return QObject::tr("Nickel  Zinc", "battery technology");
    }
    else
    {
        qDebug()<<tec<< QObject::tr("Unknown", "battery technology");
        return QObject::tr("Unknown", "battery technology");
    }
}


#include "windevice.moc"
