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

#ifndef SOLID_IFACES_OPTICALDISC_H
#define SOLID_IFACES_OPTICALDISC_H

#include <kdelibs_export.h>

#include <solid/ifaces/volume.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on optical discs.
     *
     * An optical disc is a volume that can be inserted in a cdrom drive.
     */
    class KDE_EXPORT OpticalDisc : virtual public Volume, public Enums::OpticalDisc
    {
//         Q_PROPERTY( ContentTypes availableContent READ availableContent )
//         Q_PROPERTY( DisctType discType READ discType )
//         Q_PROPERTY( bool appendable READ isAppendable )
//         Q_PROPERTY( bool blank READ isBlank )
//         Q_PROPERTY( bool rewritable READ isRewritable )
//         Q_PROPERTY( qulonglong capacity READ capacity )
//         Q_ENUMS( ContentType DiscType )

    public:
        /**
         * Destroys an OpticalDisc object.
         */
        virtual ~OpticalDisc();


        /**
         * Retrieves the content types this disc contains (audio, video,
         * data...).
         *
         * @return the flag set indicating the available contents
         */
        virtual ContentTypes availableContent() const = 0;

        /**
         * Retrieves the disc type (cdr, cdrw...).
         *
         * @return the disc type
         */
        virtual DiscType discType() const = 0;

        /**
         * Indicates if it's possible to write additional data to the disc.
         *
         * @return true if the disc is appendable, false otherwise
         */
        virtual bool isAppendable() const = 0;

        /**
         * Indicates if the disc is blank.
         *
         * @return true if the disc is blank, false otherwise
         */
        virtual bool isBlank() const = 0;

        /**
         * Indicates if the disc is rewritable.
         *
         * A disc is rewritable if you can write on it several times.
         *
         * @return true if the disc is rewritable, false otherwise
         */
        virtual bool isRewritable() const = 0;

        /**
         * Retrieves the disc capacity (that is the maximum size of a
         * volume could have on this disc).
         *
         * @return the capacity of the disc in bytes
         */
        virtual qulonglong capacity() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::OpticalDisc, "org.kde.Solid.Ifaces.OpticalDisc/0.1" )

#endif
