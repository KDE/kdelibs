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

    /**
     * This class allow to query the underlying system to obtain information
     * about the hardware available.
     *
     * It's the unique entry point for hardware discovery. Applications should use
     * it to find devices, or to be notified about hardware changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class DeviceManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Retrieves the unique instance of this class.
         *
         * @return unique instance of the class
         */
        static DeviceManager &self();



        /**
         * Retrieves all the devices available in the underlying system.
         *
         * @return the list of the devices available
         */
        DeviceList allDevices();

        /**
         * Tests if a device exists in the underlying system given its
         * Universal Device Identifier (UDI).
         *
         * @param udi the identifier of the device to check
         * @return true if a device has the given udi in the system, false otherwise
         */
        bool deviceExists( const QString &udi );

        /**
         *
         *
         * @param udi the identifier of the device to find
         * @return a device that has the given UDI in the system if possible, an
         * invalid device otherwise
         */
        Device findDevice( const QString &udi );

    signals:
        /**
         * This signal is emitted when a new device appear in the underlying system.
         *
         * @param udi the new device UDI
         */
        void deviceAdded( const QString &udi );

        /**
         * This signal is emitted when a device disappear from the underlying system.
         *
         * @param udi the old device UDI
         */
        void deviceRemoved( const QString &udi );

        /**
         * This signal is emitted when a new capability is detected in a device.
         * FIXME: Use an enum or something similar also here?
         *
         * @param udi the UDI of the device getting a new capability
         * @param capability the capability name
         */
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
