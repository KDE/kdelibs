/*
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

#include <QtCore/QtDebug>

#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HUdn>
#include <HUpnpCore/HDiscoveryType>

#include "upnpcontrolpoint.h"

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPControlPoint::UPnPControlPoint() :
    m_controlPoint(new Herqq::Upnp::HControlPoint(this))
{
    if (!m_controlPoint->init())
    {
        qDebug() << "control point init error:" << m_controlPoint->errorDescription();
        return;
    }
}

UPnPControlPoint::~UPnPControlPoint()
{
    delete m_controlPoint;
}

UPnPControlPoint* UPnPControlPoint::inst = 0;

UPnPControlPoint* UPnPControlPoint::instance()
{
    if (!inst)
    {
        inst = new UPnPControlPoint;
    }

    return inst;
}

QMutex UPnPControlPoint::mutex;

UPnPControlPoint* UPnPControlPoint::acquireInstance()
{
    mutex.lock();

    return instance();
}

void UPnPControlPoint::releaseInstance()
{
    mutex.unlock();
}

Herqq::Upnp::HControlPoint* UPnPControlPoint::controlPoint()
{
    return m_controlPoint;
}

QStringList UPnPControlPoint::allDevices()
{
    QStringList result;
    Herqq::Upnp::HDiscoveryType discoveryType = Herqq::Upnp::HDiscoveryType::createDiscoveryTypeForRootDevices();

    Herqq::Upnp::HClientDevices list = m_controlPoint->rootDevices();

    for (int i = 0; i < list.size(); ++i)
    {
        Herqq::Upnp::HClientDevice* device = list[i];
        Herqq::Upnp::HDeviceInfo info = device->info();

        result << ( QString::fromLatin1("/org/kde/upnp") + '/' + info.udn().toString() );
        qDebug() << "Found device:" << ( QString::fromLatin1("/org/kde/upnp") + '/' + info.udn().toString() );
        // listing only root devices
    }

    return result;
}

}
}
}
