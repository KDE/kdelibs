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

#include "fakenetwork.h"

QList<QHostAddress> FakeNetwork::stringlistToKIpAddress(  const QStringList & addrStringList ) const
{
    QList<QHostAddress> ipv4Addrs;
    foreach ( const QString &addrString, addrStringList )
    {
        QHostAddress addr(  addrString );
        ipv4Addrs.append(  addr );
    }
    return ipv4Addrs;
}

QList<QNetworkAddressEntry> FakeNetwork::stringlistsToQNetworkAddressEntries( const QStringList & addrStringList, const QStringList & subnets, const QStringList & bcasts ) const
{
    if ( !( addrStringList.count() == subnets.count() == bcasts.count() ) )
        return QList<QNetworkAddressEntry>();

    QList<QNetworkAddressEntry> entries;
    for ( int i = 0; i< addrStringList.count(); ++i )
    {
        QNetworkAddressEntry addr;
        addr.setIp( QHostAddress( addrStringList[ i ] ) );
        addr.setNetmask( QHostAddress( subnets[ i ] ) );
        addr.setBroadcast( QHostAddress( bcasts[ i ] ) );
        entries.append( addr );
    }
    return entries;

}

FakeNetwork::FakeNetwork( const QString & uni, const QMap<QString, QVariant> & propertyMap,
                          QObject * parent )
: QObject( parent ), mPropertyMap( propertyMap )
{
    mPropertyMap.insert( "uni", QVariant( uni ) );
}

FakeNetwork::~FakeNetwork()
{

}

QList<QNetworkAddressEntry> FakeNetwork::addressEntries() const
{
    return stringlistsToQNetworkAddressEntries( mPropertyMap[ "addresses" ].toStringList(),
                                   mPropertyMap[ "subnet" ].toStringList(),
                                   mPropertyMap[ "broadcast" ].toStringList()
                                 );
}

QString FakeNetwork::route() const
{
    return mPropertyMap[ "route" ].toString();
}

QList<QHostAddress> FakeNetwork::dnsServers() const
{
        return stringlistToKIpAddress( mPropertyMap[ "dns" ].toString().simplified().split( ',' ) );
}

void FakeNetwork::setActivated( bool active )
{
    mPropertyMap.insert( "active", QVariant( active ) );
}

bool FakeNetwork::isActive() const
{
    return mPropertyMap[ "active" ].toBool();
}

QString FakeNetwork::uni() const
{
    return mPropertyMap[ "uni" ].toString();
}

#include "fakenetwork.moc"
