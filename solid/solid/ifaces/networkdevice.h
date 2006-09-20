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

#ifndef SOLID_IFACES_NETWORKDEVICE_H
#define SOLID_IFACES_NETWORKDEVICE_H

#include <QList>
#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>
//#include <solid/ifaces/networkmanager.h>

#include <QObject>

namespace Solid
{
namespace Ifaces
{
    class Network;
    /**
     * Represents a network device as seen by the networking subsystem.
     * For non network specific hardware details,
     * @see Solid::Ifaces::NetworkIface
     */
     // TODO talk to Ervin about how to cleanly combine this with NetworkIface, perhaps a union class elsewhere
    class KDE_EXPORT NetworkDevice : public QObject, public Enums::NetworkDevice
    {
        Q_OBJECT
    public:
        NetworkDevice( QObject *parent = 0 );
        virtual ~NetworkDevice();

        virtual bool isActive() = 0;

        virtual Type type() = 0;

        virtual ConnectionState connectionState() = 0;

        virtual int signalStrength() = 0;

        virtual int speed() = 0;

        virtual bool isLinkUp() = 0;

        virtual Capabilities capabilities() = 0;

        /**
         * Get the Network object corresponding to the given UDI
         */
        virtual Network * findNetwork( const QString & udi )= 0;

        /**
         * Access the networks available via this network devices
         * For wired network devices, this will probably be a single network,
         * but with wireless, multiple networks may be accessible.
         * @return A list of network UDIs.
         */
        virtual QStringList networks() = 0;
    signals:
        void activeChanged( bool );
        void linkUpChanged( bool );
        void signalStrengthChanged( int );
        void connectionStateChanged( int /*NetworkStatus::ConnectionState*/ );
    };

    typedef QStringList NetworkDeviceList;
} //Ifaces
} //Solid

#endif
