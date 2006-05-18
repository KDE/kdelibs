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

#ifndef SOLID_IFACES_PROCESSOR_H
#define SOLID_IFACES_PROCESSOR_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on processors.
     */
    class KDE_EXPORT Processor : virtual public Capability
    {
//         Q_PROPERTY( int number READ number )
//         Q_PROPERTY( qulonglong maxSpeed READ maxSpeed )
//         Q_PROPERTY( bool canThrottle READ canThrottle )

    public:
        /**
         * Destroys a Processor object.
         */
        virtual ~Processor();

        /**
         * Retrieves the processor number in the system.
         *
         * @return the internal processor number in the system, starting from zero
         */
        virtual int number() const = 0;

        /**
         * Retrieves the maximum speed of the processor.
         *
         * @return the maximum speed in MHz
         */
        virtual qulonglong maxSpeed() const = 0;

        /**
         * Indicates if the processor can throttle.
         *
         * A processor supports throttling when it's able of decreasing
         * it's own clockspeed (generally for power management).
         *
         * @return true if the processor can throttle, false otherwise
         */
        virtual bool canThrottle() const = 0;
    };
}
}

#endif
