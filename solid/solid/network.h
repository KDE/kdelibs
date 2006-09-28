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

#include <solid/frontendobject.h>
#include <solid/ifaces/enums.h>

#include <QStringList>

namespace Solid
{
    class NetworkDevice;

    class KDE_EXPORT Network : public FrontendObject, public Ifaces::Enums::Network
    {
        Q_OBJECT
    public:
        Network( QObject * );
        virtual ~Network();
        // TODO ask Thiago whether to use QHostAddress or KIPAddress for these
        QStringList ipV4Addresses() const;
        QStringList ipV6Addresses() const;

        QString subnetMask() const;
        QString broadcastAddress() const;
        // wtf does NM use this for?
        QString route() const;

        QString primaryDNS() const;
        QString secondaryDNS() const;
        QString uni() const;
        void setActivated( bool active );
        bool isActive() const;
    Q_SIGNALS:
        void ipDetailsChanged();

    private:
        friend class NetworkDevice;
    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<Network*> NetworkList;


} //Solid

#endif

