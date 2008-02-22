/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Lubos Lunak (l.lunak@kde.org)

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

#ifndef KXUTILS_H
#define KXUTILS_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

#ifdef Q_WS_X11

#include <kdeui_export.h>

/**
 * Namespace with various generic X11-related functionality.
 */
namespace KXUtils
{

/**
 * Creates a QPixmap that contains a copy of the pixmap given by the X handle @p pixmap
 * and optionally also mask given as another X handle @mask. This function tries to
 * also handle the case when the depth of the pixmap differs from the native QPixmap depth.
 * @since 4.0.2
 */
KDEUI_EXPORT QPixmap createPixmapFromHandle( WId pixmap, WId mask = 0 );

/**
 * Compares two X timestamps, taking into account wrapping and 64bit architectures.
 * Return value is like with strcmp(), 0 for equal, -1 for time1 < time2, 1 for time1 > time2.
 * @since 4.1.0
 */
KDEUI_EXPORT int timestampCompare( unsigned long time1, unsigned long time2 );
/**
 * Returns a difference of two X timestamps, time2 - time1, where time2 must be later than time1,
 * as returned by timestampCompare().
 * @since 4.1.0
 */
KDEUI_EXPORT int timestampDiff( unsigned long time1, unsigned long time2 );

} // namespace

#endif // Q_WS_X11

#endif
