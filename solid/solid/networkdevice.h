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

#ifndef SOLID_NETWORKDEVICE_H
#define SOLID_NETWORKDEVICE_H

#include <solid/frontendobject.h>
#include <solid/ifaces/enums.h>
#include <solid/network.h>

namespace Solid
{
    class KDE_EXPORT NetworkDevice : public FrontendObject, public Ifaces::Enums::NetworkDevice
    {
        Q_OBJECT
    public:
        /**
         * Invalid device
         */
        NetworkDevice();
        NetworkDevice( const QString &uni );
        NetworkDevice( QObject * );
        NetworkDevice( const NetworkDevice & );
        ~NetworkDevice();

        NetworkDevice &operator=( const NetworkDevice & );
        QString uni() const;
        bool isActive() const;

        Type type() const;

        ConnectionState connectionState() const;

        int signalStrength() const;

        int speed() const;

        bool isLinkUp() const;

        Capabilities capabilities() const;

        Network *findNetwork( const QString & uni ) const;
        /**
         * Access the networks available via this network devices
         * For wired network devices, this will probably be a single network,
         * but with wireless, multiple networks may be accessible.
         * @return A list of Network objects.
         */
        NetworkList networks() const;
    Q_SIGNALS:
        void activeChanged( bool );
        void linkUpChanged( bool );
        void signalStrengthChanged( int );
        void connectionStateChanged( int /* ConnectionState */ );

    protected Q_SLOTS:
        void slotDestroyed( QObject *object );

    private:
        void registerBackendObject( QObject *backendObject );
        void unregisterBackendObject();

        Network *findRegisteredNetwork( const QString &uni ) const;

        class Private;
        Private * d;
};

} //Solid

#endif
