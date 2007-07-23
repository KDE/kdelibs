/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2000-2002 Stephan Kulow (coolo@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

#ifndef _KDEBUG_H_
#define _KDEBUG_H_

#include <kdeui_export.h>

#include <kdebugcore.h>

class QWidget;
class QColor;
class QPen;
class QBrush;

/** Operator to print out basic information about a QWidget.
 *  Output of class names only works if the class is moc'ified.
 * @param widget the widget to print
 * @return this stream
 */
KDEUI_EXPORT kdbgstream& operator<<( kdbgstream& stream, const QWidget* object );

/**
 * Prints the given value.
 * @param region the QRegion to print
 * @return this stream
 */
KDEUI_EXPORT kdbgstream& operator << ( kdbgstream& stream, const QRegion& region);

/**
 * Prints the given value.
 * @param color the color to print
 * @return this stream
 */
KDEUI_EXPORT kdbgstream& operator << ( kdbgstream& stream, const QColor& color);

/**
 * Prints the given value.
 * @param pen the pen to print
 * @return this stream
 */
KDEUI_EXPORT kdbgstream& operator << ( kdbgstream& stream, const QPen& pen );

/**
 * Prints the given value.
 * @param brush the brush to print
 * @return this stream
 */
KDEUI_EXPORT kdbgstream& operator << ( kdbgstream& stream, const QBrush& brush );

#endif
