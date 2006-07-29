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

#ifndef SOLID_CDROM_H
#define SOLID_CDROM_H

#include <QList>

#include <kdelibs_export.h>

#include <solid/storage.h>

namespace Solid
{
    namespace Ifaces
    {
        class Cdrom;
    }

    /**
     * This capability is available on CD-ROM drives.
     *
     * A Cdrom is a storage that can handle optical discs.
     */
    class KDE_EXPORT Cdrom : public Storage, public Ifaces::Enums::Cdrom
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Cdrom object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see Solid::Device::as()
         */
        Cdrom( Ifaces::Cdrom *iface, QObject *parent = 0 );

        /**
         * Destroys a Cdrom object.
         */
        virtual ~Cdrom();


        /**
         * Get the Solid::Capability::Type of the Cdrom capability.
         *
         * @return the Cdrom capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Cdrom; }


        /**
         * Retrieves the medium types this drive supports.
         *
         * @return the flag set indicating the supported medium types
         */
        MediumTypes supportedMedia() const;

        /**
         * Retrieves the maximum read speed of this drive in kilobytes.
         *
         * @return the maximum read speed
         */
        int readSpeed() const;

        /**
         * Retrieves the maximum write speed of this drive in kilobytes.
         *
         * @return the maximum write speed
         */
        int writeSpeed() const;

        /**
         * Retrieves the list of supported write speeds of this drive in
         * kilobytes.
         *
         * @return the list of supported write speeds
         */
        QList<int> writeSpeeds() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the eject button is pressed
         * on the drive.
         *
         * Please note that some (broken) drives doesn't report this event.
         */
        void ejectPressed();

    private Q_SLOTS:
        void slotEjectPressed();

    private:
        class Private;
        Private *d;
    };
}

#endif
