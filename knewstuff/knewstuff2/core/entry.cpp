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

using namespace KNS;

struct KNS::EntryPrivate {
    EntryPrivate() : mReleaseDate(QDate::currentDate())
            , mRelease(0)
            , mRating(0)
            , mDownloads(0)
            , mIdNumber(0)
            , mStatus(Entry::Invalid)
            , mSource(Entry::Online) {}

    QString mCategory;
    QString mLicense;
    QString mVersion;
    QDate mReleaseDate;
    Author mAuthor;
    int mRelease;
    int mRating;
    int mDownloads;
    KTranslatable mName;
    KTranslatable mSummary;
    KTranslatable mPayload;
    KTranslatable mPreview;
    QStringList mInstalledFiles;
    int mIdNumber;
    QStringList mUnInstalledFiles;

    QString mChecksum;
    QString mSignature;
    Entry::Status mStatus;
    Entry::Source mSource;
};

Entry::Entry()
        : d(new EntryPrivate)
{
}

Entry::Entry(const Entry& other)
        : d(new EntryPrivate(*other.d))
{
}

Entry& Entry::operator=(const Entry & other)
{
    *d = *other.d;
    return *this;
}

Entry::~Entry()
{
    delete d;
}

void Entry::setName(const KTranslatable& name)
{
    d->mName = name;
}

KTranslatable Entry::name() const
{
    return d->mName;
}

void Entry::setCategory(const QString& category)
{
    d->mCategory = category;
}

QString Entry::category() const
{
    return d->mCategory;
}

void Entry::setAuthor(const Author &author)
{
    d->mAuthor = author;
}

Author Entry::author() const
{
    return d->mAuthor;
}

void Entry::setLicense(const QString &license)
{
    d->mLicense = license;
}

QString Entry::license() const
{
    return d->mLicense;
}

void Entry::setSummary(const KTranslatable &text)
{
    d->mSummary = text;
}

KTranslatable Entry::summary() const
{
    return d->mSummary;
}

void Entry::setVersion(const QString& version)
{
    d->mVersion = version;
}

QString Entry::version() const
{
    return d->mVersion;
}

void Entry::setRelease(int release)
{
    d->mRelease = release;
}

int Entry::release() const
{
    return d->mRelease;
}

void Entry::setReleaseDate(const QDate& date)
{
    d->mReleaseDate = date;
}

QDate Entry::releaseDate() const
{
    return d->mReleaseDate;
}

void Entry::setPayload(const KTranslatable& url)
{
    d->mPayload = url;
}

KTranslatable Entry::payload() const
{
    return d->mPayload;
}

void Entry::setPreview(const KTranslatable& url)
{
    d->mPreview = url;
}

KTranslatable Entry::preview() const
{
    return d->mPreview;
}

void Entry::setRating(int rating)
{
    d->mRating = rating;
}

int Entry::rating() const
{
    return d->mRating;
}

void Entry::setDownloads(int downloads)
{
    d->mDownloads = downloads;
}

int Entry::downloads() const
{
    return d->mDownloads;
}

void Entry::setChecksum(const QString& checksum)
{
    d->mChecksum = checksum;
}

QString Entry::checksum() const
{
    return d->mChecksum;
}

void Entry::setSignature(const QString& signature)
{
    d->mSignature = signature;
}

QString Entry::signature() const
{
    return d->mSignature;
}

Entry::Status Entry::status()
{
    return d->mStatus;
}

void Entry::setStatus(Status status)
{
    d->mStatus = status;
}

Entry::Source Entry::source()
{
    return d->mSource;
}

void Entry::setSource(Source source)
{
    d->mSource = source;
}

void KNS::Entry::setInstalledFiles(const QStringList & files)
{
    d->mInstalledFiles = files;
}

QStringList KNS::Entry::installedFiles() const
{
    return d->mInstalledFiles;
}

void Entry::setIdNumber(int number)
{
    d->mIdNumber = number;
}

int Entry::idNumber() const
{
    return d->mIdNumber;
}

void KNS::Entry::setUnInstalledFiles(const QStringList & files)
{
    d->mUnInstalledFiles = files;
}

QStringList KNS::Entry::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}

