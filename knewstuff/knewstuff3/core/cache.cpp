/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "cache.h"

#include <QtCore/QFile>
#include <KStandardDirs>
#include <KDebug>

using namespace KNS3;

Cache::Cache(QObject* parent): QObject(parent)
{
    cachePolicy = Engine::CacheReplaceable;
}

void Cache::setCacheFileName(const QString& file)
{
    cacheFile = KStandardDirs::locateLocal("data", "knewstuff3/" + file + ".knscache");
    kDebug() << "Using Cache file: " << cacheFile;
}

void Cache::setPolicy(Engine::CachePolicy policy)
{
    cachePolicy = policy;
}

Engine::CachePolicy Cache::policy() const
{
    return cachePolicy;
}

void Cache::readCache()
{
    QFile f(cacheFile);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning() << "The file " << cacheFile << " could not be opened.";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f)) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "hotnewstuffregistry") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomElement stuff = root.firstChildElement("stuff");
    while (!stuff.isNull()) {
        Entry e;
        e.setEntryXML(stuff);
        e.setSource(Entry::Cache);
        cache.insert(e);
        stuff = stuff.nextSiblingElement("stuff");
    }
    /*
    if (root.hasAttribute("previewfile")) {
        d->previewfiles[e] = root.attribute("previewfile");
        // FIXME: check here for a [ -f previewfile ]
    }
    */

    if (root.hasAttribute("payloadfile")) {
        // FIXME d->payloadfiles[e] = root.attribute("payloadfile");
        // FIXME: check here for a [ -f payloadfile ]
    }

    kDebug() << "Cache read... entries: " << cache.size();
}


Entry::List Cache::cacheForProvider(const QString& providerId)
{
    Entry::List entries;
    foreach (const Entry& e, cache) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}


void Cache::writeCache()
{
    kDebug() << "Write Cache";

    QFile f(cacheFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << cacheFile << "'." << endl;
        return;
    }
    
    QDomDocument doc;
    QDomElement root = doc.createElement("hotnewstuffregistry");

    foreach (const Entry& entry, cache) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (cachePolicy != Engine::CacheNever || (entry.status() == Entry::Installed || entry.status() == Entry::Updateable)) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << root;
    f.close();
}

void Cache::insert(const QList< Entry >& entries)
{
    foreach(const Entry& e, entries) {
        // if we don't remove the old entry, we cannot make sure it's up to date
        kDebug() << "update cache " << e.name().representation();
        cache.remove(e);
        cache.insert(e);
    }
}


#include "cache.moc"
