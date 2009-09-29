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

#include "provider.h"

#include <kdebug.h>
#include <kio/job.h>

using namespace KNS;

class ProviderPrivate
{
public:
    ProviderPrivate() {}
};

Provider::Provider()
        : d(0)
{
}

Provider::~Provider()
{
    //qDeleteAll(mFeeds);
    //delete d;
}

void Provider::addDownloadUrlFeed(const QString& feedtype, Feed *feed)
{
    mFeeds[feedtype] = feed;
    // FIXME: check for "blessed" feed types?
}

Feed *Provider::downloadUrlFeed(const QString& feedtype) const
{
    if (mFeeds.contains(feedtype)) {
        return mFeeds[feedtype];
    } else {
        return NULL;
    }
}

void Provider::setName(const KTranslatable &name)
{
    mName = name;
}

KTranslatable Provider::name() const
{
    return mName;
}

void Provider::setIcon(const KUrl &url)
{
    mIcon = url;
}

KUrl Provider::icon() const
{
    return mIcon;
}

void Provider::setUploadUrl(const KUrl &url)
{
    mUploadUrl = url;
}

KUrl Provider::uploadUrl() const
{
    return mUploadUrl;
}

void Provider::setNoUploadUrl(const KUrl &url)
{
    mNoUploadUrl = url;
}

KUrl Provider::noUploadUrl() const
{
    return mNoUploadUrl;
}

void Provider::setWebAccess(const KUrl &url)
{
    mWebAccess = url;
}

KUrl Provider::webAccess() const
{
    return mWebAccess;
}

void Provider::setWebService(const KUrl &url)
{
    mWebService = url;
}

KUrl Provider::webService() const
{
    return mWebService;
}

QStringList Provider::feeds() const
{
    return mFeeds.keys();
}

