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

#include "core/cache.h"
#include "staticxml/staticxmlprovider.h"

class KNS3::Engine::Private {
    public:
        QList<Provider*> providers;

        // The url of the file containg information about content providers
        QString providerFileUrl;
        // Categories to search in
        QStringList categories;

        // holds all the entries
        Entry::List entries;

        // KILL THIS:
        QMap<QString, Provider*> provider_index;


        //?
        //Provider* uploadprovider;
        //Entry uploadedentry;

        // the name of the app that uses hot new stuff
        QString applicationName;

        QMap<Entry, QString> previewfiles; // why not in entry?

        // handle installation of entries
        Installation* installation;
        // read/write cache of entries
        Cache* cache;
        
        QString searchTerm;
        Provider::SortMode sortMode;

        bool initialized;

        QMap<KJob*, Entry> entry_jobs;

        Private()
            : initialized(false)
            , sortMode(Provider::Rating)
            , installation(new Installation)
            , cache(new Cache)
        {
        }
        
        ~Private()
        {
            delete installation;
            delete cache;
        }
};

using namespace KNS3;

Engine::Engine(QObject* parent)
        : QObject(parent), d(new Engine::Private)
{
}

/* maybe better to disable copying alltogether?
Engine::Engine(const KNS3::Engine& other)
    : QObject(other.parent()), d(other.d)
{
}
*/

Engine::~Engine()
{
    d->cache->writeCache(d->entries);

    d->provider_index.clear();
    qDeleteAll(d->providers);
    d->providers.clear();

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
    d->categories = group.readEntry("Categories", QStringList());

    kDebug() << "Categories: " << d->categories;
    
    //d->componentname = group.readEntry("ComponentName", QString());
    d->applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';
    
    // let installation read install specific config
    if (!d->installation->readConfig(group)) {
        return false;
    }
    
    connect(d->installation, SIGNAL(signalInstallationFinished(Entry)), SLOT(slotEntryChanged(Entry)));
    connect(d->installation, SIGNAL(signalUninstallFinished(Entry)), SLOT(slotEntryChanged(Entry)));

    CachePolicy cachePolicy;
    QString cachePolicyString = group.readEntry("CachePolicy", QString());
    if (!cachePolicyString.isEmpty()) {
        if (cachePolicyString == "never") {
            cachePolicy = CacheNever;
        } else if (cachePolicyString == "replaceable") {
            cachePolicy = CacheReplaceable;
        } else if (cachePolicyString == "resident") {
            cachePolicy = CacheResident;
        } else if (cachePolicyString == "only") {
            cachePolicy = CacheOnly;
        } else {
            kError() << "Cache policy '" + cachePolicyString + "' is unknown." << endl;
        }
    }
    kDebug() << "cache policy: " << cachePolicyString;

    d->cache->setCacheFileName(d->applicationName.split(':')[0]);
    d->cache->setPolicy(cachePolicy);
    d->cache->readCache();
    
    d->initialized = true;


    /*
    // load the registry first, so we know which entries are installed
    loadRegistry();
    */
    
    // initialize providers at this point
    // then load the providersCache if caching is enabled
    if (d->cache->policy() != CacheNever) {
        loadProvidersCache();
    }

    // load the providers
    if (d->cache->policy() != CacheOnly) {
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

    bool isAtticaProviderFile = false;
    
    // get each provider element, and create a provider object from it
    QDomElement providers = doc.documentElement();

    if (providers.tagName() == "providers") {
        isAtticaProviderFile = true;
    } else if (providers.tagName() != "ghnsproviders" && providers.tagName() != "knewstuffproviders") {
        kWarning(550) << "No document in providers.xml.";
        emit signalError(i18n("Could not load get hot new stuff providers from file: %1", d->providerFileUrl));
        return;
    }

    QDomNode n;
    for (n = providers.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement p = n.toElement();

        if (p.tagName() == "provider") {
            kDebug() << "Provider attributes: " << p.attribute("type");
            Provider* provider;
            if (isAtticaProviderFile || p.attribute("type") == "rest") {
                provider = new AtticaProvider(d->categories);
            } else {
                provider = new StaticXmlProvider;
            }
            connect(provider, SIGNAL(providerInitialized(KNS3::Provider*)), SLOT(providerInitialized(KNS3::Provider*)));

            if (provider->setProviderXML(p)) {
                d->providers.append(provider);
            }
        }
    }
}

void Engine::slotProvidersFailed()
{
    emit signalError(i18n("Loading of providers from file: %1 failed", d->providerFileUrl));
}

void Engine::providerInitialized(Provider* p)
{
    kDebug() << "providerInitialized" << p->name().representation();

    // TODO provider->setCachedEntries(cacheForProvider(provider->id()));
    
    d->provider_index[p->id()] = p;
    
    emit signalProviderLoaded(p);

    connect(p, SIGNAL(loadingFinished(KNS3::Provider::SortMode,QString,int,int,int,Entry::List)), SLOT(slotEntriesLoaded(KNS3::Provider::SortMode,QString,int,int,int,Entry::List)));
    connect(p, SIGNAL(payloadLinkLoaded(const Entry&)), SLOT(downloadLinkLoaded(const Entry&)));
    
    // TODO parameters according to search string etc
    p->loadEntries(d->sortMode, d->searchTerm);
}

void Engine::slotEntriesLoaded(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, Entry::List entries)
{
    d->entries.append(entries);
    emit signalEntriesLoaded(entries);
}

void Engine::reloadEntries()
{
    foreach (Provider* p, d->providers) {
        if (p->isInitialized()) {
            // FIXME: other parameters
            p->loadEntries(d->sortMode, d->searchTerm);
        }
    }
}

void Engine::setSortMode(Provider::SortMode mode)
{
    d->sortMode = mode;
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
// kio caches images already (?) but running jobs should be stopped if no longer neeeded...
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
        }
        // FIXME: ignore if not? shouldn't happen...

        emit signalPreviewLoaded(fcjob->destUrl());
    }
}

/*
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
*/

/*
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
*/

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

        provider = provider.nextSiblingElement("provider");
    }

    if (d->cache->policy() == CacheOnly) {
        emit signalEntriesFinished();
    }
}

/* FIXME: decide what to do with this
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
}
*/


bool Engine::providerCached(Provider *provider)
{

    if (d->cache->policy() == CacheNever) return false;

    if (d->provider_index.contains(provider->id()))
        return true;
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

/* FIXME: decide what to do with this
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
    /*
}
*/
    
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

void Engine::install(const KNS3::Entry& entry)
{
    kDebug() << "Install " << entry.name().representation()
        << entry.providerId() << d->provider_index.keys();
    Provider* p = d->provider_index[entry.providerId()];

    p->loadPayloadLink(entry);
}

void Engine::downloadLinkLoaded(const Entry& entry)
{
    d->installation->install(entry);
}

void Engine::uninstall(const KNS3::Entry& entry)
{
    d->installation->uninstall(entry);
}

void Engine::slotEntryChanged(const KNS3::Entry& entry)
{
    emit signalEntryChanged(entry);
}

void Engine::slotInstallationFailed(const KNS3::Entry& entry)
{
    kDebug() << "Installation failed: " << entry.name().representation();
    // FIXME implement warning?
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
