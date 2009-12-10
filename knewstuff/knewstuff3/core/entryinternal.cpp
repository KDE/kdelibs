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
        , mDownloads(0)
        , mStatus(EntryInternal::Invalid)
        , mSource(EntryInternal::Online)
        {}

        bool operator==(const Private& other) const
        {
            return mUniqueId == other.mUniqueId;
        }

        QString mUniqueId;
        QString mName;
        KUrl mHomepage;
        QString mCategory;
        QString mLicense;
        QString mVersion;
        QDate mReleaseDate;
        Author mAuthor;
        int mRating;
        int mDownloads;
        QString mSummary;
        QString mPayload;
        QString mPreview;
        QString mPreviewBig;
        QStringList mInstalledFiles;
        QString mProviderId;
        QStringList mUnInstalledFiles;

        QString mChecksum;
        QString mSignature;
        EntryInternal::Status mStatus;
        EntryInternal::Source mSource;
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

QString EntryInternal::previewSmall() const
{
    return d->mPreview;
}

void EntryInternal::setPreviewSmall(const QString& url)
{
    d->mPreview = url;
}

QString EntryInternal::previewBig() const
{
    return d->mPreviewBig;
}

void EntryInternal::setPreviewBig(const QString& url)
{
    d->mPreviewBig = url;
}

int EntryInternal::rating() const
{
    return d->mRating;
}

void EntryInternal::setRating(int rating)
{
    d->mRating = rating;
}

int EntryInternal::downloads() const
{
    return d->mDownloads;
}

void EntryInternal::setDownloads(int downloads)
{
    d->mDownloads = downloads;
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

EntryInternal::Status EntryInternal::status() const
{
    return d->mStatus;
}

void EntryInternal::setStatus(Status status)
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
            Author author;
            QString email = e.attribute("email");
            QString jabber = e.attribute("im");
            QString homepage = e.attribute("homepage");
            author.setName(e.text().trimmed());
            author.setEmail(email);
            author.setJabber(jabber);
            author.setHomepage(homepage);
            d->mAuthor = author;
        } else if (e.tagName() == "providerid") {
            d->mProviderId = e.text();
        } else if (e.tagName() == "homepage") {
            d->mHomepage = e.text();
        } else if (e.tagName() == "licence") { // krazy:exclude=spelling
            d->mLicense = e.text().trimmed();
        } else if (e.tagName() == "summary") {
            //QString lang = e.attribute("lang");
            //kDebug() << "adding " << e.tagName() << " to summary as language " << lang;
            d->mSummary = e.text().trimmed();
        } else if (e.tagName() == "version") {
            d->mVersion = e.text().trimmed();
        } else if (e.tagName() == "releasedate") {
            d->mReleaseDate = QDate::fromString(e.text().trimmed(), Qt::ISODate);
        } else if (e.tagName() == "preview") {
            //QString lang = e.attribute("lang");
            d->mPreview = e.text().trimmed();
        } else if (e.tagName() == "payload") {
            //QString lang = e.attribute("lang");
            d->mPayload = e.text().trimmed();
        } else if (e.tagName() == "rating") {
            d->mRating = e.text().toInt();
        } else if (e.tagName() == "downloads") {
            d->mDownloads = e.text().toInt();
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
                d->mStatus = EntryInternal::Installed;
            } else if (statusText == "updateable") {
                d->mStatus = EntryInternal::Updateable;
            }
            //kDebug() << "got id number: " << idNumber;
        }
    }

    // Validation

    if (d->mName.isEmpty()) {
        kWarning(550) << "Entry: no name given";
        return false;
    }

    if (d->mPayload.isEmpty()) {
        kWarning(550) << "Entry: no payload URL given";
        //return false;
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
    if ((d->mRating > 0) || (d->mDownloads > 0)) {
        (void)addElement(doc, el, "rating", QString::number(d->mRating));
        (void)addElement(doc, el, "downloads", QString::number(d->mDownloads));
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

    QString summary = d->mSummary;
    QString preview = d->mPreview;
    QString payload = d->mPayload;

    e = addElement(doc, el, "summary", summary);
    e = addElement(doc, el, "preview", KUrl(preview).fileName());
    e = addElement(doc, el, "payload", KUrl(payload).fileName());

    if (d->mStatus == Installed) {
        (void)addElement(doc, el, "status", "installed");
    }
    if (d->mStatus == Updateable) {
        (void)addElement(doc, el, "status", "updateable");
    }

    return el;
}

Entry EntryInternal::toEntry() const
{
    Entry e;

    e.d->mName = d->mName;
    e.d->mCategory = d->mCategory;
    e.d->mLicense = d->mLicense;
    e.d->mVersion = d->mVersion;
    e.d->mSummary = d->mSummary;
    e.d->mInstalledFiles = d->mInstalledFiles;
    e.d->mUnInstalledFiles = d->mUnInstalledFiles;

    if (d->mStatus == Installing || d->mStatus == Updating) {
        e.d->mStatus = Entry::Invalid;
    } else  {
        e.d->mStatus = (Entry::Status) d->mStatus;
    }

    return e;
}
