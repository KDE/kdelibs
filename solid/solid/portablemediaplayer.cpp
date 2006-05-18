/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#include "portablemediaplayer.h"

#include <solid/ifaces/portablemediaplayer.h>

namespace Solid
{
    class PortableMediaPlayer::Private
    {
    public:
        Private() : iface( 0 ) {};

        Ifaces::PortableMediaPlayer *iface;
    };
}

Solid::PortableMediaPlayer::PortableMediaPlayer( Ifaces::PortableMediaPlayer *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

Solid::PortableMediaPlayer::~PortableMediaPlayer()
{
    delete d;
}

Solid::PortableMediaPlayer::AccessType Solid::PortableMediaPlayer::accessMethod() const
{
    return d->iface->accessMethod();
}

QStringList Solid::PortableMediaPlayer::outputFormats() const
{
    return d->iface->outputFormats();
}

QStringList Solid::PortableMediaPlayer::inputFormats() const
{
    return d->iface->inputFormats();
}

QStringList Solid::PortableMediaPlayer::playlistFormats() const
{
    return d->iface->playlistFormats();
}

#include "portablemediaplayer.moc"
