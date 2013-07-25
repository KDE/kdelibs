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

#ifndef KLOCALE_WIN_P_H
#define KLOCALE_WIN_P_H

#include "klocale_p.h"

#include <windows.h>

class KLocaleWindowsPrivate : public KLocalePrivate
{
public:
    KLocaleWindowsPrivate(KLocale *q_ptr, KSharedConfig::Ptr config);

    KLocaleWindowsPrivate(KLocale *q_ptr,
                          const QString &language, const QString &country, KConfig *config);

    KLocaleWindowsPrivate( const KLocaleWindowsPrivate &rhs );

    KLocaleWindowsPrivate &operator=( const KLocaleWindowsPrivate &rhs );

    virtual ~KLocaleWindowsPrivate();

protected:

    /**************************
     **   Country settings   **
     **************************/

    virtual QString systemCountry() const;

    /**************************
     **  Language settings   **
     **************************/

    virtual QStringList systemLanguageList() const;

    /***************************
     **   Encoding settings   **
     ***************************/

    virtual QByteArray systemCodeset() const;

    virtual const QByteArray encoding();

private:
    QString windowsLocaleValue( LCTYPE key ) const;
    LCID m_winLocaleId;
    // Encoding settings
    char m_win32SystemEncoding[3+7]; //"cp " + lang ID
};

#endif // KLOCALE_WIN_P_H
