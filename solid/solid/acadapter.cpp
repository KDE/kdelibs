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

#include "acadapter.h"
#include "acadapter_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/acadapter.h>

Solid::AcAdapter::AcAdapter(QObject *backendObject)
    : DeviceInterface(*new AcAdapterPrivate(), backendObject)
{
    connect(backendObject, SIGNAL(plugStateChanged(bool, const QString &)),
             this, SIGNAL(plugStateChanged(bool, const QString &)));
}

Solid::AcAdapter::~AcAdapter()
{

}

bool Solid::AcAdapter::isPlugged() const
{
    Q_D(const AcAdapter);
    return_SOLID_CALL(Ifaces::AcAdapter *, d->backendObject(), false, isPlugged());
}

#include "acadapter.moc"
