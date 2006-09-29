/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#include "wirelessnetwork.h"

#include "soliddefs_p.h"
#include <solid/ifaces/wirelessnetwork.h>

Solid::WirelessNetwork::WirelessNetwork( QObject *backendObject )
    : Network( backendObject )
{
    if ( backendObject )
    {
        connect( backendObject, SIGNAL( signalStrengthChanged( int ) ),
                 this, SIGNAL( signalStrengthChanged( int ) ) );
        connect( backendObject, SIGNAL( bitrateChanged( int ) ),
                 this, SIGNAL( bitrateChanged( int ) ) );
        connect( backendObject, SIGNAL( associationChanged( bool ) ),
                 this, SIGNAL( associationChanged( bool ) ) );
        connect( backendObject, SIGNAL( authenticationNeeded() ),
                 this, SIGNAL( authenticationNeeded() ) );
    }
}

Solid::WirelessNetwork::~WirelessNetwork()
{
}

bool Solid::WirelessNetwork::isSameAs( const WirelessNetwork & other) const
{
    if ( other.essid() == essid() )
    {
        QStringListIterator it( bssList() );
        const QStringList otherBSSList = other.bssList();
        while ( it.hasNext() )
        {
            QString bss = it.next();
            if ( other.bssList().contains( bss ) )
            {
                return true;
            }
        }
    }
    return false;
}

int Solid::WirelessNetwork::signalStrength() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), 0, signalStrength() );
}

int Solid::WirelessNetwork::bitRate() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), 0, bitRate() );
}

int Solid::WirelessNetwork::frequency() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), 0, frequency() );
}

Solid::MacAddressList Solid::WirelessNetwork::bssList() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), MacAddressList(), bssList() );
}

Solid::Authentication *Solid::WirelessNetwork::authentication() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), 0, authentication() );
}

Solid::WirelessNetwork::Capabilities Solid::WirelessNetwork::capabilities() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), Capabilities(), capabilities() );
}

QString Solid::WirelessNetwork::essid() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), QString(), essid() );
}

Solid::WirelessNetwork::OperationMode Solid::WirelessNetwork::mode() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), Unassociated, mode() );
}

bool Solid::WirelessNetwork::isAssociated() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), false, isAssociated() );
}

bool Solid::WirelessNetwork::isEncrypted() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), false, isEncrypted() );
}

bool Solid::WirelessNetwork::isHidden() const
{
    return_SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), false, isHidden() );
}

void Solid::WirelessNetwork::setAuthentication( Authentication *authentication )
{
    SOLID_CALL( Ifaces::WirelessNetwork*, backendObject(), setAuthentication( authentication ) );
}

#include "wirelessnetwork.moc"
