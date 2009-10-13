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

using namespace KNS;

struct KNS::EntryPrivate {
    EntryPrivate() : mReleaseDate(QDate::currentDate())
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

bool Entry::setEntryXML(const QDomElement & xmldata)
{
    if (xmldata.tagName() != "stuff")
		return false;

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
            d->mIdNumber = e.text().toInt();
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
        return false;
    }

    return true;
}

/**
 * get the xml string for the entry
 */
QDomElement Entry::entryXML() const
{
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
    if (d->mIdNumber > 0) {
        addElement(doc, el, "id", QString::number(d->mIdNumber));
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

    return el;
}

KTranslatable Entry::name() const
{
    return d->mName;
}

QString Entry::category() const
{
    return d->mCategory;
}

Author Entry::author() const
{
    return d->mAuthor;
}

QString Entry::license() const
{
    return d->mLicense;
}

KTranslatable Entry::summary() const
{
    return d->mSummary;
}

QString Entry::version() const
{
    return d->mVersion;
}

QDate Entry::releaseDate() const
{
    return d->mReleaseDate;
}

KTranslatable Entry::payload() const
{
    return d->mPayload;
}

KTranslatable Entry::preview() const
{
    return d->mPreview;
}

int Entry::rating() const
{
    return d->mRating;
}

int Entry::downloads() const
{
    return d->mDownloads;
}

QString Entry::checksum() const
{
    return d->mChecksum;
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
