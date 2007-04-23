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

#include <QtNetwork/QAbstractSocket>
#include <QtCore/QTimer>

#include <kglobal.h>

#include "soliddefs_p.h"
#include "networking_p.h"
#include "networking.h"
#include "org_kde_solid_networking.h"

K_GLOBAL_STATIC(Solid::NetworkingPrivate, globalNetworkManager)

Solid::NetworkingPrivate::NetworkingPrivate() : iface( 
        new OrgKdeSolidNetworkingInterface( "org.kde.Solid.Networking",
            "/status",
            QDBusConnection::sessionBus(),
            this ) )
{
    connect( iface, SIGNAL( statusChanged( uint ) ), globalNetworkManager, SIGNAL( statusChanged( Networking::Status ) ) );
}

Solid::NetworkingPrivate::~NetworkingPrivate()
{
}

uint Solid::NetworkingPrivate::requestConnection(/* QObject * receiver, const char * member*/ )
{
    // register the slot and member to call when the connection attempt completes
    return iface->requestConnection();
}

void Solid::NetworkingPrivate::releaseConnection()
{
    iface->releaseConnection();
}

uint Solid::NetworkingPrivate::status() const
{
    return iface->status();
}

Solid::Networking::Result Solid::Networking::requestConnection( QObject * receiver, const char * member )
{
    return static_cast<Solid::Networking::Result>( globalNetworkManager->requestConnection() );
}

void Solid::Networking::releaseConnection()
{
    globalNetworkManager->releaseConnection();
}

Solid::Networking::Status Solid::Networking::status()
{
    return static_cast<Solid::Networking::Status>( globalNetworkManager->status() );
}

Solid::Networking::Notifier *Solid::Networking::notifier()
{
    return globalNetworkManager;
}

/* ************************************************************************ *
 * ManagedSocketContainer manages a single socket's state in accordance     *
 * with what the network management backend does.  If it detects that the   *
 * socket tries to connect and fails, it initiates a Networking connection, *
 * waits for the connection to come up, then connects the socket to its     *
 * original host.                                                           *
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
 *  ( TBD :) )
 */

Solid::ManagedSocketContainer::ManagedSocketContainer( QAbstractSocket * socket, int autoDisconnectTimeout ) : mSocket( socket ), mAutoDisconnectTimer( 0 )
{
    if ( autoDisconnectTimeout )
    {
        mAutoDisconnectTimer = new QTimer( this );
        mAutoDisconnectTimer->setInterval( autoDisconnectTimeout );
        connect( mAutoDisconnectTimer, SIGNAL( timeout() ), SLOT( autoDisconnect() ) );
    }
    connect( globalNetworkManager, SIGNAL( statusChanged( uint ) ), this, SLOT( statusChanged( Networking::Status ) ) );
}

void Solid::ManagedSocketContainer::statusChanged( Networking::Status status )
{
    // state can be offline but awaiting connection, in which case on a Connected we call
    // connectToHost on the socket
    // OR can be connected.  in which case if the status goes offline we start the disconnect timer 
    if ( mSocket->state() != QAbstractSocket::UnconnectedState )
    {
        if ( mAutoDisconnectTimer )
            mAutoDisconnectTimer->start();
    }
}

void Solid::ManagedSocketContainer::autoDisconnect()
{
    if ( mSocket->state() != QAbstractSocket::UnconnectedState )
    {
        if ( mAutoDisconnectTimer )
            mSocket->disconnectFromHost();
    }
}

void Solid::ManagedSocketContainer::socketStateChanged( QAbstractSocket::SocketState state )
{
#warning unimplemented Solid::ManagedSocketContainer::socketStateChanged()
}

void Solid::ManagedSocketContainer::socketError( QAbstractSocket::SocketError error )
{
#warning unimplemented Solid::ManagedSocketContainer::socketError()
}


#include "networking_p.moc"
#include "networking.moc"
