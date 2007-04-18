/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_DEVICE_P_H
#define SOLID_DEVICE_P_H

#include <QtCore/QObject>
#include "frontendobject_p.h"

namespace Solid
{
    class DevicePrivate : public QObject, public FrontendObjectPrivate
    {
        Q_OBJECT
    public:
        explicit DevicePrivate(const QString &udi = QString());
        ~DevicePrivate();

        virtual void setBackendObject(QObject *object);

    public Q_SLOTS:
        virtual void _k_destroyed(QObject *object);

    public:
        QString udi;
        mutable QMap<Capability::Type,Capability*> ifaces;
    };
}


#endif
