/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_UTILS_H_
#define _NEPOMUK_QUERY_UTILS_H_

#include <QtCore/QList>
#include <QtCore/QHash>

template<typename T>
bool compareQList( const QList<T>& rp1, const QList<T>& rp2 ) {
    if( rp1.count() != rp2.count() )
        return false;
    foreach( const T& rp, rp1 ) {
        if ( !rp2.contains( rp ) ) {
            return false;
        }
    }
    return true;
}

template<typename Key, typename Value>
bool compareHash( const QHash<Key, Value>& h1, const QHash<Key, Value>& h2 ) {
    if( h1.count() != h2.count() )
        return false;
    for( typename QHash<Key, Value>::ConstIterator it1 = h1.constBegin();
         it1 != h1.constEnd(); ++it1 ) {
        typename QHash<Key, Value>::ConstIterator it2 = h2.find( it1.key() );
        if( it2 == h2.constEnd() ||
            *it1 != *it2 )
            return false;
    }
    return true;
}

#endif
