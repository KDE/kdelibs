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

#include "volume.h"

#include <kdehw/ifaces/volume.h>

namespace KDEHW
{
    class Volume::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Volume *iface;
    };
}

KDEHW::Volume::Volume( Ifaces::Volume *iface, QObject *parent )
    : Block( iface, parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( mountStateChanged( bool ) ),
             this, SLOT( slotMountStateChanged( bool ) ) );
}

KDEHW::Volume::~Volume()
{
    delete d;
}

bool KDEHW::Volume::isIgnored() const
{
    return d->iface->isIgnored();
}

bool KDEHW::Volume::isMounted() const
{
    return d->iface->isMounted();
}

QString KDEHW::Volume::mountPoint() const
{
    return d->iface->mountPoint();
}

KDEHW::Volume::UsageType KDEHW::Volume::usage() const
{
    return d->iface->usage();
}

QString KDEHW::Volume::fsType() const
{
    return d->iface->fsType();
}

QString KDEHW::Volume::label() const
{
    return d->iface->label();
}

QString KDEHW::Volume::uuid() const
{
    return d->iface->uuid();
}

qulonglong KDEHW::Volume::size() const
{
    return d->iface->size();
}

KIO::Job *KDEHW::Volume::mount( bool showProgressInfo )
{
    return d->iface->mount( showProgressInfo );
}

KIO::Job *KDEHW::Volume::unmount( bool showProgressInfo )
{
    return d->iface->unmount( showProgressInfo );
}

KIO::Job *KDEHW::Volume::eject( bool showProgressInfo )
{
    return d->iface->eject( showProgressInfo );
}

void KDEHW::Volume::slotMountStateChanged( bool newState )
{
    emit mountStateChanged( newState );
}

#include "volume.moc"
