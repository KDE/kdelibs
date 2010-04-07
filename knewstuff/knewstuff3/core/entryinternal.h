/*
    knewstuff3/entry.h.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
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

#ifndef KNEWSTUFF3_ENTRY_H
#define KNEWSTUFF3_ENTRY_H

#include <QtCore/QDate>
#include <QtXml/QDomElement>
#include <QtCore/QString>

#include <kurl.h>

#include <knewstuff3/core/author.h>
#include <knewstuff3/entry.h>

namespace KNS3
{
    static const int PreviewWidth = 96;
    static const int PreviewHeight = 72;

    /**
     function to remove bb code formatting that opendesktop sends
     */
    QString replaceBBCode(const QString& unformattedText);

/**
 * @short KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 */
class EntryInternal
{
public:
    typedef QList<EntryInternal> List;

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

    enum PreviewType {
        PreviewSmall1,
        PreviewSmall2,
        PreviewSmall3,
        PreviewBig1,
        PreviewBig2,
        PreviewBig3
    };

    struct DownloadLinkInformation {
        QString name;
        QString priceAmount;
        QString distributionType;
        QString descriptionLink;
        int id;
        bool isDownloadtypeLink;
    };

    /**
     * Constructor.
     */
    EntryInternal();

    EntryInternal(const EntryInternal& other);
    EntryInternal& operator=(const EntryInternal& other);

    bool operator==(const EntryInternal& other) const;
    bool operator<(const EntryInternal& other) const;

    /**
     * Destructor.
     */
    ~EntryInternal();

    bool isValid() const;

    /**
     * Sets the name for this data object.
     */
    void setName(const QString& name);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name (potentially translated)
     */
    QString name() const;

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

    void setHomepage(const KUrl& page);
    KUrl homepage() const;

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
    void setSummary(const QString& summary);

    /**
     * Retrieve a short description about the object.
     *
     * @return object description
     */
    QString summary() const;

    /**
     * The user written changelog
     */
    void setChangelog(const QString& changelog);
    QString changelog() const;

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
     * Sets the version number that is available as update.
     */
    void setUpdateVersion(const QString& version);

    /**
     * Retrieve the version string of the object that is available as update.
     *
     * @return object version
     */
    QString updateVersion() const;

    /**
     * Sets the release date that is available as update.
     */
    void setUpdateReleaseDate(const QDate& releasedate);

    /**
     * Retrieve the date of the newer version that is available as update.
     *
     * @return object release date
     */
    QDate updateReleaseDate() const;

    /**
     * Sets the object's file.
     */
    void setPayload(const QString& url);

    /**
     * Retrieve the file name of the object.
     *
     * @return object filename
     */
    QString payload() const;

    /**
     * Sets the object's preview file, if available. This should be a
     * picture file.
     */
    void setPreviewUrl(const QString& url, PreviewType type = PreviewSmall1);

    /**
     * Retrieve the file name of an image containing a preview of the object.
     *
     * @return object preview filename
     */
    QString previewUrl(PreviewType type = PreviewSmall1) const;

    /**
     * This will not be loaded automatically, instead use Engine to load the actual images.
     */
    QImage previewImage(PreviewType type = PreviewSmall1) const;
    void setPreviewImage(const QImage& image, PreviewType type = PreviewSmall1);

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
    void setDownloadCount(int downloads);

    /**
     * Retrieve the download count for the object, which has been determined
     * by its hosting sites and thus might change over time.
     *
     * @return object download count
     */
    int downloadCount() const;

    int numberFans() const;
    void setNumberFans(int fans);
    
    int numberKnowledgebaseEntries() const;
    void setNumberKnowledgebaseEntries(int num);
    QString knowledgebaseLink() const;
    void setKnowledgebaseLink(const QString& link);

    int downloadLinkCount() const;
    QList<DownloadLinkInformation> downloadLinkInformationList() const;
    void appendDownloadLinkInformation(const DownloadLinkInformation& info);
    void clearDownloadLinkInformation();

    QString donationLink() const;
    void setDonationLink(const QString& link);
    
    /**
      The id of the provider this entry belongs to
      */
    QString providerId() const;
    void setProviderId(const QString& id);

    /**
      The source of this entry can be Cache, Registry or Online - @see source
      */
    void setSource(Source source);
    Source source() const;

    /**
     * set the xml for the entry
     * parses the xml and sets the private members accordingly
     * used to deserialize data loaded from provider
     *
     * @param xmldata string to load xml data from
     *
     * @returns whether or not setting the values was successful
     */
    bool setEntryXML(const QDomElement & xmldata);

    /**
    * get the xml string for the entry
    */
    QDomElement entryXML() const;

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
     * Sets the entry's status. If no status is set, the default will be
     * \ref Invalid.
     *
     * @param status New status of the entry
     */
    void setStatus(Entry::Status status);

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    Entry::Status status() const;

    //void setIdNumber(int number);
    //int idNumber() const;

    Entry toEntry() const;
    
    static KNS3::EntryInternal fromEntry(const KNS3::Entry& entry);
private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

inline uint qHash(const KNS3::EntryInternal& entry) {
    return qHash(entry.uniqueId());
}

}

#endif
