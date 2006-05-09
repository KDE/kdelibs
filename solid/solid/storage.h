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

#ifndef KDEHW_STORAGE_H
#define KDEHW_STORAGE_H

#include <kdelibs_export.h>

#include <kdehw/block.h>
#include <kdehw/ifaces/enums.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Storage;
    }

    /**
     * This capability is available on storage devices.
     *
     * A storage is anything that can contain a set of volumes (card reader,
     * hard disk, cdrom drive...). It's a particular kind of block device.
     */
    class KDE_EXPORT Storage : public Block, public Ifaces::Enums::Storage
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Storage object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see KDEHW::Device::as()
         */
        Storage( Ifaces::Storage *iface, QObject *parent = 0 );

        /**
         * Destroys a Storage object.
         */
        virtual ~Storage();


        /**
         * Get the KDEHW::Capability::Type of the Storage capability.
         *
         * @return the Storage capability type
         * @see KDEHW::Ifaces::Enums::Capability::Type
         */
        static Type type() { return Capability::Storage; }


        /**
         * Retrieves the type of physical interface this storage device is
         * connected to.
         *
         * @return the bus type
         * @see KDEHW::Ifaces::Enums::Storage::Bus
         */
        Bus bus() const;

        /**
         * Retrieves the type of this storage drive.
         *
         * @return the drive type
         * @see KDEHW::Ifaces::Enums::Storage::DriveType
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
        Private *d;
    };
}

#endif
