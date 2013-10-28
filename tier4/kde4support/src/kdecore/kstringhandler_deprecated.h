/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>
   Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
   Copyright (C) 2006 by Martin Pool <mbp@canonical.com>

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

#ifndef KSTRINGHANDLER_DEPRECATED_H
#define KSTRINGHANDLER_DEPRECATED_H

#include <QString>
#include <kde4support_export.h>

namespace KStringHandler
{

    /**
      Does a natural comparing of the strings. A negative value is returned if \a a
      is smaller than \a b. A positive value is returned if \a a is greater than \a b. 0
      is returned if both values are equal.

      @param a first string to compare
      @param b second string to compare
      @param caseSensitivity whether to use case sensitive compare or not

      @since 4.1
      @deprecated Should use QCollator instead
     */
    KDE4SUPPORT_DEPRECATED_EXPORT int naturalCompare( const QString& a, const QString& b, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive );
}

#endif
