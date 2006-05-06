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

#ifndef KDEHW_VOLUME_H
#define KDEHW_VOLUME_H

#include <kdehw/block.h>
#include <kdehw/ifaces/enums.h>

class KJob;

namespace KDEHW
{
    namespace Ifaces
    {
        class Volume;
    }

    /**
     * This capability is available on volume devices.
     *
     * A volume is anything that can contain data (partition, optical disc,
     * memory card). It's a particular kind of block device.
     */
    class Volume : public Block, public Ifaces::Enums::Volume
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Volume object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see KDEHW::Device::as()
         */
        Volume( Ifaces::Volume *iface, QObject *parent = 0 );

        /**
         * Destroys a Volume object.
         */
        virtual ~Volume();


        /**
         * Get the KDEHW::Capability::Type of the Volume capability.
         *
         * @return the Volume capability type
         * @see KDEHW::Ifaces::Enums::Capability::Type
         */
        static Type type() { return Capability::Volume; }


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
         * @see KDEHW::Ifaces::Enums::Volume::UsageType
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
         * @return the volume lavel if available, QString() otherwise
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
        KJob *mount();

        /**
         * Unmounts the volume.
         *
         * @return the job handling the operation
         */
        KJob *unmount();

        /**
         * Ejects the volume.
         *
         * @return the job handling the operation
         */
        KJob *eject();

    signals:
        /**
         * This signal is emitted when the mount state of this device
         * has changed.
         *
         * @param newState true if the volume is mounted, false otherwise
         */
        void mountStateChanged( bool newState );

    private slots:
        void slotMountStateChanged( bool newState );

    private:
        class Private;
        Private *d;
    };
}

#endif
