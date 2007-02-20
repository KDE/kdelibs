/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIGFLAGS_H_
#define KCONFIGFLAGS_H_

#include <qmap.h>

struct KEntry;
struct KEntryKey;
typedef QMap<KEntryKey, KEntry> KEntryMap;

class KConfigFlags
{
public:
    /**
     * Flags to control write entry
     */
    enum WriteConfigFlag
    {
        Persistent = 0x01,
        /**<
         * Save this entry when saving the config object.
         */
        Global = 0x02,
        /**<
         * Save the entry to the global %KDE config file instead of the
         * application specific config file.
         */
        Localized = 0x04,
        NLS = Localized,
        /**<
         * Add the locale tag to the key when writing it.
         */
        Normal=Persistent
        /**<
         * Save the entry to the application specific config file without
         * a locale tag. This is the default.
         */

    };
    Q_DECLARE_FLAGS(WriteConfigFlags, WriteConfigFlag)

};

#endif
