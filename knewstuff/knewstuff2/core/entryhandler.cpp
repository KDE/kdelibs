/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2007 Dirk Mueller <mueller@kde.org>

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

#include "entryhandler.h"

#include <kdebug.h>

using namespace KNS;

EntryHandler::EntryHandler(const Entry& entry)
{
    init();
    mEntry = entry;
    mEntryXML = serializeElement(entry);
}

EntryHandler::EntryHandler(const QDomElement& entryxml)
{
    init();
    mEntryXML = entryxml;
    mEntry = deserializeElement(entryxml);
}

void EntryHandler::init()
{
    mValid = false;
    mCompat = false;
}

void EntryHandler::setCompatibilityFormat()
{
    mCompat = true;
}

bool EntryHandler::isValid()
{
    return mValid;
}

QDomElement EntryHandler::entryXML()
{
    return mEntryXML;
}

Entry EntryHandler::entry()
{
    return mEntry;
}

Entry *EntryHandler::entryptr()
{
    Entry *entry = new Entry();
    entry->setName(mEntry.name());
    entry->setAuthor(mEntry.author());
    entry->setCategory(mEntry.category());
    entry->setLicense(mEntry.license());
    entry->setSummary(mEntry.summary());
    entry->setVersion(mEntry.version());
    entry->setRelease(mEntry.release());
    entry->setReleaseDate(mEntry.releaseDate());
    entry->setPayload(mEntry.payload());
    entry->setPreview(mEntry.preview());
    entry->setRating(mEntry.rating());
    entry->setDownloads(mEntry.downloads());
    entry->setInstalledFiles(mEntry.installedFiles());
    entry->setIdNumber(mEntry.idNumber());
    return entry;
}

QDomElement EntryHandler::serializeElement(const Entry& entry)
{
    QDomDocument doc;

    QDomElement el = doc.createElement("stuff");
    el.setAttribute("category", entry.category());

    KTranslatable name = entry.name();

    QStringList::ConstIterator it;
    QDomElement e;
    QStringList langs;

    langs = name.languages();
    for (it = langs.constBegin(); it != langs.constEnd(); ++it) {
        e = addElement(doc, el, "name", name.translated(*it));
        e.setAttribute("lang", *it);
    }

    QDomElement author = addElement(doc, el, "author", entry.author().name());
    if (!entry.author().email().isEmpty())
        author.setAttribute("email", entry.author().email());
    if (!entry.author().homepage().isEmpty())
        author.setAttribute("homepage", entry.author().homepage());
    if (!entry.author().jabber().isEmpty())
        author.setAttribute("im", entry.author().jabber());
    // FIXME: 'jabber' or 'im'? consult with kopete guys...

    (void)addElement(doc, el, "licence", entry.license()); // krazy:exclude=spelling
    (void)addElement(doc, el, "version", entry.version());
    if (mCompat)
        (void)addElement(doc, el, "release", QString::number(entry.release()));
    if ((entry.rating() > 0) || (entry.downloads() > 0)) {
        (void)addElement(doc, el, "rating", QString::number(entry.rating()));
        (void)addElement(doc, el, "downloads", QString::number(entry.downloads()));
    }
    if (!entry.signature().isEmpty()) {
        (void)addElement(doc, el, "signature", entry.signature());
    }
    if (!entry.checksum().isEmpty()) {
        (void)addElement(doc, el, "checksum", entry.checksum());
    }
    foreach(const QString &file, entry.installedFiles()) {
        (void)addElement(doc, el, "installedfile", file);
    }
    if (entry.idNumber() > 0) {
        addElement(doc, el, "id", QString::number(entry.idNumber()));
    }

    (void)addElement(doc, el, "releasedate",
                     entry.releaseDate().toString(Qt::ISODate));

    KTranslatable summary = entry.summary();
    KTranslatable preview = entry.preview();
    KTranslatable payload = entry.payload();

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

Entry EntryHandler::deserializeElement(const QDomElement& entryxml)
{
    Entry entry;
    KTranslatable name, summary, preview, payload;
    int idNumber = 0;
    QStringList installedFiles;

    if (entryxml.tagName() != "stuff") return entry;

    if (!mCompat) {
        QString category = entryxml.attribute("category");
        entry.setCategory(category);
    }

    QDomNode n;
    for (n = entryxml.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "name") {
            QString lang = e.attribute("lang");
            name.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "author") {
            Author author;
            QString email = e.attribute("email");
            QString jabber = e.attribute("im");
            QString homepage = e.attribute("homepage");
            author.setName(e.text().trimmed());
            author.setEmail(email);
            author.setJabber(jabber);
            author.setHomepage(homepage);
            entry.setAuthor(author);
        } else if (e.tagName() == "licence") { // krazy:exclude=spelling
            entry.setLicense(e.text().trimmed());
        } else if (e.tagName() == "summary") {
            QString lang = e.attribute("lang");
            //kDebug() << "adding " << e.tagName() << " to summary as language " << lang;
            summary.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "version") {
            entry.setVersion(e.text().trimmed());
        } else if (e.tagName() == "release") {
            if (mCompat) {
                entry.setRelease(e.text().toInt());
            }
        } else if (e.tagName() == "releasedate") {
            QDate date = QDate::fromString(e.text().trimmed(), Qt::ISODate);
            entry.setReleaseDate(date);
        } else if (e.tagName() == "preview") {
            QString lang = e.attribute("lang");
            preview.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "payload") {
            QString lang = e.attribute("lang");
            payload.addString(lang, e.text().trimmed());
        } else if (e.tagName() == "rating") {
            entry.setRating(e.text().toInt());
        } else if (e.tagName() == "downloads") {
            entry.setDownloads(e.text().toInt());
        } else if (e.tagName() == "category") {
            if (mCompat) {
                entry.setCategory(e.text());
            }
        } else if (e.tagName() == "signature") {
            entry.setSignature(e.text());
        } else if (e.tagName() == "checksum") {
            entry.setChecksum(e.text());
        } else if (e.tagName() == "installedfile") {
            installedFiles << e.text();
        } else if (e.tagName() == "id") {
            idNumber = e.text().toInt();
            //kDebug() << "got id number: " << idNumber;
        }
    }

    entry.setName(name);
    entry.setSummary(summary);
    entry.setPreview(preview);
    entry.setPayload(payload);
    entry.setInstalledFiles(installedFiles);
    entry.setIdNumber(idNumber);

    // Validation

    if (entry.name().isEmpty()) {
        kWarning(550) << "EntryHandler: no name given";
        return entry;
    }

    if (entry.payload().isEmpty()) {
        kWarning(550) << "EntryHandler: no payload URL given";
        return entry;
    }

    // Entry is valid

    mValid = true;
    return entry;
}

QDomElement EntryHandler::addElement(QDomDocument& doc, QDomElement& parent,
                                     const QString& tag, const QString& value)
{
    QDomElement n = doc.createElement(tag);
    n.appendChild(doc.createTextNode(value));
    parent.appendChild(n);

    return n;
}
