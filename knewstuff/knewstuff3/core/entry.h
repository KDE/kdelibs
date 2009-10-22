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
#ifndef KNEWSTUFF3_ENTRY_H
#define KNEWSTUFF3_ENTRY_H

#include <knewstuff3/core/author.h>
#include <knewstuff3/core/ktranslatable.h>

#include <kurl.h>

#include <QtCore/QDate>
#include <QtXml/QDomElement>
#include <QtCore/QString>

namespace KNS3
{

class EntryPrivate;

/**
 * @short KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 */
class KNEWSTUFF_EXPORT Entry
{
public:
    typedef QList<Entry*> List;

    /**
     * Constructor.
     */
    Entry();

    Entry(const Entry& other);
    Entry& operator=(const Entry& other);

    /**
     * Destructor.
     */
    virtual ~Entry();

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

    void setUniqueId(const QString& id);
    QString uniqueId() const;
    
    /**
     * Sets the data category, e.g. 'kdesktop/wallpaper'.
     */
    void setCategory(const QString& category);

    /**
     * Retrieve the category of the data object.
     *
     * @return object category
     */
    QString category() const;

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
    // FIXME I'd prefer pixmap or icon here, any good reason for ktranslatable?
    // Acutally url also would be fine I guess ?

    /**
     * Retrieve the file name of an image containing a preview of the object.
     *
     * @return object preview filename
     */
    KTranslatable preview() const;

    /**
     * Set the files that have been installed by the install command.
     * @param files local file names
     */
    void setInstalledFiles(const QStringList& files);

    /**
     * Retrieve the locally installed files.
     * @return file names
     */
    QStringList installedFiles() const;

    /**
     * Set the files that have been uninstalled by the uninstall command.
     * @param files local file names
     * @since 4.1
     */
    void setUnInstalledFiles(const QStringList& files);

    /**
     * Retrieve the locally uninstalled files.
     * @return file names
     * @since 4.1
     */
    QStringList uninstalledFiles() const;

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

    // FIXME: below here, everything under consideration

    QString providerId() const;
    void setProviderId(const QString& id);

    /**
     * set the xml for the entry
     * parses the xml and sets the private members accordingly
     * used to deserialize data loaded from provider
     *
     * @param xmldata string to load xml data from
     *
     * @returns whether or not setting the values was successful
     */
    virtual bool setEntryXML(const QDomElement & xmldata);

    /**
    * get the xml string for the entry
    */
    virtual QDomElement entryXML() const;

    /**
     * Source of the entry, A entry's data is coming from either cache, or an online provider
     * this helps the engine know which data to use when merging cached entries with online
     * entry data
     */
    enum Source {
        Cache,
        Online,
        Registry
    };

    void setSource(Source source);
    Source source() const;

    /**
     * Returns the checksum for the entry.
     *
     * If an empty string is returned, no checksum was assigned.
     *
     * @return Checksum of this entry
     */
    //QString checksum() const;

    /**
     * Sets the checksum of the entry. This will be a string representation
     * of an MD5 sum of the entry's selected payload file.
     *
     * @ref checksum Checksum for the entry
     */
    //void setChecksum(const QString& checksum);

    /**
     * Returns the signature for the entry.
     *
     * If an empty string is returned, no signature was assigned.
     *
     * @return Signature of this entry
     */
    //QString signature() const;

    /**
     * Sets the signature of the entry. This will be a digital signature
     * in OpenPGP-compliant format.
     *
     * @ref signature Signature for the entry
     */
    //void setSignature(const QString& signature);
    
    /**
     * Status of the entry. An entry will be downloadable from the provider's
     * site prior to the download. Once downloaded and installed, it will
     * be either installed or updateable, implying an out-of-date
     * installation. Finally, the entry can be deleted and hence show up as
     * downloadable again.
     * Entries not taking part in this cycle, for example those in upload,
     * have an invalid status.
     */
    enum Status {
        Invalid,
        Downloadable,
        Installed,
        Updateable,
        Deleted
    };

    /**
     * Sets the entry's status. If no status is set, the default will be
     * \ref Invalid.
     *
     * @param status New status of the entry
     */
    void setStatus(Status status);

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    Status status() const;

    //void setIdNumber(int number);
    //int idNumber() const;
protected:
    EntryPrivate* const d_ptr;
    Entry(EntryPrivate &dd);
private:
    Q_DECLARE_PRIVATE(Entry);
};

}

#endif
