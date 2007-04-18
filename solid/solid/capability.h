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

namespace Solid
{
    class Device;
    class DevicePrivate;
    class Predicate;
    class CapabilityPrivate;

    /**
     * Base class of all the capabilities.
     *
     * A capability describes what a device can do. A device generally has
     * a set of capabilities.
     */
    class SOLID_EXPORT Capability : public QObject
    {
        Q_OBJECT
        Q_ENUMS(Type)
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
        enum Type { Unknown = 0, GenericInterface = 1, Processor = 2,
                    Block = 3, Storage = 4, Cdrom = 5,
                    Volume = 6, OpticalDisc = 7,
                    Camera = 8, PortableMediaPlayer = 9,
                    NetworkHw = 10, AcAdapter = 11, Battery = 12,
                    Button = 13, Display = 14, AudioHw = 15,
                    DvbHw = 16 };

        /**
         * Destroys a Capability object.
         */
        virtual ~Capability();

        /**
         * Indicates if this capability is valid.
         * A capability is considered valid if the device it is referring is available in the system.
         *
         * @return true if this capability's device is available, false otherwise
         */
        bool isValid() const;

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

        CapabilityPrivate *d_ptr;

    private:
        Q_PRIVATE_SLOT(d_func(), void _k_destroyed(QObject*))

        friend class Device;
        friend class DevicePrivate;
    };
}

#endif
