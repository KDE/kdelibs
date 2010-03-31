/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "downloadmanager.h"

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kdebug.h>

#include "core/engine.h"

namespace KNS3 {
class DownloadManager::Private
{
public:
    DownloadManager* q;
    Engine* engine;

    Private(DownloadManager* q)
        : q(q)
        , engine(new Engine)
        , isInitialized(false)
        , checkForUpdates(false)
        , doSearch(false)
        , page(0)
        , pageSize(100)
    {}
    ~Private() { delete engine; }
    
    bool isInitialized;
    bool checkForUpdates;
    bool doSearch;
    
    int page;
    int pageSize;
    
    void init(const QString& configFile);
    void _k_slotProvidersLoaded();
    void _k_slotUpdatesLoaded(const KNS3::EntryInternal::List& entries);
    void _k_slotEntryStatusChanged(const KNS3::EntryInternal& entry);
    void _k_slotEntriesLoaded(const KNS3::EntryInternal::List& entries);
};
}

using namespace KNS3;

DownloadManager::DownloadManager(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    d->init(name + ".knsrc");
}

DownloadManager::DownloadManager(const QString& configFile, QObject * parent)
        : QObject(parent)
        , d(new Private(this))
{
    d->init(configFile);
}

void DownloadManager::Private::init(const QString& configFile)
{
    q->connect(engine, SIGNAL(signalProvidersLoaded()), q, SLOT(_k_slotProvidersLoaded()));
    q->connect(engine, SIGNAL(signalUpdateableEntriesLoaded(KNS3::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNS3::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntriesLoaded(KNS3::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNS3::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)), q, SLOT(_k_slotEntryStatusChanged(KNS3::EntryInternal)));
    engine->init(configFile);
}

DownloadManager::~DownloadManager()
{
    delete d;
}

void DownloadManager::Private::_k_slotProvidersLoaded()
{
    kDebug() << "providers loaded";
    isInitialized = true;
    if (checkForUpdates) {
        engine->checkForUpdates();
    } else if (doSearch) {
        engine->requestData(page, pageSize);
    }
}

void DownloadManager::checkForUpdates()
{
    if (d->isInitialized) {
        d->engine->checkForUpdates();
    } else {
        d->checkForUpdates = true;
    }
}

void DownloadManager::Private::_k_slotEntriesLoaded(const KNS3::EntryInternal::List& entries)
{
    KNS3::Entry::List result;
    foreach (const KNS3::EntryInternal& entry, entries) {
        result.append(entry.toEntry());
    }
    emit q->searchResult(result);
}

void KNS3::DownloadManager::Private::_k_slotEntryStatusChanged(const KNS3::EntryInternal& entry)
{
    emit q->entryStatusChanged(entry.toEntry());
}

void DownloadManager::installEntry(const KNS3::Entry& entry)
{
    KNS3::EntryInternal entryInternal = EntryInternal::fromEntry(entry);
    if (entryInternal.isValid()) {
        d->engine->install(entryInternal);
    }
}

void DownloadManager::search(int page, int pageSize)
{
    d->page = page;
    d->pageSize = pageSize;

    if (d->isInitialized) {
        d->engine->requestData(page, pageSize);
    } else {
        d->doSearch = true;
    }
}

void DownloadManager::setSearchOrder(DownloadManager::SortOrder order)
{
    switch (order) {
        case Newest:
            d->engine->setSortMode(Provider::Newest);
            break;
        case Rating:
            d->engine->setSortMode(Provider::Rating);
            break;
        case Alphabetical:
            d->engine->setSortMode(Provider::Alphabetical);
            break;
        case Downloads:
            d->engine->setSortMode(Provider::Downloads);
            break;
    }
}

void DownloadManager::setSearchTerm(const QString& searchTerm)
{
    d->engine->setSearchTerm(searchTerm);
}


#include "downloadmanager.moc"
