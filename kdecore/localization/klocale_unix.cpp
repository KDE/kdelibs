/* This file is part of the KDE libraries
   Copyright (C) 2010 John Layt <john@layt.net>

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

#include "klocale_unix_p.h"

KLocaleUnixPrivate::KLocaleUnixPrivate(KLocale *q_ptr, KSharedConfig::Ptr config)
                   :KLocalePrivate(q_ptr)
{
    init(QString(), QString(), config, 0);
}

KLocaleUnixPrivate::KLocaleUnixPrivate(KLocale *q_ptr,
                                       const QString &language,const QString &country, KConfig *config)
                   :KLocalePrivate(q_ptr)
{
    init(language, country, KSharedConfig::Ptr(), config);
}

KLocaleUnixPrivate::KLocaleUnixPrivate( const KLocaleUnixPrivate &rhs )
                  :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
}

KLocaleUnixPrivate &KLocaleUnixPrivate::operator=( const KLocaleUnixPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    return *this;
}

KLocaleUnixPrivate::~KLocaleUnixPrivate()
{
}
