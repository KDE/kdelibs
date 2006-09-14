/*  This file is part of the KDE project
    Copyright (C) 2006 Carlos Olmedo Escobar <arroba2puntos@gmail.com>

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

#ifndef SYSSTATISTICS_H_
#define SYSSTATISTICS_H_

#include <QMap>

namespace Solid
{

    /**
     * This class retrieves system statistics.
     *
     * @author Carlos Olmedo Escobar <arroba2puntos@gmail.com>
     */
    class SysStatistics
    {

    public:

        /**
         * This enum type lists the different processor states.
         *
         * - User : Running normal user processes.
         * - System : Running kernel code.
         * - Nice : Running niced processes.
         * - Idle : No executing code.
         * - IoWait : Waiting for a I/O operation to complete.
         * - HwIrq : Servicing hardware interrupts (Linux specific).
         * - SwInterrupt : Servicing software interrupts (Linux specific).
         * - Interrupt : Servicing any type of interrupts (*BSD specific).
         */
        enum ProcessorLoadType { User, System, Nice, Idle, IoWait, 
                             HwIrq, SwInterrupt, Interrupt };

        /**
         * Constructs a SysStatistics object.
         */
        SysStatistics();

        /**
         * Destroys a SysStatistics object.
         */
        ~SysStatistics();

        /**
         * Retrieves the average of time that the processors spends in each 
         * state, represented in percentages.
         * @return The associative container where the data is stored.
         * Key values are listed on the enum ProcessorLoadType.
         * If occurs an error the container will be empty.
         */
        QMap<ProcessorLoadType, float> getProcessorLoad();

        /**
         * Retrieves the time that the chosen processor spends in each state, 
         * represented in percentages.
         * @param processorNumber Index of cpu to retrieve the info (0..n).
         * @return The associative container where the data is stored.
         * Key values are listed on the enum ProcessorLoadType.
         * If occurs an error the container will be empty.
         */
        QMap<ProcessorLoadType, float> getProcessorLoad( short processorNumber );

    private:
        class Private;
        Private *d;

    };
}

#endif /*SYSSTATISTICS_H_*/
