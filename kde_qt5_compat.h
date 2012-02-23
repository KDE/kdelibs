/* This file is part of the KDE libraries
   Copyright (C) 2012 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef KDE_QT5_COMPAT_H
#define KDE_QT5_COMPAT_H

#include <qglobal.h>

/* This file is only useful while kdelibs-frameworks must compile with both Qt4 and Qt5.
 * It will be removed once we depend on Qt5 alone.
 *
 * Put here any workarounds for source-incompatible differences, which can be solved in a header file only.
 */

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define SkipSingle 0
#define SkipAll 0
#define QSKIP_PORTING(message, argument) QSKIP(message)
#else
#define QSKIP_PORTING(message, argument) QSKIP(message, argument)
#endif

#endif
