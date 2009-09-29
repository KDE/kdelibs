/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "providerbase.h"

#include "core/feed.h"

#include <kdebug.h>
#include <kio/job.h>

namespace KNS
{

class ProviderBasePrivate
{
public:
    ProviderBasePrivate() {}

    KTranslatable name;
    KUrl icon;
    // cache of all entries known from this provider so far, mapped by their id
    QMap<QString, Entry*> entries;
    QMap<QString, Feed*> feeds;

};

ProviderBase::ProviderBase()
        : d(new ProviderBasePrivate)
{
}

ProviderBase::~ProviderBase()
{
    delete d;
}

void ProviderBase::setProviderData(const QString & xmldata)
{
    
}

KTranslatable ProviderBase::name() const
{
    return d->name;
}

KUrl ProviderBase::icon() const
{
    return d->icon;
}

QStringList ProviderBase::availableFeeds() const
{
    return d->feeds.keys();
}

void ProviderBase::loadFeed(const QString & feedname, int page)
{
}

}
