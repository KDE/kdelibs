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

#include "deviceinterface.h"
#include "deviceinterface_p.h"

#include <solid/ifaces/deviceinterface.h>

#include <QtCore/QMetaEnum>


Solid::DeviceInterface::DeviceInterface(DeviceInterfacePrivate &dd, QObject *backendObject)
    : d_ptr(&dd)
{
    Q_D(DeviceInterface);

    d->setBackendObject(backendObject);
}


Solid::DeviceInterface::~DeviceInterface()
{
    delete d_ptr;
    d_ptr = 0;
}

bool Solid::DeviceInterface::isValid() const
{
    Q_D(const DeviceInterface);
    return d->backendObject()!=0;
}

QString Solid::DeviceInterface::typeToString(Type type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return QString(metaEnum.valueToKey((int)type));
}

Solid::DeviceInterface::Type Solid::DeviceInterface::stringToType(const QString &type)
{
    int index = staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    return (Type)metaEnum.keyToValue(type.toUtf8());
}

Solid::DeviceInterfacePrivate::DeviceInterfacePrivate()
    : m_backendObject(0)
{

}

Solid::DeviceInterfacePrivate::~DeviceInterfacePrivate()
{

}

QObject *Solid::DeviceInterfacePrivate::backendObject() const
{
    return m_backendObject;
}

void Solid::DeviceInterfacePrivate::setBackendObject(QObject *object)
{
    m_backendObject = object;
}

#include "deviceinterface.moc"
