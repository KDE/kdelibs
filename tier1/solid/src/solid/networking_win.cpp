/*
    Copyright 2013      Patrick von Reth <vonreth@kde.org>
    Copyright 2006-2007 Will Stephenson <wstephenson@kde.org>
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

//#include <KDebug>

#include "networking.h"
#include "networking_p.h"

#include "soliddefs_p.h"
#include "org_kde_solid_networking_client.h"



Q_GLOBAL_STATIC(Solid::NetworkingPrivate, globalNetworkManager)

Solid::NetworkingPrivate::NetworkingPrivate()
    : netStatus(Solid::Networking::Unknown),
      connectPolicy(Solid::Networking::Managed),
      disconnectPolicy(Solid::Networking::Managed),
      m_manager(new QNetworkConfigurationManager())
{
    initialize();
}

Solid::NetworkingPrivate::~NetworkingPrivate()
{
    m_manager->deleteLater();
}

Solid::Networking::Notifier::Notifier()
{
}

void Solid::NetworkingPrivate::initialize()
{

    netStatus = m_manager->isOnline()?Solid::Networking::Connected:Solid::Networking::Unconnected;

    connect(m_manager,SIGNAL(onlineStateChanged(bool)),this,SLOT(serviceStatusChanged(bool)));
}

uint Solid::NetworkingPrivate::status() const
{
    return netStatus;
}

/*=========================================================================*/

Solid::Networking::Status Solid::Networking::status()
{
    return static_cast<Solid::Networking::Status>( globalNetworkManager->status() );
}

Solid::Networking::Notifier *Solid::Networking::notifier()
{
    return globalNetworkManager;
}

void Solid::NetworkingPrivate::serviceStatusChanged( bool status )
{
//    kDebug( 921 ) ;
    netStatus = status?Solid::Networking::Connected:Solid::Networking::Unconnected;
    switch ( netStatus ) {
      case Solid::Networking::Unknown:
        break;
      case Solid::Networking::Unconnected:
        emit globalNetworkManager->shouldDisconnect();
        break;
      case Solid::Networking::Connected:
        emit globalNetworkManager->shouldConnect();
        break;
      default:
        break;
//        kDebug( 921 ) <<  "Unrecognised status code!";
    }
    emit globalNetworkManager->statusChanged( netStatus );
}

Solid::Networking::ManagementPolicy Solid::Networking::connectPolicy()
{
    return globalNetworkManager->connectPolicy;
}

void Solid::Networking::setConnectPolicy( Solid::Networking::ManagementPolicy policy )
{
    globalNetworkManager->connectPolicy = policy;
}

Solid::Networking::ManagementPolicy Solid::Networking::disconnectPolicy()
{
    return globalNetworkManager->disconnectPolicy;
}

void Solid::Networking::setDisconnectPolicy( Solid::Networking::ManagementPolicy policy )
{
    globalNetworkManager->disconnectPolicy = policy;
}

