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

#ifndef SOLID_DEVICEMANAGER_H
#define SOLID_DEVICEMANAGER_H

#include <QtCore/QObject>
#include <QList>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class Device;
    class Predicate;

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
    namespace DeviceManager
    {
        /**
         * Returns a text describing the error that occurred while loading
         * the backend.
         *
         * @return the error description, or QString() if the backend loaded successfully
         */
        SOLID_EXPORT QString errorText();

        /**
         * Retrieves all the devices available in the underlying system.
         *
         * @return the list of the devices available
         */
        SOLID_EXPORT QList<Device> allDevices();

        /**
         * Tests if a device exists in the underlying system given its
         * Universal Device Identifier (UDI).
         *
         * @param udi the identifier of the device to check
         * @return true if a device has the given udi in the system, false otherwise
         */
        SOLID_EXPORT bool deviceExists(const QString &udi);

        /**
         * Retrieves a device of the system given it's UDI.
         *
         * @param udi the identifier of the device to find
         * @return a device that has the given UDI in the system if possible, an
         * invalid device otherwise
         */
        SOLID_EXPORT Device findDevice(const QString &udi);

        /**
         * Retrieves a list of devices of the system given matching the given
         * constraints (parent and device interface type)
         *
         * @param type device interface type available on the devices we're looking for, or DeviceInterface::Unknown
         * if there's no constraint on the device interfaces
         * @param parentUdi UDI of the parent of the devices we're searching for, or QString()
         * if there's no constraint on the parent
         * @return the list of devices corresponding to the given constraints
         * @see Solid::Predicate
         */
        SOLID_EXPORT QList<Device> findDevicesFromQuery(const DeviceInterface::Type &type,
                                                        const QString &parentUdi = QString());

        /**
         * Retrieves a list of devices of the system given matching the given
         * constraints (parent and predicate)
         *
         * @param predicate Predicate that the devices we're searching for must verify
         * @param parentUdi UDI of the parent of the devices we're searching for, or QString()
         * if there's no constraint on the parent
         * @return the list of devices corresponding to the given constraints
         * @see Solid::Predicate
         */
        SOLID_EXPORT QList<Device> findDevicesFromQuery(const Predicate &predicate,
                                                        const QString &parentUdi = QString());

        /**
         * Convenience function see above.
         *
         * @param predicate
         * @param parentUdi
         * @return the list of devices
         */
        SOLID_EXPORT QList<Device> findDevicesFromQuery(const QString &predicate,
                                                        const QString &parentUdi = QString());

        class SOLID_EXPORT Notifier : public QObject
        {
            Q_OBJECT

        Q_SIGNALS:
            /**
             * This signal is emitted when a new device appear in the underlying system.
             *
             * @param udi the new device UDI
             */
            void deviceAdded(const QString &udi);

            /**
             * This signal is emitted when a device disappear from the underlying system.
             *
             * @param udi the old device UDI
             */
            void deviceRemoved(const QString &udi);

            /**
             * This signal is emitted when a new device interface is detected in a device.
             *
             * @param udi the UDI of the device getting a new device interface
             * @param type the device interface type
             */
            void newDeviceInterface(const QString &udi, int type);
        };

        SOLID_EXPORT Notifier *notifier();
    }
}

#endif
