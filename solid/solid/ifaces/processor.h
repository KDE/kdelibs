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

#include <solid/ifaces/deviceinterface.h>
#include <solid/processor.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on processors.
     */
    class Processor : virtual public DeviceInterface
    {
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
        virtual int maxSpeed() const = 0;

        /**
         * Indicates if the processor can change the CPU frequency.
         *
         * True if a processor is able to change its own CPU frequency.
         *  (generally for power management).
         *
         * @return true if the processor can change CPU frequency, false otherwise
         */
        virtual bool canChangeFrequency() const = 0;

        /**
         * Queries the instructions set extensions of the CPU.
         *
         * @return the extensions supported by the CPU
         */
        virtual Solid::Processor::InstructionSets instructionSets() const = 0;

    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Processor, "org.kde.Solid.Ifaces.Processor/0.1")

#endif
