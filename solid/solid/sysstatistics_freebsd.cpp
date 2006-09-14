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

#include <sys/sysctl.h>
#include <osreldate.h>
#if __FreeBSD_version < 500101
    #include <sys/dkstat.h>
#else
    #include <sys/resource.h>
#endif



namespace Solid
{
    class SysStatistics::Private
    {
    public:
        Private() {}

        /**
         * Calculates the processor usage percentages
         * @param cnt the number of percentages
         * @param out an array with the calculated percentages
         * @param newe an array of values obtained from sysctlbyname
         * @param old and array of values obtained from a previous call
         * @param diff an array with the difference between the old and 
         * newe values
         */
        void processorPercentages( int cnt, int * out, long * newe, long * old, long * diffs );

        long processor_time[CPUSTATES];
        long processor_old[CPUSTATES];
        long processor_diff[CPUSTATES];
        int processor_states[CPUSTATES];
    };
}



Solid::SysStatistics::SysStatistics()
    : d( new Private() )
{
}



Solid::SysStatistics::~SysStatistics()
{
    delete d;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::getProcessorLoad()
{
    QMap<ProcessorLoadType, float> map_to_fill;
    static size_t length = sizeof(d->processor_time);

    if ( sysctlbyname( "kern.cp_time", &d->processor_time, &length, NULL, 0 ) )
        return map_to_fill;

    d->processorPercentages( CPUSTATES, d->processor_states, d->processor_time, d->processor_old, d->processor_diff );

    map_to_fill.insert( User, (float) d->processor_states[CP_USER] / 10.0 );
    map_to_fill.insert( System, (float) d->processor_states[CP_SYS] / 10.0 );
    map_to_fill.insert( Nice, (float) d->processor_states[CP_NICE] / 10.0 );
    map_to_fill.insert( Idle, (float) d->processor_states[CP_IDLE] / 10.0 );
    map_to_fill.insert( Interrupt, (float) d->processor_states[CP_INTR] / 10.0 );

    return map_to_fill;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::getProcessorLoad( short processorNumber )
{
    return getProcessorLoad();
}



/*
 * This method is ripped from top.
 */
/*  Top users/processes display for Unix
 *  Version 3
 *
 *  This program may be freely redistributed,
 *  but this entire comment MUST remain intact.
 *
 *  Copyright (c) 1984, 1989, William LeFebvre, Rice University
 *  Copyright (c) 1989, 1990, 1992, William LeFebvre, Northwestern University
 */
void Solid::SysStatistics::Private::processorPercentages( int cnt, int * out, long *newe, long * old, long * diffs )
{
    register short i;
    register long change;
    register long total_change;
    register long * dp;
    long half_total;

    /* initialization */
    total_change = 0;
    dp = diffs;

    /* calculate changes for each state and the overall change */
    for ( i = 0; i < cnt; i++ )
    {
        if ( ( change = *newe - *old ) < 0 )
        {
            /* this only happens when the counter wraps */
            change = ( int ) ( ( unsigned long ) * newe - ( unsigned long ) * old );
        }
        total_change += ( *dp++ = change );
        *old++ = *newe++;
    }

    /* avoid divide by zero potential */
    if ( total_change == 0 )
        total_change = 1;

    /* calculate percentages based on overall change, rounding up */
    half_total = total_change / 2l;
    for ( i = 0; i < cnt; i++ )
    {
        *out++ = ( int ) ( ( *diffs++ * 1000 + half_total ) / total_change );
    }
}
