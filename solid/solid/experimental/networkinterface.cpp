/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QMap>
#include <QStringList>

#include <solid/experimental/ifaces/network.h>
#include <solid/experimental/ifaces/networkinterface.h>
#include <solid/experimental/ifaces/wirelessnetwork.h>

#include "frontendobject_p.h"

#include "../soliddefs_p.h"
#include "networkmanager.h"
#include "network.h"
#include "network_p.h"
#include "networkinterface.h"
#include "wirelessnetwork.h"

namespace SolidExperimental
{
    class NetworkInterfacePrivate : public FrontendObjectPrivate
    {
    public:
        NetworkInterfacePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
        void _k_destroyed(QObject *object);

        SolidExperimental::Network *findRegisteredNetwork(const QString &uni) const;


        mutable QMap<QString, Network*> networkMap;
        mutable Network invalidNetwork;
    };
}


SolidExperimental::NetworkInterface::NetworkInterface()
    : QObject(), d(new NetworkInterfacePrivate(this))
{
}

SolidExperimental::NetworkInterface::NetworkInterface( const QString &uni )
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    const NetworkInterface &device = NetworkManager::findNetworkInterface(uni);
    d->setBackendObject(device.d->backendObject());
}

SolidExperimental::NetworkInterface::NetworkInterface( QObject *backendObject )
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    d->setBackendObject(backendObject);
}

SolidExperimental::NetworkInterface::NetworkInterface( const NetworkInterface &device )
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
}

SolidExperimental::NetworkInterface::~NetworkInterface()
{
    foreach( QObject *network, d->networkMap )
    {
        delete network;
    }
}

SolidExperimental::NetworkInterface &SolidExperimental::NetworkInterface::operator=( const SolidExperimental::NetworkInterface & dev )
{
    d->setBackendObject(dev.d->backendObject());
    return *this;
}

bool SolidExperimental::NetworkInterface::isValid() const
{
    return d->backendObject()!=0;
}

QString SolidExperimental::NetworkInterface::uni() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), QString(), uni());
}

bool SolidExperimental::NetworkInterface::isActive() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), false, isActive());
}

SolidExperimental::NetworkInterface::Type SolidExperimental::NetworkInterface::type() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), UnknownType, type());
}
SolidExperimental::NetworkInterface::ConnectionState SolidExperimental::NetworkInterface::connectionState() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), UnknownState, connectionState());
}

int SolidExperimental::NetworkInterface::signalStrength() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), 0, signalStrength());
}

int SolidExperimental::NetworkInterface::designSpeed() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), 0, designSpeed());
}

bool SolidExperimental::NetworkInterface::isLinkUp() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), false, isLinkUp());
}

SolidExperimental::NetworkInterface::Capabilities SolidExperimental::NetworkInterface::capabilities() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface*, d->backendObject(), Capabilities(), capabilities());
}

SolidExperimental::Network * SolidExperimental::NetworkInterface::findNetwork( const QString & uni ) const
{
    if ( !isValid() ) return 0;

    Network *network = d->findRegisteredNetwork( uni );

    if ( network!=0 )
    {
        return network;
    }
    else
    {
        return &( d->invalidNetwork );
    }
}

SolidExperimental::NetworkList SolidExperimental::NetworkInterface::networks() const
{
    NetworkList list;
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>(d->backendObject());

    if ( device==0 ) return list;

    QStringList uniList = device->networks();

    foreach( const QString &uni, uniList )
    {
        Network *network = d->findRegisteredNetwork(uni);
        if ( network!=0 )
        {
            list.append( network );
        }
    }

    return list;
}

void SolidExperimental::NetworkInterfacePrivate::_k_destroyed(QObject *object)
{
    if ( object == backendObject() )
    {
        FrontendObjectPrivate::_k_destroyed(object);

        foreach( Network *network, networkMap )
        {
            delete network->d_func()->backendObject();
            delete network;
        }

        networkMap.clear();
    }
}

void SolidExperimental::NetworkInterfacePrivate::setBackendObject(QObject *object)
{
    foreach (QObject *network, networkMap) {
        delete network;
    }

    networkMap.clear();


    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(activeChanged(bool)),
                         parent(), SIGNAL(activeChanged(bool)));
        QObject::connect(object, SIGNAL(linkUpChanged(bool)),
                         parent(), SIGNAL(linkUpChanged(bool)));
        QObject::connect(object, SIGNAL(signalStrengthChanged(int)),
                         parent(), SIGNAL(signalStrengthChanged(int)));
        QObject::connect(object, SIGNAL(connectionStateChanged(int)),
                         parent(), SIGNAL(connectionStateChanged(int)));
        QObject::connect(object, SIGNAL(networkAppeared(const QString&)),
                         parent(), SIGNAL(networkAppeared(const QString&)));
        QObject::connect(object, SIGNAL(networkDisappeared(const QString&)),
                         parent(), SIGNAL(networkDisappeared(const QString&)));
    }
}

SolidExperimental::Network *SolidExperimental::NetworkInterfacePrivate::findRegisteredNetwork(const QString &uni) const
{
    Network *network = 0;

    if (networkMap.contains(uni)) {
        network = networkMap[uni];
    } else {
        Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>(backendObject());

        if ( device!=0 )
        {
            QObject *iface = device->createNetwork( uni );

            if ( qobject_cast<Ifaces::WirelessNetwork *>( iface )!=0 )
            {
                network = new WirelessNetwork( iface );
            }
            else if ( qobject_cast<Ifaces::Network *>( iface )!=0 )
            {
                network = new Network( iface );
            }

            if ( network != 0 )
            {
                networkMap[uni] = network;
            }
        }
    }

    return network;
}

#include "networkinterface.moc"
