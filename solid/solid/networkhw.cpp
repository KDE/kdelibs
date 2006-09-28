/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "networkhw.h"

#include "soliddefs_p.h"
#include <solid/ifaces/networkhw.h>


Solid::NetworkHw::NetworkHw( QObject *backendObject )
    : Capability( backendObject )
{
}

Solid::NetworkHw::~NetworkHw()
{
}

QString Solid::NetworkHw::ifaceName() const
{
    return_SOLID_CALL( Ifaces::NetworkHw*, backendObject(), QString(), ifaceName() );
}

bool Solid::NetworkHw::isWireless() const
{
    return_SOLID_CALL( Ifaces::NetworkHw*, backendObject(), false, isWireless() );
}

QString Solid::NetworkHw::hwAddress() const
{
    return_SOLID_CALL( Ifaces::NetworkHw*, backendObject(), QString(), hwAddress() );
}

qulonglong Solid::NetworkHw::macAddress() const
{
    return_SOLID_CALL( Ifaces::NetworkHw*, backendObject(), 0, macAddress() );
}

#include "networkhw.moc"
