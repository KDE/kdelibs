/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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
#ifndef _KIDNA_H
#define _KIDNA_H

#include <qstring.h>
#include "kdelibs_export.h"

namespace KIDNA {
    /**
     * Converts an International Domain Name @p idna to 
     * its ASCII representation
     *
     * If conversion is not possible, an empty string is returned.
     */
    KDECORE_EXPORT QCString toAsciiCString(const QString &idna);

    /**
     * Converts an International Domain Name @p idna to 
     * its ASCII representation
     *
     * If conversion is not possible, an empty string is returned.
     */
    KDECORE_EXPORT QString toAscii(const QString &idna);

    /**
     * Converts an International Domain Name @p idna to 
     * its UNICODE representation
     */
    KDECORE_EXPORT QString toUnicode(const QString &idna);
}

#endif /* _KIDNA_H */
