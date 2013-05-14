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

#include "winblock.h"

#include <QDebug>

using namespace Solid::Backends::Win;


QMap<QString,QString> WinBlock::m_driveLetters = QMap<QString,QString>();

WinBlock::WinBlock(WinDevice *device):
    WinInterface(device),
    m_major(-1),
    m_minor(-1)
{
    if(m_device->type() == Solid::DeviceInterface::StorageVolume)
    {
        STORAGE_DEVICE_NUMBER info = WinDeviceManager::getDeviceInfo<STORAGE_DEVICE_NUMBER,void*>(driveLetter(m_device->udi()),IOCTL_STORAGE_GET_DEVICE_NUMBER);
        m_major = info.DeviceNumber;
        m_minor = info.PartitionNumber;
    }
    else if(m_device->type() == Solid::DeviceInterface::StorageDrive ||
            m_device->type() == Solid::DeviceInterface::OpticalDrive ||
            m_device->type() == Solid::DeviceInterface::OpticalDisc)
    {
        m_major = m_device->udi().mid(m_device->udi().length()-1).toInt();
    }
    else
    {
        qFatal("Not implemented device type %i",m_device->type());
    }

}

WinBlock::~WinBlock()
{
}

int WinBlock::deviceMajor() const
{
    Q_ASSERT(m_major != -1);
    return m_major;
}

int WinBlock::deviceMinor() const
{
    return m_minor;
}

QString WinBlock::device() const
{
    return driveLetter(m_device->udi());
}

QSet<QString> WinBlock::getUdis()
{
    QSet<QString> list;
    DWORD word = GetLogicalDrives();
    char c = 'A';
    int i = 0;
    while(word != 0)
    {
        if(word & 1 ){

            QString drive = QString("%1:").arg((char)(c+i));

            STORAGE_DEVICE_NUMBER info = WinDeviceManager::getDeviceInfo<STORAGE_DEVICE_NUMBER,void*>(drive,IOCTL_STORAGE_GET_DEVICE_NUMBER);

            if(info.DeviceType == FILE_DEVICE_DISK)
            {
                QString udi = QString("/org/kde/solid/win/volume/disk#%1,partition#%2").arg(info.DeviceNumber).arg(info.PartitionNumber);
                list<<udi;
                m_driveLetters[udi] = drive;
                list<<QString("/org/kde/solid/win/storage/disk#%1").arg(info.DeviceNumber);
            }
            else if(info.DeviceType == FILE_DEVICE_CD_ROM || info.DeviceType == FILE_DEVICE_DVD)
            {
                QString udi = QString("/org/kde/solid/win/storage.cdrom/disk#%1").arg(info.DeviceNumber);
                list<<udi;
                m_driveLetters[udi] = drive;

                udi = QString("/org/kde/solid/win/volume.cdrom/disk#%1").arg(info.DeviceNumber);
                list<<udi;
                m_driveLetters[udi] = drive;
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
    return list;
}

QString WinBlock::driveLetter(const QString &udi)
{
    if(!m_driveLetters.contains(udi))
        qWarning()<<udi<<"is not connected to a drive";
    return m_driveLetters[udi];
}

#include "winblock.moc"
