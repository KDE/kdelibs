/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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
#include "portablemediaplayer_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/portablemediaplayer.h>

Solid::PortableMediaPlayer::PortableMediaPlayer( QObject *backendObject )
    : DeviceInterface(*new PortableMediaPlayerPrivate(), backendObject)
{
}

Solid::PortableMediaPlayer::~PortableMediaPlayer()
{

}

Solid::PortableMediaPlayer::AccessType Solid::PortableMediaPlayer::accessMethod() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer*, d->backendObject(), Proprietary, accessMethod());
}

QStringList Solid::PortableMediaPlayer::outputFormats() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer*, d->backendObject(), QStringList(), outputFormats());
}

QStringList Solid::PortableMediaPlayer::inputFormats() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer*, d->backendObject(), QStringList(), inputFormats());
}

QStringList Solid::PortableMediaPlayer::playlistFormats() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer*, d->backendObject(), QStringList(), playlistFormats());
}

#include "portablemediaplayer.moc"
