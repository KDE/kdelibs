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

#include "entry.h"
#include "knewstuff3/core/entry_p.h"

#include "xmlloader.h"

#include <kdebug.h>

using namespace KNS3;


Entry::Entry()
    : d_ptr(new EntryPrivate)
{
}

Entry::Entry(const Entry& other)
    : d_ptr(new EntryPrivate(*other.d_ptr))
{
}

Entry& Entry::operator=(const Entry & other)
{
    *d_ptr = *other.d_ptr;
    return *this;
}

Entry::~Entry()
{
    delete d_ptr;
}

KTranslatable Entry::name() const
{
    Q_D(const Entry);
    return d->mName;
}

QString Entry::uniqueId() const
{
    Q_D(const Entry);
    return d->mUniqueId;
}

void Entry::setUniqueId(const QString& id)
{
    Q_D(Entry);
    d->mUniqueId = id;
}

QString Entry::providerId() const
{
    Q_D(const Entry);
    return d->mProviderId;
}

void Entry::setProviderId(const QString& id)
{
    Q_D(Entry);
    d->mProviderId = id;
}

QString Entry::category() const
{
    Q_D(const Entry);
    return d->mCategory;
}

Author Entry::author() const
{
    Q_D(const Entry);
    return d->mAuthor;
}

QString Entry::license() const
{
    Q_D(const Entry);
    return d->mLicense;
}

KTranslatable Entry::summary() const
{
    Q_D(const Entry);
    return d->mSummary;
}

QString Entry::version() const
{
    Q_D(const Entry);
    return d->mVersion;
}

QDate Entry::releaseDate() const
{
    Q_D(const Entry);
    return d->mReleaseDate;
}

KTranslatable Entry::payload() const
{
    Q_D(const Entry);
    return d->mPayload;
}

KTranslatable Entry::preview() const
{
    Q_D(const Entry);
    return d->mPreview;
}

int Entry::rating() const
{
    Q_D(const Entry);
    return d->mRating;
}

int Entry::downloads() const
{
    Q_D(const Entry);
    return d->mDownloads;
}

/*
QString Entry::checksum() const
{
    Q_D(const Entry);
    return d->mChecksum;
}

QString Entry::signature() const
{
    Q_D(const Entry);
    return d->mSignature;
}
*/

Entry::Source Entry::source() const
{
    Q_D(const Entry);
    return d->mSource;
}

void Entry::setSource(Source source)
{
    Q_D(Entry);
    d->mSource = source;
}

Entry::Status Entry::status() const
{
    Q_D(const Entry);
    return d->mStatus;
}

void Entry::setStatus(Status status)
{
    Q_D(Entry);
    d->mStatus = status;
}

void KNS3::Entry::setInstalledFiles(const QStringList & files)
{
    Q_D(Entry);
    d->mInstalledFiles = files;
}

QStringList KNS3::Entry::installedFiles() const
{
    Q_D(const Entry);
    return d->mInstalledFiles;
}

void KNS3::Entry::setUnInstalledFiles(const QStringList & files)
{
    Q_D(Entry);
    d->mUnInstalledFiles = files;
}

QStringList KNS3::Entry::uninstalledFiles() const
{
    Q_D(const Entry);
    return d->mUnInstalledFiles;
}
