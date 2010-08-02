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

#include "klocale_mac_p.h"

KLocaleMacPrivate::KLocaleMacPrivate( KLocale *q_ptr, const QString &catalog, KConfig *config,
                                      const QString &language, const QString &country )
                  :KLocalePrivate( q_ptr, catalog, config, language, country )
{
}

KLocaleMacPrivate::KLocaleMacPrivate( const KLocaleMacPrivate &rhs )
                  :KLocalePrivate( rhs )
{
    KLocalePrivate::copy( rhs );
}

KLocaleMacPrivate &KLocaleMacPrivate::operator=( const KLocaleMacPrivate &rhs )
{
    KLocalePrivate::copy( rhs );
    return *this;
}

KLocaleMacPrivate::~KLocaleMacPrivate()
{
}

QByteArray KLocaleMacPrivate::systemCodeset() const
{
    return QByteArray( "UTF-8" );
}
