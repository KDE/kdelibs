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
#include "winprocessor.h"
#include "winblock.h"


#include <QDebug>
#include <QTimer>
#include <QTime>


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

    QTimer *timer = new QTimer(parent);

    timer->setInterval(10000);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateDeviceList()));
    timer->start();

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
    if(m_devices.isEmpty())
    {
        updateDeviceList();
    }
    return m_devicesList;
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
    if (allDevices().contains(udi)) {
        return new WinDevice(udi);
    } else {
        return 0;
    }
}

void WinDeviceManager::updateDeviceList()
{
    QTime t;
    t.start();
    QSet<QString> devices = WinProcessor::getUdis();
    devices |= WinBlock::getUdis();

    qDebug()<<"Generation of device list took"<<t.elapsed();

    if(!m_devices.isEmpty())
    {
        QSet<QString> added = devices - m_devices;
        foreach(const QString & s,added)
        {
            qDebug()<<"Device added"<<s;
            emit deviceAdded(s);
        }
        QSet<QString> removed = m_devices - devices;
        foreach(const QString & s,removed)
        {
            qDebug()<<"Device removed"<<s;
            emit deviceRemoved(s);
        }
    }

    m_devices = devices;
    m_devicesList = m_devices.toList();
    qSort(m_devicesList);
}


#include <windevicemanager.moc>

