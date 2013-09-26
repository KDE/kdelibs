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

#include "kusertimestamp.h"

#include "config-kwindowsystem.h"

#if HAVE_X11
#include <netwm.h>
#include <QX11Info>
#endif

unsigned long KUserTimestamp::userTimestamp()
{
#if HAVE_X11
    return QX11Info::appUserTime();
#else
    return 0;
#endif
}

void KUserTimestamp::updateUserTimestamp(unsigned long time)
{
#if HAVE_X11
    if (time == 0) { // get current X timestamp
        time = QX11Info::getTimestamp();
    }

    if (QX11Info::appUserTime() == 0
     || NET::timestampCompare(time, QX11Info::appUserTime()) > 0) // time > appUserTime
        QX11Info::setAppUserTime(time);
    if (QX11Info::appTime() == 0
     || NET::timestampCompare(time, QX11Info::appTime()) > 0) // time > appTime
        QX11Info::setAppTime(time);
#endif
}

