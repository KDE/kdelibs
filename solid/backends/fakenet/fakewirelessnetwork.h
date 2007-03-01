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

#ifndef FAKE_WIRELESSNETWORK_H
#define FAKE_WIRELESSNETWORK_H

#include <QObject>
#include <QString>
#include <QVariant>

#include <kdemacros.h>

#include <solid/ifaces/wirelessnetwork.h>
#include "fakenetwork.h"

using namespace Solid::Ifaces;
/**
 * Fakes a wireless network.
 * TODO: Add some kind of dynamic behaviour
 */
class KDE_EXPORT FakeWirelessNetwork : public FakeNetwork, virtual public Solid::Ifaces::WirelessNetwork
{
Q_OBJECT
Q_INTERFACES( Solid::Ifaces::WirelessNetwork )

public:
    FakeWirelessNetwork( const QString & uni, const QMap<QString, QVariant> & propertyMap,
                 QObject * parent = 0 );
    virtual ~FakeWirelessNetwork();

    // PHY stuff
    int signalStrength() const;

    int bitrate() const;

    int frequency() const;

    Solid::WirelessNetwork::Capabilities capabilities() const;

    // Service Set stuff
    QString essid() const;

    Solid::WirelessNetwork::OperationMode mode() const;

    bool isAssociated() const;

    bool isEncrypted() const;

    bool isHidden() const;

    /**
     * List of access points or ad hoc network nodes making up the
     * network that are currently visible to the card.
     */
    MacAddressList bssList() const;

    Authentication *authentication() const;
    /**
     * set the authentication currently in use on this network
     */
    void setAuthentication( Authentication * );
Q_SIGNALS:
    void signalStrengthChanged( int );
    void bitrateChanged( int );
    void associationChanged( bool );
    /**
        * Emitted when the network requires authentication data in order to be able to connect.
        * Respond to this by calling setAuthentication.
        */
    void authenticationNeeded();
private:
    Authentication * mAuthentication;
};

#endif
