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

#include "acadapter.h"

#include <solid/ifaces/acadapter.h>

namespace Solid
{
    class AcAdapter::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::AcAdapter *iface;
    };
}

Solid::AcAdapter::AcAdapter( Ifaces::AcAdapter *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( plugStateChanged( bool ) ),
             this, SIGNAL( plugStateChanged( bool ) ) );
}

Solid::AcAdapter::~AcAdapter()
{
    delete d;
}

bool Solid::AcAdapter::isPlugged() const
{
    return d->iface->isPlugged();
}

#include "acadapter.moc"
