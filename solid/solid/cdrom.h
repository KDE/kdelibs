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

#ifndef SOLID_CDROM_H
#define SOLID_CDROM_H

#include <QList>

#include <solid/solid_export.h>

#include <solid/storage.h>

namespace Solid
{
    class CdromPrivate;

    /**
     * This capability is available on CD-ROM drives.
     *
     * A Cdrom is a storage that can handle optical discs.
     */
    class SOLID_EXPORT Cdrom : public Storage
    {
        Q_OBJECT
        Q_ENUMS( MediumType )
        Q_FLAGS( MediumTypes )
        Q_PROPERTY( MediumTypes supportedMedia READ supportedMedia )
        Q_PROPERTY( int readSpeed READ readSpeed )
        Q_PROPERTY( int writeSpeed READ writeSpeed )
        Q_PROPERTY( QList<int> writeSpeeds READ writeSpeeds )
        Q_DECLARE_PRIVATE(Cdrom)

    public:
        /**
         * This enum type defines the type of medium a cdrom drive supports.
         *
         * - Cdr : A Recordable Compact Disc (CD-R)
         * - Cdrw : A ReWritable Compact Disc (CD-RW)
         * - Dvd : A Digital Versatile Disc (DVD)
         * - Dvdr : A Recordable Digital Versatile Disc (DVD-R)
         * - Dvdrw : A ReWritable Digital Versatile Disc (DVD-RW)
         * - Dvdram : A Random Access Memory Digital Versatile Disc (DVD-RAM)
         * - Dvdplusr : A Recordable Digital Versatile Disc (DVD+R)
         * - Dvdplusrw : A ReWritable Digital Versatile Disc (DVD+RW)
         * - Dvdplusdl : A Dual Layer Digital Versatile Disc (DVD+R DL)
         */
        enum MediumType { Cdr=0x001, Cdrw=0x002, Dvd=0x004, Dvdr=0x008,
                          Dvdrw=0x010, Dvdram=0x020, Dvdplusr=0x040,
                          Dvdplusrw=0x080, Dvdplusdl=0x100 };

        /**
         * This type stores an OR combination of MediumType values.
         */
        Q_DECLARE_FLAGS( MediumTypes, MediumType )



        /**
         * Creates a new Cdrom object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Cdrom( QObject *backendObject );

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

    protected:
        /**
         * @internal
         */
        Cdrom(CdromPrivate &dd, QObject *backendObject);
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Solid::Cdrom::MediumTypes )

#endif
