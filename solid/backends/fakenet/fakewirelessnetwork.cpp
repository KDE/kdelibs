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

#include "fakewirelessnetwork.h"

FakeWirelessNetwork::FakeWirelessNetwork( const QString & uni,
                                          const QMap<QString, QVariant> & propertyMap,
                                          QObject * parent )
: FakeNetwork( uni, propertyMap, parent )
{
}

FakeWirelessNetwork::~FakeWirelessNetwork()
{

}

// PHY stuff
int FakeWirelessNetwork::signalStrength() const
{
    return mPropertyMap[ "signalstrength" ].toInt();
}

int FakeWirelessNetwork::bitrate() const
{
    return mPropertyMap[ "bitrate" ].toInt();
}

double FakeWirelessNetwork::frequency() const
{
    return mPropertyMap[ "frequency" ].toInt();
}

SolidExperimental::WirelessNetwork::Capabilities FakeWirelessNetwork::capabilities() const
{
    QStringList capStrings = mPropertyMap[ "capabilities" ].toStringList();

    SolidExperimental::WirelessNetwork::Capabilities caps = 0;
    if ( capStrings.contains( "wep" ) )
        caps |= SolidExperimental::WirelessNetwork::Wep;
    if ( capStrings.contains( "wpa" ) )
        caps |= SolidExperimental::WirelessNetwork::Wpa;
    if ( capStrings.contains( "wpa2" ) )
        caps |= SolidExperimental::WirelessNetwork::Wpa2;
    if ( capStrings.contains( "psk" ) )
        caps |= SolidExperimental::WirelessNetwork::Psk;
    if ( capStrings.contains( "ieee8021x" ) )
        caps |= SolidExperimental::WirelessNetwork::Ieee8021x;
    if ( capStrings.contains( "wep40" ) )
        caps |= SolidExperimental::WirelessNetwork::Wep40;
    if ( capStrings.contains( "wep104" ) )
        caps |= SolidExperimental::WirelessNetwork::Wep104;
    if ( capStrings.contains( "wep192" ) )
        caps |= SolidExperimental::WirelessNetwork::Wep192;
    if ( capStrings.contains( "wep256" ) )
        caps |= SolidExperimental::WirelessNetwork::Wep256;
    if ( capStrings.contains( "wep_other" ) )
        caps |= SolidExperimental::WirelessNetwork::WepOther;
    if ( capStrings.contains( "tkip" ) )
        caps |= SolidExperimental::WirelessNetwork::Tkip;
    if ( capStrings.contains( "ccmp" ) )
        caps |= SolidExperimental::WirelessNetwork::Ccmp;

    return caps;
}

QString FakeWirelessNetwork::essid() const
{
    return mPropertyMap[ "essid" ].toString();
}

SolidExperimental::WirelessNetwork::OperationMode FakeWirelessNetwork::mode() const
{
    QString modeName = mPropertyMap[ "mode" ].toString();

    if ( modeName == "adhoc" )
        return SolidExperimental::WirelessNetwork::Adhoc;
    else if ( modeName == "managed" )
        return SolidExperimental::WirelessNetwork::Managed;
    else if ( modeName == "master" )
        return SolidExperimental::WirelessNetwork::Master;
    else if ( modeName == "repeater" )
        return SolidExperimental::WirelessNetwork::Repeater;
    else
        return SolidExperimental::WirelessNetwork::Unassociated;
}

bool FakeWirelessNetwork::isAssociated() const
{
    return mPropertyMap[ "signalstrength" ].toBool();
}

bool FakeWirelessNetwork::isEncrypted() const
{
    return mPropertyMap[ "encrypted" ].toBool();
}

bool FakeWirelessNetwork::isHidden() const
{
    return mPropertyMap[ "hidden" ].toBool();
}

MacAddressList FakeWirelessNetwork::bssList() const
{
    return mPropertyMap[ "bsslist" ].toStringList();
}

SolidExperimental::Authentication *FakeWirelessNetwork::authentication() const
{
    return mAuthentication;
}

void FakeWirelessNetwork::setAuthentication( SolidExperimental::Authentication *authentication )
{
    mAuthentication = authentication;
}


#include "fakewirelessnetwork.moc"
