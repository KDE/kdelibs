/*
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
#ifndef SOLID_NETWORKING_P_H
#define SOLID_NETWORKING_P_H

#include <QtCore/QMap>

#include "networking.h"

class OrgKdeSolidNetworkingClientInterface;
class QAbstractSocket;
class QTimer;


namespace Solid
{
    class ManagedSocketContainer;

    class NetworkingPrivate : public Networking::Notifier
    {
    Q_OBJECT
    Q_PROPERTY( uint Status  READ status )
    Q_CLASSINFO( "D-Bus Interface", "org.kde.Solid.Networking.Client" )
    public:
        NetworkingPrivate();
        ~NetworkingPrivate();
        void shouldConnect() { Networking::Notifier::shouldConnect(); }
        void shouldDisconnect() { Networking::Notifier::shouldDisconnect(); }
        Networking::Status netStatus;
        Networking::ManagementPolicy connectPolicy;
        Networking::ManagementPolicy disconnectPolicy;
    public Q_SLOTS:
        uint status() const;
        /**
         * Called on DBus signal from the network status service
         */
        void serviceStatusChanged( uint status );
        /**
         * Detects when kded restarts, and sets status to NoNetworks so that apps
         * may proceed
         */
        void serviceOwnerChanged( const QString &, const QString &, const QString & );
    private:
        void initialize();
        OrgKdeSolidNetworkingClientInterface * iface;
    };
} // namespace Solid
#endif
