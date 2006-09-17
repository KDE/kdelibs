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

#include "sysstatistics.h"

#include <QFile>
#include <QMap>

#include <stdio.h>

#define MAX_SIZE_OF_LINE 100
#define CPU_STATES 7
#define CPU_USER 0
#define CPU_SYSTEM 1
#define CPU_NICE 2
#define CPU_IDLE 3
#define CPU_IOWAIT 4
#define CPU_HW_IRQ 5
#define CPU_SW_INTERR 6



namespace Solid
{
    class SysStatistics::Private
    {
    public:
        Private() {}

        /**
         * This is the method that really calculates the load from a given processor.
         * @param mapToFill The associative container where the data is stored.
         * Key values are listed on the enum ProcessorLoadType.
         * @param processorNumber Index of cpu to retrieve the info (0..n).
         * @return false if occurs an error, true otherwise.
         */
        bool processorLoad( QMap<ProcessorLoadType, float> * mapToFill, short processorNumber );

        /**
         * Parses a line of /proc/stat and calculates the percentages
         * @return false if occurs an error, true otherwise.
         */
        bool processorParseLine();

        QFile file_proc_stat;
        unsigned long long real_processor_times[CPU_STATES];
        unsigned long long processor_total_time;
        char buffer[MAX_SIZE_OF_LINE];
    };
}



Solid::SysStatistics::SysStatistics()
    : d( new Private() )
{
    d->file_proc_stat.setFileName("/proc/stat");
}



Solid::SysStatistics::~SysStatistics()
{
    if ( d->file_proc_stat.isOpen() )
        d->file_proc_stat.close();

    delete d;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad()
{
    QMap<ProcessorLoadType, float> map_to_fill;

    d->processorLoad( &map_to_fill, -1 );
    return map_to_fill;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad( short processorNumber )
{
    QMap<ProcessorLoadType, float> map_to_fill;

    if ( processorNumber >= 0 )
        d->processorLoad( &map_to_fill, processorNumber );
    return map_to_fill;
}



bool Solid::SysStatistics::Private::processorLoad( QMap<ProcessorLoadType, float> * mapToFill, short processorNumber )
{
    register short i;

// Buffering this file is nonsense because its already in memory and its data is valid only once
    if ( file_proc_stat.isOpen() )
        file_proc_stat.seek( 0 );		// Reopen is not necessary
    else
        if ( file_proc_stat.open( QIODevice::ReadOnly | QIODevice::Text | QIODevice::Unbuffered ) == false )
            return false;

    for ( i = 0; i <= processorNumber + 1; i++ )
    {
        if ( file_proc_stat.readLine( buffer, MAX_SIZE_OF_LINE - 1 ) == -1 )
            return false;
    }

    if ( processorParseLine() == false )
        return false;

    mapToFill->insert( User, (float) ( real_processor_times[CPU_USER] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( System, (float) ( real_processor_times[CPU_SYSTEM] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( Nice, (float) ( real_processor_times[CPU_NICE] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( Idle, (float) ( real_processor_times[CPU_IDLE] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( IoWait, (float) ( real_processor_times[CPU_IOWAIT] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( HwIrq, (float) ( real_processor_times[CPU_HW_IRQ] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( SwInterrupt, (float) ( real_processor_times[CPU_SW_INTERR] * 100.0 ) / (float) processor_total_time );

    return true;
}



bool Solid::SysStatistics::Private::processorParseLine()
{
    register short i;
    static char cpu_name[10];
    static unsigned long long saved_processor_times[CPU_STATES];
    static unsigned long long current_processor_times[CPU_STATES];

    if ( strncmp( buffer, "cpu", sizeof("cpu") - 1 ) )
        return false;

    if ( sscanf( buffer, "%9s %Lu %Lu %Lu %Lu %Lu %Lu %Lu", cpu_name, &current_processor_times[0], 
            &current_processor_times[1], &current_processor_times[2], &current_processor_times[3], 
            &current_processor_times[4], &current_processor_times[5], &current_processor_times[6] ) < 7 )
        return false;

    processor_total_time = 0;
    for ( i = 0; i < CPU_STATES; i++ )
    {
        real_processor_times[i] = current_processor_times[i] - saved_processor_times[i];
        processor_total_time += real_processor_times[i];
        saved_processor_times[i] = current_processor_times[i];
    }

    return true;
}
