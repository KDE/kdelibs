/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KDEHW_DEVICEMANAGER_H
#define KDEHW_DEVICEMANAGER_H

#include <QObject>
#include <QList>

#include <kstaticdeleter.h>

namespace KDEHW
{
    class Device;
    typedef QList<Device> DeviceList;

    class DeviceManager : public QObject
    {
        Q_OBJECT

    public:
        static DeviceManager &self();

        DeviceList allDevices();
        bool deviceExists( const QString &udi );

        Device findDevice( const QString &udi );

    signals:
        void deviceAdded( const QString &udi );
        void deviceRemoved( const QString &udi );
        void newCapability( const QString &udi, const QString &capability );

    private:
        DeviceManager();
        ~DeviceManager();

        void slotDeviceAdded( const QString &udi );
        void slotDeviceRemoved( const QString &udi );
        void slotNewCapability( const QString &udi, const QString &capability );
        void slotDestroyed( QObject *object );

        static DeviceManager *s_self;
        class Private;
        Private *d;

        friend void ::KStaticDeleter<DeviceManager>::destructObject();
    };
}

#endif
