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

int FakeWirelessNetwork::bitRate() const
{
    return mPropertyMap[ "bitrate" ].toInt();
}

int FakeWirelessNetwork::frequency() const
{
    return mPropertyMap[ "frequency" ].toInt();
}

Enums::WirelessNetwork::Capabilities FakeWirelessNetwork::capabilities() const
{
    QStringList capStrings = mPropertyMap[ "capabilities" ].toStringList();

    Capabilities caps = 0;
    if ( capStrings.contains( "wep" ) )
        caps |= Wep;
    if ( capStrings.contains( "wpa" ) )
        caps |= Wpa;
    if ( capStrings.contains( "wpa2" ) )
        caps |= Wpa2;
    if ( capStrings.contains( "psk" ) )
        caps |= Psk;
    if ( capStrings.contains( "ieee8021x" ) )
        caps |= Ieee8021x;
    if ( capStrings.contains( "wep40" ) )
        caps |= Wep40;
    if ( capStrings.contains( "wep104" ) )
        caps |= Wep104;
    if ( capStrings.contains( "wep192" ) )
        caps |= Wep192;
    if ( capStrings.contains( "wep256" ) )
        caps |= Wep256;
    if ( capStrings.contains( "wep_other" ) )
        caps |= WepOther;
    if ( capStrings.contains( "tkip" ) )
        caps |= Tkip;
    if ( capStrings.contains( "ccmp" ) )
        caps |= Ccmp;

    return caps;
}

QString FakeWirelessNetwork::essid() const
{
    return mPropertyMap[ "essid" ].toString();
}

Enums::WirelessNetwork::OperationMode FakeWirelessNetwork::mode() const
{
    QString modeName = mPropertyMap[ "mode" ].toString();

    if ( modeName == "adhoc" )
        return Adhoc;
    else if ( modeName == "managed" )
        return Managed;
    else if ( modeName == "master" )
        return Master;
    else if ( modeName == "repeater" )
        return Repeater;
    else
        return Unassociated;
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

Authentication *FakeWirelessNetwork::authentication() const
{
    return mAuthentication;
}

void FakeWirelessNetwork::setAuthentication( Authentication *authentication )
{
    mAuthentication = authentication;
}


#include "fakewirelessnetwork.moc"
