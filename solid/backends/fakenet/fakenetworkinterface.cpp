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

#include <QStringList>

#include "fakenetworkinterface.h"
#include "fakenetwork.h"

#include <kdebug.h>

FakeNetworkInterface::FakeNetworkInterface( const QMap<QString, QVariant> & propertyMap, QObject * parent )
: SolidExperimental::Ifaces::NetworkInterface( parent ), mPropertyMap( propertyMap )
{
}

FakeNetworkInterface::~FakeNetworkInterface()
{
}


QString FakeNetworkInterface::uni() const
{
    return mPropertyMap[ "uni" ].toString();
}

bool FakeNetworkInterface::isActive() const
{
    return mPropertyMap[ "active" ].toBool();
}

SolidExperimental::NetworkInterface::Type FakeNetworkInterface::type() const
{
    QString typeString = mPropertyMap[ "type" ].toString();

    if ( typeString == "ieee8023" )
        return SolidExperimental::NetworkInterface::Ieee8023;
    else if ( typeString == "ieee80211" )
        return SolidExperimental::NetworkInterface::Ieee80211;
    else
        return SolidExperimental::NetworkInterface::UnknownType;
}

SolidExperimental::NetworkInterface::ConnectionState FakeNetworkInterface::connectionState() const
{
    QString stateString = mPropertyMap[ "state" ].toString();

    if ( stateString == "prepare" )
        return SolidExperimental::NetworkInterface::Prepare;
    else if ( stateString == "configure" )
        return SolidExperimental::NetworkInterface::Configure;
    else if ( stateString == "needuserkey" )
        return SolidExperimental::NetworkInterface::NeedUserKey;
    else if ( stateString == "ipstart" )
        return SolidExperimental::NetworkInterface::IPStart;
    else if ( stateString == "ipget" )
        return SolidExperimental::NetworkInterface::IPGet;
    else if ( stateString == "ipcommit" )
        return SolidExperimental::NetworkInterface::IPCommit;
    else if ( stateString == "activated" )
        return SolidExperimental::NetworkInterface::Activated;
    else if ( stateString == "failed" )
        return SolidExperimental::NetworkInterface::Failed;
    else if ( stateString == "cancelled" )
        return SolidExperimental::NetworkInterface::Cancelled;
    else
        return SolidExperimental::NetworkInterface::UnknownState;
}

int FakeNetworkInterface::signalStrength() const
{
    return mPropertyMap[ "signalstrength" ].toInt();
}

int FakeNetworkInterface::designSpeed() const
{
    return mPropertyMap[ "speed" ].toInt();
}

bool FakeNetworkInterface::isLinkUp() const
{
    return mPropertyMap[ "linkup" ].toBool();
}

SolidExperimental::NetworkInterface::Capabilities FakeNetworkInterface::capabilities() const
{
    QStringList capStrings = mPropertyMap[ "capabilities" ].toString().simplified().split( ',' );

    SolidExperimental::NetworkInterface::Capabilities caps = 0;
    if ( capStrings.contains( "manageable" ) )
        caps |= SolidExperimental::NetworkInterface::IsManageable;
    if ( capStrings.contains( "carrierdetect" ) )
        caps |= SolidExperimental::NetworkInterface::SupportsCarrierDetect;
    if ( capStrings.contains( "wirelessscan" ) )
        caps |= SolidExperimental::NetworkInterface::SupportsWirelessScan;

    return caps;
}

QObject * FakeNetworkInterface::createNetwork( const QString & uni )
{
    if ( mNetworks.contains( uni ) )
    {
        kDebug() << k_funcinfo << "found " << uni << endl;
        return mNetworks[ uni ];
    }
    else
        kDebug() << k_funcinfo << "NOT found " << uni << endl;
        return 0;
}

QStringList FakeNetworkInterface::networks() const
{
    return mNetworks.keys();
}
void FakeNetworkInterface::setActive( bool active )
{
    mPropertyMap.insert( "active", QVariant( active ) );
}

void FakeNetworkInterface::injectNetwork( const QString & uni, FakeNetwork * net )
{
   mNetworks.insert( uni, net );
}

#include "fakenetworkinterface.moc"
