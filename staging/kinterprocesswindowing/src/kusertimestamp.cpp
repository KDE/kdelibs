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

#include "config-kinterprocesswindowing.h"

#if HAVE_X11
#include <qx11info_x11.h>
#include "kwindowsystem.h"
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

void KUserTimestamp::updateUserTimestamp(int time)
{
#if HAVE_X11
    if (time == 0) { // get current X timestamp
        Window w = XCreateSimpleWindow( QX11Info::display(), QX11Info::appRootWindow(), 0, 0, 1, 1, 0, 0, 0 );
        XSelectInput( QX11Info::display(), w, PropertyChangeMask );
        unsigned char data[ 1 ];
        XChangeProperty( QX11Info::display(), w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
        XEvent ev;
        XWindowEvent( QX11Info::display(), w, PropertyChangeMask, &ev );
        time = ev.xproperty.time;
        XDestroyWindow( QX11Info::display(), w );
    }

    if (QX11Info::appUserTime() == 0
     || NET::timestampCompare(time, QX11Info::appUserTime()) > 0) // time > appUserTime
        QX11Info::setAppUserTime(time);
    if (QX11Info::appTime() == 0
     || NET::timestampCompare(time, QX11Info::appTime()) > 0) // time > appTime
        QX11Info::setAppTime(time);
#endif
}

