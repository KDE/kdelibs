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

#include <CoreFoundation/CoreFoundation.h>

class KLocaleMacPrivate : public KLocalePrivate
{
public:
    KLocaleMacPrivate(KLocale *q_ptr, KSharedConfig::Ptr config);

    KLocaleMacPrivate(KLocale *q_ptr,
                      const QString &language, const QString &country, KConfig *config);

    KLocaleMacPrivate( const KLocaleMacPrivate &rhs );

    KLocaleMacPrivate &operator=( const KLocaleMacPrivate &rhs );

    virtual ~KLocaleMacPrivate();

protected:

    /**************************
     **   Country settings   **
     **************************/

    virtual QString systemCountry() const;

    /***************************
     **   Encoding settings   **
     ***************************/

    virtual QByteArray systemCodeset() const;

private:
    QString macLocaleValue( CFStringRef key ) const;
    CFLocaleRef m_macLocale;
};

#endif // KLOCALE_MAC_P_H
