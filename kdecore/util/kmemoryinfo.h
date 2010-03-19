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

#ifndef KMEMORYINFO_H
#define KMEMORYINFO_H

#include <kdecore_export.h>

class QDateTime;
class KMemoryInfoData;

/**
 * KMemoryInfo provides a simply way to query for the memory information
 * (RAM) of the current system.
 *
 * A typical usage is constructing a KMemoryInfo object, and requesting an
 * update of the memory details you are interested in. For example, knowing
 * the total and the free memory:
 *
 * @code
 * KMemoryInfo info;
 * if (info.update(KMemoryInfo::TotalRam | KMemoryInfo::FreeRam)) {
 *   // the updating succeeded, we can use its data
 *   qint64 totalRam = info.detail(KMemoryInfo::TotalRam);
 *   qint64 freeRam = info.detail(KMemoryInfo::FreeRam);
 *   // ...
 * } else {
 *   // updating failed
 * }
 * @endcode
 *
 * @since 4.5
 */
class KDECORE_EXPORT KMemoryInfo
{
public:
    /**
     * A detail of memory.
     */
    enum MemoryDetail
    {
        TotalRam = 1,
        FreeRam = 2,
        SharedRam = 4,
        BufferRam = 8,
        CachedRam = 16,
        TotalSwap = 32,
        FreeSwap = 64
    };
    Q_DECLARE_FLAGS(MemoryDetails, MemoryDetail)

    /**
     * Constructs a memory information object which has no updated information.
     * @see update()
     */
    explicit KMemoryInfo();
    /**
     * Copy constructor.
     */
    KMemoryInfo(const KMemoryInfo &info);
    /**
     * Destructor.
     */
    ~KMemoryInfo();

    /**
     * Assignment operator.
     */
    KMemoryInfo& operator=(const KMemoryInfo &info);

    /**
     * Returns the specified memory @p detail, as it was read by the last
     * update().
     * @returns the value of the specified detail if available, or -1 if that
     *          detail was not requested in the last update()
     */
    qint64 detail(MemoryDetail detail) const;
    /**
     * Returns the timestamp of the last update, or a null one if the current
     * memory information was never updated.
     */
    QDateTime lastUpdate() const;
    /**
     * Returns the information requested with the last update.
     */
    MemoryDetails requestedDetails() const;

    /**
     * Request an update of the specified @p details of the system memory.
     * @returns whether the update was successful
     */
    bool update(MemoryDetails details);

private:
    KMemoryInfoData *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KMemoryInfo::MemoryDetails)

#endif
