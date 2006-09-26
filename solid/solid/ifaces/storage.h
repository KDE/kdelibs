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

#ifndef SOLID_IFACES_STORAGE_H
#define SOLID_IFACES_STORAGE_H

#include <kdelibs_export.h>

#include <solid/ifaces/block.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on storage devices.
     *
     * A storage is anything that can contain a set of volumes (card reader,
     * hard disk, cdrom drive...). It's a particular kind of block device.
     */
    class KDE_EXPORT Storage : virtual public Block, public Enums::Storage
    {
//         Q_PROPERTY( Bus bus READ bus )
//         Q_PROPERTY( DriveType driveType READ driveType )
//         Q_PROPERTY( bool removable READ isRemovable )
//         Q_PROPERTY( bool ejectRequired READ isEjectRequired )
//         Q_PROPERTY( bool hotpluggable READ isHotpluggable )
//         Q_PROPERTY( bool mediaCheckEnabled READ isMediaCheckEnabled )
//         Q_PROPERTY( QString vendor READ vendor )
//         Q_PROPERTY( QString product READ product )
//         Q_ENUMS( Bus DriveType )

    public:
        /**
         * Destroys a Storage object.
         */
        virtual ~Storage();


        /**
         * Retrieves the type of physical interface this storage device is
         * connected to.
         *
         * @return the bus type
         * @see Solid::Ifaces::Enums::Storage::Bus
         */
        virtual Bus bus() const = 0;

        /**
         * Retrieves the type of this storage drive.
         *
         * @return the drive type
         * @see Solid::Ifaces::Enums::Storage::DriveType
         */
        virtual DriveType driveType() const = 0;


        /**
         * Indicates if the media contained by this drive can be removed.
         *
         * For example memory card can be removed from the drive by the user,
         * while partitions can't be removed from hard disks.
         *
         * @return true if media can be removed, false otherwise.
         */
        virtual bool isRemovable() const = 0;

        /**
         * Indicates if calling eject is required to properly remove
         * media from this drive.
         *
         * @return true an eject command should be issued on medium removal
         */
        virtual bool isEjectRequired() const = 0;

        /**
         * Indicates if this storage device can be plugged or unplugged while
         * the computer is running.
         *
         * @return true if this storage supports hotplug, false otherwise
         */
        virtual bool isHotpluggable() const = 0;

        /**
         * Indicates if this storage device supports media detection.
         *
         * It generally means that the drive is polled to detect when a medium
         * is inserted, or that it can notify the system on insert.
         *
         * @return true is media detection is enabled
         */
        virtual bool isMediaCheckEnabled() const = 0;


        /**
         * Retrieves the name of this storage vendor.
         *
         * @return the vendor name
         */
        virtual QString vendor() const = 0;

        /**
         * Retrieves the name of the product corresponding to this storage.
         *
         * @return the product name
         */
        virtual QString product() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::Storage, "org.kde.Solid.Ifaces.Storage/0.1" )

#endif
