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

#include "upnpdevicemanager.h"
#include "upnpdevice.h"

#include <QtCore/QtDebug>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <HUpnpCore/HDiscoveryType>
#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HUdn>

#include "../shared/rootdevice.h"


using namespace Solid::Backends::UPnP;
using namespace Solid::Backends::Shared;

UPnPDeviceManager::UPnPDeviceManager(QObject* parent) :
    Solid::Ifaces::DeviceManager(parent),
    m_supportedInterfaces()
    //m_upnpControlPoint(Solid::Backends::UPnP::UPnPControlPoint::instance())
{
    UPnPControlPoint* upnpControlPoint = UPnPControlPoint::acquireInstance();

    connect(
        upnpControlPoint->controlPoint(),
        SIGNAL(rootDeviceOnline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOnline(Herqq::Upnp::HClientDevice*)));

    connect(
        upnpControlPoint->controlPoint(),
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HClientDevice*)),
        this,
        SLOT(rootDeviceOffline(Herqq::Upnp::HClientDevice*)));

    UPnPControlPoint::releaseInstance();

    m_supportedInterfaces << Solid::DeviceInterface::StorageAccess;
}

UPnPDeviceManager::~UPnPDeviceManager()
{
}

QString UPnPDeviceManager::udiPrefix() const
{
    return QString::fromLatin1("/org/kde/upnp");
}

QSet< Solid::DeviceInterface::Type > UPnPDeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList UPnPDeviceManager::allDevices()
{
    QStringList result;

    result << udiPrefix();

    UPnPControlPoint* upnpControlPoint = UPnPControlPoint::acquireInstance();

    result+= upnpControlPoint->allDevices();

    UPnPControlPoint::releaseInstance();

    return result;
}

QStringList UPnPDeviceManager::devicesFromQuery(const QString& parentUdi, Solid::DeviceInterface::Type type)
{
    Q_UNUSED(parentUdi)
    Q_UNUSED(type)
    return QStringList(); //FIXME implement it!
}

QObject *UPnPDeviceManager::createDevice(const QString& udi)
{
    if (udi==udiPrefix()) {
        RootDevice *root = new RootDevice(udiPrefix());

        root->setProduct(tr("UPnP Devices"));
        root->setDescription(tr("UPnP devices detected on your network"));
        root->setIcon("network-server");

        return root;
    }

    QString udnFromUdi = udi.mid(udiPrefix().length() + 1);
    Herqq::Upnp::HUdn udn(udnFromUdi);
    if (udn.isValid(Herqq::Upnp::LooseChecks))
    {
        UPnPControlPoint* upnpControlPoint = UPnPControlPoint::acquireInstance();

        Herqq::Upnp::HClientDevice* device = upnpControlPoint->controlPoint()->device(udn);

        UPnPControlPoint::releaseInstance();
        if (device)
        {
            return new Solid::Backends::UPnP::UPnPDevice(device);
        }
    }

    return 0;
}

void UPnPDeviceManager::rootDeviceOnline(Herqq::Upnp::HClientDevice* device)
{
    QString udn = device->info().udn().toString();
    qDebug() << "UPnP device entered:" << udn;
    emit deviceAdded(udiPrefix() + '/' + udn);
}

void UPnPDeviceManager::rootDeviceOffline(Herqq::Upnp::HClientDevice* device)
{
    QString udn = device->info().udn().toString();
    qDebug() << "UPnP device gone:" << udn;
    emit deviceRemoved(udiPrefix() + '/' + udn);

    UPnPControlPoint* upnpControlPoint = UPnPControlPoint::acquireInstance();

    upnpControlPoint->controlPoint()->removeRootDevice(device);

    UPnPControlPoint::releaseInstance();
}

