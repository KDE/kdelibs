/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "camera.h"
#include "camera_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/camera.h>

Solid::Camera::Camera(QObject *backendObject)
    : DeviceInterface(*new CameraPrivate(), backendObject)
{
}

Solid::Camera::~Camera()
{

}

QStringList Solid::Camera::supportedProtocols() const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QStringList(), supportedProtocols());
}

QStringList Solid::Camera::supportedDrivers(QString protocol) const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QStringList(), supportedDrivers(protocol));
}

QVariant Solid::Camera::driverHandle(const QString &driver) const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QVariant(), driverHandle(driver));
}

#include "camera.moc"

