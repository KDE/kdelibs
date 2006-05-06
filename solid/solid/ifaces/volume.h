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

#ifndef KDEHW_IFACES_VOLUME_H
#define KDEHW_IFACES_VOLUME_H

#include <kdehw/ifaces/block.h>
#include <kdehw/ifaces/enums.h>

class KJob;

namespace KDEHW
{
namespace Ifaces
{
    /**
     * This capability is available on volume devices.
     *
     * A volume is anything that can contain data (partition, optical disc,
     * memory card). It's a particular kind of block device.
     */
    class Volume : virtual public Block, public Enums::Volume
    {
//         Q_PROPERTY( bool ignored READ isIgnored )
//         Q_PROPERTY( bool mounted READ isMounted )
//         Q_PROPERTY( QString mountPoint READ mountPoint )
//         Q_PROPERTY( UsageType usage READ usage )
//         Q_PROPERTY( QString fsType READ fsType )
//         Q_PROPERTY( QString label READ label )
//         Q_PROPERTY( QString uuid READ uuid )
//         Q_PROPERTY( qulonglong size READ size )
//         Q_ENUMS( UsageType )

    public:
        /**
         * Destroys a Volume object.
         */
        virtual ~Volume();


        /**
         * Indicates if this volume should be ignored by applications.
         *
         * If it should be ignored, it generally means that it should be
         * invisible to the user. It's useful for firmware partitions or
         * OS reinstall partitions on some systems.
         *
         * @return true if the volume should be ignored
         */
        virtual bool isIgnored() const = 0;

        /**
         * Indicates if this volume is mounted.
         *
         * @return true if the volume is mounted
         */
        virtual bool isMounted() const = 0;

        /**
         * Retrieves the absolute path of this volume mountpoint.
         *
         * @return the absolute path to the mount point if the volume is
         * mounted, QString() otherwise
         */
        virtual QString mountPoint() const = 0;

        /**
         * Retrieves the type of use for this volume (for example filesystem).
         *
         * @return the usage type
         * @see KDEHW::Ifaces::Enums::Volume::UsageType
         */
        virtual UsageType usage() const = 0;

        /**
         * Retrieves the filesystem type of this volume.
         *
         * @return the filesystem type if applicable, QString() otherwise
         */
        virtual QString fsType() const = 0;

        /**
         * Retrieves this volume label.
         *
         * @return the volume lavel if available, QString() otherwise
         */
        virtual QString label() const = 0;

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
        virtual QString uuid() const = 0;

        /**
         * Retrieves this volume size in bytes.
         *
         * @return the size of this volume
         */
        virtual qulonglong size() const = 0;


        /**
         * Mounts the volume.
         *
         * @return the job handling the operation
         */
        virtual KJob *mount() = 0;

        /**
         * Unmounts the volume.
         *
         * @return the job handling the operation
         */
        virtual KJob *unmount() = 0;

        /**
         * Ejects the volume.
         *
         * @return the job handling the operation
         */
        virtual KJob *eject() = 0;

    protected:
    //signals:
        /**
         * This signal is emitted when the mount state of this device
         * has changed.
         *
         * @param newState true if the volume is mounted, false otherwise
         */
        virtual void mountStateChanged( bool newState ) = 0;
    };
}
}

#endif
