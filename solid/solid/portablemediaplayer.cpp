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

#include <kdehw/ifaces/portablemediaplayer.h>

namespace KDEHW
{
    class PortableMediaPlayer::Private
    {
    public:
        Private() : iface( 0 ) {};

        Ifaces::PortableMediaPlayer *iface;
    };
}

KDEHW::PortableMediaPlayer::PortableMediaPlayer( Ifaces::PortableMediaPlayer *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::PortableMediaPlayer::~PortableMediaPlayer()
{
    delete d;
}

KDEHW::PortableMediaPlayer::AccessType KDEHW::PortableMediaPlayer::accessMethod() const
{
    return d->iface->accessMethod();
}

QStringList KDEHW::PortableMediaPlayer::outputFormats() const
{
    return d->iface->outputFormats();
}

QStringList KDEHW::PortableMediaPlayer::inputFormats() const
{
    return d->iface->inputFormats();
}

QStringList KDEHW::PortableMediaPlayer::playlistFormats() const
{
    return d->iface->playlistFormats();
}

#include "portablemediaplayer.moc"
