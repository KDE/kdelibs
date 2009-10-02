/*
    knewstuff3/provider.cpp
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

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

#include "provider.h"

#include "core/feed.h"

#include <kdebug.h>
#include <kio/job.h>

namespace KNS
{

class ProviderPrivate
{
public:
    ProviderPrivate() {}

    KTranslatable name;
    KUrl icon;
    // cache of all entries known from this provider so far, mapped by their id
    QMap<QString, Entry*> entries;
    QMap<QString, Feed*> feeds;

};

Provider::Provider()
        : d(new ProviderPrivate)
{
}

Provider::~Provider()
{
    delete d;
}

void Provider::setProviderData(const QString & xmldata)
{
    
}

KTranslatable Provider::name() const
{
    return d->name;
}

KUrl Provider::icon() const
{
    return d->icon;
}

QStringList Provider::availableFeeds() const
{
    return d->feeds.keys();
}

void Provider::loadFeed(const QString & feedname, int page)
{
}

}

#include "provider.moc"
