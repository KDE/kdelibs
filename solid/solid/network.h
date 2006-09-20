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

#ifndef SOLID_NETWORK_H
#define SOLID_NETWORK_H

#include <solid/ifaces/enums.h>

#include <QList>
#include <QObject>

namespace Solid
{
    namespace Ifaces
    {
        class Network;
    }

    class Network : public QObject, public Ifaces::Enums::Network
    {
        Q_OBJECT
    public:
        Network( Ifaces::Network * );
        virtual ~Network();
        // TODO ask Thiago whether to use QHostAddress or KIPAddress for these
        QString ipV4Address();
        // QString ipV6Address();

        QString subnetMask();
        QString broadcastAddress();
        // wtf does NM use this for?
        QString route();

        QString primaryDNS();
        QString secondaryDNS();
        QString uni();
        void setActivated( bool active );
    signals:
        void ipDetailsChanged();

    private:
        class Private;
        Private * d;

    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<Network*> NetworkList;


} //Solid

#endif

