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

#include "xmlloader.h"

#include <kdebug.h>

using namespace KNS3;

class Entry::Private : public QSharedData
{
    public:
        Private()
        : mReleaseDate(QDate::currentDate())
        , mRating(0)
        , mDownloads(0)
        , mStatus(Entry::Invalid)
        , mSource(Entry::Online)
        {}

        bool operator==(const Private& other) const
        {
            return mUniqueId == other.mUniqueId;
        }

        QString mUniqueId;
        KTranslatable mName;
        
        QString mCategory;
        QString mLicense;
        QString mVersion;
        QDate mReleaseDate;
        Author mAuthor;
        int mRating;
        int mDownloads;
        KTranslatable mSummary;
        KTranslatable mPayload;
        KTranslatable mPreview;
        QStringList mInstalledFiles;
        QString mProviderId;
        QStringList mUnInstalledFiles;

        QString mChecksum;
        QString mSignature;
        Entry::Status mStatus;
        Entry::Source mSource;
};

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

bool Entry::operator<(const KNS3::Entry& other) const
{
    return d->mUniqueId < other.d->mUniqueId;
}

bool Entry::operator==(const KNS3::Entry& other) const
{
    return d->mUniqueId == other.d->mUniqueId && d->mProviderId == other.d->mProviderId;
}

Entry::~Entry()
{
}

bool Entry::isValid() const
{
    return !d->mUniqueId.isEmpty();
}

KTranslatable Entry::name() const
{
    return d->mName;
}

void Entry::setName(const KNS3::KTranslatable& name)
{
    d->mName = name;
}

QString Entry::uniqueId() const
{
    return d->mUniqueId;
}

void Entry::setUniqueId(const QString& id)
{
    d->mUniqueId = id;
}

QString Entry::providerId() const
{
    return d->mProviderId;
}

void Entry::setProviderId(const QString& id)
{
    d->mProviderId = id;
}

QString Entry::category() const
{
    return d->mCategory;
}

void Entry::setCategory(const QString& category)
{
    d->mCategory = category;
}

Author Entry::author() const
{
    return d->mAuthor;
}

void Entry::setAuthor(const KNS3::Author& author)
{
    d->mAuthor = author;
}

QString Entry::license() const
{
    return d->mLicense;
}

void Entry::setLicense(const QString& license)
{
    d->mLicense = license;
}

KTranslatable Entry::summary() const
{
    return d->mSummary;
}

void Entry::setSummary(const KNS3::KTranslatable& summary)
{
    d->mSummary = summary;
}

QString Entry::version() const
{
    return d->mVersion;
}

void Entry::setVersion(const QString& version)
{
    d->mVersion = version;
}

QDate Entry::releaseDate() const
{
    return d->mReleaseDate;
}

void Entry::setReleaseDate(const QDate& releasedate)
{
    d->mReleaseDate = releasedate;
}

KTranslatable Entry::payload() const
{
    return d->mPayload;
}

void Entry::setPayload(const KNS3::KTranslatable& url)
{
    d->mPayload = url;
}

KTranslatable Entry::preview() const
{
    return d->mPreview;
}

void Entry::setPreview(const KNS3::KTranslatable& url)
{
    d->mPreview = url;
}

int Entry::rating() const
{
    return d->mRating;
}

void Entry::setRating(int rating)
{
    d->mRating = rating;
}

int Entry::downloads() const
{
    return d->mDownloads;
}

void Entry::setDownloads(int downloads)
{
    d->mDownloads = downloads;
}

/*
QString Entry::checksum() const
{

    return d->mChecksum;
}

QString Entry::signature() const
{

    return d->mSignature;
}
*/

Entry::Source Entry::source() const
{
    return d->mSource;
}

void Entry::setSource(Source source)
{
    d->mSource = source;
}

Entry::Status Entry::status() const
{
    return d->mStatus;
}

void Entry::setStatus(Status status)
{
    d->mStatus = status;
}

void KNS3::Entry::setInstalledFiles(const QStringList & files)
{
    d->mInstalledFiles = files;
}

QStringList KNS3::Entry::installedFiles() const
{
    return d->mInstalledFiles;
}

void KNS3::Entry::setUnInstalledFiles(const QStringList & files)
{
    d->mUnInstalledFiles = files;
}

QStringList KNS3::Entry::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}

bool KNS3::Entry::setEntryXML(const QDomElement & xmldata)
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
            QString lang = e.attribute("lang");
            d->mName.addString(lang, e.text().trimmed());
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
        } else if (e.tagName() == "licence") { // krazy:exclude=spelling
            d->mLicense = e.text().trimmed();
        } else if (e.tagName() == "summary") {
            QString lang = e.attribute("lang");
            //kDebug() << "adding " << e.tagName() << " to summary as language " << lang;
            d->mSummary.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "version") {
            d->mVersion = e.text().trimmed();
        } else if (e.tagName() == "releasedate") {
            d->mReleaseDate = QDate::fromString(e.text().trimmed(), Qt::ISODate);
        } else if (e.tagName() == "preview") {
            QString lang = e.attribute("lang");
            d->mPreview.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "payload") {
            QString lang = e.attribute("lang");
            d->mPayload.addString(lang, e.text().trimmed());
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
            d->mInstalledFiles << e.text();
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
QDomElement KNS3::Entry::entryXML() const
{
    Q_ASSERT(!d->mUniqueId.isEmpty());
    Q_ASSERT(!d->mProviderId.isEmpty());
    
    QDomDocument doc;

    QDomElement el = doc.createElement("stuff");
    el.setAttribute("category", d->mCategory);

    KTranslatable name = d->mName;

    QStringList::ConstIterator it;
    QDomElement e;
    QStringList langs;

    langs = name.languages();
    for (it = langs.constBegin(); it != langs.constEnd(); ++it) {
        e = addElement(doc, el, "name", name.translated(*it));
        e.setAttribute("lang", *it);
    }

    (void)addElement(doc, el, "providerid", d->mProviderId);

    QDomElement author = addElement(doc, el, "author", d->mAuthor.name());
    if (!d->mAuthor.email().isEmpty())
        author.setAttribute("email", d->mAuthor.email());
    if (!d->mAuthor.homepage().isEmpty())
        author.setAttribute("homepage", d->mAuthor.homepage());
    if (!d->mAuthor.jabber().isEmpty())
        author.setAttribute("im", d->mAuthor.jabber());
    // FIXME: 'jabber' or 'im'? consult with kopete guys...

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

    KTranslatable summary = d->mSummary;
    KTranslatable preview = d->mPreview;
    KTranslatable payload = d->mPayload;

    langs = summary.languages();
    for (it = langs.constBegin(); it != langs.constEnd(); ++it) {
        e = addElement(doc, el, "summary", summary.translated(*it));
        e.setAttribute("lang", *it);
    }

    langs = preview.languages();
    for (it = langs.constBegin(); it != langs.constEnd(); ++it) {
        e = addElement(doc, el, "preview", KUrl(preview.translated(*it)).fileName());
        e.setAttribute("lang", *it);
    }

    langs = payload.languages();
    for (it = langs.constBegin(); it != langs.constEnd(); ++it) {
        e = addElement(doc, el, "payload", KUrl(payload.translated(*it)).fileName());
        e.setAttribute("lang", *it);
    }

    if (d->mStatus == Installed) {
        (void)addElement(doc, el, "status", "installed");
    }
    if (d->mStatus == Updateable) {
        (void)addElement(doc, el, "status", "updateable");
    }

    return el;
}


