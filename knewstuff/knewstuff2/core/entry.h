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
#ifndef KNEWSTUFF2_ENTRY_H
#define KNEWSTUFF2_ENTRY_H

#include <knewstuff2/author.h>
#include <knewstuff2/ktranslatable.h>

#include <kurl.h>

#include <qdatetime.h>
#include <qstring.h>

namespace KNS {

/**
 * @short KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class Entry
{
  public:
    /**
     * Constructor.
     */
    Entry();

    /**
     * Destructor.
     */
    ~Entry();

    /**
     * Sets the name for this data object.
     */
    void setName(const KTranslatable& name);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name (potentially translated)
     */
    KTranslatable name() const;

    /**
     * Sets the application type, e.g. 'kdesktop/wallpaper'.
     */
    void setType(const QString& type);

    /**
     * Retrieve the type of the data object.
     *
     * @return object type
     */
    QString type() const;

    /**
     * Sets the author of the object.
     */
    void setAuthor(const Author& author);

    /**
     * Retrieve the author of the object.
     *
     * @return object author
     */
    Author author() const;

    /**
     * Sets the license (abbreviation) applicable to the object.
     */
    void setLicense(const QString& license);

    /**
     * Retrieve the license name of the object.
     *
     * @return object license
     */
    QString license() const;

    /**
     * Sets a short description on what the object is all about.
     */
    void setSummary(const KTranslatable& summary);

    /**
     * Retrieve a short description about the object.
     *
     * @return object description
     */
    KTranslatable summary() const;

    /**
     * Sets the version number.
     */
    void setVersion(const QString& version);

    /**
     * Retrieve the version string of the object.
     *
     * @return object version
     */
    QString version() const;

    /**
     * Sets the release number, which is increased for feature-equal objects
     * with the same version number, but slightly updated contents.
     */
    void setRelease(int release);

    /**
     * Retrieve the release number of the object
     *
     * @return object release
     */
    int release() const;

    /**
     * Sets the release date.
     */
    void setReleaseDate(const QDate& releasedate);

    /**
     * Retrieve the date of the object's publication.
     *
     * @return object release date
     */
    QDate releaseDate() const;

    /**
     * Sets the object's file.
     */
    void setPayload(const KTranslatable& url);

    /**
     * Retrieve the file name of the object.
     *
     * @return object filename
     */
    KTranslatable payload() const;

    /**
     * Sets the object's preview file, if available. This should be a
     * picture file.
     */
    void setPreview(const KTranslatable& url);

    /**
     * Retrieve the file name of an image containing a preview of the object.
     *
     * @return object preview filename
     */
    KTranslatable preview() const;

    /**
     * Sets the rating between 0 (worst) and 100 (best).
     *
     * @internal
     */
    void setRating(int rating);

    /**
     * Retrieve the rating for the object, which has been determined by its
     * users and thus might change over time.
     *
     * @return object rating
     */
    int rating() const;

    /**
     * Sets the number of downloads.
     * 
     * @internal
     */
    void setDownloads(int downloads);

    /**
     * Retrieve the download count for the object, which has been determined
     * by its hosting sites and thus might change over time.
     *
     * @return object download count
     */
    int downloads() const;

  private:
    QString mType;
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

    class EntryPrivate *d;
};

}

#endif
