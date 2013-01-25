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

#include <solid/deviceinterface.h>
#include "windevicemanager.h"
#include "windevice.h"


#include <QDebug>

using namespace Solid::Backends::Win;

WinDeviceManager::WinDeviceManager(QObject *parent)
    :DeviceManager(parent)
{
    m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                             //                          << Solid::DeviceInterface::Block
                          << Solid::DeviceInterface::StorageAccess
                          << Solid::DeviceInterface::StorageDrive
                          << Solid::DeviceInterface::OpticalDrive
                          << Solid::DeviceInterface::StorageVolume
                          << Solid::DeviceInterface::OpticalDisc;

}

QString WinDeviceManager::udiPrefix() const
{
    return QString();
}


QSet<Solid::DeviceInterface::Type> Solid::Backends::Win::WinDeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList WinDeviceManager::allDevices()
{
    if(!m_devices.isEmpty())
        return m_devices;
    QSet<QString> list;
    DWORD word = GetLogicalDrives();
    char c = 'A';
    int i = 0;
    while(word != 0)
    {
        if(word & 1 ){

            QString drive = QString("%1:").arg((char)(c+i));

            STORAGE_DEVICE_NUMBER info = getDeviceInfo<STORAGE_DEVICE_NUMBER,void*>(drive,IOCTL_STORAGE_GET_DEVICE_NUMBER);



            if(info.DeviceType == FILE_DEVICE_DISK)
            {
                QString udi = QString("/org/kde/solid/win/volume/disk#%1,partition#%2").arg(info.DeviceNumber).arg(info.PartitionNumber);
                list<<udi;
                WinDevice::m_driveLetters[udi] = drive;
                list<<QString("/org/kde/solid/win/storage/disk#%1").arg(info.DeviceNumber);
            }
            else if(info.DeviceType == FILE_DEVICE_CD_ROM || info.DeviceType == FILE_DEVICE_DVD)
            {
                QString udi = QString("/org/kde/solid/win/storage.cdrom/disk#%1").arg(info.DeviceNumber);
                list<<udi;
                WinDevice::m_driveLetters[udi] = drive;

                udi = QString("/org/kde/solid/win/volume.cdrom/disk#%1").arg(info.DeviceNumber);
                list<<udi;
                WinDevice::m_driveLetters[udi] = drive;
            }
            else if(info.DeviceType == 0)
            {
                //subst drive
            }
            else
            {
                qDebug()<<"unknown device"<<drive<<info.DeviceType<<info.DeviceNumber<<info.PartitionNumber;
            }

        }
        word = (word >> 1);
        ++i;
    }
    m_devices = list.toList();
    return m_devices;
}


QStringList WinDeviceManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{

    QStringList list;
    if (!parentUdi.isEmpty())
    {
        foreach(const QString &udi,allDevices()){
            WinDevice device(udi);
            if(device.type() == type && device.parentUdi() == parentUdi ){
                list<<udi;
            }
        }
    } else if (type!=Solid::DeviceInterface::Unknown) {
        foreach(const QString &udi,allDevices()){
            WinDevice device(udi);
            if(device.type() == type){
                list<<udi;
            }
        }
    } else {
        list<<allDevices();
    }
    return list;

}


QObject *Solid::Backends::Win::WinDeviceManager::createDevice(const QString &udi)
{
    //TODO:: implement
    if (allDevices().contains(udi)) {
        return new WinDevice(udi);
    } else {
        return 0;
    }
}


#include <windevicemanager.moc>

