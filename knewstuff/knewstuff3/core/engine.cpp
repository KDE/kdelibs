/*
    knewstuff3/engine.cpp
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
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

#include "entry.h"
#include "core/installation.h"
#include "core/xmlloader.h"
#include "ui/entrydetailsdialog.h"

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

#include "attica/atticaprovider.h"

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
        // handle installation of entries
        Installation* installation;
        // read/write cache of entries
        Cache* cache;
        QTimer* searchTimer;
        // The url of the file containing information about content providers
        QString providerFileUrl;
        // Categories from knsrc file
        QStringList categories;

        QHash<QString, ProviderInformation> providers;

        // the name of the app that uses hot new stuff
        QString applicationName;

        QMap<EntryInternal, QString> previewfiles; // why not in entry?

        QMap<KJob*, EntryInternal> previewPictureJobs;

        // the current request from providers
        Provider::SearchRequest currentRequest;

        // the page that is currently displayed, so it is not requested repeatedly
        int currentPage;

        // when requesting entries from a provider, how many to ask for
        int pageSize;

        Private()
            : initialized(false)
            , installation(new Installation)
            , cache(new Cache)
            , searchTimer(new QTimer)
            , currentPage(-1)
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
    connect(this, SIGNAL(signalEntryChanged(const KNS3::EntryInternal&)), d->cache, SLOT(registerChangedEntry(const KNS3::EntryInternal&)));
}

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

    kDebug() << "Categories: " << d->categories;
    d->providerFileUrl = group.readEntry("ProvidersUrl", QString());
    d->applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';

    // let installation read install specific config
    if (!d->installation->readConfig(group)) {
        return false;
    }

    connect(d->installation, SIGNAL(signalEntryChanged(const KNS3::EntryInternal&)), SLOT(slotEntryChanged(const KNS3::EntryInternal&)));

    d->cache->setRegistryFileName(d->applicationName.split(':')[0]);
    d->cache->readRegistry();

    d->initialized = true;

    // load the providers
    loadProviders();

    return true;
}

QStringList Engine::categories() const
{
    return d->categories;
}

QStringList Engine::categoriesFilter() const
{
    return d->currentRequest.categories;
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

    QDomElement n = providers.firstChildElement("provider");
    while (!n.isNull()) {
        kDebug() << "Provider attributes: " << n.attribute("type");

        QSharedPointer<KNS3::Provider> provider;
        if (isAtticaProviderFile || n.attribute("type").toLower() == "rest") {
            provider = QSharedPointer<KNS3::Provider> (new AtticaProvider(d->categories));
        } else {
            provider = QSharedPointer<KNS3::Provider> (new StaticXmlProvider);
        }

        connect(provider.data(), SIGNAL(providerInitialized(KNS3::Provider*)), SLOT(providerInitialized(KNS3::Provider*)));
        connect(provider.data(), SIGNAL(loadingFinished(KNS3::Provider::SearchRequest, KNS3::EntryInternal::List)),
                SLOT(slotEntriesLoaded(KNS3::Provider::SearchRequest, KNS3::EntryInternal::List)));
        connect(provider.data(), SIGNAL(payloadLinkLoaded(const KNS3::EntryInternal&)), SLOT(downloadLinkLoaded(const KNS3::EntryInternal&)));
        connect(provider.data(), SIGNAL(jobStarted(KJob*)), this, SLOT(providerJobStarted(KJob*)));

        if (provider->setProviderXML(n)) {
            ProviderInformation providerInfo(provider);
            d->providers.insert(provider->id(), providerInfo);
        }
        n = n.nextSiblingElement();
    }
}

void Engine::providerJobStarted ( KJob* job )
{
    emit jobStarted(job, i18n("Loading data from provider"));
}

void Engine::slotProvidersFailed()
{
    emit signalError(i18n("Loading of providers from file: %1 failed", d->providerFileUrl));
}

void Engine::providerInitialized(Provider* p)
{
    kDebug() << "providerInitialized" << p->name();
    p->setCachedEntries(d->cache->registryForProvider(p->id()));

    p->loadEntries(d->currentRequest);
}

void Engine::slotEntriesLoaded(const KNS3::Provider::SearchRequest& request, KNS3::EntryInternal::List entries)
{
    d->currentPage = qMax<int>(request.page, d->currentPage);
    kDebug() << "loaded page " << request.page << "current page" << d->currentPage;

    d->cache->insertRequest(request, entries);
    emit signalEntriesLoaded(entries);
}

void Engine::reloadEntries()
{
    emit signalResetView();
    d->currentPage = -1;
    d->currentRequest.page = 0;

    foreach (const ProviderInformation &p, d->providers) {
        if (p.provider->isInitialized()) {
            if (d->currentRequest.sortMode == Provider::Installed) {
                // when asking for installed entries, never use the cache
                p.provider->loadEntries(d->currentRequest);
            } else {
                // take entries from cache until there are no more
                EntryInternal::List cache = d->cache->requestFromCache(d->currentRequest);
                while (!cache.isEmpty()) {
                    kDebug() << "From cache";
                    emit signalEntriesLoaded(cache);

                    d->currentPage = d->currentRequest.page;
                    ++d->currentRequest.page;
                    cache = d->cache->requestFromCache(d->currentRequest);
                }
                // if the cache was empty, request data from provider
                if (d->currentPage == -1) {
                    kDebug() << "From provider";
                    p.provider->loadEntries(d->currentRequest);
                }
            }
        }
    }
}

void Engine::setCategoriesFilter(const QStringList& categories)
{
    d->currentRequest.categories = categories;
    reloadEntries();
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (d->currentRequest.sortMode != mode) {
        d->currentRequest.page = -1;
    }
    d->currentRequest.sortMode = mode;
    reloadEntries();
}

void Engine::setSearchTerm(const QString& searchString)
{
    d->searchTimer->stop();
    d->currentRequest.searchTerm = searchString;
    EntryInternal::List cache = d->cache->requestFromCache(d->currentRequest);
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

void Engine::requestMoreData()
{
    kDebug() << "Get more data! current page: " << d->currentPage  << " requested: " << d->currentRequest.page;

    if (d->currentPage < d->currentRequest.page) {
        return;
    }

    d->currentRequest.page++;

    foreach (const ProviderInformation &p, d->providers) {
        if (p.provider->isInitialized()) {
            p.provider->loadEntries(d->currentRequest);
        }
    }
}

void Engine::slotPerformAction(KNS3::Engine::EntryAction action, EntryInternal entry)
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
    case ShowDetails: {
        EntryDetailsDialog dialog(this, entry, 0);
        dialog.exec();
        break;
    }
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
            EntryInternal entry = d->previewPictureJobs[job];
            d->previewPictureJobs.remove(job);
            d->previewfiles[entry] = fcjob->destUrl().path();
        }
        // FIXME: ignore if not? shouldn't happen...

        emit signalPreviewLoaded(fcjob->destUrl());
    }
}


bool Engine::entryChanged(const EntryInternal& oldentry, const EntryInternal& entry)
{
    // possibly return true if the status changed? depends on when this is called
    if ((!oldentry.isValid()) || (entry.releaseDate() > oldentry.releaseDate())
            || (entry.version() > oldentry.version()))
        return true;
    return false;
}


void Engine::install(KNS3::EntryInternal entry)
{
    if (entry.status() == EntryInternal::Updateable) {
        entry.setStatus(EntryInternal::Updating);
    } else  {
        entry.setStatus(EntryInternal::Installing);
    }
    emit signalEntryChanged(entry);

    kDebug() << "Install " << entry.name()
        << " from: " << entry.providerId();
    ProviderInformation i = d->providers.value(entry.providerId());
    if (i.provider) {
        i.provider->loadPayloadLink(entry);
    }
}

void Engine::downloadLinkLoaded(const KNS3::EntryInternal& entry)
{
    d->installation->install(entry);
}

void Engine::uninstall(KNS3::EntryInternal entry)
{
    // FIXME: change the status?
    entry.setStatus(EntryInternal::Installing);
    emit signalEntryChanged(entry);
    d->installation->uninstall(entry);
}

void Engine::slotEntryChanged(const KNS3::EntryInternal& entry)
{
    emit signalEntryChanged(entry);
}

void Engine::slotInstallationFailed(const KNS3::EntryInternal& entry)
{
    kDebug() << "Installation failed: " << entry.name();
    // FIXME implement warning?
}

bool Engine::userCanVote(const EntryInternal& entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId()).provider;
    return p->userCanVote();
}

void Engine::vote(const EntryInternal& entry, bool positiveVote)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId()).provider;
    p->vote(entry, positiveVote);
}

bool Engine::userCanBecomeFan(const EntryInternal& entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId()).provider;
    return p->userCanBecomeFan();
}

void Engine::becomeFan(const EntryInternal& entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId()).provider;
    p->becomeFan(entry);
}

#include "engine.moc"
