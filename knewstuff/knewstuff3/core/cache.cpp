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
    kDebug() << "Reading Cache...";

    bool ret;
    QFile f(cacheFile);
    ret = f.open(QIODevice::ReadOnly);
    if (!ret) {
        kWarning() << "The file " << cacheFile << " could not be opened.";
        return;
    }

    QDomDocument doc;
    ret = doc.setContent(&f);
    if (!ret) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "ghnscache") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomElement stuff = root.firstChildElement("stuff");
    if (stuff.isNull()) {
        kWarning() << "Missing GHNS cache metadata.";
        return;
    }

    // FIXME use the right sub class of entry
    Entry e;
    e.setEntryXML(stuff);
    //if (!handler.isValid()) {
    //    kWarning() << "Invalid GHNS installation metadata.";
    //    return NULL;
    //}

/* FIXME !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    e.setStatus(Entry::Installed);
    e.setSource(Entry::Registry);
    e.setStatus(Entry::Downloadable);
    */

    e.setSource(Entry::Cache);


    
    Entry::List entries;
    entries.append(e);

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

    kDebug() << "Reading Cache..." << entries.size();
}






/*



void Engine::loadRegistry()
{
    KStandardDirs standardDirs;

    kDebug() << "Loading registry of files for the component: " << d->applicationName;

    QString realAppName = d->applicationName.split(':')[0];

    // this must be same as in registerEntry()
    const QStringList dirs = standardDirs.findDirs("data", "knewstuff2-entries.registry");
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        //kDebug() << " + Load from directory '" + (*it) + "'.";
        QDir dir((*it));
        const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QStringList::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
            QString filepath = (*it) + '/' + (*fit);
            //kDebug() << "  + Load from file '" + filepath + "'.";

            bool ret;
            QFileInfo info(filepath);
            QFile f(filepath);

            // first see if this file is even for this app
            // because the registry contains entries for all apps
            // FIXMEE: should be able to do this with a filter on the entryList above probably
            QString thisAppName = QString::fromUtf8(QByteArray::fromBase64(info.baseName().toUtf8()));

            // NOTE: the ":" needs to always coincide with the separator character used in
            // the id(Entry*) method
            thisAppName = thisAppName.split(':')[0];

            if (thisAppName != realAppName) {
                continue;
            }

            ret = f.open(QIODevice::ReadOnly);
            if (!ret) {
                kWarning() << "The file could not be opened.";
                continue;
            }

            QDomDocument doc;
            ret = doc.setContent(&f);
            if (!ret) {
                kWarning() << "The file could not be parsed.";
                continue;
            }

            QDomElement root = doc.documentElement();
            if (root.tagName() != "ghnsinstall") {
                kWarning() << "The file doesn't seem to be of interest.";
                continue;
            }

            QDomElement stuff = root.firstChildElement("stuff");
            if (stuff.isNull()) {
                kWarning() << "Missing GHNS installation metadata.";
                continue;
            }

            Entry e;
            e.setEntryXML(stuff);
            //if (!e->isValid()) {
            //    kWarning() << "Invalid GHNS installation metadata.";
            //    continue;
            //}

            e.setStatus(Entry::Installed);
            e.setSource(Entry::Registry);
            d->entry_registry.append(e);
            //QString thisid = id(e);

            // we must overwrite cache entries with registered entries
            // and not just append the latter ones
            //if (entryCached(e)) {
            //    // it's in the cache, so replace the cache entry with the registered entry
            //    Entry * oldEntry = d->entry_index[thisid];
            //    int index = d->entries.indexOf(oldEntry);
            //    d->entries[index] = e;
            //    //delete oldEntry;
            //}
            //else {
            //    d->entries.append(e);
            //}
            //d->entry_index[thisid] = e;
        }
    }
}
*/


void Cache::writeCache(const Entry::List& entries)
{
    kDebug() << "Write Cache";

    QFile f(cacheFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << cacheFile << "'." << endl;
        return;
    }
    
    QDomDocument doc;
    QDomElement root = doc.createElement("hotnewstuffregistry");

    foreach (const Entry& entry, entries) {
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




// da old stuff
/*
// FIXME: not needed anymore?
#if 0
void Engine::loadEntriesCache()
{
    KStandardDirs d;

    //kDebug() << "Loading entry cache.";

    QStringList cachedirs = d.findDirs("cache", "knewstuff2-entries.cache/" + d->componentname);
    if (cachedirs.size() == 0) {
        //kDebug() << "Cache directory not present, skip loading.";
        return;
    }
    QString cachedir = cachedirs.first();

    //kDebug() << " + Load from directory '" + cachedir + "'.";

    QDir dir(cachedir);
    QStringList files = dir.entryList(QDir::Files | QDir::Readable);
    for (QStringList::iterator fit = files.begin(); fit != files.end(); ++fit) {
        QString filepath = cachedir + '/' + (*fit);
        //kDebug() << "  + Load from file '" + filepath + "'.";

        Entry *e = loadEntryCache(filepath);

        if (e) {
            // FIXME: load provider/feed information first
            emit signalEntryLoaded(e, NULL, NULL);
        }
    }
}
#endif
*/

/*
void Engine::cacheEntry(const Entry& entry)
{
    KStandardDirs standardDirs;

    QString cachedir = standardDirs.saveLocation("cache", "knewstuff2-entries.cache/");

    kDebug() << "Caching entry in directory '" + cachedir + "'.";

    //FIXME: this must be deterministic, but it could also be an OOB random string
    //which gets stored into <ghnscache> just like preview...
    QString idbase64 = QString(entry.uniqueId().toUtf8().toBase64());
    QString cachefile = idbase64 + ".meta";

    kDebug() << "Caching to file '" + cachefile + "'.";

    // FIXME: adhere to meta naming rules as discussed
    // FIXME: maybe related filename to base64-encoded id(), or the reverse?

    QDomElement exml = entry.entryXML();

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnscache");
    root.appendChild(exml);

    if (d->previewfiles.contains(entry)) {
        root.setAttribute("previewfile", d->previewfiles[entry]);
    }
    //if (d->payloadfiles.contains(entry)) {
    //    root.setAttribute("payloadfile", d->payloadfiles[entry]);
    //}

    QFile f(cachedir + cachefile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kError() << "Cannot write meta information to '" << cachedir + cachefile << "'." << endl;
        // FIXME: ignore?
        return;
    }
    QTextStream metastream(&f);
    metastream << root;
    f.close();
}
*/



#include "cache.moc"
