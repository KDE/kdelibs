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
#include <QtCore/QSharedData>
#include <QtCore/QWeakPointer>

namespace Solid
{
    namespace Ifaces
    {
        class Device;
    }

    class DevicePrivate : public QObject, public QSharedData
    {
        Q_OBJECT
    public:
        explicit DevicePrivate(const QString &udi);
        ~DevicePrivate();

        QString udi() const { return m_udi; }

        Ifaces::Device *backendObject() const { return m_backendObject.data(); }
        void setBackendObject(Ifaces::Device *object);

        DeviceInterface *interface(const DeviceInterface::Type &type) const;
        void setInterface(const DeviceInterface::Type &type, DeviceInterface *interface);

    public Q_SLOTS:
        void _k_destroyed(QObject *object);

    private:
        QString m_udi;
        QWeakPointer<Ifaces::Device> m_backendObject;
        QMap<DeviceInterface::Type, DeviceInterface *> m_ifaces;
    };
}


#endif
