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

#ifndef SOLID_VOLUME_H
#define SOLID_VOLUME_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class StorageVolumePrivate;

    /**
     * This device interface is available on volume devices.
     *
     * A volume is anything that can contain data (partition, optical disc,
     * memory card). It's a particular kind of block device.
     */
    class SOLID_EXPORT StorageVolume : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(UsageType)
        Q_PROPERTY(bool ignored READ isIgnored)
        Q_PROPERTY(bool mounted READ isMounted)
        Q_PROPERTY(QString mountPoint READ mountPoint)
        Q_PROPERTY(UsageType usage READ usage)
        Q_PROPERTY(QString fsType READ fsType)
        Q_PROPERTY(QString label READ label)
        Q_PROPERTY(QString uuid READ uuid)
        Q_PROPERTY(qulonglong size READ size)
        Q_DECLARE_PRIVATE(StorageVolume)

    public:
        /**
         * This enum type defines the how a volume is used.
         *
         * - FileSystem : A mountable filesystem volume
         * - PartitionTable : A volume containing a partition table
         * - Raid : A volume member of a raid set (not mountable)
         * - Other : A not mountable volume (like a swap partition)
         * - Unused : An unused or free volume
         */
        enum UsageType { FileSystem, PartitionTable, Raid, Other, Unused };



        /**
         * Creates a new StorageVolume object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit StorageVolume(QObject *backendObject);

        /**
         * Destroys a StorageVolume object.
         */
        virtual ~StorageVolume();


        /**
         * Get the Solid::DeviceInterface::Type of the StorageVolume device interface.
         *
         * @return the StorageVolume device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::StorageVolume; }


        /**
         * Indicates if this volume should be ignored by applications.
         *
         * If it should be ignored, it generally means that it should be
         * invisible to the user. It's useful for firmware partitions or
         * OS reinstall partitions on some systems.
         *
         * @return true if the volume should be ignored
         */
        bool isIgnored() const;

        /**
         * Indicates if this volume is mounted.
         *
         * @return true if the volume is mounted
         */
        bool isMounted() const;

        /**
         * Retrieves the absolute path of this volume mountpoint.
         *
         * @return the absolute path to the mount point if the volume is
         * mounted, QString() otherwise
         */
        QString mountPoint() const;

        /**
         * Retrieves the type of use for this volume (for example filesystem).
         *
         * @return the usage type
         * @see Solid::Ifaces::Enums::StorageVolume::UsageType
         */
        UsageType usage() const;

        /**
         * Retrieves the filesystem type of this volume.
         *
         * FIXME: It's a platform dependent string, maybe we should switch to
         * an enum?
         *
         * @return the filesystem type if applicable, QString() otherwise
         */
        QString fsType() const;

        /**
         * Retrieves this volume label.
         *
         * @return the volume label if available, QString() otherwise
         */
        QString label() const;

        /**
         * Retrieves this volume Universal Unique IDentifier (UUID).
         *
         * You can generally assume that this identifier is unique with reasonable
         * confidence. Except if the volume UUID has been forged to intentionally
         * provoke a collision, the probability to have two volumes having the same
         * UUID is low.
         *
         * @return the Universal Unique IDentifier if available, QString() otherwise
         */
        QString uuid() const;

        /**
         * Retrieves this volume size in bytes.
         *
         * @return the size of this volume
         */
        qulonglong size() const;


        /**
         * Mounts the volume.
         *
         * @return the job handling the operation
         */
        void mount(QObject *receiver, const char *member);

        /**
         * Unmounts the volume.
         *
         * @return the job handling the operation
         */
        void unmount(QObject *receiver, const char *member);

        /**
         * Ejects the volume.
         *
         * @return the job handling the operation
         */
        void eject(QObject *receiver, const char *member);

    Q_SIGNALS:
        /**
         * This signal is emitted when the mount state of this device
         * has changed.
         *
         * @param newState true if the volume is mounted, false otherwise
         */
        void mountStateChanged(bool newState);

    protected:
        /**
         * @internal
         */
        StorageVolume(StorageVolumePrivate &dd, QObject *backendObject);
    };
}

#endif
