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
    QSet<QString> list;
    DWORD word = GetLogicalDrives();
    char c = 'A';
    int i = 0;
    while(word != 0)
    {
        if(word & 1 ){

            QString drive = QString("%1:").arg((char)(c+i));

            STORAGE_DEVICE_NUMBER info = getDeviceInfo<STORAGE_DEVICE_NUMBER,void*>(drive,IOCTL_STORAGE_GET_DEVICE_NUMBER,NULL);

            QString udi;

            if(info.DeviceType == FILE_DEVICE_DISK)
            {
                udi = QString("/org/kde/solid/win/volume/disk #%1, partition #%2").arg(info.DeviceNumber).arg(info.PartitionNumber);
                list<<QString("/org/kde/solid/win/storage/disk #%1").arg(info.DeviceNumber);
            }//TODO: handle subst

            else if(info.DeviceType == FILE_DEVICE_CD_ROM)
            {
                udi = QString("/org/kde/solid/win/storage.cdrom/disk #%1").arg(info.DeviceNumber);
            }


            if(!udi.isNull())
            {
                list<<udi;
                WinDevice::m_driveLetters[udi] = drive;
            }
        }
        word = (word >> 1);
        ++i;
    }
    return list.toList();
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
    //    if (deviceExists(udi)) {
    return new WinDevice(udi);
    //    } else {
    //        return 0;
    //    }
}



//template <typename  INFO>
//INFO WinDeviceManager::getDeviceInfo(QString devName, int code)
//{
//    wchar_t buff[MAX_PATH];
//    QString dev = QString("\\\\.\\%1").arg(devName);
//    buff[dev.toWCharArray(buff)] = 0;
//    HANDLE h = ::CreateFile(buff, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

//    INFO info;
//    ZeroMemory(&info,sizeof(info));

//    DWORD bytesReturned =  0;


//    ::DeviceIoControl(h, code, NULL, 0, &info, sizeof(info), &bytesReturned, NULL);
//    ::CloseHandle(h);
//    return info;
//}


#include <windevicemanager.moc>

