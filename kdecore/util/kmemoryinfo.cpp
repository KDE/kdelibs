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

#include "kmemoryinfo.h"

#include <QtCore/QDateTime>

struct KMemoryInfoData
{
    KMemoryInfoData()
        : details(0)
    {
        clear();
    }

    void clear();

    KMemoryInfo::MemoryDetails details;
    QDateTime lastUpdate;
    qint64 totalRam;
    qint64 freeRam;
    qint64 sharedRam;
    qint64 bufferRam;
    qint64 totalSwap;
    qint64 freeSwap;
};

void KMemoryInfoData::clear()
{
    totalRam = -1;
    freeRam = -1;
    sharedRam = -1;
    bufferRam = -1;
    totalSwap = -1;
    freeSwap = -1;
}

static bool fillMemoryInfo(KMemoryInfoData *data);


KMemoryInfo::KMemoryInfo()
    : d(new KMemoryInfoData)
{
}

KMemoryInfo::KMemoryInfo(const KMemoryInfo &info)
    : d(new KMemoryInfoData(*info.d))
{
}

KMemoryInfo::~KMemoryInfo()
{
    delete d;
}

KMemoryInfo& KMemoryInfo::operator=(const KMemoryInfo &info)
{
    if (this != &info) {
        *d = *info.d;
    }
    return *this;
}

qint64 KMemoryInfo::detail(KMemoryInfo::MemoryDetail detail) const
{
    switch (detail) {
    case KMemoryInfo::TotalRam:
        return d->totalRam;
    case KMemoryInfo::FreeRam:
        return d->freeRam;
    case KMemoryInfo::SharedRam:
        return d->sharedRam;
    case KMemoryInfo::BufferRam:
        return d->bufferRam;
    case KMemoryInfo::TotalSwap:
        return d->totalSwap;
    case KMemoryInfo::FreeSwap:
        return d->freeSwap;
    }

    return -1;
}

QDateTime KMemoryInfo::lastUpdate() const
{
    return d->lastUpdate;
}

KMemoryInfo::MemoryDetails KMemoryInfo::requestedDetails() const
{
    return d->details;
}

bool KMemoryInfo::update(KMemoryInfo::MemoryDetails details)
{
    d->clear();
    d->details = details;
    const bool res = fillMemoryInfo(d);
    d->lastUpdate = QDateTime::currentDateTime();
    return res;
}

#if defined(Q_OS_LINUX)
#  include "kmemoryinfo_linux.cpp"
#elif defined(Q_OS_WIN32)
#  include "kmemoryinfo_win.cpp"
#else

#ifdef __GNUC__
#warning Using the null memory implementation of fillMemoryInfo()
#warning implement it for the current OS
#endif
static bool fillMemoryInfo(KMemoryInfoData *data)
{
    Q_UNUSED(data)
    return false;
}

#endif

