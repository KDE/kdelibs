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

#include "camera.h"

#include <kdehw/ifaces/camera.h>

namespace KDEHW
{
    class Camera::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Camera *iface;
    };
}

KDEHW::Camera::Camera( Ifaces::Camera *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::Camera::~Camera()
{
    delete d;
}


KDEHW::Camera::AccessType KDEHW::Camera::accessMethod() const
{
    return d->iface->accessMethod();
}

bool KDEHW::Camera::isGphotoSupported() const
{
    return d->iface->isGphotoSupported();
}

#include "camera.moc"

