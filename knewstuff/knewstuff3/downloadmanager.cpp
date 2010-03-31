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
    {}
    ~Private() { delete engine; }
    
    bool isInitialized;
    bool checkForUpdates;

    void _k_slotProvidersLoaded();
    void _k_slotUpdatesLoaded(const KNS3::EntryInternal::List& entries);
    void _k_slotEntryStatusChanged(const KNS3::EntryInternal& entry);
};
}

using namespace KNS3;

DownloadManager::DownloadManager(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

DownloadManager::DownloadManager(const QString& configFile, QObject * parent)
        : QObject(parent)
        , d(new Private(this))
{
    init(configFile);
}

void DownloadManager::init(const QString& configFile)
{
    connect(d->engine, SIGNAL(signalProvidersLoaded()), this, SLOT(_k_slotProvidersLoaded()));
    connect(d->engine, SIGNAL(signalUpdateableEntriesLoaded(KNS3::EntryInternal::List)), this, SLOT(_k_slotUpdatesLoaded(KNS3::EntryInternal::List)));
    connect(d->engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)), this, SLOT(_k_slotEntryStatusChanged(KNS3::EntryInternal)));
    d->engine->init(configFile);
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

void DownloadManager::Private::_k_slotUpdatesLoaded(const KNS3::EntryInternal::List& entries)
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


#include "downloadmanager.moc"
