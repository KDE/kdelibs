/*
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include <QtCore/QStringList>
#include <knewstuff3/entry_p.h>

using namespace KNS3;

Entry::Entry()
    : d(new Private)
{
}

Entry::Entry(const Entry& other)
    : d(other.d)
{
}

Entry& Entry::operator=(const Entry& other)
{
    d = other.d;
    return *this;
}

Entry::~Entry()
{
}

QString Entry::id() const
{
    return d->mId;
}

QString Entry::providerId() const
{
    return d->mProviderId;
}

QString Entry::name() const
{
    return d->mName;
}

QString Entry::category() const
{
    return d->mCategory;
}

QString Entry::license() const
{
    return d->mLicense;
}


QString Entry::summary() const
{
    return d->mSummary;
}

QString Entry::version() const
{
    return d->mVersion;
}

Entry::Status Entry::status() const
{
    return d->mStatus;
}

QStringList KNS3::Entry::installedFiles() const
{
    return d->mInstalledFiles;
}

QStringList KNS3::Entry::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}
