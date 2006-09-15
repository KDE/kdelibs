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

#ifndef SOLID_IFACES_NETWORK_H
#define SOLID_IFACES_NETWORK_H

#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>

#include <QObject>

namespace Solid
{
namespace Ifaces
{
    /**
     * Represents a generic IP network which we may be connected to
     * TODO what about QNetworkAddressEntry
     */
    class KDE_EXPORT Network : public QObject, public Enums::Network
    {
        Q_OBJECT
    public:
        Network( QObject *parent = 0 );
        virtual ~Network();
        // TODO ask Thiago whether to use QHostAddress or KIPAddress for these
        virtual QString ipV4Address() = 0;
        // virtual QString ipV6Address() = 0;

        virtual QString subnetMask() = 0;
        virtual QString broadcastAddress() = 0;
        // wtf does NM use this for?
        virtual QString route() = 0;

        virtual QString primaryDNS() = 0;
        virtual QString secondaryDNS() = 0;

    signals:
        void ipDetailsChanged();
    };
} //Ifaces
} //Solid

#endif
