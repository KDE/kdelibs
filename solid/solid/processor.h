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

#ifndef SOLID_PROCESSOR_H
#define SOLID_PROCESSOR_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    /**
     * This capability is available on processors.
     */
    class KDE_EXPORT Processor : public Capability
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Processor object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        Processor( QObject *backendObject );

        /**
         * Destroys a Processor object.
         */
        virtual ~Processor();


        /**
         * Get the Solid::Capability::Type of the Processor capability.
         *
         * @return the Processor capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Processor; }

        /**
         * Retrieves the processor number in the system.
         *
         * @return the internal processor number in the system, starting from zero
         */
        int number() const;

        /**
         * Retrieves the maximum speed of the processor.
         *
         * @return the maximum speed in MHz
         */
        qulonglong maxSpeed() const;

        /**
         * Indicates if the processor can throttle.
         *
         * A processor supports throttling when it's able of decreasing
         * it's own clockspeed (generally for power management).
         *
         * @return true if the processor can throttle, false otherwise
         */
        bool canThrottle() const;
    };
}

#endif
