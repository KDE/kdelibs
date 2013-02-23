/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>

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
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>
#include <qstandardpaths.h>
#include <kdebug.h>

using namespace KNS3;

typedef QHash<QString, QWeakPointer<Cache> > CacheHash;
Q_GLOBAL_STATIC(CacheHash, s_caches)

Cache::Cache(const QString &appName): QObject(0)
{
    m_kns2ComponentName = appName;

    registryFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "knewstuff3/" + appName + ".knsregistry";
    kDebug() << "Using registry file: " << registryFile;
}

QSharedPointer<Cache> Cache::getCache(const QString &appName)
{
    CacheHash::const_iterator it = s_caches()->constFind(appName);
    if ((it != s_caches()->constEnd()) && !(*it).isNull()) {
        return QSharedPointer<Cache>(*it);
    }

    QSharedPointer<Cache> p(new Cache(appName));
    s_caches()->insert(appName, QWeakPointer<Cache>(p));

    return p;
}

Cache::~Cache()
{
}

void Cache::readRegistry()
{
    // read KNS2 registry first to migrate it
    readKns2MetaFiles();

    QFile f(registryFile);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning() << "The file " << registryFile << " could not be opened.";
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
        EntryInternal e;
        e.setEntryXML(stuff);
        e.setSource(EntryInternal::Cache);
        cache.insert(e);
        stuff = stuff.nextSiblingElement("stuff");
    }

    kDebug() << "Cache read... entries: " << cache.size();
}

void Cache::readKns2MetaFiles()
{
    kDebug() << "Loading KNS2 registry of files for the component: " << m_kns2ComponentName;

    QString realAppName = m_kns2ComponentName.split(':')[0];

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "knewstuff2-entries.registry", QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        //kDebug() << " + Load from directory '" + (*it) + "'.";
        QDir dir((*it));
        const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QStringList::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
            QString filepath = (*it) + '/' + (*fit);

            kDebug() << " Load from file '" + filepath + "'.";

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

            if (!f.open(QIODevice::ReadOnly)) {
                kWarning() << "The file: " << filepath << " could not be opened.";
                continue;
            }

            QDomDocument doc;
            if (!doc.setContent(&f)) {
                kWarning() << "The file could not be parsed.";
                return;
            }
            kDebug() << "found entry: " << doc.toString();

            QDomElement root = doc.documentElement();
            if (root.tagName() != "ghnsinstall") {
                kWarning() << "The file doesn't seem to be of interest.";
                return;
            }

            // The .meta files only contain one entry
            QDomElement stuff = root.firstChildElement("stuff");
            EntryInternal e;
            e.setEntryXML(stuff);
            e.setSource(EntryInternal::Cache);

            if (e.payload().startsWith(QLatin1String("http://download.kde.org/khotnewstuff"))) {
                // This is 99% sure a opendesktop file, make it a real one.
                e.setProviderId(QLatin1String("https://api.opendesktop.org/v1/"));
                e.setHomepage(QUrl(QString(QLatin1String("http://opendesktop.org/content/show.php?content=") + e.uniqueId())));

            } else if (e.payload().startsWith(QLatin1String("http://edu.kde.org/contrib/kvtml/"))) {
                // kvmtl-1
                e.setProviderId("http://edu.kde.org/contrib/kvtml/kvtml.xml");
            } else if (e.payload().startsWith(QLatin1String("http://edu.kde.org/contrib/kvtml2/"))) {
                // kvmtl-2
                e.setProviderId("http://edu.kde.org/contrib/kvtml2/provider41.xml");
            } else {
                // we failed, skip
                kWarning() << "Could not load entry: " << filepath;
                continue;
            }

            e.setStatus(Entry::Installed);

            cache.insert(e);
            QDomDocument tmp("yay");
            tmp.appendChild(e.entryXML());
            kDebug() << "new entry: " << tmp.toString();

            f.close();

            QDir dir;
            if (!dir.remove(filepath)) {
                kWarning() << "could not delete old kns2 .meta file: " << filepath;
            } else {
                kDebug() << "Migrated KNS2 entry to KNS3.";
            }

        }
    }
}

EntryInternal::List Cache::registryForProvider(const QString& providerId)
{
    EntryInternal::List entries;
    foreach (const EntryInternal& e, cache) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}


void Cache::writeRegistry()
{
    kDebug() << "Write registry";

    QFile f(registryFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << registryFile << "'." << endl;
        return;
    }

    QDomDocument doc("khotnewstuff3");
    doc.appendChild(doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ));
    QDomElement root = doc.createElement("hotnewstuffregistry");
    doc.appendChild(root);

    foreach (const EntryInternal& entry, cache) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (entry.status() == Entry::Installed || entry.status() == Entry::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    f.close();
}

void Cache::registerChangedEntry(const KNS3::EntryInternal& entry)
{
    cache.insert(entry);
}

void Cache::insertRequest(const KNS3::Provider::SearchRequest& request, const KNS3::EntryInternal::List& entries)
{
    // append new entries
    requestCache[request.hashForRequest()].append(entries);
    kDebug() << request.hashForRequest() << " add: " << entries.size() << " keys: " << requestCache.keys();
}

EntryInternal::List Cache::requestFromCache(const KNS3::Provider::SearchRequest& request)
{
    kDebug() << request.hashForRequest();
    return requestCache.value(request.hashForRequest());
}

