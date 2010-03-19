/*
   Copyright 2010 Pino Toscano <pino@kde.org>
   Copyright 2010 Patrick Spendrin <ps_ml@gmx.de>

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

#define _WIN32_WINNT 0x0500
#include <windows.h>

static bool fillMemoryInfo(KMemoryInfoData *data)
{
    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&stat)) {
        return false;
    }

    if (data->details & KMemoryInfo::TotalRam) {
        data->totalRam = stat.ullTotalPhys;
    }
    if (data->details & KMemoryInfo::FreeRam) {
        data->freeRam = stat.ullAvailPhys;
    }
    // the following two are not available
    if (data->details & KMemoryInfo::SharedRam) {
        data->sharedRam = 0;
    }
    if (data->details & KMemoryInfo::BufferRam) {
        data->bufferRam = 0;
    }
    // instead of the size of the swap partition, use the page file instead
    if (data->details & KMemoryInfo::TotalSwap) {
        data->totalSwap = stat.ullTotalPageFile;
    }
    if (data->details & KMemoryInfo::FreeSwap) {
        data->freeSwap = stat.ullAvailPageFile;
    }

    return true;
}
