/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
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

#include "entryinternal.h"

#include <QtCore/QStringList>
#include <QtGui/QImage>
#include <kdebug.h>

#include "core/xmlloader.h"

#include <knewstuff3/entry_p.h>


using namespace KNS3;

class EntryInternal::Private : public QSharedData
{
    public:
        Private()
        : mReleaseDate(QDate::currentDate())
        , mRating(0)
        , mDownloadCount(0)
        , mNumberFans(0)
        , mNumberKnowledgebaseEntries(0)
        , mStatus(Entry::Invalid)
        , mSource(EntryInternal::Online)
        {}

        bool operator==(const Private& other) const
        {
            return mUniqueId == other.mUniqueId && mProviderId == other.mProviderId;
        }

        QString mUniqueId;
        QString mName;
        KUrl mHomepage;
        QString mCategory;
        QString mLicense;
        QString mVersion;
        QDate mReleaseDate;

        // Version and date if a newer version is found (updateable)
        QString mUpdateVersion;
        QDate mUpdateReleaseDate;

        Author mAuthor;
        int mRating;
        int mDownloadCount;
        int mNumberFans;
        int mNumberKnowledgebaseEntries;
        QString mKnowledgebaseLink;
        QString mSummary;
        QString mChangelog;
        QString mPayload;
        QStringList mInstalledFiles;
        QString mProviderId;
        QStringList mUnInstalledFiles;
        QString mDonationLink;

        QString mChecksum;
        QString mSignature;
        Entry::Status mStatus;
        EntryInternal::Source mSource;

        QString mPreviewUrl[6];
        QImage mPreviewImage[6];

        QList<EntryInternal::DownloadLinkInformation> mDownloadLinkInformationList;
};

EntryInternal::EntryInternal()
    : d(new Private)
{
}

EntryInternal::EntryInternal(const EntryInternal& other)
    : d(other.d)
{
}

EntryInternal& EntryInternal::operator=(const EntryInternal& other)
{
    d = other.d;
    return *this;
}

bool EntryInternal::operator<(const KNS3::EntryInternal& other) const
{
    return d->mUniqueId < other.d->mUniqueId;
}

bool EntryInternal::operator==(const KNS3::EntryInternal& other) const
{
    return d->mUniqueId == other.d->mUniqueId && d->mProviderId == other.d->mProviderId;
}

EntryInternal::~EntryInternal()
{
}

bool EntryInternal::isValid() const
{
    return !d->mUniqueId.isEmpty();
}

QString EntryInternal::name() const
{
    return d->mName;
}

void EntryInternal::setName(const QString& name)
{
    d->mName = name;
}

QString EntryInternal::uniqueId() const
{
    return d->mUniqueId;
}

void EntryInternal::setUniqueId(const QString& id)
{
    d->mUniqueId = id;
}

QString EntryInternal::providerId() const
{
    return d->mProviderId;
}

void EntryInternal::setProviderId(const QString& id)
{
    d->mProviderId = id;
}

QString EntryInternal::category() const
{
    return d->mCategory;
}

void EntryInternal::setCategory(const QString& category)
{
    d->mCategory = category;
}

KUrl EntryInternal::homepage() const
{
    return d->mHomepage;
}

void EntryInternal::setHomepage(const KUrl& page)
{
    d->mHomepage = page;
}

Author EntryInternal::author() const
{
    return d->mAuthor;
}

void EntryInternal::setAuthor(const KNS3::Author& author)
{
    d->mAuthor = author;
}

QString EntryInternal::license() const
{
    return d->mLicense;
}

void EntryInternal::setLicense(const QString& license)
{
    d->mLicense = license;
}

QString EntryInternal::summary() const
{
    return d->mSummary;
}

void EntryInternal::setSummary(const QString& summary)
{
    d->mSummary = summary;
}

void EntryInternal::setChangelog(const QString& changelog)
{
    d->mChangelog = changelog;
}

QString EntryInternal::changelog() const
{
    return d->mChangelog;
}

QString EntryInternal::version() const
{
    return d->mVersion;
}

void EntryInternal::setVersion(const QString& version)
{
    d->mVersion = version;
}

QDate EntryInternal::releaseDate() const
{
    return d->mReleaseDate;
}

void EntryInternal::setReleaseDate(const QDate& releasedate)
{
    d->mReleaseDate = releasedate;
}

QString EntryInternal::payload() const
{
    return d->mPayload;
}

void EntryInternal::setPayload(const QString& url)
{
    d->mPayload = url;
}

QDate EntryInternal::updateReleaseDate() const
{
    return d->mUpdateReleaseDate;
}

void EntryInternal::setUpdateReleaseDate(const QDate& releasedate)
{
    d->mUpdateReleaseDate = releasedate;
}

QString EntryInternal::updateVersion() const
{
    return d->mUpdateVersion;
}

void EntryInternal::setUpdateVersion(const QString& version)
{
    d->mUpdateVersion = version;
}

QString EntryInternal::previewUrl(PreviewType type) const
{
    return d->mPreviewUrl[type];
}

void EntryInternal::setPreviewUrl(const QString& url, PreviewType type)
{
    d->mPreviewUrl[type] = url;
}

QImage EntryInternal::previewImage(PreviewType type) const
{
    return d->mPreviewImage[type];
}

void EntryInternal::setPreviewImage(const QImage& image, PreviewType type)
{
    d->mPreviewImage[type] = image;
}

int EntryInternal::rating() const
{
    return d->mRating;
}

void EntryInternal::setRating(int rating)
{
    d->mRating = rating;
}

int EntryInternal::downloadCount() const
{
    return d->mDownloadCount;
}

void EntryInternal::setDownloadCount(int downloads)
{
    d->mDownloadCount = downloads;
}

int EntryInternal::numberFans() const
{
    return d->mNumberFans;
}

void EntryInternal::setNumberFans(int fans)
{
    d->mNumberFans = fans;
}

QString EntryInternal::donationLink() const
{
    return d->mDonationLink;
}

void EntryInternal::setDonationLink(const QString& link)
{
    d->mDonationLink = link;
}

int EntryInternal::numberKnowledgebaseEntries() const
{
    return d->mNumberKnowledgebaseEntries;
}
void EntryInternal::setNumberKnowledgebaseEntries(int num)
{
    d->mNumberKnowledgebaseEntries = num;
}

QString EntryInternal::knowledgebaseLink() const
{
    return d->mKnowledgebaseLink;
}
void EntryInternal::setKnowledgebaseLink(const QString& link)
{
    d->mKnowledgebaseLink = link;
}


/*
QString EntryInternal::checksum() const
{

    return d->mChecksum;
}

QString EntryInternal::signature() const
{

    return d->mSignature;
}
*/

EntryInternal::Source EntryInternal::source() const
{
    return d->mSource;
}

void EntryInternal::setSource(Source source)
{
    d->mSource = source;
}

Entry::Status EntryInternal::status() const
{
    return d->mStatus;
}

void EntryInternal::setStatus(Entry::Status status)
{
    d->mStatus = status;
}

void KNS3::EntryInternal::setInstalledFiles(const QStringList & files)
{
    d->mInstalledFiles = files;
}

QStringList KNS3::EntryInternal::installedFiles() const
{
    return d->mInstalledFiles;
}

void KNS3::EntryInternal::setUnInstalledFiles(const QStringList & files)
{
    d->mUnInstalledFiles = files;
}

QStringList KNS3::EntryInternal::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}

int KNS3::EntryInternal::downloadLinkCount() const
{
    return d->mDownloadLinkInformationList.size();
}

QList<KNS3::EntryInternal::DownloadLinkInformation> KNS3::EntryInternal::downloadLinkInformationList() const
{
    return d->mDownloadLinkInformationList;
}

void KNS3::EntryInternal::appendDownloadLinkInformation(const KNS3::EntryInternal::DownloadLinkInformation& info)
{
    d->mDownloadLinkInformationList.append(info);
}

void EntryInternal::clearDownloadLinkInformation()
{
    d->mDownloadLinkInformationList.clear();
}

bool KNS3::EntryInternal::setEntryXML(const QDomElement & xmldata)
{
    if (xmldata.tagName() != "stuff") {
        kWarning() << "Parsing Entry from invalid XML";
        return false;
    }

    d->mCategory = xmldata.attribute("category");

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "name") {
            // TODO maybe do something with the language attribute? QString lang = e.attribute("lang");
            d->mName = e.text().trimmed();
        } else if (e.tagName() == "author") {
            QString email = e.attribute("email");
            QString jabber = e.attribute("im");
            QString homepage = e.attribute("homepage");
            d->mAuthor.setName(e.text().trimmed());
            d->mAuthor.setEmail(email);
            d->mAuthor.setJabber(jabber);
            d->mAuthor.setHomepage(homepage);
        } else if (e.tagName() == "providerid") {
            d->mProviderId = e.text();
        } else if (e.tagName() == "homepage") {
            d->mHomepage = e.text();
        } else if (e.tagName() == "licence") { // krazy:exclude=spelling
            d->mLicense = e.text().trimmed();
        } else if (e.tagName() == "summary") {
            d->mSummary = e.text();
        } else if (e.tagName() == "changelog") {
            d->mChangelog = e.text();
        } else if (e.tagName() == "version") {
            d->mVersion = e.text().trimmed();
        } else if (e.tagName() == "releasedate") {
            d->mReleaseDate = QDate::fromString(e.text().trimmed(), Qt::ISODate);
        } else if (e.tagName() == "preview") {
            // TODO support for all 6 image links
            d->mPreviewUrl[PreviewSmall1] = e.text().trimmed();
        } else if (e.tagName() == "previewBig") {
            d->mPreviewUrl[PreviewBig1] = e.text().trimmed();
        } else if (e.tagName() == "payload") {
            d->mPayload = e.text().trimmed();
        } else if (e.tagName() == "rating") {
            d->mRating = e.text().toInt();
        } else if (e.tagName() == "downloads") {
            d->mDownloadCount = e.text().toInt();
        } else if (e.tagName() == "category") {
            d->mCategory = e.text();
        } else if (e.tagName() == "signature") {
            d->mSignature = e.text();
        } else if (e.tagName() == "checksum") {
            d->mChecksum = e.text();
        } else if (e.tagName() == "installedfile") {
            d->mInstalledFiles.append(e.text());
        } else if (e.tagName() == "id") {
            d->mUniqueId = e.text();
        } else if (e.tagName() == "status") {
            QString statusText = e.text();
            if (statusText == "installed") {
                kDebug() << "Found an installed entry in registry";
                d->mStatus = Entry::Installed;
            } else if (statusText == "updateable") {
                d->mStatus = Entry::Updateable;
            }
        }
    }

    // Validation
    if (d->mName.isEmpty()) {
        kWarning(550) << "Entry: no name given";
        return false;
    }

    if (d->mUniqueId.isEmpty()) {
        if (!d->mPayload.isEmpty()) {
            d->mUniqueId = d->mPayload;
        } else {
            d->mUniqueId = d->mName;
        }
    }

    if (d->mPayload.isEmpty()) {
        kWarning(550) << "Entry: no payload URL given for: " << d->mName << " - " << d->mUniqueId;
        return false;
    }
    return true;
}

/**
 * get the xml string for the entry
 */
QDomElement KNS3::EntryInternal::entryXML() const
{
    Q_ASSERT(!d->mUniqueId.isEmpty());
    Q_ASSERT(!d->mProviderId.isEmpty());

    QDomDocument doc;

    QDomElement el = doc.createElement("stuff");
    el.setAttribute("category", d->mCategory);

    QString name = d->mName;

    QDomElement e;
    e = addElement(doc, el, "name", name);
    // todo: add language attribute
    (void)addElement(doc, el, "providerid", d->mProviderId);

    QDomElement author = addElement(doc, el, "author", d->mAuthor.name());
    if (!d->mAuthor.email().isEmpty())
        author.setAttribute("email", d->mAuthor.email());
    if (!d->mAuthor.homepage().isEmpty())
        author.setAttribute("homepage", d->mAuthor.homepage());
    if (!d->mAuthor.jabber().isEmpty())
        author.setAttribute("im", d->mAuthor.jabber());
    // FIXME: 'jabber' or 'im'? consult with kopete guys...
    addElement(doc, el, "homepage", d->mHomepage.url());
    (void)addElement(doc, el, "licence", d->mLicense); // krazy:exclude=spelling
    (void)addElement(doc, el, "version", d->mVersion);
    if ((d->mRating > 0) || (d->mDownloadCount > 0)) {
        (void)addElement(doc, el, "rating", QString::number(d->mRating));
        (void)addElement(doc, el, "downloads", QString::number(d->mDownloadCount));
    }
    if (!d->mSignature.isEmpty()) {
        (void)addElement(doc, el, "signature", d->mSignature);
    }
    if (!d->mChecksum.isEmpty()) {
        (void)addElement(doc, el, "checksum", d->mChecksum);
    }
    foreach(const QString &file, d->mInstalledFiles) {
        (void)addElement(doc, el, "installedfile", file);
    }
    if (!d->mUniqueId.isEmpty()) {
        addElement(doc, el, "id", d->mUniqueId);
    }

    (void)addElement(doc, el, "releasedate",
                     d->mReleaseDate.toString(Qt::ISODate));

    e = addElement(doc, el, "summary", d->mSummary);
    e = addElement(doc, el, "changelog", d->mChangelog);
    e = addElement(doc, el, "preview", d->mPreviewUrl[PreviewSmall1]);
    e = addElement(doc, el, "previewBig", d->mPreviewUrl[PreviewBig1]);
    e = addElement(doc, el, "payload", d->mPayload);

    if (d->mStatus == Entry::Installed) {
        (void)addElement(doc, el, "status", "installed");
    }
    if (d->mStatus == Entry::Updateable) {
        (void)addElement(doc, el, "status", "updateable");
    }

    return el;
}

Entry EntryInternal::toEntry() const
{
    Entry e;
    e.d->e = *this;
    return e;
}

KNS3::EntryInternal EntryInternal::fromEntry(const KNS3::Entry& entry)
{
    return entry.d->e;
}

QString KNS3::replaceBBCode(const QString& unformattedText)
{
    QString text(unformattedText);
    text.replace("[b]", "<b>");
    text.replace("[/b]", "</b>");
    text.replace("[i]", "<i>");
    text.replace("[/i]", "</i>");
    text.replace("[u]", "<i>");
    text.replace("[/u]", "</i>");
    text.replace("\\\"", "\"");
    text.replace("\\\'", "\'");
    text.replace("[li]", "* "); // TODO: better replacement for list elements?
    text.remove("[/li]");
    text.remove("[url]");
    text.remove("[/url]");
    return text;
}
