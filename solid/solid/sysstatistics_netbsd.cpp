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

#include <sys/dkstat.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/swap.h>



namespace Solid
{
    class SysStatistics::Private
    {
    public:
        Private() {}

        /**
         * The internal method that calls sysctl with the passed values.
         * @param mapToFill the map that should be filled.
         * @param levelsLength the length of the array of levels. 2 for 
         * retrive the average of the averall cpus and 3 for a specific
         * cpu. The third level specifies a cpu.
         * @param processorNameLevels the array used as parameter for 
         * sysctl.
         * @return True if successful, false otherwise.
         */
        bool processorLoad( QMap<ProcessorLoadType, float> * mapToFill, u_int numberOfLevels, int * processorNameLevels );

        /**
         * Calculates the processor usage percentages
         * @param out an array with the calculated percentages
         * @param newe an array of values obtained from sysctlbyname
         */
        void processorPercentages( int64_t * out, int64_t * newe );

        /**
         * Initialices the memory resources.
         * @return True if successful, false otherwise.
         */
        bool initializeMemory();

        int64_t processor_time[CPUSTATES];
        int64_t processor_states[CPUSTATES];

        bool memory_initialized;
        int pagesize;
        size_t total_ram;
        int n_swap_devs;
        struct swapent * swap_devs;
    };
}



Solid::SysStatistics::SysStatistics()
    : d( new Private() )
{
    d->swap_devs = NULL;
}



Solid::SysStatistics::~SysStatistics()
{
    if ( d->swap_devs != NULL )
        free( d->swap_devs );

    delete d;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad()
{
    QMap<ProcessorLoadType, float> map_to_fill;
    static int processor_name_levels[2] = { CTL_KERN, KERN_CP_TIME };

    d->processorLoad( &map_to_fill, 2, processor_name_levels );

    return map_to_fill;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad( qint16 processorNumber )
{
    QMap<ProcessorLoadType, float> map_to_fill;
    static int processor_name_levels[3] = { CTL_KERN, KERN_CP_TIME };

    if ( processorNumber < 0 )
        return map_to_fill;

    processor_name_levels[2] = processorNumber;
    d->processorLoad( &map_to_fill, 3 , processor_name_levels);

    return map_to_fill;
}



QMap<Solid::SysStatistics::MemoryLoadType, qint64> Solid::SysStatistics::memoryLoad()
{
    QMap<MemoryLoadType, qint64> map_to_fill;
    static struct vmtotal vm_stats;
    static int n_swap_devs_tmp, mem_name_levels[2] = { CTL_VM, VM_METER };
    static size_t length = sizeof(vm_stats);
    static qint64 used_swap, total_swap;
    static qint16 i;

    if ( !d->memory_initialized )
    {
        if ( d->initializeMemory() )
            d->memory_initialized = true;
        else
            return map_to_fill;
    }

    map_to_fill.insert( TotalRam, d->total_ram );

    if ( sysctl( mem_name_levels, 2, &vm_stats, &length, NULL, 0 ) != -1 )
    {
        map_to_fill.insert( SharedRam, (qint64) vm_stats.t_rmshr * d->pagesize );
        map_to_fill.insert( FreeRam, (qint64) vm_stats.t_free * d->pagesize );
    }

    n_swap_devs_tmp = swapctl( SWAP_STATS, d->swap_devs, d->n_swap_devs );
    if ( n_swap_devs_tmp == -1 )
        return map_to_fill;

    if ( n_swap_devs_tmp != d->n_swap_devs )
        d->memory_initialized = false;    // Swap stuff should be re-initialized
    else
    {
        used_swap = total_swap = 0;
        for ( i = 0; i < n_swap_devs_tmp; i++ )
        {
            if ( d->swap_devs[i].se_flags & SWF_ENABLE )
            {
                used_swap += (qint64) d->swap_devs[i].se_inuse * DEV_BSIZE;
                total_swap += (qint64) d->swap_devs[i].se_nblks * DEV_BSIZE;
            }
        }
        map_to_fill.insert( TotalSwap, total_swap );
        map_to_fill.insert( FreeSwap, total_swap - used_swap );
    }

    return map_to_fill;
}



bool Solid::SysStatistics::Private::processorLoad( QMap<ProcessorLoadType, float> * mapToFill, u_int numberOfLevels, int * processorNameLevels )
{
    static size_t length = CPUSTATES * sizeof(int64_t);

    if ( sysctl( processorNameLevels, numberOfLevels, &processor_time, &length, NULL, 0 ) == -1 )
        return false;

    processorPercentages( processor_states, processor_time );

    mapToFill->insert( User, (float) processor_states[CP_USER] / 10.0 );
    mapToFill->insert( System, (float) processor_states[CP_SYS] / 10.0 );
    mapToFill->insert( Nice, (float) processor_states[CP_NICE] / 10.0 );
    mapToFill->insert( Idle, (float) processor_states[CP_IDLE] / 10.0 );
    mapToFill->insert( Interrupt, (float) processor_states[CP_INTR] / 10.0 );

    return true;
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
void Solid::SysStatistics::Private::processorPercentages( int64_t * out, int64_t * newe )
{
    static int64_t processor_old[CPUSTATES];
    static int64_t processor_diff[CPUSTATES];
    static int64_t * old, * diffs;
    static int64_t change, total_change, half_total;
    register quint8 i;

    // initialization
    total_change = 0;
    old = processor_old;
    diffs = processor_diff;

    // calculate changes for each state and the overall change
    for ( i = 0; i < CPUSTATES; i++ )
    {
        if ( ( change = *newe - *old ) < 0 )
        {
            // this only happens when the counter wraps
            change = ( *newe - *old );
        }
        total_change += ( *diffs++ = change );
        *old++ = *newe++;
    }
    diffs = processor_diff;

    // avoid divide by zero potential
    if ( total_change == 0 )
        total_change = 1;

    // calculate percentages based on overall change, rounding up
    half_total = total_change / 2l;
    for ( i = 0; i < CPUSTATES; i++ )
        *out++ = ( ( *diffs++ * 1000 + half_total ) / total_change );
}



bool Solid::SysStatistics::Private::initializeMemory()
{
    size_t length = sizeof(total_ram);
    int name_levels[2] = { CTL_HW, HW_PHYSMEM };

    pagesize = getpagesize();

    if ( sysctl( name_levels, 2, &total_ram, &length, NULL, 0 ) == -1 )
        return false;

    n_swap_devs = swapctl( SWAP_NSWAP, 0, 0 );
    if ( n_swap_devs == -1 )
        return false;

    if ( n_swap_devs > 0 )
    {
// allocing swap_devs if pointer is NULL (on first time) or reallocing if initializeMemory is called again (if n_swap_devs has changed)
        swap_devs = (struct swapent *) realloc( swap_devs, n_swap_devs * sizeof(struct swapent) );
        if ( swap_devs == NULL )
            return false;
    }

    return true;
}
