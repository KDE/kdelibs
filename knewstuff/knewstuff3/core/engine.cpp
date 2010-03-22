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
#include "ui/imageloader.h"

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

using namespace KNS3;

Engine::Engine(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_installation(new Installation)
    , m_cache(new Cache)
    , m_searchTimer(new QTimer)
    , m_currentPage(-1)
    , m_pageSize(20)
{
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(1000);
    connect(m_searchTimer, SIGNAL(timeout()), SLOT(slotSearchTimerExpired()));
    connect(this, SIGNAL(signalEntryChanged(const KNS3::EntryInternal&)), m_cache, SLOT(registerChangedEntry(const KNS3::EntryInternal&)));
}

Engine::~Engine()
{
    m_cache->writeRegistry();
    delete m_searchTimer;
    delete m_installation;
    delete m_cache;
}

bool Engine::init(const QString &configfile)
{
    kDebug() << "Initializing KNS3::Engine from '" << configfile << "'";

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

    m_categories = group.readEntry("Categories", QStringList());

    kDebug() << "Categories: " << m_categories;
    m_providerFileUrl = group.readEntry("ProvidersUrl", QString());
    m_applicationName = QFileInfo(KStandardDirs::locate("config", configfile)).baseName() + ':';

    // let installation read install specific config
    if (!m_installation->readConfig(group)) {
        return false;
    }

    connect(m_installation, SIGNAL(signalEntryChanged(const KNS3::EntryInternal&)), SLOT(slotEntryChanged(const KNS3::EntryInternal&)));

    m_cache->setRegistryFileName(m_applicationName.split(':')[0]);
    m_cache->readRegistry();

    m_initialized = true;

    // load the providers
    loadProviders();

    return true;
}

QStringList Engine::categories() const
{
    return m_categories;
}

QStringList Engine::categoriesFilter() const
{
    return m_currentRequest.categories;
}

void Engine::loadProviders()
{
    kDebug(550) << "loading providers from " << m_providerFileUrl;

    XmlLoader * loader = new XmlLoader(this);
    connect(loader, SIGNAL(signalLoaded(const QDomDocument&)), SLOT(slotProviderFileLoaded(const QDomDocument&)));
    connect(loader, SIGNAL(signalFailed()), SLOT(slotProvidersFailed()));

    loader->load(KUrl(m_providerFileUrl));
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
        emit signalError(i18n("Could not load get hot new stuff providers from file: %1", m_providerFileUrl));
        return;
    }

    QDomElement n = providers.firstChildElement("provider");
    while (!n.isNull()) {
        kDebug() << "Provider attributes: " << n.attribute("type");

        QSharedPointer<KNS3::Provider> provider;
        if (isAtticaProviderFile || n.attribute("type").toLower() == "rest") {
            provider = QSharedPointer<KNS3::Provider> (new AtticaProvider(m_categories));
        } else {
            provider = QSharedPointer<KNS3::Provider> (new StaticXmlProvider);
        }

        connect(provider.data(), SIGNAL(providerInitialized(KNS3::Provider*)), SLOT(providerInitialized(KNS3::Provider*)));
        connect(provider.data(), SIGNAL(loadingFinished(KNS3::Provider::SearchRequest, KNS3::EntryInternal::List)),
                SLOT(slotEntriesLoaded(KNS3::Provider::SearchRequest, KNS3::EntryInternal::List)));
        connect(provider.data(), SIGNAL(entryDetailsLoaded(KNS3::EntryInternal)), SLOT(slotEntryDetailsLoaded(KNS3::EntryInternal)));
        connect(provider.data(), SIGNAL(payloadLinkLoaded(const KNS3::EntryInternal&)), SLOT(downloadLinkLoaded(const KNS3::EntryInternal&)));
        connect(provider.data(), SIGNAL(jobStarted(KJob*)), this, SLOT(providerJobStarted(KJob*)));

        if (provider->setProviderXML(n)) {
            m_providers.insert(provider->id(), provider);
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
    emit signalError(i18n("Loading of providers from file: %1 failed", m_providerFileUrl));
}

void Engine::providerInitialized(Provider* p)
{
    kDebug() << "providerInitialized" << p->name();
    p->setCachedEntries(m_cache->registryForProvider(p->id()));

    p->loadEntries(m_currentRequest);
}

void Engine::slotEntriesLoaded(const KNS3::Provider::SearchRequest& request, KNS3::EntryInternal::List entries)
{
    m_currentPage = qMax<int>(request.page, m_currentPage);
    kDebug() << "loaded page " << request.page << "current page" << m_currentPage;

    m_cache->insertRequest(request, entries);
    emit signalEntriesLoaded(entries);
}

void Engine::reloadEntries()
{
    emit signalResetView();
    m_currentPage = -1;
    m_currentRequest.page = 0;

    foreach (const QSharedPointer<KNS3::Provider> &p, m_providers) {
        if (p->isInitialized()) {
            if (m_currentRequest.sortMode == Provider::Installed) {
                // when asking for installed entries, never use the cache
                p->loadEntries(m_currentRequest);
            } else {
                // take entries from cache until there are no more
                EntryInternal::List cache = m_cache->requestFromCache(m_currentRequest);
                while (!cache.isEmpty()) {
                    kDebug() << "From cache";
                    emit signalEntriesLoaded(cache);

                    m_currentPage = m_currentRequest.page;
                    ++m_currentRequest.page;
                    cache = m_cache->requestFromCache(m_currentRequest);
                }
                // if the cache was empty, request data from provider
                if (m_currentPage == -1) {
                    kDebug() << "From provider";
                    p->loadEntries(m_currentRequest);
                }
            }
        }
    }
}

void Engine::setCategoriesFilter(const QStringList& categories)
{
    m_currentRequest.categories = categories;
    reloadEntries();
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (m_currentRequest.sortMode != mode) {
        m_currentRequest.page = -1;
    }
    m_currentRequest.sortMode = mode;
    reloadEntries();
}

void Engine::setSearchTerm(const QString& searchString)
{
    m_searchTimer->stop();
    m_currentRequest.searchTerm = searchString;
    EntryInternal::List cache = m_cache->requestFromCache(m_currentRequest);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        m_searchTimer->start();
    }
}

void Engine::slotSearchTimerExpired()
{
    reloadEntries();
}

void Engine::requestMoreData()
{
    kDebug() << "Get more data! current page: " << m_currentPage  << " requested: " << m_currentRequest.page;

    if (m_currentPage < m_currentRequest.page) {
        return;
    }

    m_currentRequest.page++;

    foreach (const QSharedPointer<KNS3::Provider> &p, m_providers) {
        if (p->isInitialized()) {
            p->loadEntries(m_currentRequest);
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

    QString message = i18n("loading %1",url);
    emit signalProgress(message, percent);
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
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    if (p) {
        p->loadPayloadLink(entry);
    }
}

void Engine::slotEntryDetailsLoaded(const KNS3::EntryInternal& entry)
{
    emit signalEntryDetailsLoaded(entry);
}

void Engine::downloadLinkLoaded(const KNS3::EntryInternal& entry)
{
    m_installation->install(entry);
}

void Engine::uninstall(KNS3::EntryInternal entry)
{
    // FIXME: change the status?
    entry.setStatus(EntryInternal::Installing);
    emit signalEntryChanged(entry);
    m_installation->uninstall(entry);
}

void Engine::loadDetails(const KNS3::EntryInternal &entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->loadEntryDetails(entry);
}

void Engine::loadPreview(const KNS3::EntryInternal& entry, EntryInternal::PreviewType type)
{
    ImageLoader* l = new ImageLoader(entry, type, this);
    connect(l, SIGNAL(signalPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)), this, SLOT(slotPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)));
    l->start();
}

void Engine::slotPreviewLoaded(const KNS3::EntryInternal& entry, EntryInternal::PreviewType type)
{
    kDebug() << "loaded: " << entry.name();
    emit signalEntryPreviewLoaded(entry, type);
}

void Engine::contactAuthor(const EntryInternal &entry)
{
    if (!entry.author().email().isEmpty()) {
        // invoke mail with the address of the author
        KToolInvocation::invokeMailer(entry.author().email(), i18n("Re: %1", entry.name()));
    } else if (!entry.author().homepage().isEmpty()) {
        KToolInvocation::invokeBrowser(entry.author().homepage());
    }
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
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    return p->userCanVote();
}

void Engine::vote(const EntryInternal& entry, bool positiveVote)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->vote(entry, positiveVote);
}

bool Engine::userCanBecomeFan(const EntryInternal& entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    return p->userCanBecomeFan();
}

void Engine::becomeFan(const EntryInternal& entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->becomeFan(entry);
}

#include "engine.moc"
