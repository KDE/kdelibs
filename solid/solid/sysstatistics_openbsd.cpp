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

#include <QMap>

#include <sys/dkstat.h>
#include <sys/param.h>
#include <sys/sysctl.h>



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
        void processorPercentages(short cnt, int64_t *out, int64_t *newe, int64_t *old, int64_t *diffs);
    
        int64_t processor_time[CPUSTATES];
        int64_t processor_old[CPUSTATES];
        int64_t processor_diff[CPUSTATES];
        int64_t processor_states[CPUSTATES];
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
    register short i;
    QMap<ProcessorLoadType, float> map_to_fill;
    static int name_levels[] = { CTL_KERN, KERN_CPTIME };
    static long tmp_processor_time[CPUSTATES];
    static size_t length = sizeof(tmp_processor_time);

    if ( sysctl( name_levels, 2, tmp_processor_time, &length, NULL, 0 ) == -1 )
        return map_to_fill;

    for ( i = 0; i < CPUSTATES; i++ )
    {
        d->processor_time[i] = tmp_processor_time[i];
    }

    d->processorPercentages( CPUSTATES, d->processor_states, d->processor_time, d->processor_old, d->processor_diff );

    map_to_fill.insert( User, (float) d->processor_states[CP_USER] / 10.0 );
    map_to_fill.insert( System, (float) d->processor_states[CP_SYS] / 10.0 );
    map_to_fill.insert( Nice, (float) d->processor_states[CP_NICE] / 10.0 );
    map_to_fill.insert( Idle, (float) d->processor_states[CP_IDLE] / 10.0 );
    map_to_fill.insert( Interrupt, (float) d->processor_states[CP_INTR] / 10.0 );

    return map_to_fill;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::getProcessorLoad( short cpuNumber )
{
    QMap<ProcessorLoadType, float> map_to_fill;
    int name_levels[] = { CTL_KERN, KERN_CPTIME2, cpuNumber };
    static size_t length = CPUSTATES * sizeof(int64_t);

    if ( sysctl( name_levels, 3, &d->processor_time, &length, NULL, 0 ) == -1 )
        return map_to_fill;

    d->processorPercentages( CPUSTATES, d->processor_states, d->processor_time, d->processor_old, d->processor_diff );

    map_to_fill.insert( User, (float) d->processor_states[CP_USER] / 10.0 );
    map_to_fill.insert( System, (float) d->processor_states[CP_SYS] / 10.0 );
    map_to_fill.insert( Nice, (float) d->processor_states[CP_NICE] / 10.0 );
    map_to_fill.insert( Idle, (float) d->processor_states[CP_IDLE] / 10.0 );
    map_to_fill.insert( Interrupt, (float) d->processor_states[CP_INTR] / 10.0 );

    return map_to_fill;
}



/*
 * This method is ripped from top.
 */
/*
 *  Top users/processes display for Unix
 *  Version 3
 *
 * Copyright (c) 1984, 1989, William LeFebvre, Rice University
 * Copyright (c) 1989, 1990, 1992, William LeFebvre, Northwestern University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS EMPLOYER BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
void Solid::SysStatistics::Private::processorPercentages( short cnt, int64_t *out, int64_t *newe, int64_t *old, int64_t *diffs )
{
    int64_t change, total_change, *dp, half_total;
    register short i;

    // initialization
    total_change = 0;
    dp = diffs;

    // calculate changes for each state and the overall change
    for ( i = 0; i < cnt; i++ )
    {
        if ( ( change = *newe - *old ) < 0 )
        {
            // this only happens when the counter wraps
            change = ( *newe - *old );
        }
        total_change += ( *dp++ = change );
        *old++ = *newe++;
    }

    // avoid divide by zero potential
    if ( total_change == 0 )
        total_change = 1;

    // calculate percentages based on overall change, rounding up
    half_total = total_change / 2l;
    for ( i = 0; i < cnt; i++ )
        *out++ = ( ( *diffs++ * 1000 + half_total ) / total_change );
}
