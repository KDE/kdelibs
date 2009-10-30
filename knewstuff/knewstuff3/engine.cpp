/*
    knewstuff3/engine.cpp
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

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

#include "engine.h"

#include "knewstuff3/core/entry.h"
#include "knewstuff3/core/installation.h"

#include "knewstuff3/xmlloader.h"


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


#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QtCore/Q_PID>

#if defined(Q_OS_WIN)
#include <windows.h>
#define _WIN32_IE 0x0500
#include <shlobj.h>
#endif
#include "attica/atticaprovider.h"
#include "staticxml/staticxmlprovider.h"

class KNS3::Engine::Private {
    public:
        QList<Provider*> providers;
        QString providerFileUrl;

        // holds all the entries
        QList<Entry> entries;

        // FIXME: die!
        // holds the registered entries mapped by their id
        QList<Entry> entry_registry;

        // KILL THIS:
        QMap<QString, Provider*> provider_index;

        // FIXME: die!
        QMap<QString, Entry> entry_index;

        Entry uploadedentry;

        //?
        Provider *uploadprovider;

        // the name of the app that uses hot new stuff
        QString applicationName;

        QMap<Entry, QString> previewfiles; // why not in entry?

        Installation *installation;

         // what is this? kill it?
        //int activefeeds;
        QString searchTerm;

        bool initialized;
        CachePolicy cachepolicy;

        QMap<KJob*, Entry> entry_jobs;

        Private()
            : uploadprovider(NULL), installation(NULL),
                initialized(false), cachepolicy(CacheNever)
        {
        }
        
};

using namespace KNS3;

Engine::Engine(QObject* parent)
        : QObject(parent), d(new Engine::Private)
{
    d->installation = new Installation(this);
}

/* maybe better to disable copying alltogether?
Engine::Engine(const KNS3::Engine& other)
    : QObject(other.parent()), d(other.d)
{
}
*/

Engine::~Engine()
{
    shutdown();
    delete d;
}

bool Engine::init(const QString &configfile)
{

    kDebug() << "Initializing KNS::Engine from '" << configfile << "'";

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
    d->providerFileUrl = group.readEntry("ProvidersUrl", QString());
    //d->componentname = group.readEntry("ComponentName", QString());
    d->applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';

    // FIXME: add support for several categories later on
    // FIXME: read out only when actually installing as a performance improvement?
    d->installation = new Installation(this);
    QString uncompresssetting = group.readEntry("Uncompress", QString("never"));
    // support old value of true as equivalent of always
    if (uncompresssetting == "true") {
        uncompresssetting = "always";
    }
    if (uncompresssetting != "always" && uncompresssetting != "archive" && uncompresssetting != "never") {
        kError() << "invalid Uncompress setting chosen, must be one of: always, archive, or never" << endl;
        return false;
    }
    d->installation->setUncompression(uncompresssetting);

    d->installation->setCommand(group.readEntry("InstallationCommand", QString()));
    d->installation->setUninstallCommand(group.readEntry("UninstallCommand", QString()));
    d->installation->setStandardResourceDir(group.readEntry("StandardResource", QString()));
    d->installation->setTargetDir(group.readEntry("TargetDir", QString()));
    d->installation->setInstallPath(group.readEntry("InstallPath", QString()));
    d->installation->setAbsoluteInstallPath(group.readEntry("AbsoluteInstallPath", QString()));
    d->installation->setCustomName(group.readEntry("CustomName", false));

    QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
    if (!checksumpolicy.isEmpty()) {
        if (checksumpolicy == "never")
            d->installation->setChecksumPolicy(Installation::CheckNever);
        else if (checksumpolicy == "ifpossible")
            d->installation->setChecksumPolicy(Installation::CheckIfPossible);
        else if (checksumpolicy == "always")
            d->installation->setChecksumPolicy(Installation::CheckAlways);
        else {
            kError() << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
    if (!signaturepolicy.isEmpty()) {
        if (signaturepolicy == "never")
            d->installation->setSignaturePolicy(Installation::CheckNever);
        else if (signaturepolicy == "ifpossible")
            d->installation->setSignaturePolicy(Installation::CheckIfPossible);
        else if (signaturepolicy == "always")
            d->installation->setSignaturePolicy(Installation::CheckAlways);
        else {
            kError() << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString scope = group.readEntry("Scope", QString());
    if (!scope.isEmpty()) {
        if (scope == "user")
            d->installation->setScope(Installation::ScopeUser);
        else if (scope == "system")
            d->installation->setScope(Installation::ScopeSystem);
        else {
            kError() << "The scope '" + scope + "' is unknown." << endl;
            return false;
        }

        if (d->installation->scope() == Installation::ScopeSystem) {
            if (!d->installation->installPath().isEmpty()) {
                kError() << "System installation cannot be mixed with InstallPath." << endl;
                return false;
            }
        }
    }

    QString cachePolicy = group.readEntry("CachePolicy", QString());
    if (!cachePolicy.isEmpty()) {
        if (cachePolicy == "never") {
            d->cachepolicy = CacheNever;
        } else if (cachePolicy == "replaceable") {
            d->cachepolicy = CacheReplaceable;
        } else if (cachePolicy == "resident") {
            d->cachepolicy = CacheResident;
        } else if (cachePolicy == "only") {
            d->cachepolicy = CacheOnly;
        } else {
            kError() << "Cache policy '" + cachePolicy + "' is unknown." << endl;
        }
    }
    kDebug() << "cache policy: " << cachePolicy;

    d->initialized = true;

    // load the registry first, so we know which entries are installed
    loadRegistry();
    
    // initialize providers at this point
    // then load the providersCache if caching is enabled
    if (d->cachepolicy != CacheNever) {
        loadProvidersCache();
    }

    // load the providers
    if (d->cachepolicy != CacheOnly) {
        loadProviders();
    }

    return true;
}

QString Engine::componentName() const
{
    if (!d->initialized) {
        return QString();
    }

    return d->applicationName;
}

void Engine::loadProviders()
{
    kDebug(550) << "loading providers from " << d->providerFileUrl;

    XmlLoader * loader = new XmlLoader(this);
    connect(loader, SIGNAL(signalLoaded(const QDomDocument&)), SLOT(slotProviderFileLoaded(const QDomDocument&)));
    connect(loader, SIGNAL(signalFailed()), SLOT(slotProvidersFailed()));

    loader->load(KUrl(d->providerFileUrl));
}

//void Engine::loadEntries(Provider *provider)
//{
//    //kDebug() << "loading entries";

//    if (d->cachepolicy == CacheOnly) {
//        return;
//    }

    //if (provider != d->provider_index[providerId(provider)]) {
    //    // this is the cached provider, and a new provider has been loaded from the internet
    //    // also, this provider's feeds have already been loaded including it's entries
    //    d->provider_cache.removeAll(provider); // just in case it's still in there
    //    return;
    //}

    //QStringList feeds = provider->availableFeeds();
    //for (int i = 0; i < feeds.count(); i++) {
    //    Feed *feed = provider->downloadUrlFeed(feeds.at(i));
    //    if (feed) {
    //        ++d->activefeeds;

    //        EntryLoader *entry_loader = new EntryLoader(this);

    //        connect(entry_loader,
    //                SIGNAL(signalEntriesLoaded(KNS::Entry::List)),
    //                SLOT(slotEntriesLoaded(KNS::Entry::List)));
    //        connect(entry_loader,
    //                SIGNAL(signalEntriesFailed()),
    //                SLOT(slotEntriesFailed()));
    //        connect(entry_loader,
    //                SIGNAL(signalProgress(KJob*, unsigned long)),
    //                SLOT(slotProgress(KJob*, unsigned long)));

    //        entry_loader->load(provider, feed);
    //    }
    //}
//}

//void Engine::downloadPreview(Entry *entry)
//{
//    if (d->previewfiles.contains(entry)) {
//        // FIXME: ensure somewhere else that preview file even exists
//        //kDebug() << "Reusing preview from '" << d->previewfiles[entry] << "'";
//        emit signalPreviewLoaded(KUrl::fromPath(d->previewfiles[entry]));
//        return;
//    }

//    KUrl source = KUrl(entry.preview().representation());

//    if (!source.isValid()) {
//        kError() << "The entry doesn't have a preview." << endl;
//        return;
//    }

//    KUrl destination = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10));
//    //kDebug() << "Downloading preview '" << source << "' to '" << destination << "'";

//    // FIXME: check for validity
//    KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
//    connect(job,
//            SIGNAL(result(KJob*)),
//            SLOT(slotPreviewResult(KJob*)));
//    connect(job,
//            SIGNAL(progress(KJob*, unsigned long)),
//            SLOT(slotProgress(KJob*, unsigned long)));

//    d->entry_jobs[job] = entry;
//}


bool Engine::uploadEntry(Provider *provider, const Entry& entry)
{
    //kDebug() << "Uploading " << entry.name().representation() << "...";

    //if (d->uploadedentry) {
    //    kError() << "Another upload is in progress!" << endl;
    //    return false;
    //}

    //if (!provider->uploadUrl().isValid()) {
    //    kError() << "The provider doesn't support uploads." << endl;
    //    return false;

    //    // FIXME: support for <noupload> will go here (file bundle creation etc.)
    //}

    //// FIXME: validate files etc.
    //d->uploadprovider = provider;
    //d->uploadedentry = entry;

    //KUrl sourcepayload = KUrl(entry.payload().representation());
    //KUrl destfolder = provider->uploadUrl();

    //destfolder.setFileName(sourcepayload.fileName());

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepayload, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadPayloadResult(KJob*)));

    return true;
}

void Engine::slotProviderFileLoaded(const QDomDocument& doc)
{

    kDebug() << "slotProvidersLoaded";

    // get each provider element, and create a provider object from it
    QDomElement providers = doc.documentElement();

    if (providers.tagName() != "ghnsproviders" &&
            providers.tagName() != "knewstuffproviders") {
        kWarning(550) << "No document in providers.xml.";
        emit signalProvidersFailed();
        return;
    }

    QDomNode n;
    for (n = providers.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement p = n.toElement();

        if (p.tagName() == "provider") {
            kDebug() << "Provider attributes: " << p.attribute("type");
            Provider* provider;
            if (p.attribute("type") == "rest") {
                provider = new AtticaProvider;
            } else {
                provider = new StaticXmlProvider;
            }
            connect(provider, SIGNAL(providerInitialized(KNS3::Provider*)), SLOT(providerInitialized(KNS3::Provider*)));
            
            if (provider->setProviderXML(p)) {
                d->providers.append(provider);
            }
        }
    }

    // note: this is only called from loading the online providers
    //ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
    //delete loader;

    //mergeProviders(list);
}

void Engine::slotProvidersFailed()
{
    //kDebug() << "slotProvidersFailed";
    //ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
    //delete loader;

    emit signalProvidersFailed();
}

void Engine::providerInitialized(Provider* p)
{
    kDebug() << "providerInitialized" << p->name().representation();
    emit signalProviderLoaded(p);

    connect(p, SIGNAL(loadingFinished(QString,QString,int,int,int,Entry::List)), SLOT(slotEntriesLoaded(QString,QString,int,int,int,Entry::List)));
    // TODO parameters according to search string etc
    p->loadEntries(QString(), d->searchTerm);
}

void Engine::slotEntriesLoaded(const QString& sortMode, const QString& searchstring, int page, int pageSize, int totalpages, Entry::List entries)
{
    emit signalEntriesLoaded(entries);
}

void Engine::slotEntriesFailed()
{
    //EntryLoader *loader = dynamic_cast<EntryLoader*>(sender());
    //delete loader;
    //d->activefeeds--;

    //emit signalEntriesFailed();
}

void Engine::reloadEntries()
{
    foreach (Provider* p, d->providers) {
        if (p->isInitialized()) {
            // FIXME: other parameters
            p->loadEntries(QString(), d->searchTerm);
        }
    }
}

void Engine::setSearchTerm(const QString& searchString)
{
    d->searchTerm = searchString;
}

void Engine::slotProgress(KJob *job, unsigned long percent)
{
    QString url;
    KIO::FileCopyJob * copyJob = qobject_cast<KIO::FileCopyJob*>(job);
    KIO::TransferJob * transferJob = qobject_cast<KIO::TransferJob*>(job);
    if (copyJob != NULL) {
        url = copyJob->srcUrl().fileName();
    } else if (transferJob != NULL) {
        url = transferJob->url().fileName();
    }

    QString message = QString("loading %1").arg(url);
    emit signalProgress(message, percent);
}

// FIXME: this should be handled more internally to return a (cached) preview image
void Engine::slotPreviewResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot load preview file." << endl;
        kError() << job->errorString() << endl;

        d->entry_jobs.remove(job);
        emit signalPreviewFailed();
    } else {
        KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

        if (d->entry_jobs.contains(job)) {
            // now, assign temporary filename to entry and update entry cache
            Entry entry = d->entry_jobs[job];
            d->entry_jobs.remove(job);
            d->previewfiles[entry] = fcjob->destUrl().path();
            cacheEntry(entry);
        }
        // FIXME: ignore if not? shouldn't happen...

        emit signalPreviewLoaded(fcjob->destUrl());
    }
}

void Engine::slotUploadPayloadResult(KJob *job)
{
    //if (job->error()) {
    //    kError() << "Cannot upload payload file." << endl;
    //    kError() << job->errorString() << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}

    //if (d->uploadedentry.preview().representation().isEmpty()) {
    //    // FIXME: we abuse 'job' here for the shortcut if there's no preview
    //    slotUploadPreviewResult(job);
    //    return;
    //}

    //KUrl sourcepreview = KUrl(d->uploadedentry.preview().representation());
    //KUrl destfolder = d->uploadprovider->uploadUrl();

    //destfolder.setFileName(sourcepreview.fileName());

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepreview, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadPreviewResult(KJob*)));
}

void Engine::slotUploadPreviewResult(KJob *job)
{
    //if (job->error()) {
    //    kError() << "Cannot upload preview file." << endl;
    //    kError() << job->errorString() << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}

    //// FIXME: the following save code is also in cacheEntry()
    //// when we upload, the entry should probably be cached!

    //// FIXME: adhere to meta naming rules as discussed
    //KUrl sourcemeta = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".meta");
    //KUrl destfolder = d->uploadprovider->uploadUrl();

    //destfolder.setFileName(sourcemeta.fileName());

    //EntryHandler eh(*d->uploadedentry);
    //QDomElement exml = eh.entryXML();

    //QDomDocument doc;
    //QDomElement root = doc.createElement("ghnsupload");
    //root.appendChild(exml);

    //QFile f(sourcemeta.path());
    //if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //    kError() << "Cannot write meta information to '" << sourcemeta << "'." << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}
    //QTextStream metastream(&f);
    //metastream << root;
    //f.close();

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcemeta, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadMetaResult(KJob*)));
}

void Engine::slotUploadMetaResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot upload meta file." << endl;
        kError() << job->errorString() << endl;

        d->uploadedentry = Entry();
        d->uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    } else {
        d->uploadedentry = Entry();
        d->uploadprovider = NULL;

        //KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
        emit signalEntryUploaded();
    }
}

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

void Engine::loadProvidersCache()
{
    KStandardDirs standardDirs;

    // use the componentname so we get the cache specific to this knsrc (kanagram, wallpaper, etc.)
    QString cachefile = standardDirs.findResource("cache", d->applicationName + "kns2providers.cache.xml");
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
        //ProviderHandler handler(provider);
        //if (!handler.isValid()) {
        //    kWarning() << "Invalid provider metadata.";
        //    continue;
        //}

        //Provider *p = handler.providerptr();
        //d->provider_cache.append(p);
        //d->provider_index[providerId(p)] = p;

        //emit signalProviderLoaded(p);

        //loadFeedCache(p);

        //// no longer needed because EnginePrivate::slotProviderLoaded calls loadEntries
        ////if (d->automationpolicy == AutomationOn) {
        ////    loadEntries(p);
        ////}

        provider = provider.nextSiblingElement("provider");
    }

    if (d->cachepolicy == CacheOnly) {
        emit signalEntriesFinished();
    }
}

void Engine::loadFeedCache(Provider *provider)
{
    KStandardDirs standardDirs;

    kDebug() << "Loading feed cache.";

    QStringList cachedirs = standardDirs.findDirs("cache", d->applicationName + "kns2feeds.cache");
    if (cachedirs.size() == 0) {
        kDebug() << "Cache directory not present, skip loading.";
        return;
    }
    QString cachedir = cachedirs.first();

    QStringList entrycachedirs = standardDirs.findDirs("cache", "knewstuff2-entries.cache/");
    if (entrycachedirs.size() == 0) {
        kDebug() << "Cache directory not present, skip loading.";
        return;
    }
    QString entrycachedir = entrycachedirs.first();

    kDebug() << "Load from directory: " + cachedir;

    QStringList feeds = provider->availableSortingCriteria();
    for (int i = 0; i < feeds.count(); i++) {
        //Feed *feed = provider->downloadUrlFeed(feeds.at(i));
        //QString feedname = feeds.at(i);

        //QString idbase64 = QString(providerId(provider).toUtf8().toBase64() + '-' + feedname);
        //QString cachefile = cachedir + '/' + idbase64 + ".xml";

        //kDebug() << "  + Load from file: " + cachefile;

        //bool ret;
        //QFile f(cachefile);
        //ret = f.open(QIODevice::ReadOnly);
        //if (!ret) {
        //    kWarning() << "The file could not be opened.";
        //    return;
        //}

        //QDomDocument doc;
        //ret = doc.setContent(&f);
        //if (!ret) {
        //    kWarning() << "The file could not be parsed.";
        //    return;
        //}

        //QDomElement root = doc.documentElement();
        //if (root.tagName() != "ghnsfeeds") {
        //    kWarning() << "The file doesn't seem to be of interest.";
        //    return;
        //}

        //QDomElement entryel = root.firstChildElement("entry-id");
        //if (entryel.isNull()) {
        //    kWarning() << "Missing entries in the cache.";
        //    return;
        //}

        //while (!entryel.isNull()) {
        //    QString idbase64 = entryel.text();
        //    //kDebug() << "loading cache for entry: " << QByteArray::fromBase64(idbase64.toUtf8());

        //    QString filepath = entrycachedir + '/' + idbase64 + ".meta";

        //    //kDebug() << "from file '" + filepath + "'.";

        //    // FIXME: pass feed and make loadEntryCache return void for consistency?
        //    Entry *entry = loadEntryCache(filepath);
        //    if (entry) {
        //        QString entryid = id(entry);

        //        if (d->entry_registry.contains(entryid)) {
        //            Entry * registryEntry = d->entry_registry.value(entryid);
        //            entry.setStatus(registryEntry->status());
        //            entry.setInstalledFiles(registryEntry->installedFiles());
        //        }

        //        feed->addEntry(entry);
        //        //kDebug() << "entry " << entry.name().representation() << " loaded from cache";
        //        emit signalEntryLoaded(entry, feed, provider);
        //    }

        //    entryel = entryel.nextSiblingElement("entry-id");
        //}
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
    d->entry_index[entryId(e)] = e;

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

void Engine::shutdown()
{
    d->entry_index.clear();
    d->provider_index.clear();

    qDeleteAll(d->providers);

    d->entries.clear();
    d->providers.clear();

    delete d->installation;
}

bool Engine::providerCached(Provider *provider)
{

    if (d->cachepolicy == CacheNever) return false;

    if (d->provider_index.contains(providerId(provider)))
        return true;
    return false;
}

bool Engine::providerChanged(Provider *oldprovider, Provider *provider)
{
    QStringList oldfeeds = oldprovider->availableSortingCriteria();
    QStringList feeds = provider->availableSortingCriteria();
    if (oldfeeds.count() != feeds.count())
        return true;
    for (int i = 0; i < feeds.count(); i++) {
        //Feed *oldfeed = oldprovider->downloadUrlFeed(feeds.at(i));
        //Feed *feed = provider->downloadUrlFeed(feeds.at(i));
        //if (!oldfeed)
        //    return true;
        //if (feed->feedUrl() != oldfeed->feedUrl())
        //    return true;
    }
    return false;
}

void Engine::mergeProviders(Provider::List providers)
{
    for (Provider::List::Iterator it = providers.begin(); it != providers.end(); ++it) {
        Provider *p = (*it);

        if (providerCached(p)) {
            kDebug() << "CACHE: hit provider " << p->name().representation();
            Provider *oldprovider = d->provider_index[providerId(p)];
            if (providerChanged(oldprovider, p)) {
                kDebug() << "CACHE: update provider";
                cacheProvider(p);
                emit signalProviderChanged(p);
            }
            // oldprovider can now be deleted, see entry hit case
            // also take it out of d->provider_cache and d->provider_index
            //d->provider_cache.removeAll(oldprovider);
            //delete oldprovider;
        } else {
            if (d->cachepolicy != CacheNever) {
                kDebug() << "CACHE: miss provider " << p->name().representation();
                cacheProvider(p);
            }
            emit signalProviderLoaded(p);

            // no longer needed, because slotProviderLoaded calls loadEntries()
            //if (d->automationpolicy == AutomationOn) {
            //    loadEntries(p);
            //}
        }

        d->providers.append(p);
        d->provider_index[providerId(p)] = p;
    }

    emit signalProvidersFinished();
}

bool Engine::entryCached(const Entry& entry)
{
    if (d->cachepolicy == CacheNever) return false;

    // Direct cache lookup first
    // FIXME: probably better use URL (changes less frequently) and do iteration
    if (d->entry_index.contains(entry.uniqueId()) && d->entry_index[entry.uniqueId()].source() == Entry::Cache) {
        return true;
    }

    // If entry wasn't found, either
    // - a translation was added which matches our locale better, or
    // - our locale preferences changed, or both.
    // In that case we've got to find the old name in the new entry,
    // since we assume that translations are always added but never removed.

    // BIGFIXME: the code below is incomplete, if we are looking for a translation
    // id(entry) will not work, as it uses the current locale to get the id

    for (int i = 0; i < d->entries.count(); i++) {
        Entry oldentry = d->entries.at(i);
        if (entryId(entry) == entryId(oldentry)) return true;
        //QString lang = id(oldentry).section(":", 0, 0);
        //QString oldname = oldentry.name().translated(lang);
        //QString name = entry.name().translated(lang);
        ////kDebug() << "CACHE: compare entry names " << oldname << '/' << name;
        //if (name == oldname) return true;
    }

    return false;
}

bool Engine::entryChanged(const Entry& oldentry, const Entry& entry)
{
    // possibly return true if the status changed? depends on when this is called
    if ((!oldentry.isValid()) || (entry.releaseDate() > oldentry.releaseDate())
            || (entry.version() > oldentry.version()))
        return true;
    return false;
}

//void Engine::mergeEntries(Entry::List entries, Feed *feed, const Provider *provider)
//{
//    for (Entry::List::Iterator it = entries.begin(); it != entries.end(); ++it) {
//        // TODO: find entry in entrycache, replace if needed
//        // don't forget marking as 'updateable'
//        Entry *e = (*it);
//        QString thisId = id(e);
//        // set it to Installed if it's in the registry

//        if (d->entry_registry.contains(thisId)) {
//            // see if the one online is newer (higher version, release, or release date)
//            Entry *registryentry = d->entry_registry[thisId];
//            e->setInstalledFiles(registryentry.installedFiles());

//            if (entryChanged(registryentry, e)) {
//                e->setStatus(Entry::Updateable);
//                emit signalEntryChanged(e);
//            } else {
//                // it hasn't changed, so set the status to that of the registry entry
//                e->setStatus(registryentry.status());
//            }

//            if (entryCached(e)) {
//                // in the registry and the cache, so take the cached one out
//                Entry * cachedentry = d->entry_index[thisId];
//                if (entryChanged(cachedentry, e)) {
//                    //kDebug() << "CACHE: update entry";
//                    cachedentry.setStatus(Entry::Updateable);
//                    // entry has changed
//                    if (d->cachepolicy != CacheNever) {
//                        cacheEntry(e);
//                    }
//                    emit signalEntryChanged(e);
//                }

//                // take cachedentry out of the feed
//                feed->removeEntry(cachedentry);
//                //emit signalEntryRemoved(cachedentry, feed);
//            } else {
//                emit signalEntryLoaded(e, feed, provider);
//            }

//        } else {
//            e->setStatus(Entry::Downloadable);

//            if (entryCached(e)) {
//                //kDebug() << "CACHE: hit entry " << e->name().representation();
//                // FIXME: separate version updates from server-side translation updates?
//                Entry *cachedentry = d->entry_index[thisId];
//                if (entryChanged(cachedentry, e)) {
//                    //kDebug() << "CACHE: update entry";
//                    e->setStatus(Entry::Updateable);
//                    // entry has changed
//                    if (d->cachepolicy != CacheNever) {
//                        cacheEntry(e);
//                    }
//                    emit signalEntryChanged(e);
//                    // FIXME: cachedentry can now be deleted, but it's still in the list!
//                    // FIXME: better: assigne all values to 'e', keeps refs intact
//                }
//                // take cachedentry out of the feed
//                feed->removeEntry(cachedentry);
//                //emit signalEntryRemoved(cachedentry, feed);
//            } else {
//                if (d->cachepolicy != CacheNever) {
//                    //kDebug() << "CACHE: miss entry " << e->name().representation();
//                    cacheEntry(e);
//                }
//                emit signalEntryLoaded(e, feed, provider);
//            }

//            d->entries.append(e);
//            d->entry_index[thisId] = e;
//        }
//    }

//    if (d->cachepolicy != CacheNever) {
//        // extra code to get the feedname from the provider, we could use feed->name().representation()
//        // but would need to remove spaces, and latinize it since it can be any encoding
//        // besides feeds.size() has a max of 4 currently (unsorted, score, downloads, and latest)
//        QStringList feeds = provider->feeds();
//        QString feedname;
//        for (int i = 0; i < feeds.size(); ++i) {
//            if (provider->downloadUrlFeed(feeds[i]) == feed) {
//                feedname = feeds[i];
//            }
//        }
//        cacheFeed(provider, feedname, feed, entries);
//    }

//    emit signalEntriesFeedFinished(feed);
//    if (d->activefeeds == 0) {
//        emit signalEntriesFinished();
//    }
//}

void Engine::cacheProvider(Provider *provider)
{
    KStandardDirs dirs;

    kDebug() << "Caching provider.";

    QString cachedir = dirs.saveLocation("cache");
    QString cachefile = cachedir + d->applicationName + "kns2providers.cache.xml";

    kDebug() << " + Save to file '" + cachefile + "'.";

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsproviders");

    for (Provider::List::Iterator it = d->providers.begin(); it != d->providers.end(); ++it) {
        Provider *p = (*it);
        QDomElement pxml = p->providerXML();
        root.appendChild(pxml);
    }
    //ProviderHandler ph(*provider);
    //QDomElement pxml = ph.providerXML();
    //root.appendChild(pxml);

    //QFile f(cachefile);
    //if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //    kError() << "Cannot write meta information to '" << cachedir << "'." << endl;
    //    // FIXME: ignore?
    //    return;
    //}
    //QTextStream metastream(&f);
    //metastream << root;
    //f.close();

    /*QStringList feeds = p->feeds();
    for(int i = 0; i < feeds.count(); i++) {
        Feed *feed = p->downloadUrlFeed(feeds.at(i));
        cacheFeed(p, feeds.at(i), feed);
    }*/
}

//void Engine::cacheFeed(const Provider *provider, const QString & feedname, const Feed *feed, Entry::List entries)
//{
//    // feed cache file is a list of entry-id's that are part of this feed
//    KStandardDirs d;

//    Q_UNUSED(feed);

//    QString cachedir = d.saveLocation("cache", d->componentname + "kns2feeds.cache");

//    QString idbase64 = QString(providerId(provider).toUtf8().toBase64() + '-' + feedname);
//    QString cachefile = idbase64 + ".xml";

//    kDebug() << "Caching feed to file '" + cachefile + "'.";

//    QDomDocument doc;
//    QDomElement root = doc.createElement("ghnsfeeds");
//    for (int i = 0; i < entries.count(); i++) {
//        QString idbase64 = id(entries.at(i)).toUtf8().toBase64();
//        QDomElement entryel = doc.createElement("entry-id");
//        root.appendChild(entryel);
//        QDomText entrytext = doc.createTextNode(idbase64);
//        entryel.appendChild(entrytext);
//    }

//    QFile f(cachedir + cachefile);
//    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        kError() << "Cannot write meta information to '" << cachedir + cachefile << "'." << endl;
//        // FIXME: ignore?
//        return;
//    }
//    QTextStream metastream(&f);
//    metastream << root;
//    f.close();
//}

void Engine::cacheEntry(const Entry& entry)
{
    KStandardDirs standardDirs;

    QString cachedir = standardDirs.saveLocation("cache", "knewstuff2-entries.cache/");

    kDebug() << "Caching entry in directory '" + cachedir + "'.";

    //FIXME: this must be deterministic, but it could also be an OOB random string
    //which gets stored into <ghnscache> just like preview...
    QString idbase64 = QString(entryId(entry).toUtf8().toBase64());
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
    /*if (d->payloadfiles.contains(entry)) {
        root.setAttribute("payloadfile", d->payloadfiles[entry]);
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

void Engine::registerEntry(const Entry& entry)
{
    d->entry_registry.append(entry);
    KStandardDirs standardDirs;

    //kDebug() << "Registering entry.";

    // NOTE: this directory must match loadRegistry
    QString registrydir = standardDirs.saveLocation("data", "knewstuff2-entries.registry");

    //kDebug() << " + Save to directory '" + registrydir + "'.";

    // FIXME: see cacheEntry() for naming-related discussion
    QString registryfile = QString(entryId(entry).toUtf8().toBase64()) + ".meta";

    //kDebug() << " + Save to file '" + registryfile + "'.";

// TODO: serialization of entries
/*
    QDomElement exml = entry.entryXML();

    QDomDocument doc;
    QDomElement root = doc.createElement("ghnsinstall");
    root.appendChild(exml);

    if (d->payloadfiles.contains(entry)) {
        root.setAttribute("payloadfile", d->payloadfiles[entry]);
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
*/
}

void KNS3::Engine::unregisterEntry(const Entry& entry)
{
    KStandardDirs standardDirs;

    // NOTE: this directory must match loadRegistry
    QString registrydir = standardDirs.saveLocation("data", "knewstuff2-entries.registry");

    // FIXME: see cacheEntry() for naming-related discussion
    QString registryfile = QString(entryId(entry).toUtf8().toBase64()) + ".meta";

    QFile::remove(registrydir + registryfile);

    // remove the entry from d->entry_registry
    d->entry_registry.removeAll(entry);
}

QString Engine::entryId(const Entry& e)
{
    // This is the primary key of an entry:
    // A lookup on the name, which must exist but might be translated
    // This requires some care for comparison since translations might be added
    return d->applicationName + e.name().language() + ':' + e.name().representation();
}

QString Engine::providerId(const Provider *p)
{
    // This is the primary key of a provider:
    // The download URL, which is never translated
    // If no download URL exists, a feed or web service URL must exist
    // if (p->downloadUrl().isValid())
    // return p->downloadUrl().url();
    QStringList feeds = p->availableSortingCriteria();
    for (int i = 0; i < feeds.count(); i++) {
        QString feedtype = feeds.at(i);
        //Feed *f = p->downloadUrlFeed(feedtype);
        //if (f->feedUrl().isValid())
        //    return d->componentname + f->feedUrl().url();
    }
    //if (p->webService().isValid())
    //    return d->componentname + p->webService().url();
    return d->applicationName;
}


bool Engine::install(const KNS3::Entry& entry)
{
    kDebug() << "Install " << entry.name().representation();
    d->installation->install(entry);
}

bool Engine::uninstall(const KNS3::Entry& entry)
{
    d->installation->uninstall(entry);
}

// maybe just have one method to update according to internal state of entry and emit signalEntryChanged
void Engine::slotInstallationFinished(const KNS3::Entry& entry)
{
    kDebug() << "Installation finished: " << entry.name().representation();
    registerEntry(entry);
    // FIXME: hm, do we need to update the cache really?
    // only registration is probably needed here
}

void Engine::slotInstallationFailed(const KNS3::Entry& entry)
{
    kDebug() << "Installation failed: " << entry.name().representation();
    // FIXME implement warning?
}

void Engine::slotUninstallFinished(const KNS3::Entry& entry)
{
    unregisterEntry(entry);
    emit signalEntryChanged(entry);
}

Engine::CollaborationFeatures Engine::collaborationFeatures(const KNS3::Entry& entry)
{
    /* FIXME the strings in provider_index need to match the entry.providerId() if we go with this
    Provider* p = d->provider_index[entry.providerId()];
    Q_ASSERT(p);
    CollaborationFeatures features;
    if (p->hasRatings()) {
        features |= Ratings;
    }
    if (p->hasCommenting()) {
        features |= Comments;
    }
    */
    
    CollaborationFeatures features;
    return features;
}

    
#include "engine.moc"
