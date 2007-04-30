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

#include "opticaldisc.h"
#include "opticaldisc_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/opticaldisc.h>

Solid::OpticalDisc::OpticalDisc(QObject *backendObject)
    : StorageVolume(*new OpticalDiscPrivate(), backendObject)
{
}

Solid::OpticalDisc::~OpticalDisc()
{

}

Solid::OpticalDisc::ContentTypes Solid::OpticalDisc::availableContent() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), ContentTypes(), availableContent());
}

Solid::OpticalDisc::DiscType Solid::OpticalDisc::discType() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), UnknownDiscType, discType());
}

bool Solid::OpticalDisc::isAppendable() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isAppendable());
}

bool Solid::OpticalDisc::isBlank() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isBlank());
}

bool Solid::OpticalDisc::isRewritable() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), false, isRewritable());
}

qulonglong Solid::OpticalDisc::capacity() const
{
    Q_D(const OpticalDisc);
    return_SOLID_CALL(Ifaces::OpticalDisc *, d->backendObject(), 0, capacity());
}

#include "opticaldisc.moc"
