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

#ifndef SOLID_CAPABILITY_H
#define SOLID_CAPABILITY_H

#include <QObject>
#include <QFlags>

#include <solid/solid_export.h>

#include <solid/frontendobject.h>

namespace Solid
{
    class Device;
    class Predicate;
    class CapabilityPrivate;

    /**
     * Base class of all the capabilities.
     *
     * A capability describes what a device can do. A device generally has
     * a set of capabilities.
     */
    class SOLID_EXPORT Capability : public FrontendObject
    {
        Q_OBJECT
        Q_ENUMS(Type)
        Q_FLAGS(Types)
        Q_DECLARE_PRIVATE(Capability)

    public:
        /**
         * This enum type defines the type of capability that a Device can have.
         *
         * - Unknown : An undetermined capability
         * - Processor : A processor
         * - Block : A block device
         * - Storage : A storage drive
         * - Cdrom : A CD-ROM drive
         * - Volume : A volume
         * - OpticalDisc : An optical disc
         * - Camera : A digital camera
         * - PortableMediaPlayer: A portable media player
         * - NetworkHw: A network interface
         * - Display : A video display
         */
        enum Type { Unknown = 0, Processor = 1, Block = 2,
                    Storage = 4, Cdrom = 8,
                    Volume = 16, OpticalDisc = 32,
                    Camera = 64, PortableMediaPlayer = 128,
                    NetworkHw = 256, AcAdapter = 512,
                    Battery = 1024, Button = 2048,
                    Display = 4096, AudioHw = 8192,
                    DvbHw = 16384 };

        /**
         * This type stores an OR combination of Type values.
         */
        Q_DECLARE_FLAGS( Types, Type )


        /**
         * Destroys a Capability object.
         */
        virtual ~Capability();

        /**
         *
         * @return the name of the capability type
         */
        static QString typeToString(Type type);

        /**
         *
         * @return the capability type for the given name
         */
        static Type stringToType(const QString &type);

    protected:
        /**
         * @internal
         * Creates a new Capability object.
         *
         * @param dd the private d member
         * @param backendObject the capability object provided by the backend
         */
        Capability(CapabilityPrivate &dd, QObject *backendObject);

    private:
        friend class Device;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Solid::Capability::Types )

#endif
