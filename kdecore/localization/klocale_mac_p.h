/*  This file is part of the KDE libraries
 *  Copyright 2010 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KLOCALE_MAC_P_H
#define KLOCALE_MAC_P_H

#include "klocale_p.h"

class KLocaleMacPrivate : public KLocalePrivate
{
public:
    KLocaleMacPrivate( KLocale *q, const QString &catalog, KConfig *config,
                       const QString &language = QString(), const QString &country = QString() );

    KLocaleMacPrivate( const KLocaleMacPrivate &rhs );

    KLocaleMacPrivate &operator=( const KLocaleMacPrivate &rhs );

    virtual ~KLocaleMacPrivate();

    /***************************
     **   Encoding settings   **
     ***************************/

    virtual QByteArray systemCodeset() const;

};

#endif // KLOCALE_MAC_P_H
