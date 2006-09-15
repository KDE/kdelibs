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

#include "network.h"

namespace Solid
{
namespace Net
{
    class Network::Private
    {
        public:
            Ifaces::Network * iface;
    };
}
}

Solid::Net::Network::Network( Ifaces::Network * iface, QObject * parent ) : QObject( parent ), d( new Private )
{
    d->iface = iface;

    connect( d->iface, SIGNAL( ipDetailsChanged() ), this, SIGNAL( ipDetailsChanged() ) );
}

Solid::Net::Network::~Network()
{
    delete d;
}

QString Solid::Net::Network::ipV4Address()
{
    return d->iface->ipV4Address();
}

QString Solid::Net::Network::subnetMask()
{
    return d->iface->subnetMask();

}

QString Solid::Net::Network::broadcastAddress()
{
    return d->iface->broadcastAddress();
}

QString Solid::Net::Network::route()
{
    return d->iface->route();
}

QString Solid::Net::Network::primaryDNS()
{
    return d->iface->primaryDNS();
}

QString Solid::Net::Network::secondaryDNS()
{
    return d->iface->secondaryDNS();
}

#include "network.moc"
