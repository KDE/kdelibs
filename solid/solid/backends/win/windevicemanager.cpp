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

#include <solid/deviceinterface.h>
#include "windevicemanager.h"
#include "windevice.h"

#include <windows.h>
#include <winioctl.h>




using namespace Solid::Backends::Win;

WinDeviceManager::WinDeviceManager(QObject *parent)
    :DeviceManager(parent)
{
    m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                          << Solid::DeviceInterface::Block
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
    QStringList list;
    foreach(const Solid::DeviceInterface::Type &type,m_supportedInterfaces){
        list<<devicesFromQuery("",type);
    }
    return list;
}

QStringList WinDeviceManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList list;
    if(type & (Solid::DeviceInterface::StorageVolume | Solid::DeviceInterface::StorageAccess | Solid::DeviceInterface::StorageDrive)){
        QSet<int> storageDrives;
        DWORD word = GetLogicalDrives();
        wchar_t buff[MAX_PATH];
        char c = 'A';
        int i = 0;
        while(word != 0)
        {
            if(word & 1 ){

                QString drive = QString("%1:").arg((char)(c+i));
                QString dev = QString("\\\\.\\%1").arg(drive);
                dev.toWCharArray(buff);
                HANDLE h = ::CreateFile(buff, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

                STORAGE_DEVICE_NUMBER info = {};
                DWORD bytesReturned =  0;

                ::DeviceIoControl(h, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &info, sizeof(info), &bytesReturned, NULL);
                ::CloseHandle(h);
                if(info.DeviceType == FILE_DEVICE_DISK)
                {
                    QString udi = QString("/org/kde/solid/win/volume/disk #%1, partition #%2").arg(info.DeviceNumber).arg(info.PartitionNumber);
                    list<<udi;
                    storageDrives.insert(info.DeviceNumber);
                    WinDevice::m_driveLetters[udi] = drive;
                }//TODO: handle subst
            }
            word = (word >> 1);
            ++i;
        }
        foreach(int i,storageDrives){
            list<<QString("/org/kde/solid/win/storage/disk #%1").arg(i);
        }

    }


    return list;
}


QObject *Solid::Backends::Win::WinDeviceManager::createDevice(const QString &udi)
{
    //TODO:: implement
    //    if (deviceExists(udi)) {
    return new WinDevice(udi);
    //    } else {
    //        return 0;
    //    }
}



#include <windevicemanager.moc>

