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

#ifndef SOLID_OPTICALDISC_H
#define SOLID_OPTICALDISC_H

#include <kdelibs_export.h>

#include <solid/volume.h>

namespace Solid
{
    namespace Ifaces
    {
        class OpticalDisc;
    }

    /**
     * This capability is available on optical discs.
     *
     * An optical disc is a volume that can be inserted in a cdrom drive.
     */
    class KDE_EXPORT OpticalDisc : public Volume, public Ifaces::Enums::OpticalDisc
    {
        Q_OBJECT
    public:
        /**
         * Creates a new OpticalDisc object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see Solid::Device::as()
         */
        OpticalDisc( Ifaces::OpticalDisc *iface, QObject *parent = 0 );

        /**
         * Destroys an OpticalDisc object.
         */
        virtual ~OpticalDisc();


        /**
         * Get the Solid::Capability::Type of the OpticalDisc capability.
         *
         * @return the OpticalDisc capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::OpticalDisc; }


        /**
         * Retrieves the content types this disc contains (audio, video,
         * data...).
         *
         * @return the flag set indicating the available contents
         */
        ContentTypes availableContent() const;

        /**
         * Retrieves the disc type (cdr, cdrw...).
         *
         * @return the disc type
         */
        DiscType discType() const;

        /**
         * Indicates if it's possible to write additional data to the disc.
         *
         * @return true if the disc is appendable, false otherwise
         */
        bool isAppendable() const;

        /**
         * Indicates if the disc is blank.
         *
         * @return true if the disc is blank, false otherwise
         */
        bool isBlank() const;

        /**
         * Indicates if the disc is rewritable.
         *
         * A disc is rewritable if you can write on it several times.
         *
         * @return true if the disc is rewritable, false otherwise
         */
        bool isRewritable() const;

        /**
         * Retrieves the disc capacity (that is the maximum size of a
         * volume could have on this disc).
         *
         * @return the capacity of the disc in bytes
         */
        qulonglong capacity() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
