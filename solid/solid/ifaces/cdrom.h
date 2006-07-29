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

#ifndef SOLID_IFACES_CDROM_H
#define SOLID_IFACES_CDROM_H

#include <QList>

#include <kdelibs_export.h>

#include <solid/ifaces/storage.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on CD-ROM drives.
     *
     * A Cdrom is a storage that can handle optical discs.
     */
    class KDE_EXPORT Cdrom : virtual public Storage, public Enums::Cdrom
    {
//         Q_PROPERTY( MediumTypes supportedMedia READ supportedMedia )
//         Q_PROPERTY( int readSpeed READ readSpeed )
//         Q_PROPERTY( int writeSpeed READ writeSpeed )
//         Q_PROPERTY( QList<int> writeSpeeds READ writeSpeeds )
//         Q_ENUMS( MediumType )

    public:
        /**
         * Destroys a Cdrom object.
         */
        virtual ~Cdrom();

        /**
         * Retrieves the medium types this drive supports.
         *
         * @return the flag set indicating the supported medium types
         */
        virtual MediumTypes supportedMedia() const = 0;

        /**
         * Retrieves the maximum read speed of this drive in kilobytes.
         *
         * @return the maximum read speed
         */
        virtual int readSpeed() const = 0;

        /**
         * Retrieves the maximum write speed of this drive in kilobytes.
         *
         * @return the maximum write speed
         */
        virtual int writeSpeed() const = 0;

        /**
         * Retrieves the list of supported write speeds of this drive in
         * kilobytes.
         *
         * @return the list of supported write speeds
         */
        virtual QList<int> writeSpeeds() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the eject button is pressed
         * on the drive.
         *
         * Please note that some (broken) drives doesn't report this event.
         */
        virtual void ejectPressed() = 0;
    };
}
}

#endif
