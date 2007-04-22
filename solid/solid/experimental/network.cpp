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

#include "network.h"
#include "network_p.h"

#include "../soliddefs_p.h"
#include <solid/experimental/ifaces/network.h>

SolidExperimental::Network::Network( QObject *backendObject )
    : QObject(), d_ptr(new NetworkPrivate(this))
{
    Q_D(Network);
    d->setBackendObject(backendObject);
}

SolidExperimental::Network::Network( const Network &network )
    : QObject(), d_ptr(new NetworkPrivate(this))
{
    Q_D(Network);
    d->setBackendObject(network.d_ptr->backendObject());
}

SolidExperimental::Network::Network(NetworkPrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_D(Network);
    d->setBackendObject(backendObject);
}

SolidExperimental::Network::Network(NetworkPrivate &dd, const Network &network)
    : d_ptr(&dd)
{
    Q_D(Network);
    d->setBackendObject(network.d_ptr->backendObject());
}

SolidExperimental::Network::~Network()
{

}

bool SolidExperimental::Network::isValid() const
{
    Q_D(const Network);
    return d->backendObject()!=0;
}

QList<QNetworkAddressEntry> SolidExperimental::Network::addressEntries() const
{
    Q_D(const Network);
    return_SOLID_CALL(Ifaces::Network*, d->backendObject(), QList<QNetworkAddressEntry>(), addressEntries());
}

QString SolidExperimental::Network::route() const
{
    Q_D(const Network);
    return_SOLID_CALL(Ifaces::Network*, d->backendObject(), QString(), route());
}

QList<QHostAddress> SolidExperimental::Network::dnsServers() const
{
    Q_D(const Network);
    return_SOLID_CALL(Ifaces::Network*, d->backendObject(), QList<QHostAddress>(), dnsServers());
}

void SolidExperimental::Network::setActivated( bool active )
{
    Q_D(const Network);
    SOLID_CALL(Ifaces::Network*, d->backendObject(), setActivated(active));
}

bool SolidExperimental::Network::isActive() const
{
    Q_D(const Network);
    return_SOLID_CALL(Ifaces::Network*, d->backendObject(), false, isActive());
}

QString SolidExperimental::Network::uni() const
{
    Q_D(const Network);
    return_SOLID_CALL(Ifaces::Network*, d->backendObject(), QString(), uni());
}

void SolidExperimental::NetworkPrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(ipDetailsChanged()),
                         parent(), SIGNAL(ipDetailsChanged()));
        QObject::connect(object, SIGNAL(activationStateChanged(bool)),
                         parent(), SIGNAL(activationStateChanged(bool)));
    }
}

#include "network.moc"
