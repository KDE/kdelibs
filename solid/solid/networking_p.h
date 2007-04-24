/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Will Stephenson <wstephenson@kde.org>
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
#ifndef SOLID_NETWORKING_P_H
#define SOLID_NETWORKING_P_H

#include <QtCore/QMap>

#include "networking.h"

class OrgKdeSolidNetworkingInterface;
class QAbstractSocket;
class QTimer;


namespace Solid
{
    class ManagedSocketContainer;

    class NetworkingPrivate : public Networking::Notifier
    {
    Q_OBJECT
    Q_PROPERTY( uint Status  READ status )
    Q_CLASSINFO( "D-Bus Interface", "org.kde.Solid.Networking" )
    public:
        NetworkingPrivate();
        ~NetworkingPrivate();
        Networking::Result beginManagingSocket( QAbstractSocket * socket, int autoDisconnectTimeout );
        void stopManagingSocket( QAbstractSocket * socket );
    public Q_SLOTS:
        uint requestConnection( QObject* receiver = 0, const char * member = 0 ); /*Result*/
        void releaseConnection();
        uint status() const;
    Q_SIGNALS:
        /**
         * Indicates the result of the previous requestConnection() call
         */
        void connectionResult( Networking::Status );
    private Q_SLOTS:
        void networkStatusChanged( Networking::Status );
    private:
        OrgKdeSolidNetworkingInterface * iface;
        QMap<QAbstractSocket *, Solid::ManagedSocketContainer*> mManagedSockets;
        bool mNotifyConnectionResult;
    };

    /**
     * ManagedSocketContainer manages a single socket's state in accordance
     * with what the network management backend does.  If it detects that the
     * socket tries to connect and fails, it initiates a Networking connection,
     * waits for the connection to come up, then connects the socket to its
     * original host.
     *
     * STATE DIAGRAM
     *
     * Disconnected
     *      |
     * (sock connecting)
     *      |
     *      V
     * Connecting
     *      |
     *  ( TBD - currently exists on paper )
     */
    class ManagedSocketContainer : public QObject
    {
    Q_OBJECT
    public:
        enum State { SocketUnconnected, SocketConnecting, AwaitingNetworkConnection, SocketConnected, DisconnectWait };
        ManagedSocketContainer( QAbstractSocket * socket, int autoDisconnectTimeout = -1 );
    private Q_SLOTS:
        // update our state and try to connect the socket if waiting for a connection
        void networkStatusChanged( Networking::Status );
        // the socket errored, go to AwaitingConnection if offline
        void socketError( QAbstractSocket::SocketError );
        // update our state and connect the socket if required
        void socketStateChanged( QAbstractSocket::SocketState );
        // perform an automatic disconnect
        void autoDisconnect();
        void socketDestroyed();
    private:
        void performConnectToHost();
        QAbstractSocket * mSocket;
        QTimer * mAutoDisconnectTimer;
        uint mAutoDisconnectTimeout;
        State mState;
        QString mPeerName;
        quint16 mPeerPort;
        QIODevice::OpenMode mSocketOpenMode;
    };
}
#endif
