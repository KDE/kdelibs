/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#include "coreengine.h"

#include "entryhandler.h"
#include "providerhandler.h"
#include "entryloader.h"
#include "providerloader.h"
#include "installation.h"
#include "security.h"

#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kcodecs.h>
#include <kprocess.h>
#include <kshell.h>

#include <kio/job.h>
#include <kmimetype.h>
#include <krandom.h>
#include <ktar.h>
#include <kzip.h>

#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QtCore/Q_PID>

#if defined(Q_OS_WIN)
#include <windows.h>
#define _WIN32_IE 0x0500
#include <shlobj.h>
#endif

using namespace KNS;

CoreEngine::CoreEngine(QObject* parent)
        : QObject(parent), m_uploadedentry(NULL), m_uploadprovider(NULL), m_installation(NULL), m_activefeeds(0),
        m_initialized(false), m_cachepolicy(CacheNever), m_automationpolicy(AutomationOn)
{
}

CoreEngine::~CoreEngine()
{
    shutdown();
}

bool CoreEngine::init(const QString &configfile)
{
    kDebug() << "Initializing KNS::CoreEngine from '" << configfile << "'";

    KConfig conf(configfile);
    if (conf.accessMode() == KConfig::NoAccess) {
        kError() << "No knsrc file named '" << configfile << "' was found." << endl;
        return false;
    }
    // FIXME: accessMode() doesn't return NoAccess for non-existing files
    // - bug in kdecore?
    // - this needs to be looked at again until KConfig backend changes for KDE 4
    // the check below is a workaround
    if (KStandardDirs::locate("config", configfile).isEmpty()) {
        kError() << "No knsrc file named '" << configfile << "' was found." << endl;
        return false;
    }

    if (!conf.hasGroup("KNewStuff2")) {
        kError() << "A knsrc file was found but it doesn't contain a KNewStuff2 section." << endl;
        return false;
    }

    KConfigGroup group = conf.group("KNewStuff2");
    m_providersurl = group.readEntry("ProvidersUrl", QString());
    //m_componentname = group.readEntry("ComponentName", QString());
    m_componentname = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';

    // FIXME: add support for several categories later on
    // FIXME: read out only when actually installing as a performance improvement?
    m_installation = new Installation();
    QString uncompresssetting = group.readEntry("Uncompress", QString("never"));
    // support old value of true as equivalent of always
    if (uncompresssetting == "true") {
        uncompresssetting = "always";
    }
    if (uncompresssetting != "always" && uncompresssetting != "archive" && uncompresssetting != "never") {
        kError() << "invalid Uncompress setting chosen, must be one of: always, archive, or never" << endl;
        return false;
    }
    m_installation->setUncompression(uncompresssetting);

    m_installation->setCommand(group.readEntry("InstallationCommand", QString()));
    m_installation->setUninstallCommand(group.readEntry("UninstallCommand", QString()));
    m_installation->setStandardResourceDir(group.readEntry("StandardResource", QString()));
    m_installation->setTargetDir(group.readEntry("TargetDir", QString()));
    m_installation->setInstallPath(group.readEntry("InstallPath", QString()));
    m_installation->setAbsoluteInstallPath(group.readEntry("AbsoluteInstallPath", QString()));
    m_installation->setCustomName(group.readEntry("CustomName", false));

    QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
    if (!checksumpolicy.isEmpty()) {
        if (checksumpolicy == "never")
            m_installation->setChecksumPolicy(Installation::CheckNever);
        else if (checksumpolicy == "ifpossible")
            m_installation->setChecksumPolicy(Installation::CheckIfPossible);
        else if (checksumpolicy == "always")
            m_installation->setChecksumPolicy(Installation::CheckAlways);
        else {
            kError() << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
    if (!signaturepolicy.isEmpty()) {
        if (signaturepolicy == "never")
            m_installation->setSignaturePolicy(Installation::CheckNever);
        else if (signaturepolicy == "ifpossible")
            m_installation->setSignaturePolicy(Installation::CheckIfPossible);
        else if (signaturepolicy == "always")
            m_installation->setSignaturePolicy(Installation::CheckAlways);
        else {
            kError() << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString scope = group.readEntry("Scope", QString());
    if (!scope.isEmpty()) {
        if (scope == "user")
            m_installation->setScope(Installation::ScopeUser);
        else if (scope == "system")
            m_installation->setScope(Installation::ScopeSystem);
        else {
            kError() << "The scope '" + scope + "' is unknown." << endl;
            return false;
        }

        if (m_installation->scope() == Installation::ScopeSystem) {
            if (!m_installation->installPath().isEmpty()) {
                kError() << "System installation cannot be mixed with InstallPath." << endl;
                return false;
            }
        }
    }

    QString cachePolicy = group.readEntry("CachePolicy", QString());
    if (!cachePolicy.isEmpty()) {
        if (cachePolicy == "never") {
            m_cachepolicy = CacheNever;
        } else if (cachePolicy == "replaceable") {
            m_cachepolicy = CacheReplaceable;
        } else if (cachePolicy == "resident") {
            m_cachepolicy = CacheResident;
        } else if (cachePolicy == "only") {
            m_cachepolicy = CacheOnly;
        } else {
            kError() << "Cache policy '" + cachePolicy + "' is unknown." << endl;
        }
    }
    kDebug() << "cache policy: " << cachePolicy;

    m_initialized = true;

    return true;
}

QString CoreEngine::componentName() const
{
    if (!m_initialized) {
        return QString();
    }

    return m_componentname;
}

void CoreEngine::start()
{
    //kDebug() << "starting engine";

    if (!m_initialized) {
        kError() << "Must call KNS::CoreEngine::init() first." << endl;
        return;
    }

    // first load the registry, so we know which entries are installed
    loadRegistry();

    // then load the providersCache if caching is enabled
    if (m_cachepolicy != CacheNever) {
        loadProvidersCache();
    }

    // FIXME: also return if CacheResident and its conditions fulfilled
    if (m_cachepolicy == CacheOnly) {
        //emit signalEntriesFinished();
        return;
    }

    ProviderLoader *provider_loader = new ProviderLoader(this);

    // make connections before loading, just in case the iojob is very fast
    connect(provider_loader,
            SIGNAL(signalProvidersLoaded(KNS::Provider::List)),
            SLOT(slotProvidersLoaded(KNS::Provider::List)));
    connect(provider_loader,
            SIGNAL(signalProvidersFailed()),
            SLOT(slotProvidersFailed()));

    provider_loader->load(m_providersurl);
}

void CoreEngine::loadEntries(Provider *provider)
{
    //kDebug() << "loading entries";

    if (m_cachepolicy == CacheOnly) {
        return;
    }

    //if (provider != m_provider_index[pid(provider)]) {
    //    // this is the cached provider, and a new provider has been loaded from the internet
    //    // also, this provider's feeds have already been loaded including it's entries
    //    m_provider_cache.removeAll(provider); // just in case it's still in there
    //    return;
    //}

    QStringList feeds = provider->feeds();
    for (int i = 0; i < feeds.count(); i++) {
        Feed *feed = provider->downloadUrlFeed(feeds.at(i));
        if (feed) {
            ++m_activefeeds;

            EntryLoader *entry_loader = new EntryLoader(this);

            connect(entry_loader,
                    SIGNAL(signalEntriesLoaded(KNS::Entry::List)),
                    SLOT(slotEntriesLoaded(KNS::Entry::List)));
            connect(entry_loader,
                    SIGNAL(signalEntriesFailed()),
                    SLOT(slotEntriesFailed()));
            connect(entry_loader,
                    SIGNAL(signalProgress(KJob*, unsigned long)),
                    SLOT(slotProgress(KJob*, unsigned long)));

            entry_loader->load(provider, feed);
        }
    }
}

void CoreEngine::downloadPreview(Entry *entry)
{
    if (m_previewfiles.contains(entry)) {
        // FIXME: ensure somewhere else that preview file even exists
        //kDebug() << "Reusing preview from '" << m_previewfiles[entry] << "'";
        emit signalPreviewLoaded(KUrl::fromPath(m_previewfiles[entry]));
        return;
    }

    KUrl source = KUrl(entry->preview().representation());

    if (!source.isValid()) {
        kError() << "The entry doesn't have a preview." << endl;
        return;
    }

    KUrl destination = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10));
    //kDebug() << "Downloading preview '" << source << "' to '" << destination << "'";

    // FIXME: check for validity
    KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(job,
            SIGNAL(result(KJob*)),
            SLOT(slotPreviewResult(KJob*)));
    connect(job,
            SIGNAL(progress(KJob*, unsigned long)),
            SLOT(slotProgress(KJob*, unsigned long)));

    m_entry_jobs[job] = entry;
}

void CoreEngine::downloadPayload(Entry *entry)
{
    if(!entry) {
        emit signalPayloadFailed(entry);
        return;
    }
    KUrl source = KUrl(entry->payload().representation());

    if (!source.isValid()) {
        kError() << "The entry doesn't have a payload." << endl;
        emit signalPayloadFailed(entry);
        return;
    }

    if (m_installation->isRemote()) {
        // Remote resource
        //kDebug() << "Relaying remote payload '" << source << "'";
        entry->setStatus(Entry::Installed);
        m_payloadfiles[entry] = entry->payload().representation();
        install(source.pathOrUrl());
        emit signalPayloadLoaded(source);
        // FIXME: we still need registration for eventual deletion
        return;
    }

    KUrl destination = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10));
    kDebug() << "Downloading payload '" << source << "' to '" << destination << "'";

    // FIXME: check for validity
    KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(job,
            SIGNAL(result(KJob*)),
            SLOT(slotPayloadResult(KJob*)));
    connect(job,
            SIGNAL(percent(KJob*, unsigned long)),
            SLOT(slotProgress(KJob*, unsigned long)));

    m_entry_jobs[job] = entry;
}

bool CoreEngine::uploadEntry(Provider *provider, Entry *entry)
{
    //kDebug() << "Uploading " << entry->name().representation() << "...";

    if (m_uploadedentry) {
        kError() << "Another upload is in progress!" << endl;
        return false;
    }

    if (!provider->uploadUrl().isValid()) {
        kError() << "The provider doesn't support uploads." << endl;
        return false;

        // FIXME: support for <noupload> will go here (file bundle creation etc.)
    }

    // FIXME: validate files etc.
    m_uploadprovider = provider;
    m_uploadedentry = entry;

    KUrl sourcepayload = KUrl(entry->payload().representation());
    KUrl destfolder = provider->uploadUrl();

    destfolder.setFileName(sourcepayload.fileName());

    KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepayload, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(fcjob,
            SIGNAL(result(KJob*)),
            SLOT(slotUploadPayloadResult(KJob*)));

    return true;
}

void CoreEngine::slotProvidersLoaded(KNS::Provider::List list)
{
    // note: this is only called from loading the online providers
    ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
    delete loader;

    mergeProviders(list);
}

void CoreEngine::slotProvidersFailed()
{
    kDebug() << "slotProvidersFailed";
    ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
    delete loader;

    emit signalProvidersFailed();
}

void CoreEngine::slotEntriesLoaded(KNS::Entry::List list)
{
    EntryLoader *loader = dynamic_cast<EntryLoader*>(sender());
    if (!loader) return;
    const Provider *provider = loader->provider();
    Feed *feed = loader->feed();
    delete loader;
    m_activefeeds--;
    //kDebug() << "entriesloaded m_activefeeds: " << m_activefeeds;

    //kDebug() << "Provider source " << provider->name().representation();
    //kDebug() << "Feed source " << feed->name().representation();
    //kDebug() << "Feed data: " << feed;

    mergeEntries(list, feed, provider);
}

void CoreEngine::slotEntriesFailed()
{
    EntryLoader *loader = dynamic_cast<EntryLoader*>(sender());
    delete loader;
    m_activefeeds--;

    emit signalEntriesFailed();
}

void CoreEngine::slotProgress(KJob *job, unsigned long percent)
{
    QString url;
    KIO::FileCopyJob * copyJob = qobject_cast<KIO::FileCopyJob*>(job);
    KIO::TransferJob * transferJob = qobject_cast<KIO::TransferJob*>(job);
    if (copyJob != NULL) {
        url = copyJob->srcUrl().fileName();
    } else if (transferJob != NULL) {
        url = transferJob->url().fileName();
    }

    QString message = i18n("loading %1",url);
    emit signalProgress(message, percent);
}

void CoreEngine::slotPayloadResult(KJob *job)
{
    // for some reason this slot is getting called 3 times on one job error
    if (m_entry_jobs.contains(job)) {
        Entry *entry = m_entry_jobs[job];
        m_entry_jobs.remove(job);

        if (job->error()) {
            kError() << "Cannot load payload file." << endl;
            kError() << job->errorString() << endl;

            emit signalPayloadFailed(entry);
        } else {
            KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
            m_payloadfiles[entry] = fcjob->destUrl().path();

            install(fcjob->destUrl().pathOrUrl());

            emit signalPayloadLoaded(fcjob->destUrl());
        }
    }
}

// FIXME: this should be handled more internally to return a (cached) preview image
void CoreEngine::slotPreviewResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot load preview file." << endl;
        kError() << job->errorString() << endl;

        m_entry_jobs.remove(job);
        emit signalPreviewFailed();
    } else {
        KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

        if (m_entry_jobs.contains(job)) {
            // now, assign temporary filename to entry and update entry cache
            Entry *entry = m_entry_jobs[job];
            m_entry_jobs.remove(job);
            m_previewfiles[entry] = fcjob->destUrl().path();
            cacheEntry(entry);
        }
        // FIXME: ignore if not? shouldn't happen...

        emit signalPreviewLoaded(fcjob->destUrl());
    }
}

void CoreEngine::slotUploadPayloadResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot upload payload file." << endl;
        kError() << job->errorString() << endl;

        m_uploadedentry = NULL;
        m_uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    }

    if (m_uploadedentry->preview().representation().isEmpty()) {
        // FIXME: we abuse 'job' here for the shortcut if there's no preview
        slotUploadPreviewResult(job);
        return;
    }

    KUrl sourcepreview = KUrl(m_uploadedentry->preview().representation());
    KUrl destfolder = m_uploadprovider->uploadUrl();

    destfolder.setFileName(sourcepreview.fileName());

    KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepreview, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(fcjob,
            SIGNAL(result(KJob*)),
            SLOT(slotUploadPreviewResult(KJob*)));
}

void CoreEngine::slotUploadPreviewResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot upload preview file." << endl;
        kError() << job->errorString() << endl;

        m_uploadedentry = NULL;
        m_uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    }

    // FIXME: the following save code is also in cacheEntry()
    // when we upload, the entry should probably be cached!

    // FIXME: adhere to meta naming rules as discussed
    KUrl sourcemeta = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".meta");
    KUrl destfolder = m_uploadprovider->uploadUrl();

    destfolder.setFileName(sourcemeta.fileName());

    EntryHandler eh(*m_uploadedentry);
    QDomElement exml = eh.entryXML();

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsupload");
    root.appendChild(exml);

    QFile f(sourcemeta.path());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kError() << "Cannot write meta information to '" << sourcemeta << "'." << endl;

        m_uploadedentry = NULL;
        m_uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    }
    QTextStream metastream(&f);
    metastream << root;
    f.close();

    KIO::FileCopyJob *fcjob = KIO::file_copy(sourcemeta, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(fcjob,
            SIGNAL(result(KJob*)),
            SLOT(slotUploadMetaResult(KJob*)));
}

void CoreEngine::slotUploadMetaResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot upload meta file." << endl;
        kError() << job->errorString() << endl;

        m_uploadedentry = NULL;
        m_uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    } else {
        m_uploadedentry = NULL;
        m_uploadprovider = NULL;

        //KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
        emit signalEntryUploaded();
    }
}

void CoreEngine::loadRegistry()
{
    KStandardDirs d;

    //kDebug() << "Loading registry of files for the component: " << m_componentname;

    QString realAppName = m_componentname.split(':')[0];

    // this must be same as in registerEntry()
    const QStringList dirs = d.findDirs("data", "knewstuff2-entries.registry");
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

            EntryHandler handler(stuff);
            if (!handler.isValid()) {
                kWarning() << "Invalid GHNS installation metadata.";
                continue;
            }

            Entry *e = handler.entryptr();
            e->setStatus(Entry::Installed);
            e->setSource(Entry::Registry);
            m_entry_registry.insert(id(e), e);
            //QString thisid = id(e);

            // we must overwrite cache entries with registered entries
            // and not just append the latter ones
            //if (entryCached(e)) {
            //    // it's in the cache, so replace the cache entry with the registered entry
            //    Entry * oldEntry = m_entry_index[thisid];
            //    int index = m_entry_cache.indexOf(oldEntry);
            //    m_entry_cache[index] = e;
            //    //delete oldEntry;
            //}
            //else {
            //    m_entry_cache.append(e);
            //}
            //m_entry_index[thisid] = e;
        }
    }
}

void CoreEngine::loadProvidersCache()
{
    KStandardDirs d;

    // use the componentname so we get the cache specific to this knsrc (kanagram, wallpaper, etc.)
    QString cachefile = d.findResource("cache", m_componentname + "kns2providers.cache.xml");
    if (cachefile.isEmpty()) {
        kDebug() << "Cache not present, skip loading.";
        return;
    }

    kDebug() << "Loading provider cache from file '" + cachefile + "'.";

    // make sure we can open and read the file
    bool ret;
    QFile f(cachefile);
    ret = f.open(QIODevice::ReadOnly);
    if (!ret) {
        kWarning() << "The file could not be opened.";
        return;
    }

    // make sure it's valid xml
    QDomDocument doc;
    ret = doc.setContent(&f);
    if (!ret) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    // make sure there's a root tag
    QDomElement root = doc.documentElement();
    if (root.tagName() != "ghnsproviders") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    // get the first provider
    QDomElement provider = root.firstChildElement("provider");
    if (provider.isNull()) {
        kWarning() << "Missing provider entries in the cache.";
        return;
    }

    // handle each provider
    while (!provider.isNull()) {
        ProviderHandler handler(provider);
        if (!handler.isValid()) {
            kWarning() << "Invalid provider metadata.";
            continue;
        }

        Provider *p = handler.providerptr();
        m_provider_cache.append(p);
        m_provider_index[pid(p)] = p;

        emit signalProviderLoaded(p);

        loadFeedCache(p);

        // no longer needed because EnginePrivate::slotProviderLoaded calls loadEntries
        //if (m_automationpolicy == AutomationOn) {
        //    loadEntries(p);
        //}

        provider = provider.nextSiblingElement("provider");
    }

    if (m_cachepolicy == CacheOnly) {
        emit signalEntriesFinished();
    }
}

void CoreEngine::loadFeedCache(Provider *provider)
{
    KStandardDirs d;

    kDebug() << "Loading feed cache.";

    QStringList cachedirs = d.findDirs("cache", m_componentname + "kns2feeds.cache");
    if (cachedirs.size() == 0) {
        kDebug() << "Cache directory not present, skip loading.";
        return;
    }
    QString cachedir = cachedirs.first();

    QStringList entrycachedirs = d.findDirs("cache", "knewstuff2-entries.cache/");
    if (entrycachedirs.size() == 0) {
        kDebug() << "Cache directory not present, skip loading.";
        return;
    }
    QString entrycachedir = entrycachedirs.first();

    kDebug() << "Load from directory: " + cachedir;

    QStringList feeds = provider->feeds();
    for (int i = 0; i < feeds.count(); i++) {
        Feed *feed = provider->downloadUrlFeed(feeds.at(i));
        QString feedname = feeds.at(i);

        QString idbase64 = QString(pid(provider).toUtf8().toBase64() + '-' + feedname);
        QString cachefile = cachedir + '/' + idbase64 + ".xml";

        kDebug() << "  + Load from file: " + cachefile;

        bool ret;
        QFile f(cachefile);
        ret = f.open(QIODevice::ReadOnly);
        if (!ret) {
            kWarning() << "The file could not be opened.";
            return;
        }

        QDomDocument doc;
        ret = doc.setContent(&f);
        if (!ret) {
            kWarning() << "The file could not be parsed.";
            return;
        }

        QDomElement root = doc.documentElement();
        if (root.tagName() != "ghnsfeeds") {
            kWarning() << "The file doesn't seem to be of interest.";
            return;
        }

        QDomElement entryel = root.firstChildElement("entry-id");
        if (entryel.isNull()) {
            kWarning() << "Missing entries in the cache.";
            return;
        }

        while (!entryel.isNull()) {
            QString idbase64 = entryel.text();
            //kDebug() << "loading cache for entry: " << QByteArray::fromBase64(idbase64.toUtf8());

            QString filepath = entrycachedir + '/' + idbase64 + ".meta";

            //kDebug() << "from file '" + filepath + "'.";

            // FIXME: pass feed and make loadEntryCache return void for consistency?
            Entry *entry = loadEntryCache(filepath);
            if (entry) {
                QString entryid = id(entry);

                if (m_entry_registry.contains(entryid)) {
                    Entry * registryEntry = m_entry_registry.value(entryid);
                    entry->setStatus(registryEntry->status());
                    entry->setInstalledFiles(registryEntry->installedFiles());
                }

                feed->addEntry(entry);
                //kDebug() << "entry " << entry->name().representation() << " loaded from cache";
                emit signalEntryLoaded(entry, feed, provider);
            }

            entryel = entryel.nextSiblingElement("entry-id");
        }
    }
}

KNS::Entry *CoreEngine::loadEntryCache(const QString& filepath)
{
    bool ret;
    QFile f(filepath);
    ret = f.open(QIODevice::ReadOnly);
    if (!ret) {
        kWarning() << "The file " << filepath << " could not be opened.";
        return NULL;
    }

    QDomDocument doc;
    ret = doc.setContent(&f);
    if (!ret) {
        kWarning() << "The file could not be parsed.";
        return NULL;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "ghnscache") {
        kWarning() << "The file doesn't seem to be of interest.";
        return NULL;
    }

    QDomElement stuff = root.firstChildElement("stuff");
    if (stuff.isNull()) {
        kWarning() << "Missing GHNS cache metadata.";
        return NULL;
    }

    EntryHandler handler(stuff);
    if (!handler.isValid()) {
        kWarning() << "Invalid GHNS installation metadata.";
        return NULL;
    }

    Entry *e = handler.entryptr();
    e->setStatus(Entry::Downloadable);
    m_entry_cache.append(e);
    m_entry_index[id(e)] = e;

    if (root.hasAttribute("previewfile")) {
        m_previewfiles[e] = root.attribute("previewfile");
        // FIXME: check here for a [ -f previewfile ]
    }

    if (root.hasAttribute("payloadfile")) {
        m_payloadfiles[e] = root.attribute("payloadfile");
        // FIXME: check here for a [ -f payloadfile ]
    }

    e->setSource(Entry::Cache);

    return e;
}

// FIXME: not needed anymore?
#if 0
void CoreEngine::loadEntriesCache()
{
    KStandardDirs d;

    //kDebug() << "Loading entry cache.";

    QStringList cachedirs = d.findDirs("cache", "knewstuff2-entries.cache/" + m_componentname);
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

void CoreEngine::shutdown()
{
    m_entry_index.clear();
    m_provider_index.clear();

    qDeleteAll(m_entry_cache);
    qDeleteAll(m_provider_cache);

    m_entry_cache.clear();
    m_provider_cache.clear();

    delete m_installation;
}

bool CoreEngine::providerCached(Provider *provider)
{
    if (m_cachepolicy == CacheNever) return false;

    if (m_provider_index.contains(pid(provider)))
        return true;
    return false;
}

bool CoreEngine::providerChanged(Provider *oldprovider, Provider *provider)
{
    QStringList oldfeeds = oldprovider->feeds();
    QStringList feeds = provider->feeds();
    if (oldfeeds.count() != feeds.count())
        return true;
    for (int i = 0; i < feeds.count(); i++) {
        Feed *oldfeed = oldprovider->downloadUrlFeed(feeds.at(i));
        Feed *feed = provider->downloadUrlFeed(feeds.at(i));
        if (!oldfeed)
            return true;
        if (feed->feedUrl() != oldfeed->feedUrl())
            return true;
    }
    return false;
}

void CoreEngine::mergeProviders(Provider::List providers)
{
    for (Provider::List::Iterator it = providers.begin(); it != providers.end(); ++it) {
        Provider *p = (*it);

        if (providerCached(p)) {
            kDebug() << "CACHE: hit provider " << p->name().representation();
            Provider *oldprovider = m_provider_index[pid(p)];
            if (providerChanged(oldprovider, p)) {
                kDebug() << "CACHE: update provider";
                cacheProvider(p);
                emit signalProviderChanged(p);
            }
            // oldprovider can now be deleted, see entry hit case
            // also take it out of m_provider_cache and m_provider_index
            //m_provider_cache.removeAll(oldprovider);
            //delete oldprovider;
        } else {
            if (m_cachepolicy != CacheNever) {
                kDebug() << "CACHE: miss provider " << p->name().representation();
                cacheProvider(p);
            }
            emit signalProviderLoaded(p);

            // no longer needed, because slotProviderLoaded calls loadEntries()
            //if (m_automationpolicy == AutomationOn) {
            //    loadEntries(p);
            //}
        }

        m_provider_cache.append(p);
        m_provider_index[pid(p)] = p;
    }

    emit signalProvidersFinished();
}

bool CoreEngine::entryCached(Entry *entry)
{
    if (m_cachepolicy == CacheNever) return false;

    // Direct cache lookup first
    // FIXME: probably better use URL (changes less frequently) and do iteration
    if (m_entry_index.contains(id(entry)) && m_entry_index[id(entry)]->source() == Entry::Cache) {
        return true;
    }

    // If entry wasn't found, either
    // - a translation was added which matches our locale better, or
    // - our locale preferences changed, or both.
    // In that case we've got to find the old name in the new entry,
    // since we assume that translations are always added but never removed.

    // BIGFIXME: the code below is incomplete, if we are looking for a translation
    // id(entry) will not work, as it uses the current locale to get the id

    for (int i = 0; i < m_entry_cache.count(); i++) {
        Entry *oldentry = m_entry_cache.at(i);
        if (id(entry) == id(oldentry)) return true;
        //QString lang = id(oldentry).section(":", 0, 0);
        //QString oldname = oldentry->name().translated(lang);
        //QString name = entry->name().translated(lang);
        ////kDebug() << "CACHE: compare entry names " << oldname << '/' << name;
        //if (name == oldname) return true;
    }

    return false;
}

bool CoreEngine::entryChanged(Entry *oldentry, Entry *entry)
{
    // possibly return true if the status changed? depends on when this is called
    if ((!oldentry) || (entry->releaseDate() > oldentry->releaseDate())
            || (entry->version() > oldentry->version())
            || (entry->release() > oldentry->release()))
        return true;
    return false;
}

void CoreEngine::mergeEntries(Entry::List entries, Feed *feed, const Provider *provider)
{
    for (Entry::List::Iterator it = entries.begin(); it != entries.end(); ++it) {
        // TODO: find entry in entrycache, replace if needed
        // don't forget marking as 'updateable'
        Entry *e = (*it);
        QString thisId = id(e);
        // set it to Installed if it's in the registry

        if (m_entry_registry.contains(thisId)) {
            // see if the one online is newer (higher version, release, or release date)
            Entry *registryentry = m_entry_registry[thisId];
            e->setInstalledFiles(registryentry->installedFiles());

            if (entryChanged(registryentry, e)) {
                e->setStatus(Entry::Updateable);
                emit signalEntryChanged(e);
            } else {
                // it hasn't changed, so set the status to that of the registry entry
                e->setStatus(registryentry->status());
            }

            if (entryCached(e)) {
                // in the registry and the cache, so take the cached one out
                Entry * cachedentry = m_entry_index[thisId];
                if (entryChanged(cachedentry, e)) {
                    //kDebug() << "CACHE: update entry";
                    cachedentry->setStatus(Entry::Updateable);
                    // entry has changed
                    if (m_cachepolicy != CacheNever) {
                        cacheEntry(e);
                    }
                    emit signalEntryChanged(e);
                }

                // take cachedentry out of the feed
                feed->removeEntry(cachedentry);
                //emit signalEntryRemoved(cachedentry, feed);
            } else {
                emit signalEntryLoaded(e, feed, provider);
            }

        } else {
            e->setStatus(Entry::Downloadable);

            if (entryCached(e)) {
                //kDebug() << "CACHE: hit entry " << e->name().representation();
                // FIXME: separate version updates from server-side translation updates?
                Entry *cachedentry = m_entry_index[thisId];
                if (entryChanged(cachedentry, e)) {
                    //kDebug() << "CACHE: update entry";
                    e->setStatus(Entry::Updateable);
                    // entry has changed
                    if (m_cachepolicy != CacheNever) {
                        cacheEntry(e);
                    }
                    emit signalEntryChanged(e);
                    // FIXME: cachedentry can now be deleted, but it's still in the list!
                    // FIXME: better: assigne all values to 'e', keeps refs intact
                }
                // take cachedentry out of the feed
                feed->removeEntry(cachedentry);
                //emit signalEntryRemoved(cachedentry, feed);
            } else {
                if (m_cachepolicy != CacheNever) {
                    //kDebug() << "CACHE: miss entry " << e->name().representation();
                    cacheEntry(e);
                }
                emit signalEntryLoaded(e, feed, provider);
            }

            m_entry_cache.append(e);
            m_entry_index[thisId] = e;
        }
    }

    if (m_cachepolicy != CacheNever) {
        // extra code to get the feedname from the provider, we could use feed->name().representation()
        // but would need to remove spaces, and latinize it since it can be any encoding
        // besides feeds.size() has a max of 4 currently (unsorted, score, downloads, and latest)
        QStringList feeds = provider->feeds();
        QString feedname;
        for (int i = 0; i < feeds.size(); ++i) {
            if (provider->downloadUrlFeed(feeds[i]) == feed) {
                feedname = feeds[i];
            }
        }
        cacheFeed(provider, feedname, feed, entries);
    }

    emit signalEntriesFeedFinished(feed);
    if (m_activefeeds == 0) {
        emit signalEntriesFinished();
    }
}

void CoreEngine::cacheProvider(Provider *provider)
{
    KStandardDirs d;

    kDebug() << "Caching provider.";

    QString cachedir = d.saveLocation("cache");
    QString cachefile = cachedir + m_componentname + "kns2providers.cache.xml";

    kDebug() << " + Save to file '" + cachefile + "'.";

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsproviders");

    for (Provider::List::Iterator it = m_provider_cache.begin(); it != m_provider_cache.end(); ++it) {
        Provider *p = (*it);
        ProviderHandler ph(*p);
        QDomElement pxml = ph.providerXML();
        root.appendChild(pxml);
    }
    ProviderHandler ph(*provider);
    QDomElement pxml = ph.providerXML();
    root.appendChild(pxml);

    QFile f(cachefile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kError() << "Cannot write meta information to '" << cachedir << "'." << endl;
        // FIXME: ignore?
        return;
    }
    QTextStream metastream(&f);
    metastream << root;
    f.close();

    /*QStringList feeds = p->feeds();
    for(int i = 0; i < feeds.count(); i++) {
        Feed *feed = p->downloadUrlFeed(feeds.at(i));
        cacheFeed(p, feeds.at(i), feed);
    }*/
}

void CoreEngine::cacheFeed(const Provider *provider, const QString & feedname, const Feed *feed, Entry::List entries)
{
    // feed cache file is a list of entry-id's that are part of this feed
    KStandardDirs d;

    Q_UNUSED(feed);

    QString cachedir = d.saveLocation("cache", m_componentname + "kns2feeds.cache");

    QString idbase64 = QString(pid(provider).toUtf8().toBase64() + '-' + feedname);
    QString cachefile = idbase64 + ".xml";

    kDebug() << "Caching feed to file '" + cachefile + "'.";

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsfeeds");
    for (int i = 0; i < entries.count(); i++) {
        QString idbase64 = id(entries.at(i)).toUtf8().toBase64();
        QDomElement entryel = doc.createElement("entry-id");
        root.appendChild(entryel);
        QDomText entrytext = doc.createTextNode(idbase64);
        entryel.appendChild(entrytext);
    }

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

void CoreEngine::cacheEntry(Entry *entry)
{
    KStandardDirs d;

    QString cachedir = d.saveLocation("cache", "knewstuff2-entries.cache/");

    kDebug() << "Caching entry in directory '" + cachedir + "'.";

    //FIXME: this must be deterministic, but it could also be an OOB random string
    //which gets stored into <ghnscache> just like preview...
    QString idbase64 = QString(id(entry).toUtf8().toBase64());
    QString cachefile = idbase64 + ".meta";

    kDebug() << "Caching to file '" + cachefile + "'.";

    // FIXME: adhere to meta naming rules as discussed
    // FIXME: maybe related filename to base64-encoded id(), or the reverse?

    EntryHandler eh(*entry);
    QDomElement exml = eh.entryXML();

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnscache");
    root.appendChild(exml);

    if (m_previewfiles.contains(entry)) {
        root.setAttribute("previewfile", m_previewfiles[entry]);
    }
    /*if (m_payloadfiles.contains(entry)) {
        root.setAttribute("payloadfile", m_payloadfiles[entry]);
    }*/

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

void CoreEngine::registerEntry(Entry *entry)
{
    m_entry_registry.insert(id(entry), entry);
    KStandardDirs d;

    //kDebug() << "Registering entry.";

    // NOTE: this directory must match loadRegistry
    QString registrydir = d.saveLocation("data", "knewstuff2-entries.registry");

    //kDebug() << " + Save to directory '" + registrydir + "'.";

    // FIXME: see cacheEntry() for naming-related discussion
    QString registryfile = QString(id(entry).toUtf8().toBase64()) + ".meta";

    //kDebug() << " + Save to file '" + registryfile + "'.";

    EntryHandler eh(*entry);
    QDomElement exml = eh.entryXML();

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsinstall");
    root.appendChild(exml);

    if (m_payloadfiles.contains(entry)) {
        root.setAttribute("payloadfile", m_payloadfiles[entry]);
    }

    QFile f(registrydir + registryfile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kError() << "Cannot write meta information to '" << registrydir + registryfile << "'." << endl;
        // FIXME: ignore?
        return;
    }
    QTextStream metastream(&f);
    metastream << root;
    f.close();
}

void KNS::CoreEngine::unregisterEntry(Entry * entry)
{
    KStandardDirs d;

    // NOTE: this directory must match loadRegistry
    QString registrydir = d.saveLocation("data", "knewstuff2-entries.registry");

    // FIXME: see cacheEntry() for naming-related discussion
    QString registryfile = QString(id(entry).toUtf8().toBase64()) + ".meta";

    QFile::remove(registrydir + registryfile);

    // remove the entry from m_entry_registry
    m_entry_registry.remove(id(entry));
}

QString CoreEngine::id(Entry *e)
{
    // This is the primary key of an entry:
    // A lookup on the name, which must exist but might be translated
    // This requires some care for comparison since translations might be added
    return m_componentname + e->name().language() + ':' + e->name().representation();
}

QString CoreEngine::pid(const Provider *p)
{
    // This is the primary key of a provider:
    // The download URL, which is never translated
    // If no download URL exists, a feed or web service URL must exist
    // if (p->downloadUrl().isValid())
    // return p->downloadUrl().url();
    QStringList feeds = p->feeds();
    for (int i = 0; i < feeds.count(); i++) {
        QString feedtype = feeds.at(i);
        Feed *f = p->downloadUrlFeed(feedtype);
        if (f->feedUrl().isValid())
            return m_componentname + f->feedUrl().url();
    }
    if (p->webService().isValid())
        return m_componentname + p->webService().url();
    return m_componentname;
}

bool CoreEngine::install(const QString &payloadfile)
{
    QList<Entry*> entries = m_payloadfiles.keys(payloadfile);
    if (entries.size() != 1) {
        // FIXME: shouldn't ever happen - make this an assertion?
        kError() << "ASSERT: payloadfile is not associated" << endl;
        return false;
    }
    Entry *entry = entries.first();

    bool update = (entry->status() == Entry::Updateable);
    // FIXME: this is only so exposing the KUrl suffices for downloaded entries
    entry->setStatus(Entry::Installed);

    // FIXME: first of all, do the security stuff here
    // this means check sum comparison and signature verification
    // signature verification might take a long time - make async?!

    if (m_installation->checksumPolicy() != Installation::CheckNever) {
        if (entry->checksum().isEmpty()) {
            if (m_installation->checksumPolicy() == Installation::CheckIfPossible) {
                //kDebug() << "Skip checksum verification";
            } else {
                kError() << "Checksum verification not possible" << endl;
                return false;
            }
        } else {
            //kDebug() << "Verify checksum...";
        }
    }
    if (m_installation->signaturePolicy() != Installation::CheckNever) {
        if (entry->signature().isEmpty()) {
            if (m_installation->signaturePolicy() == Installation::CheckIfPossible) {
                //kDebug() << "Skip signature verification";
            } else {
                kError() << "Signature verification not possible" << endl;
                return false;
            }
        } else {
            //kDebug() << "Verify signature...";
        }
    }

    //kDebug() << "INSTALL resourceDir " << m_installation->standardResourceDir();
    //kDebug() << "INSTALL targetDir " << m_installation->targetDir();
    //kDebug() << "INSTALL installPath " << m_installation->installPath();
    //kDebug() << "INSTALL + scope " << m_installation->scope();
    //kDebug() << "INSTALL + customName" << m_installation->customName();
    //kDebug() << "INSTALL + uncompression " << m_installation->uncompression();
    //kDebug() << "INSTALL + command " << m_installation->command();

    // Collect all files that were installed
    QStringList installedFiles;
    QString installpath(payloadfile);
    if (!m_installation->isRemote()) {
        // installdir is the target directory
        QString installdir;
        // installpath also contains the file name if it's a single file, otherwise equal to installdir
        int pathcounter = 0;
        if (!m_installation->standardResourceDir().isEmpty()) {
            if (m_installation->scope() == Installation::ScopeUser) {
                installdir = KStandardDirs::locateLocal(m_installation->standardResourceDir().toUtf8(), "/");
            } else { // system scope
                installdir = KStandardDirs::installPath(m_installation->standardResourceDir().toUtf8());
            }
            pathcounter++;
        }
        if (!m_installation->targetDir().isEmpty()) {
            if (m_installation->scope() == Installation::ScopeUser) {
                installdir = KStandardDirs::locateLocal("data", m_installation->targetDir() + '/');
            } else { // system scope
                installdir = KStandardDirs::installPath("data") + m_installation->targetDir() + '/';
            }
            pathcounter++;
        }
        if (!m_installation->installPath().isEmpty()) {
#if defined(Q_WS_WIN)
#ifndef _WIN32_WCE
            WCHAR wPath[MAX_PATH+1];
            if ( SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
                installdir = QString::fromUtf16((const ushort *) wPath) + QLatin1Char('/') + m_installation->installPath() + QLatin1Char('/');
            } else {
#endif
                installdir =  QDir::home().path() + QLatin1Char('/') + m_installation->installPath() + QLatin1Char('/');
#ifndef _WIN32_WCE
            }
#endif
#else
            installdir = QDir::home().path() + '/' + m_installation->installPath() + '/';
#endif
            pathcounter++;
        }
        if (!m_installation->absoluteInstallPath().isEmpty()) {
            installdir = m_installation->absoluteInstallPath() + '/';
            pathcounter++;
        }
        if (pathcounter != 1) {
            kError() << "Wrong number of installation directories given." << endl;
            return false;
        }

        kDebug() << "installdir: " << installdir;
        bool isarchive = true;

        // respect the uncompress flag in the knsrc
        if (m_installation->uncompression() == "always" || m_installation->uncompression() == "archive") {
            // this is weird but a decompression is not a single name, so take the path instead
            installpath = installdir;
            KMimeType::Ptr mimeType = KMimeType::findByPath(payloadfile);
            //kDebug() << "Postinstallation: uncompress the file";

            // FIXME: check for overwriting, malicious archive entries (../foo) etc.
            // FIXME: KArchive should provide "safe mode" for this!
            KArchive *archive = 0;

            if (mimeType->name() == "application/zip") {
                archive = new KZip(payloadfile);
            } else if (mimeType->name() == "application/tar"
                       || mimeType->name() == "application/x-gzip"
                       || mimeType->name() == "application/x-bzip"
                       || mimeType->name() == "application/x-lzma"
                       || mimeType->name() == "application/x-xz") {
                archive = new KTar(payloadfile);
            } else {
                delete archive;
                kError() << "Could not determine type of archive file '" << payloadfile << "'";
                if (m_installation->uncompression() == "always") {
                    return false;
                }
                isarchive = false;
            }

            if (isarchive) {
                bool success = archive->open(QIODevice::ReadOnly);
                if (!success) {
                    kError() << "Cannot open archive file '" << payloadfile << "'";
                    if (m_installation->uncompression() == "always") {
                        return false;
                    }
                    // otherwise, just copy the file
                    isarchive = false;
                }

                if (isarchive) {
                    const KArchiveDirectory *dir = archive->directory();
                    dir->copyTo(installdir);

                    installedFiles << archiveEntries(installdir, dir);
                    installedFiles << installdir + '/';

                    archive->close();
                    QFile::remove(payloadfile);
                    delete archive;
                }
            }
        }

        kDebug() << "isarchive: " << isarchive;

        if (m_installation->uncompression() == "never" || (m_installation->uncompression() == "archive" && !isarchive)) {
            // no decompress but move to target

            /// @todo when using KIO::get the http header can be accessed and it contains a real file name.
            // FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
            KUrl source = KUrl(entry->payload().representation());
            kDebug() << "installing non-archive from " << source.url();
            QString installfile;
            QString ext = source.fileName().section('.', -1);
            if (m_installation->customName()) {
                installfile = entry->name().representation();
                installfile += '-' + entry->version();
                if (!ext.isEmpty()) installfile += '.' + ext;
            } else {
                installfile = source.fileName();
            }
            installpath = installdir + '/' + installfile;

            //kDebug() << "Install to file " << installpath;
            // FIXME: copy goes here (including overwrite checking)
            // FIXME: what must be done now is to update the cache *again*
            //        in order to set the new payload filename (on root tag only)
            //        - this might or might not need to take uncompression into account
            // FIXME: for updates, we might need to force an overwrite (that is, deleting before)
            QFile file(payloadfile);
            bool success = true;

            if (QFile::exists(installpath) && update) {
                success = QFile::remove(installpath);
            }
            if (success) {
                success = file.rename(installpath);
            }
            if (!success) {
                kError() << "Cannot move file '" << payloadfile << "' to destination '"  << installpath << "'";
                return false;
            }
            installedFiles << installpath;
            installedFiles << installdir + '/';
        }
    }

    entry->setInstalledFiles(installedFiles);

    if (!m_installation->command().isEmpty()) {
        KProcess process;
        QString command(m_installation->command());
        QString fileArg(KShell::quoteArg(installpath));
        command.replace("%f", fileArg);

        //kDebug() << "Postinstallation: execute command";
        //kDebug() << "Command is: " << command;

        process.setShellCommand(command);
        int exitcode = process.execute();

        if (exitcode) {
            kError() << "Command failed" << endl;
        } else {
            //kDebug() << "Command executed successfully";
        }
    }

    // ==== FIXME: security code below must go above, when async handling is complete ====

    // FIXME: security object lifecycle - it is a singleton!
    Security *sec = Security::ref();

    connect(sec,
            SIGNAL(validityResult(int)),
            SLOT(slotInstallationVerification(int)));

    // FIXME: change to accept filename + signature
    sec->checkValidity(QString());

    m_payloadfiles[entry] = installpath;
    registerEntry(entry);
    // FIXME: hm, do we need to update the cache really?
    // only registration is probably needed here

    emit signalEntryChanged(entry);

    return true;
}

bool CoreEngine::uninstall(KNS::Entry *entry)
{
    entry->setStatus(Entry::Deleted);

    if (!m_installation->uninstallCommand().isEmpty()) {
        KProcess process;
        foreach (const QString& file, entry->installedFiles()) {
            QFileInfo info(file);
            if (info.isFile()) {
                QString fileArg(KShell::quoteArg(file));
                QString command(m_installation->uninstallCommand());
                command.replace("%f", fileArg);

                process.setShellCommand(command);
                int exitcode = process.execute();

                if (exitcode) {
                    kError() << "Command failed" << endl;
                } else {
                    //kDebug() << "Command executed successfully";
                }
            }
        }
    }

    foreach(const QString &file, entry->installedFiles()) {
        if (file.endsWith('/')) {
            QDir dir;
            bool worked = dir.rmdir(file);
            if (!worked) {
                // Maybe directory contains user created files, ignore it
                continue;
            }
        } else {
            if (QFile::exists(file)) {
                bool worked = QFile::remove(file);
                if (!worked) {
                    kWarning() << "unable to delete file " << file;
                    return false;
                }
            } else {
                kWarning() << "unable to delete file " << file << ". file does not exist.";
            }
        }
    }
    entry->setUnInstalledFiles(entry->installedFiles());
    entry->setInstalledFiles(QStringList());
    unregisterEntry(entry);

    emit signalEntryChanged(entry);

    return true;
}

void CoreEngine::slotInstallationVerification(int result)
{
    //kDebug() << "SECURITY result " << result;

    if (result & Security::SIGNED_OK)
        emit signalInstallationFinished();
    else
        emit signalInstallationFailed();
}

void CoreEngine::setAutomationPolicy(AutomationPolicy policy)
{
    m_automationpolicy = policy;
}

void CoreEngine::setCachePolicy(CachePolicy policy)
{
    m_cachepolicy = policy;
}

QStringList KNS::CoreEngine::archiveEntries(const QString& path, const KArchiveDirectory * dir)
{
    QStringList files;
    foreach(const QString &entry, dir->entries()) {
        QString childPath = path + '/' + entry;
        if (dir->entry(entry)->isFile()) {
            files << childPath;
        }

        if (dir->entry(entry)->isDirectory()) {
            const KArchiveDirectory* childDir = static_cast<const KArchiveDirectory*>(dir->entry(entry));
            files << archiveEntries(childPath, childDir);
            files << childPath + '/';
        }
    }
    return files;
}


#include "coreengine.moc"
