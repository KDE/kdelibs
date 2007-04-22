/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "ifaces/networkmanager.h"
#include "ifaces/networkinterface.h"

#include "../soliddefs_p.h"
#include "networkmanager_p.h"
#include "networkinterface.h"

#include "networkmanager.h"

#include <kglobal.h>

K_GLOBAL_STATIC(SolidExperimental::NetworkManagerPrivate, globalNetworkManager)

SolidExperimental::NetworkManagerPrivate::NetworkManagerPrivate()
{
    loadBackend("Network Management",
                "SolidNetworkManager",
                "SolidExperimental::Ifaces::NetworkManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(networkInterfaceAdded(const QString&)),
                this, SLOT(_k_networkInterfaceAdded(const QString&)));
        connect(managerBackend(), SIGNAL(networkInterfaceRemoved(const QString&)),
                this, SLOT(_k_networkInterfaceRemoved(const QString&)));
    }
}

SolidExperimental::NetworkManagerPrivate::~NetworkManagerPrivate()
{
    // Delete all the devices, they are now outdated
    typedef QPair<NetworkInterface*, Ifaces::NetworkInterface*> NetworkInterfaceIfacePair;

    // Delete all the devices, they are now outdated
    foreach (const NetworkInterfaceIfacePair &pair, m_networkInterfaceMap.values()) {
        delete pair.first;
        delete pair.second;
    }

    m_networkInterfaceMap.clear();
}

SolidExperimental::NetworkInterfaceList SolidExperimental::NetworkManagerPrivate::buildDeviceList(const QStringList &uniList)
{
    NetworkInterfaceList list;
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>(managerBackend());

    if ( backend == 0 ) return list;

    foreach( const QString &uni, uniList )
    {
        QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = findRegisteredNetworkInterface(uni);

        if ( pair.first!= 0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

SolidExperimental::NetworkInterfaceList SolidExperimental::NetworkManagerPrivate::networkInterfaces()
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>(managerBackend());

    if ( backend!= 0 )
    {
        return buildDeviceList( backend->networkInterfaces() );
    }
    else
    {
        return NetworkInterfaceList();
    }
}

SolidExperimental::NetworkInterfaceList SolidExperimental::NetworkManager::networkInterfaces()
{
    return globalNetworkManager->networkInterfaces();
}

bool SolidExperimental::NetworkManager::isNetworkingEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManager*, globalNetworkManager->managerBackend(), false, isNetworkingEnabled());
}

bool SolidExperimental::NetworkManager::isWirelessEnabled()
{
    return_SOLID_CALL(Ifaces::NetworkManager*, globalNetworkManager->managerBackend(), false, isWirelessEnabled());
}

void SolidExperimental::NetworkManager::setNetworkingEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManager*, globalNetworkManager->managerBackend(), setNetworkingEnabled(enabled));
}

void SolidExperimental::NetworkManager::setWirelessEnabled(bool enabled)
{
    SOLID_CALL(Ifaces::NetworkManager*, globalNetworkManager->managerBackend(), setWirelessEnabled(enabled));
}

void SolidExperimental::NetworkManager::notifyHiddenNetwork(const QString &networkName)
{
    SOLID_CALL(Ifaces::NetworkManager*, globalNetworkManager->managerBackend(), notifyHiddenNetwork(networkName));
}

const SolidExperimental::NetworkInterface &SolidExperimental::NetworkManagerPrivate::findNetworkInterface(const QString &uni)
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>(managerBackend());

    if ( backend == 0 ) return m_invalidDevice;

    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = findRegisteredNetworkInterface(uni);

    if ( pair.first != 0 )
    {
        return *pair.first;
    }
    else
    {
        return m_invalidDevice;
    }
}

const SolidExperimental::NetworkInterface &SolidExperimental::NetworkManager::findNetworkInterface(const QString &uni)
{
    return globalNetworkManager->findNetworkInterface(uni);
}

void SolidExperimental::NetworkManagerPrivate::_k_networkInterfaceAdded(const QString &uni)
{
    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = m_networkInterfaceMap.take(uni);

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceAdded(uni);
}

void SolidExperimental::NetworkManagerPrivate::_k_networkInterfaceRemoved(const QString &uni)
{
    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = m_networkInterfaceMap.take(uni);

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceRemoved(uni);
}

void SolidExperimental::NetworkManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>( object );

    if ( device!=0 )
    {
        QString uni = device->uni();
        QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = m_networkInterfaceMap.take(uni);
        delete pair.first;
    }
}

/***************************************************************************/

QPair<SolidExperimental::NetworkInterface*, SolidExperimental::Ifaces::NetworkInterface*>
SolidExperimental::NetworkManagerPrivate::findRegisteredNetworkInterface(const QString &uni)
{
    if (m_networkInterfaceMap.contains(uni)) {
        return m_networkInterfaceMap[uni];
    } else {
        Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>(managerBackend());
        Ifaces::NetworkInterface *iface = 0;

        if ( backend!=0 )
        {
            iface = qobject_cast<Ifaces::NetworkInterface*>( backend->createNetworkInterface( uni ) );
        }

        if ( iface!=0 )
        {
            NetworkInterface *device = new NetworkInterface( iface );
            QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair( device, iface );
            connect(iface, SIGNAL(destroyed(QObject*)),
                    this, SLOT(_k_destroyed(QObject*)));
            m_networkInterfaceMap[uni] = pair;
            return pair;
        }
        else
        {
            return QPair<NetworkInterface*, Ifaces::NetworkInterface*>( 0, 0 );
        }
    }
}

#include "networkmanager_p.moc"
#include "networkmanager.moc"
