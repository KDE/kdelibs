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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QSet>

#include <kapplication.h>

#include "historyprovider.h"
#include <kstaticdeleter.h>

using namespace KParts;

HistoryProvider * HistoryProvider::s_self = 0L;
static KStaticDeleter<HistoryProvider> historyProviderSd;

class HistoryProvider::HistoryProviderPrivate
{
public:
    QSet<QString> dict;
};

HistoryProvider * HistoryProvider::self()
{
    if ( !s_self ) {
        historyProviderSd.setObject( s_self, new HistoryProvider() );
        s_self->setObjectName( "history provider" );
    }
    return s_self;
}

HistoryProvider::HistoryProvider( QObject *parent )
    : QObject( parent ),d(new HistoryProviderPrivate)
{
    if ( !s_self )
	s_self = this;

}

HistoryProvider::~HistoryProvider()
{
    delete d;

    if ( s_self == this )
	s_self = 0;
}

bool HistoryProvider::contains( const QString& item ) const
{
    return d->dict.contains( item );
}

void HistoryProvider::insert( const QString& item )
{
    d->dict.insert( item );
}

void HistoryProvider::remove( const QString& item )
{
    d->dict.remove( item );
}

void HistoryProvider::clear()
{
    d->dict.clear();
    emit cleared();
}

#include "historyprovider.moc"
