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
#include <kvm.h>
#include <fcntl.h>
#include <sys/vmmeter.h>
#include <vm/vm_param.h>



namespace Solid
{
    class SysStatistics::Private
    {
    public:
        Private() {}

        /**
         * Calculates the processor usage percentages.
         * @param cnt the number of percentages
         * @param out an array with the calculated percentages
         * @param newe an array of values obtained from sysctlbyname
         * @param old and array of values obtained from a previous call
         * @param diff an array with the difference between the old and 
         * newe values
         */
        void processorPercentages( int cnt, int * out, long * newe );
        
        /**
         * Initialices the memory resources.
         * @return True if successful, false otherwise.
         */
        bool initializeMemory();
        
        long processor_time[CPUSTATES];
        int processor_states[CPUSTATES];
        
        kvm_t * kvm_d;
        size_t total_ram;
        int pagesize;
        bool memory_initialized;
    };
}



Solid::SysStatistics::SysStatistics()
    : d( new Private() )
{
    d->memory_initialized = false;
}



Solid::SysStatistics::~SysStatistics()
{
    if ( d->kvm_d != NULL )
        kvm_close( d->kvm_d );
    delete d;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad()
{
    QMap<ProcessorLoadType, float> map_to_fill;
    static size_t length = sizeof(d->processor_time);

    if ( sysctlbyname( "kern.cp_time", &d->processor_time, &length, NULL, 0 ) )
        return map_to_fill;

    d->processorPercentages( CPUSTATES, d->processor_states, d->processor_time );

    map_to_fill.insert( User, (float) d->processor_states[CP_USER] / 10.0 );
    map_to_fill.insert( System, (float) d->processor_states[CP_SYS] / 10.0 );
    map_to_fill.insert( Nice, (float) d->processor_states[CP_NICE] / 10.0 );
    map_to_fill.insert( Idle, (float) d->processor_states[CP_IDLE] / 10.0 );
    map_to_fill.insert( Interrupt, (float) d->processor_states[CP_INTR] / 10.0 );

    return map_to_fill;
}



QMap<Solid::SysStatistics::ProcessorLoadType, float> Solid::SysStatistics::processorLoad( qint16 processorNumber )
{
    return processorLoad();
}



QMap<Solid::SysStatistics::MemoryLoadType, qint64> Solid::SysStatistics::memoryLoad()
{
    QMap<MemoryLoadType, qint64> map_to_fill;
    static struct vmtotal vm_stats;
    static struct kvm_swap swap_stats[1];
    static int name_levels[2] = { CTL_VM, VM_TOTAL };
    static size_t amount_of_mem, length_of_size_t = sizeof(size_t);
    static size_t length_of_vmtotal = sizeof(struct vmtotal);

    if ( !d->memory_initialized )
    {
        if ( d->initializeMemory() )
            d->memory_initialized = true;
        else
            return map_to_fill;
    }

    map_to_fill.insert( TotalRam, d->total_ram );

    if ( sysctl( name_levels, 2, &vm_stats, &length_of_vmtotal, NULL, 0 ) != -1 )
    {
        map_to_fill.insert( SharedRam, (qint64) vm_stats.t_rmshr * d->pagesize );
        map_to_fill.insert( FreeRam, (qint64) vm_stats.t_free * d->pagesize );
    }

    if ( sysctlbyname("vfs.bufspace", &amount_of_mem, &length_of_size_t, NULL, 0) != -1 )
        map_to_fill.insert( BufferRam, amount_of_mem );

    if ( kvm_getswapinfo( d->kvm_d, swap_stats, 1, 0 ) != -1 )
    {
        map_to_fill.insert( TotalSwap, (qint64) swap_stats[0].ksw_total * d->pagesize );
        map_to_fill.insert( FreeSwap, (qint64) ( swap_stats[0].ksw_total - swap_stats[0].ksw_used ) * d->pagesize );
    }

    return map_to_fill;
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
void Solid::SysStatistics::Private::processorPercentages( int cnt, int * out, long * newe )
{
    static long processor_old[CPUSTATES];
    static long processor_diff[CPUSTATES];
    static long * old, * diffs;
    static long change, total_change, half_total;
    register quint8 i;

    /* initialization */
    total_change = 0;
    old = processor_old;
    diffs = processor_diff;

    /* calculate changes for each state and the overall change */
    for ( i = 0; i < cnt; i++ )
    {
        if ( ( change = *newe - *old ) < 0 )
        {
            /* this only happens when the counter wraps */
            change = ( int ) ( ( unsigned long ) *newe - ( unsigned long ) *old );
        }
        total_change += ( *diffs++ = change );
        *old++ = *newe++;
    }

    diffs = processor_diff;

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



bool Solid::SysStatistics::Private::initializeMemory()
{
    size_t length = sizeof(total_ram);
    int name_levels[2] = { CTL_HW, HW_PHYSMEM };

    pagesize = getpagesize();	// Getting the pagesize

    if ( sysctl( name_levels, 2, &total_ram, &length, NULL, 0 ) == -1 )
        return false;

    if ( kvm_d == NULL )
    {
        kvm_d = kvm_open( NULL, NULL, NULL, O_RDONLY, NULL );
        if ( kvm_d == NULL )
            return false;
    }

    return true;
}
