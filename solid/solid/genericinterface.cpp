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

#include "genericinterface.h"
#include "genericinterface_p.h"
#include "soliddefs_p.h"

#include <solid/ifaces/genericinterface.h>


Solid::GenericInterface::GenericInterface(QObject *backendObject)
    : DeviceInterface(*new GenericInterfacePrivate(), backendObject)
{
    if (backendObject) {
        connect(backendObject, SIGNAL(propertyChanged(const QMap<QString,int> &)),
                this, SIGNAL(propertyChanged(const QMap<QString,int> &)));
        connect(backendObject, SIGNAL(conditionRaised(const QString &, const QString &)),
                this, SIGNAL(conditionRaised(const QString &, const QString &)));
    }
}


Solid::GenericInterface::~GenericInterface()
{

}

QVariant Solid::GenericInterface::property(const QString &key) const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), QVariant(), property(key));
}

QMap<QString, QVariant> Solid::GenericInterface::allProperties() const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), QVariantMap(), allProperties());
}

bool Solid::GenericInterface::propertyExists(const QString &key) const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), false, propertyExists(key));
}

#include "genericinterface.moc"
