/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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

#include "entry.h"

using namespace KNS;

class EntryPrivate
{
  public:
  EntryPrivate(){}
};

Entry::Entry() :
  mReleaseDate(QDate::currentDate()),
  mRelease(0),
  mRating(0),
  mDownloads(0)
{
  d = NULL;
}

Entry::~Entry()
{
}

void Entry::setName(const KTranslatable& name)
{
  mName = name;
}

KTranslatable Entry::name() const
{
  return mName;
}

void Entry::setType(const QString& type)
{
  mType = type;
}

QString Entry::type() const
{
  return mType;
}

void Entry::setAuthor(const Author &author)
{
  mAuthor = author;
}

Author Entry::author() const
{
  return mAuthor;
}

void Entry::setLicense(const QString &license)
{
  mLicense = license;
}

QString Entry::license() const
{
  return mLicense;
}

void Entry::setSummary(const KTranslatable &text)
{
  mSummary = text;
}

KTranslatable Entry::summary() const
{
  return mSummary;
}

void Entry::setVersion(const QString& version)
{
  mVersion = version;
}

QString Entry::version() const
{
  return mVersion;
}

void Entry::setRelease(int release)
{
  mRelease = release;
}

int Entry::release() const
{
  return mRelease;
}

void Entry::setReleaseDate(const QDate& d)
{
  mReleaseDate = d;
}

QDate Entry::releaseDate() const
{
  return mReleaseDate;
}

void Entry::setPayload(const KTranslatable& url)
{
  mPayload = url;
}

KTranslatable Entry::payload() const
{
  return mPayload;
}

void Entry::setPreview(const KTranslatable& url)
{
  mPreview = url;
}

KTranslatable Entry::preview() const
{
  return mPreview;
}

void Entry::setRating(int rating)
{
  mRating = rating;
}

int Entry::rating() const
{
  return mRating;
}

void Entry::setDownloads(int downloads)
{
  mDownloads = downloads;
}

int Entry::downloads() const
{
  return mDownloads;
}

