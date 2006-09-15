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

#include <QObject>

#include <solid/ifaces/enums.h>

namespace Solid
{
    namespace Ifaces
    {
        class NetworkDevice;
    }

    class NetworkDevice : public QObject, public Ifaces::Enums::NetworkDevice
    {
        Q_OBJECT
    public:
        NetworkDevice( Ifaces::NetworkDevice *, QObject * parent );
        ~NetworkDevice();

        bool isActive();

        Type type();

        ConnectionState connectionState();

        int signalStrength();

        int speed();

        bool isLinkUp();

        Capabilities capabilities();

    signals:
        void activeChanged( bool );
        void linkUpChanged( bool );
        void signalStrengthChanged( int );
        void connectionStateChanged( int /* ConnectionState */ );

    private:
        class Private;
        Private * d;
};

} //Solid

#endif
