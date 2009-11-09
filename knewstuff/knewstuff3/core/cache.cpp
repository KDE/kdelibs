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

#include <KDebug>

using namespace KNS3;

Cache::Cache(QObject* parent): QObject(parent)
{
    cachePolicy = Engine::CacheNever;
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
    kDebug() << "Read Cache";


    
}

void Cache::writeCache()
{
    kDebug() << "Write Cache";
}









// da old stuff
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


KNS3::Entry Engine::loadEntryCache(const QString& filepath)
{
    bool ret;
    QFile f(filepath);
    ret = f.open(QIODevice::ReadOnly);
    if (!ret) {
        kWarning() << "The file " << filepath << " could not be opened.";
        return Entry();
    }

    QDomDocument doc;
    ret = doc.setContent(&f);
    if (!ret) {
        kWarning() << "The file could not be parsed.";
        return Entry();
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "ghnscache") {
        kWarning() << "The file doesn't seem to be of interest.";
        return Entry();
    }

    QDomElement stuff = root.firstChildElement("stuff");
    if (stuff.isNull()) {
        kWarning() << "Missing GHNS cache metadata.";
        return Entry();
    }

    // FIXME use the right sub class of entry
    Entry e;
    e.setEntryXML(stuff);
    //if (!handler.isValid()) {
    //    kWarning() << "Invalid GHNS installation metadata.";
    //    return NULL;
    //}

    e.setStatus(Entry::Downloadable);
    d->entries.append(e);

    if (root.hasAttribute("previewfile")) {
        d->previewfiles[e] = root.attribute("previewfile");
        // FIXME: check here for a [ -f previewfile ]
    }

    if (root.hasAttribute("payloadfile")) {
        // FIXME d->payloadfiles[e] = root.attribute("payloadfile");
        // FIXME: check here for a [ -f payloadfile ]
    }

    e.setSource(Entry::Cache);

    return e;
}

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


#include "cache.moc"
