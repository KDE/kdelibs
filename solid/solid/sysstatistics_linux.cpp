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
#include <sys/sysinfo.h>

#define MAX_SIZE_OF_LINE 100
#define PROCESSOR_STATES 7
#define PROCESSOR_USER 0
#define PROCESSOR_SYSTEM 1
#define PROCESSOR_NICE 2
#define PROCESSOR_IDLE 3
#define PROCESSOR_IOWAIT 4
#define PROCESSOR_HW_IRQ 5
#define PROCESSOR_SW_INTERR 6



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
         * @param buffer The line to parse.
         * @return false if occurs an error, true otherwise.
         */
        bool processorParseLine( char * buffer );

        QFile file_proc_stat;
        quint64 real_processor_times[PROCESSOR_STATES];
        quint64 processor_total_time;
    };
}



Solid::SysStatistics::SysStatistics()
    : d( new Private() )
{
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



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad( qint16 processorNumber )
{
    QMap<ProcessorLoadType, float> map_to_fill;

    if ( processorNumber >= 0 )
        d->processorLoad( &map_to_fill, processorNumber );
    return map_to_fill;
}



QMap<Solid::SysStatistics::MemoryLoadType, qint64> Solid::SysStatistics::memoryLoad()
{
    QMap<MemoryLoadType, qint64> map_to_fill;
    static struct sysinfo mem_stats;

    if ( sysinfo( &mem_stats ) == -1 )
        return map_to_fill;

    map_to_fill.insert( TotalRam, (qint64) mem_stats.totalram * mem_stats.mem_unit );
    map_to_fill.insert( FreeRam, (qint64) mem_stats.freeram * mem_stats.mem_unit );
    map_to_fill.insert( SharedRam, (qint64) mem_stats.sharedram * mem_stats.mem_unit );
    map_to_fill.insert( BufferRam, (qint64) mem_stats.bufferram * mem_stats.mem_unit );
    map_to_fill.insert( TotalSwap, (qint64) mem_stats.totalswap * mem_stats.mem_unit );
    map_to_fill.insert( FreeSwap, (qint64) mem_stats.freeswap * mem_stats.mem_unit );

    return map_to_fill;
}



bool Solid::SysStatistics::Private::processorLoad( QMap<ProcessorLoadType, float> * mapToFill, qint16 processorNumber )
{
    static char buffer[MAX_SIZE_OF_LINE];
    register qint16 i;

// Buffering this file is nonsense because its already in memory and its data is valid only once
    if ( file_proc_stat.isOpen() )
        file_proc_stat.seek( 0 );		// Reopen is not necessary
    else
    {
        file_proc_stat.setFileName("/proc/stat");
        if ( file_proc_stat.open( QIODevice::ReadOnly | QIODevice::Text | QIODevice::Unbuffered ) == false )
            return false;
    }

    for ( i = 0; i <= processorNumber + 1; i++ )
    {
        if ( file_proc_stat.readLine( buffer, MAX_SIZE_OF_LINE - 1 ) == -1 )
            return false;
    }

    if ( processorParseLine( buffer ) == false )
        return false;

    mapToFill->insert( User, (float) ( real_processor_times[PROCESSOR_USER] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( System, (float) ( real_processor_times[PROCESSOR_SYSTEM] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( Nice, (float) ( real_processor_times[PROCESSOR_NICE] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( Idle, (float) ( real_processor_times[PROCESSOR_IDLE] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( IoWait, (float) ( real_processor_times[PROCESSOR_IOWAIT] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( HwIrq, (float) ( real_processor_times[PROCESSOR_HW_IRQ] * 100.0 ) / (float) processor_total_time );
    mapToFill->insert( SwInterrupt, (float) ( real_processor_times[PROCESSOR_SW_INTERR] * 100.0 ) / (float) processor_total_time );

    return true;
}



bool Solid::SysStatistics::Private::processorParseLine( char * buffer )
{
    static char cpu_name[10];
    static quint64 saved_processor_times[PROCESSOR_STATES];
    static quint64 current_processor_times[PROCESSOR_STATES];
    register qint8 i;

    if ( strncmp( buffer, "cpu", sizeof("cpu") - 1 ) )
        return false;

    if ( sscanf( buffer, "%9s %Lu %Lu %Lu %Lu %Lu %Lu %Lu", cpu_name, &current_processor_times[0], 
            &current_processor_times[1], &current_processor_times[2], &current_processor_times[3], 
            &current_processor_times[4], &current_processor_times[5], &current_processor_times[6] ) < 8 )
        return false;

    processor_total_time = 0;
    for ( i = 0; i < PROCESSOR_STATES; i++ )
    {
        real_processor_times[i] = current_processor_times[i] - saved_processor_times[i];
        processor_total_time += real_processor_times[i];
        saved_processor_times[i] = current_processor_times[i];
    }

    return true;
}
