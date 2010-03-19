/*
   Copyright 2010 Pino Toscano <pino@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool fillMemoryInfo(KMemoryInfoData *data)
{
    struct sysinfo info;
    if (sysinfo(&info) == -1) {
        return false;
    }

    const qint64 unit = info.mem_unit;
    if (data->details & KMemoryInfo::TotalRam) {
        data->totalRam = unit * info.totalram;
    }
    if (data->details & KMemoryInfo::FreeRam) {
        data->freeRam = unit * info.freeram;
    }
    if (data->details & KMemoryInfo::SharedRam) {
        data->sharedRam = unit * info.sharedram;
    }
    if (data->details & KMemoryInfo::BufferRam) {
        data->bufferRam = unit * info.bufferram;
    }
    if (data->details & KMemoryInfo::TotalSwap) {
        data->totalSwap = unit * info.totalswap;
    }
    if (data->details & KMemoryInfo::FreeSwap) {
        data->freeSwap = unit * info.freeswap;
    }

    // open meminfo only for reading "cached"
    if (data->details & KMemoryInfo::CachedRam) {
        FILE *meminfo = fopen("/proc/meminfo", "r");
        if (meminfo) {
            char buffer[100];
            char *end = 0;
            while(fgets(buffer, sizeof(buffer) - 1, meminfo)) {
                if (strstr(buffer, "Cached:") == buffer) {
                    data->cachedRam = strtol(buffer + 7, &end, 10);
                    break;
                }
            }
            fclose(meminfo);
        }
    }

    return true;
}
