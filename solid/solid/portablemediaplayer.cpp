/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

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

Solid::PortableMediaPlayer::PortableMediaPlayer(QObject *backendObject)
    : DeviceInterface(*new PortableMediaPlayerPrivate(), backendObject)
{
}

Solid::PortableMediaPlayer::~PortableMediaPlayer()
{

}

QStringList Solid::PortableMediaPlayer::supportedProtocols() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QStringList(), supportedProtocols());
}

QStringList Solid::PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QStringList(), supportedDrivers(protocol));
}

QVariant Solid::PortableMediaPlayer::driverHandle(const QString &driver) const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QVariant(), driverHandle(driver));
}

#include "portablemediaplayer.moc"
