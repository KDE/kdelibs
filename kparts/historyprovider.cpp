/* This file is part of the KDE project
   Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <q3dict.h>

#include <kapplication.h>

#include "historyprovider.h"

using namespace KParts;
template class Q3Dict<void>;

HistoryProvider * HistoryProvider::s_self = 0L;

class HistoryProvider::HistoryProviderPrivate
{
public:
    HistoryProviderPrivate()
	: dict( 1009 ) {}

    Q3Dict<void> dict;
};

HistoryProvider * HistoryProvider::self()
{
    if ( !s_self ) {
	s_self = new HistoryProvider( kapp );
        s_self->setObjectName( "history provider" );
    }
    return s_self;
}

HistoryProvider::HistoryProvider( QObject *parent )
    : QObject( parent )
{
    if ( !s_self )
	s_self = this;

    d = new HistoryProviderPrivate;
}

HistoryProvider::~HistoryProvider()
{
    delete d;

    if ( s_self == this )
	s_self = 0;
}

bool HistoryProvider::contains( const QString& item ) const
{
    return (bool) d->dict.find( item );
}

void HistoryProvider::insert( const QString& item )
{
    // no need to allocate memory, we only want to have fast lookup, no mapping
    d->dict.replace( item, (void*) 1 );
}

void HistoryProvider::remove( const QString& item )
{
    (void) d->dict.remove( item );
}

void HistoryProvider::clear()
{
    d->dict.clear();
    emit cleared();
}

void HistoryProvider::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "historyprovider.moc"
