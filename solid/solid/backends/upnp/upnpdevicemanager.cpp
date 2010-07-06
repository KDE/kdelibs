/*
   This file is part of the KDE project
   
   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>
   
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

#include "upnpdevicemanager.h"
#include "upnpdevice.h"

#include <QtCore/QtDebug>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <HDiscoveryType>
#include <HDeviceInfo>
#include <HDeviceProxy>
#include <HUdn>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPDeviceManager::UPnPDeviceManager(QObject* parent) :
    Solid::Ifaces::DeviceManager(parent),
    m_supportedInterfaces(),
    m_controlPoint(new Herqq::Upnp::HControlPoint(this))
{
    connect(
        m_controlPoint,
        SIGNAL(rootDeviceOnline(Herqq::Upnp::HDeviceProxy*)),
        this,
        SLOT(rootDeviceOnline(Herqq::Upnp::HDeviceProxy*)));

    connect(
        m_controlPoint,
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HDeviceProxy*)),
        this,
        SLOT(rootDeviceOffline(Herqq::Upnp::HDeviceProxy*)));

    if (!m_controlPoint->init())
    {
        qDebug() << "control point init error:" << m_controlPoint->errorDescription();
        return;
    }

    m_supportedInterfaces << Solid::DeviceInterface::StorageAccess;
}

UPnPDeviceManager::~UPnPDeviceManager()
{
    delete m_controlPoint;
}

QString UPnPDeviceManager::udiPrefix() const
{
    return QString::fromLatin1("/org/kde/upnp");
}

QSet< DeviceInterface::Type > UPnPDeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList UPnPDeviceManager::allDevices()
{
    QStringList result;

    result << udiPrefix();

    Herqq::Upnp::HDiscoveryType discoveryType = Herqq::Upnp::HDiscoveryType::createDiscoveryTypeForRootDevices();
    if (m_controlPoint->scan(discoveryType))
    {
        Herqq::Upnp::HDeviceProxies list = m_controlPoint->rootDevices();
        for (int i = 0; i < list.size(); ++i)
        {
            Herqq::Upnp::HDeviceProxy* device = list[i];
            Herqq::Upnp::HDeviceInfo info = device->deviceInfo();

            result << ( udiPrefix() + '/' + info.udn().toString() );
            qDebug() << "Found device:" << ( udiPrefix() + '/' + info.udn().toString() );
            //TODO listing only root devices
        }
    }
    else
    {
        qDebug() << "scan error:" << m_controlPoint->errorDescription();
    }

    return result;
}

QStringList UPnPDeviceManager::devicesFromQuery(const QString& parentUdi, DeviceInterface::Type type)
{
    Q_UNUSED(parentUdi)
    Q_UNUSED(type)
    return QStringList(); //FIXME implement it!
}

QObject* UPnPDeviceManager::createDevice(const QString& udi)
{
    QString udnFromUdi = udi.mid(udiPrefix().length() + 1);
    Herqq::Upnp::HUdn udn(udnFromUdi);
    if (udn.isValid())
    {
        Herqq::Upnp::HDeviceProxy* device = m_controlPoint->device(udn);
        if (device)
        {
            return new Solid::Backends::UPnP::UPnPDevice(device);
        }
    }

    return 0;
}

void UPnPDeviceManager::rootDeviceOnline(Herqq::Upnp::HDeviceProxy* device)
{
    QString udn = device->deviceInfo().udn().toString();

    emit deviceAdded(udiPrefix() + '/' + udn);
}

void UPnPDeviceManager::rootDeviceOffline(Herqq::Upnp::HDeviceProxy* device)
{
    QString udn = device->deviceInfo().udn().toString();

    emit deviceRemoved(udiPrefix() + '/' + udn);

    m_controlPoint->removeRootDevice(device);
}

}
}
}