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

#ifndef FAKE_NETWORK_INTERFACE_H
#define FAKE_NETWORK_INTERFACE_H


#include <QVariant>

#include <kdemacros.h>

#include <solid/ifaces/networkinterface.h>

using namespace Solid::Ifaces;

class FakeNetwork;

class KDE_EXPORT FakeNetworkInterface : public Solid::Ifaces::NetworkInterface
{
    friend class FakeNetworkManager;
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::NetworkInterface )
public:
        FakeNetworkInterface( const QMap<QString, QVariant> & propertyMap,
                          QObject *parent = 0 );
        virtual ~FakeNetworkInterface();
        QString uni() const;
        bool isActive() const;
        Solid::NetworkInterface::Type type() const;
        Solid::NetworkInterface::ConnectionState connectionState() const;
        int signalStrength() const;
        int designSpeed() const;
        bool isLinkUp() const;
        Solid::NetworkInterface::Capabilities capabilities() const;
        QObject * createNetwork( const QString & uni );
        QStringList networks() const;
    Q_SIGNALS:
        void activeChanged( bool );
        void linkUpChanged( bool );
        void signalStrengthChanged( int );
        void connectionStateChanged( int /*NetworkStatus::ConnectionState*/ );
        void networkAppeared( const QString & uni );
        void networkDisappeared( const QString & uni );

    protected:
        /* These methods are operations that are carried out by the manager
           In a real backend they would be events coming up from the network layer */
        void setActive( bool active );
        void injectNetwork( const QString & uni, FakeNetwork * net );
        QMap<QString, FakeNetwork *> mNetworks;
        QMap<QString, QVariant> mPropertyMap;
};

#endif
