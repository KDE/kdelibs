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

#ifndef SOLID_STORAGEACCESS_H
#define SOLID_STORAGEACCESS_H

#include <solid/solid_export.h>

#include <solid/solidnamespace.h>
#include <solid/deviceinterface.h>
#include <QtCore/QVariant>

namespace Solid
{
    class StorageAccessPrivate;
    class Device;

    /**
     * This device interface is available on volume devices.
     *
     * A volume is anything that can contain data (partition, optical disc,
     * memory card). It's a particular kind of block device.
     */
    class SOLID_EXPORT StorageAccess : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(bool accessible READ isAccessible)
        Q_PROPERTY(QString filePath READ filePath)
        Q_DECLARE_PRIVATE(StorageAccess)
        friend class Device;

    private:
        /**
         * Creates a new StorageAccess object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit StorageAccess(QObject *backendObject);

    public:
        /**
         * Destroys a StorageAccess object.
         */
        virtual ~StorageAccess();


        /**
         * Get the Solid::DeviceInterface::Type of the StorageAccess device interface.
         *
         * @return the StorageVolume device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::StorageAccess; }


        /**
         * Indicates if this volume is mounted.
         *
         * @return true if the volume is mounted
         */
        bool isAccessible() const;

        /**
         * Retrieves the absolute path of this volume mountpoint.
         *
         * @return the absolute path to the mount point if the volume is
         * mounted, QString() otherwise
         */
        QString filePath() const;

        /**
         * Mounts the volume.
         *
         * @return false if the operation is not supported, true if the
         * operation is attempted
         */
        bool setup();

        /**
         * Unmounts the volume.
         *
         * @return false if the operation is not supported, true if the
         * operation is attempted
         */
        bool teardown();

    Q_SIGNALS:
        /**
         * This signal is emitted when the accessiblity of this device
         * has changed.
         *
         * @param accessible true if the volume is accessible, false otherwise
         * @param udi the UDI of the volume
         */
        void accessibilityChanged(bool accessible, const QString &udi);

        /**
         * This signal is emitted when the attempted setting up of this
         * device is completed.
         *
         * @param error type of error that occurred, if any
         * @param errorData more information about the error, if any
         * @param udi the UDI of the volume
         */
        void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

        /**
         * This signal is emitted when the attempted tearing down of this
         * device is completed.
         *
         * @param error type of error that occurred, if any
         * @param errorData more information about the error, if any
         * @param udi the UDI of the volume
         */
        void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    protected:
        /**
         * @internal
         */
        StorageAccess(StorageAccessPrivate &dd, QObject *backendObject);
    };
}

#endif
