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

#include "historyprovider.h"

#include <QtCore/QSet>

#include <kapplication.h>

using namespace KParts;

class KParts::HistoryProviderPrivate
{
public:
    HistoryProviderPrivate()
        : q(0)
    {
    }

    ~HistoryProviderPrivate()
    {
        delete q;
    }

    QSet<QString> dict;
    HistoryProvider *q;
};

K_GLOBAL_STATIC(HistoryProviderPrivate, historyProviderPrivate)

HistoryProvider * HistoryProvider::self()
{
    if (!historyProviderPrivate->q) {
        new HistoryProvider;
    }

    return historyProviderPrivate->q;
}

bool HistoryProvider::exists()
{
    return historyProviderPrivate->q;
}

HistoryProvider::HistoryProvider( QObject *parent )
    : QObject( parent ), d(historyProviderPrivate)
{
    Q_ASSERT(!historyProviderPrivate->q);
    historyProviderPrivate->q = this;
    setObjectName("history provider");
}

HistoryProvider::~HistoryProvider()
{
    if (!historyProviderPrivate.isDestroyed() &&
        historyProviderPrivate->q == this)
        historyProviderPrivate->q = 0;
}

bool HistoryProvider::contains( const QString& item ) const
{
    return d->dict.contains( item );
}

void HistoryProvider::insert( const QString& item )
{
    d->dict.insert( item );
    emit inserted( item );
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
