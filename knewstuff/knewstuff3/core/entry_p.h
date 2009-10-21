/*
    knewstuff3/entry.h.
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
#ifndef KNEWSTUFF3_ENTRY_P_H
#define KNEWSTUFF3_ENTRY_P_H

#include <knewstuff3/core/author.h>
#include <knewstuff3/core/ktranslatable.h>

#include <kurl.h>

#include <QtCore/QDate>
#include <QtCore/QString>

namespace KNS3
{
class EntryPrivate {
    public:
        EntryPrivate()
        : mReleaseDate(QDate::currentDate())
        , mRating(0)
        , mDownloads(0)
        , mStatus(Entry::Invalid)  
        , mSource(Entry::Online)
        {}
        
        virtual ~EntryPrivate() {}
        
        QString mCategory;
        QString mLicense;
        QString mVersion;
        QDate mReleaseDate;
        Author mAuthor;
        int mRating;
        int mDownloads;
        KTranslatable mName;
        KTranslatable mSummary;
        KTranslatable mPayload;
        KTranslatable mPreview;
        QStringList mInstalledFiles;
        QString mUniqueId;
        QString mProviderId;
        QStringList mUnInstalledFiles;
        
        QString mChecksum;
        QString mSignature;
        Entry::Status mStatus;
        Entry::Source mSource;
};
}

#endif
