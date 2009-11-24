/*
    knewstuff3/engine.cpp
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
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

#include "config-knewstuff3.h"

#include "entry.h"
#include "core/installation.h"
#include "core/xmlloader.h"

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
#include <ktoolinvocation.h>

#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QtCore/Q_PID>

#if defined(Q_OS_WIN)
#include <windows.h>
#define _WIN32_IE 0x0500
#include <shlobj.h>
#endif

#if defined(HAVE_LIBATTICA)
#include "attica/atticaprovider.h"
#endif

#include "core/cache.h"
#include "staticxml/staticxmlprovider.h"

class KNS3::Engine::ProviderInformation {
public:
    QSharedPointer<Provider>  provider;
    int pagesInCurrentRequest;

    ProviderInformation()
        :provider(0)
    {}
    
    ProviderInformation(QSharedPointer<Provider>  p)
        :provider(p)
    {
        pagesInCurrentRequest = -1;
    }
};

class KNS3::Engine::Private {
    public:
        // If the provider is ready to be used
        bool initialized;
        Provider::SortMode sortMode;
        // handle installation of entries
        Installation* installation;
        // read/write cache of entries
        Cache* cache;
        QTimer* searchTimer;
        QString searchTerm;
        // The url of the file containg information about content providers
        QString providerFileUrl;
        // Categories to search in
        QStringList categories;
        QStringList categoriesPattern;

        QHash<QString, ProviderInformation> providers;

        // the name of the app that uses hot new stuff
        QString applicationName;

        QMap<Entry, QString> previewfiles; // why not in entry?


        QMap<KJob*, Entry> previewPictureJobs;

        // the current page that has been requested from providers
        int currentPage;
        // the page that was last requested, so it is not requested repeatedly
        int requestedPage;
        // when requesting entries from a provider, how many to ask for
        int pageSize;
        
        Private()
            : initialized(false)
            , sortMode(Provider::Rating)
            , installation(new Installation)
            , cache(new Cache)
            , searchTimer(new QTimer)
            , currentPage(0)
            , requestedPage(0)
            , pageSize(20)
        {
            searchTimer->setSingleShot(true);
            searchTimer->setInterval(1000);
        }
        
        ~Private()
        {
            delete searchTimer;
            delete installation;
            delete cache;
        }
};

using namespace KNS3;

Engine::Engine(QObject* parent)
        : QObject(parent), d(new Engine::Private)
{
    connect(d->searchTimer, SIGNAL(timeout()), SLOT(slotSearchTimerExpired()));
    connect(this, SIGNAL(signalEntryChanged(const KNS3::Entry&)), d->cache, SLOT(registerChangedEntry(const KNS3::Entry&)));
}

/* maybe better to disable copying alltogether?
Engine::Engine(const KNS3::Engine& other)
    : QObject(other.parent()), d(other.d)
{
}
*/

Engine::~Engine()
{
    d->cache->writeRegistry();
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
    
    KConfigGroup group;
    if (conf.hasGroup("KNewStuff3")) {
        kDebug() << "Loading KNewStuff3 config: " << configfile;
        group = conf.group("KNewStuff3");
    } else if (conf.hasGroup("KNewStuff2")) {
        kDebug() << "Loading KNewStuff2 config: " << configfile;
        group = conf.group("KNewStuff2");
    } else {
        kError() << "A knsrc file was found but it doesn't contain a KNewStuff3 section." << endl;
        return false;
    }

    d->categories = group.readEntry("Categories", QStringList());
    d->categoriesPattern = group.readEntry("CategoryPattern", QStringList());
    kDebug() << "Categories: " << d->categories << " pattern: " << d->categoriesPattern;
    d->providerFileUrl = group.readEntry("ProvidersUrl", QString());
    d->applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';
    
    // let installation read install specific config
    if (!d->installation->readConfig(group)) {
        return false;
    }
    
    connect(d->installation, SIGNAL(signalEntryChanged(const KNS3::Entry&)), SLOT(slotEntryChanged(const KNS3::Entry&)));

    d->cache->setRegistryFileName(d->applicationName.split(':')[0]);
    d->cache->readRegistry();
    
    d->initialized = true;

    // initialize providers at this point
    // then load the providersCache if caching is enabled
    loadProvidersCache();  // FIXME  do we really do anything useful here?

    // load the providers
    loadProviders();

    return true;
}

void Engine::loadProviders()
{
    kDebug(550) << "loading providers from " << d->providerFileUrl;

    XmlLoader * loader = new XmlLoader(this);
    connect(loader, SIGNAL(signalLoaded(const QDomDocument&)), SLOT(slotProviderFileLoaded(const QDomDocument&)));
    connect(loader, SIGNAL(signalFailed()), SLOT(slotProvidersFailed()));

    loader->load(KUrl(d->providerFileUrl));
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
            QSharedPointer<KNS3::Provider> provider;
            if (isAtticaProviderFile || p.attribute("type").toLower() == "rest") {
                #if defined(HAVE_LIBATTICA)
                provider = QSharedPointer<KNS3::Provider> (new AtticaProvider(d->categories, d->categoriesPattern));
                #else
                kDebug() << "KHotNewStuff compiled without attica support, could not load provider.";
                break;
                #endif
            } else {
                provider = QSharedPointer<KNS3::Provider> (new StaticXmlProvider);
            }

            connect(provider.data(), SIGNAL(providerInitialized(KNS3::Provider*)), SLOT(providerInitialized(KNS3::Provider*)));
            connect(provider.data(), SIGNAL(loadingFinished(KNS3::Provider::SortMode, const QString&,int,int,int, const KNS3::Entry::List&)),
                SLOT(slotEntriesLoaded(KNS3::Provider::SortMode, const QString&,int,int,int, const KNS3::Entry::List&)));
            connect(provider.data(), SIGNAL(payloadLinkLoaded(const KNS3::Entry&)), SLOT(downloadLinkLoaded(const KNS3::Entry&)));
            
            if (provider->setProviderXML(p)) {
                ProviderInformation providerInfo(provider);
                d->providers.insert(provider->id(), providerInfo);
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
    kDebug() << "providerInitialized" << p->name();
    p->setCachedEntries(d->cache->registryForProvider(p->id()));
    
    // TODO parameters according to search string etc
    p->loadEntries(d->sortMode, d->searchTerm, 0, d->pageSize);
}

void Engine::slotEntriesLoaded(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, KNS3::Entry::List entries)
{
    Q_UNUSED(sortMode)
    Q_UNUSED(searchstring)
    Q_UNUSED(page)
    Q_UNUSED(pageSize)
    Q_UNUSED(totalpages)
    kDebug() << "loaded " << page;
    d->currentPage = qMax<int>(page, d->currentPage);
    kDebug() << "current page" << d->currentPage;
    
    //d->cache->insertEntries(entries);
    d->cache->insertRequest(d->sortMode, d->searchTerm, d->currentPage, d->pageSize, entries);
    emit signalEntriesLoaded(entries);
}

void Engine::reloadEntries()
{
    emit signalResetView();
    d->currentPage = 0;
    d->requestedPage = 0;
    foreach (ProviderInformation p, d->providers) {
        if (p.provider->isInitialized()) {
            // FIXME: other parameters
            // FIXME use cache, if this request was sent already, take it from the cache

            int page = 0;
            while (true) {
                Entry::List cache = d->cache->requestFromCache(d->sortMode, d->searchTerm, page, d->pageSize);
                if (!cache.isEmpty()) {
                    kDebug() << "From cache";
                    emit signalEntriesLoaded(cache);
                    d->currentPage = page;
                    d->requestedPage = page;
                    ++page;
                } else {
                    break;
                }
            }
            if (page == 0) {
                kDebug() << "From provider";
                p.provider->loadEntries(d->sortMode, d->searchTerm, 0, d->pageSize);
            }
        }
    }
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (d->sortMode != mode) {
        d->currentPage = -1;
    }
    d->sortMode = mode;
}

void Engine::setSearchTerm(const QString& searchString)
{
    d->searchTimer->stop();
    d->searchTerm = searchString;
    Entry::List cache = d->cache->requestFromCache(d->sortMode, d->searchTerm, 0, 20);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer->start();
    }
}

void Engine::slotSearchTimerExpired()
{
    reloadEntries();
}

void Engine::slotRequestMoreData()
{
    kDebug() << "Get more data! cur "  << d->currentPage << " req " << d->requestedPage;

    if (d->currentPage < d->requestedPage) {
        return;
    }

    d->requestedPage++;

    foreach (ProviderInformation p, d->providers) {
        if (p.provider->isInitialized()) {
            // FIXME: other parameters
            // FIXME use cache, if this request was sent already, take it from the cache
            Entry::List cache = d->cache->requestFromCache(d->sortMode, d->searchTerm, d->requestedPage, 20);
            if (!cache.isEmpty()) {
                kDebug() << "From cache";
                emit signalEntriesLoaded(cache);
            } else {
                kDebug() << "From provider";
                p.provider->loadEntries(d->sortMode, d->searchTerm, d->requestedPage, d->pageSize);
            }
        }
    }
}

void Engine::slotPerformAction(KNS3::Engine::EntryAction action, Entry entry)
{
    kDebug(551) << "perform action: " << action;

    switch (action) {
    case ViewInfo:
        //if (provider && dxs) {
            //if (provider->webService().isValid()) {
            //    dxs->call_info();
            //} else {
                //slotInfo(provider->name().representation(),
                //         provider->webAccess().pathOrUrl(),
                //         QString());
            //}
        //}
        break;
    case Comments:
        // show the entry's comments
        //if (provider && dxs) {
        //    connect(dxs, SIGNAL(signalComments(QStringList)), this, SLOT(slotComments(QStringList)));
        //    dxs->call_comments(entry->idNumber());
        //}
        break;
    case Changes:
        // show the entry's changelog
        break;
    case ContactEmail:
        // invoke mail with the address of the author
        KToolInvocation::invokeMailer(entry.author().email(), i18n("Re: %1", entry.name()));
        break;
    case ContactJabber:
        // start jabber with author's info
        break;
    case CollabTranslate:
        // open translation dialog
        break;
    case CollabRemoval:
        // verify removal, maybe authenticate?
        break;
    case CollabSubscribe:
        // subscribe to changes
        break;
    case Uninstall:
        uninstall(entry);
        break;
    case Install:
        install(entry);
        break;
    case AddComment: {
        // open comment dialog
        //QPointer<KDXSComment> commentDialog = new KDXSComment(this);
        //int ret = commentDialog->exec();
        //if (ret == QDialog::Accepted) {
        //    QString s = commentDialog->comment();
            //if (dxs && !s.isEmpty()) {
            //    dxs->call_comment(entry->idNumber(), s);
            //}
        //}
    }
    break;
    case Rate: {
        // prompt for rating, and send to provider
    }
    break;
    }
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

        d->previewPictureJobs.remove(job);
        emit signalPreviewFailed();
    } else {
        KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

        if (d->previewPictureJobs.contains(job)) {
            // now, assign temporary filename to entry and update entry cache
            Entry entry = d->previewPictureJobs[job];
            d->previewPictureJobs.remove(job);
            d->previewfiles[entry] = fcjob->destUrl().path();
        }
        // FIXME: ignore if not? shouldn't happen...

        emit signalPreviewLoaded(fcjob->destUrl());
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

        //loadFeedCache(p);

        provider = provider.nextSiblingElement("provider");
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
    if (d->providers.contains(provider->id()))
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

void Engine::install(KNS3::Entry entry)
{
    if (entry.status() == Entry::Updateable) {
        entry.setStatus(Entry::Updating);
    } else  {
        entry.setStatus(Entry::Installing);
    }
    emit signalEntryChanged(entry);
    
    kDebug() << "Install " << entry.name()
        << " from: " << entry.providerId();
    ProviderInformation i = d->providers.value(entry.providerId());
    if (i.provider) {
        i.provider->loadPayloadLink(entry);
    }
}

void Engine::downloadLinkLoaded(const KNS3::Entry& entry)
{
    d->installation->install(entry);
}

void Engine::uninstall(KNS3::Entry entry)
{
    // FIXME: change the status?
    entry.setStatus(Entry::Installing);
    emit signalEntryChanged(entry);
    d->installation->uninstall(entry);
}

void Engine::slotEntryChanged(const KNS3::Entry& entry)
{
    emit signalEntryChanged(entry);
}

void Engine::slotInstallationFailed(const KNS3::Entry& entry)
{
    kDebug() << "Installation failed: " << entry.name();
    // FIXME implement warning?
}

#include "engine.moc"
