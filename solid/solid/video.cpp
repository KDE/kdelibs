/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007      Will Stephenson <wstephenson@kde.org>

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

#include "video.h"
#include "video_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/video.h>

Solid::Video::Video(QObject *backendObject)
    : DeviceInterface(*new VideoPrivate(), backendObject)
{
}

Solid::Video::~Video()
{

}

QStringList Solid::Video::supportedProtocols() const
{
    Q_D(const Video);
    return_SOLID_CALL(Ifaces::Video *, d->backendObject(), QStringList(), supportedProtocols());
}

QStringList Solid::Video::supportedDrivers(QString protocol) const
{
    Q_D(const Video);
    return_SOLID_CALL(Ifaces::Video *, d->backendObject(), QStringList(), supportedDrivers(protocol));
}

QVariant Solid::Video::driverHandle(const QString &driver) const
{
    Q_D(const Video);
    return_SOLID_CALL(Ifaces::Video *, d->backendObject(), QVariant(), driverHandle(driver));
}

#include "video.moc"

