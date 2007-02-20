/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_STORAGE_H
#define SOLID_STORAGE_H

#include <kdelibs_export.h>

#include <solid/block.h>

namespace Solid
{
    /**
     * This capability is available on storage devices.
     *
     * A storage is anything that can contain a set of volumes (card reader,
     * hard disk, cdrom drive...). It's a particular kind of block device.
     */
    class SOLID_EXPORT Storage : public Block
    {
        Q_OBJECT
        Q_ENUMS( Bus DriveType )
        Q_PROPERTY( Bus bus READ bus )
        Q_PROPERTY( DriveType driveType READ driveType )
        Q_PROPERTY( bool removable READ isRemovable )
        Q_PROPERTY( bool ejectRequired READ isEjectRequired )
        Q_PROPERTY( bool hotpluggable READ isHotpluggable )
        Q_PROPERTY( bool mediaCheckEnabled READ isMediaCheckEnabled )
        Q_PROPERTY( QString vendor READ vendor )
        Q_PROPERTY( QString product READ product )

    public:
        /**
         * This enum type defines the type of bus a storage device is attached to.
         *
         * - Ide : An Integrated Drive Electronics (IDE) bus, also known as ATA
         * - Usb : An Universal Serial Bus (USB)
         * - Ieee1394 : An Ieee1394 bus, also known as Firewire
         * - Scsi : A Small Computer System Interface bus
         * - Sata : A Serial Advanced Technology Attachment (SATA) bus
         * - Platform : A legacy bus that is part of the underlying platform
         */
        enum Bus { Ide, Usb, Ieee1394, Scsi, Sata, Platform };

        /**
         * This enum type defines the type of drive a storage device can be.
         *
         * - HardDisk : A hard disk
         * - CdromDrive : An optical drive
         * - Floppy : A floppy disk drive
         * - Tape : A tape drive
         * - CompactFlash : A Compact Flash card reader
         * - MemoryStick : A Memory Stick card reader
         * - SmartMedia : A Smart Media card reader
         * - SdMmc : A SecureDigital/MultiMediaCard card reader
         */
        enum DriveType { HardDisk, CdromDrive, Floppy, Tape, CompactFlash, MemoryStick, SmartMedia, SdMmc };



        /**
         * Creates a new Storage object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Storage( QObject *backendObject );

        /**
         * Destroys a Storage object.
         */
        virtual ~Storage();


        /**
         * Get the Solid::Capability::Type of the Storage capability.
         *
         * @return the Storage capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Storage; }


        /**
         * Retrieves the type of physical interface this storage device is
         * connected to.
         *
         * @return the bus type
         * @see Solid::Ifaces::Enums::Storage::Bus
         */
        Bus bus() const;

        /**
         * Retrieves the type of this storage drive.
         *
         * @return the drive type
         * @see Solid::Ifaces::Enums::Storage::DriveType
         */
        DriveType driveType() const;

        /**
         * Indicates if the media contained by this drive can be removed.
         *
         * For example memory card can be removed from the drive by the user,
         * while partitions can't be removed from hard disks.
         *
         * @return true if media can be removed, false otherwise.
         */
        bool isRemovable() const;

        /**
         * Indicates if calling eject is required to properly remove
         * media from this drive.
         *
         * @return true an eject command should be issued on medium removal
         */
        bool isEjectRequired() const;

        /**
         * Indicates if this storage device can be plugged or unplugged while
         * the computer is running.
         *
         * @return true if this storage supports hotplug, false otherwise
         */
        bool isHotpluggable() const;

        /**
         * Indicates if this storage device supports media detection.
         *
         * It generally means that the drive is polled to detect when a medium
         * is inserted, or that it can notify the system on insert.
         *
         * @return true is media detection is enabled
         */
        bool isMediaCheckEnabled() const;


        /**
         * Retrieves the name of this storage vendor.
         *
         * @return the vendor name
         */
        QString vendor() const;

        /**
         * Retrieves the name of the product corresponding to this storage.
         *
         * @return the product name
         */
        QString product() const;

    private:
        class Private;
        Private * const d;
    };
}

#endif
