/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#include "serialinterface.h"
#include "serialinterface_p.h"

#include <QtCore/qvariant.h>

#include "soliddefs_p.h"
#include <solid/ifaces/serialinterface.h>

Solid::SerialInterface::SerialInterface(QObject *backendObject)
    : DeviceInterface(*new SerialInterfacePrivate(), backendObject)
{
}

Solid::SerialInterface::~SerialInterface()
{

}

QVariant Solid::SerialInterface::driverHandle() const
{
    Q_D(const SerialInterface);
    return_SOLID_CALL(Ifaces::SerialInterface *, d->backendObject(), QVariant(), driverHandle());
}

Solid::SerialInterface::SerialType Solid::SerialInterface::serialType() const
{
    Q_D(const SerialInterface);
    return_SOLID_CALL(Ifaces::SerialInterface *, d->backendObject(), Unknown, serialType());
}

int Solid::SerialInterface::port() const
{
    Q_D(const SerialInterface);
    return_SOLID_CALL(Ifaces::SerialInterface *, d->backendObject(), -1, port());
}

#include "serialinterface.moc"
