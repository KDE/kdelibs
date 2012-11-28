/* This file is part of the KDE libraries
    Copyright (c) 2003 Luboš Luňák <l.lunak@kde.org>

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

#ifndef KUSERTIMESTAMP_H
#define KUSERTIMESTAMP_H

#include <kinterprocesswindowing_export.h>

namespace KUserTimestamp
{
    /**
     * Returns the last user action timestamp or 0 if no user activity has taken place yet.
     * @see updateuserTimestamp
     */
    KINTERPROCESSWINDOWING_EXPORT unsigned long userTimestamp();

    /**
     * Updates the last user action timestamp to the given time, or to the current time,
     * if 0 is given. Do not use unless you're really sure what you're doing.
     * Consult focus stealing prevention section in kdebase/kwin/README.
     */
    KINTERPROCESSWINDOWING_EXPORT void updateUserTimestamp(int time = 0);
};

#endif

