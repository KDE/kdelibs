/*  This file is part of the KDE project
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

#include "dvbinterface.h"
#include "dvbinterface_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/dvbinterface.h>

Solid::DvbInterface::DvbInterface(QObject *backendObject)
    : DeviceInterface(*new DvbInterfacePrivate(), backendObject)
{
}

Solid::DvbInterface::~DvbInterface()
{

}

QString Solid::DvbInterface::device() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), QString(), device());
}

int Solid::DvbInterface::deviceAdapter() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), -1, deviceAdapter());
}

Solid::DvbInterface::DeviceType Solid::DvbInterface::deviceType() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), DvbUnknown, deviceType());
}

int Solid::DvbInterface::deviceIndex() const
{
    Q_D(const DvbInterface);
    return_SOLID_CALL(Ifaces::DvbInterface *, d->backendObject(), -1, deviceIndex());
}

#include "dvbinterface.moc"
