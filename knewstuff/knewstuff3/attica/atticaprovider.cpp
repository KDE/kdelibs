/*
    Copyright (c) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "atticaprovider.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/listjob.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <attica/accountbalance.h>
#include <attica/person.h>

using namespace Attica;

namespace KNS3
{

AtticaProvider::AtticaProvider(const QStringList& categories)
    : mEntryJob(0)
    , mInitialized(false)
{
    // init categories map with invalid categories
    foreach (const QString& category, categories)
        mCategoryMap.insert(category, Attica::Category());

    connect(&m_providerManager, SIGNAL(providerAdded(const Attica::Provider&)), SLOT(providerLoaded(const Attica::Provider&)));
    connect(&m_providerManager, SIGNAL(authenticationCredentialsMissing(const Provider&)),
            SLOT(authenticationCredentialsMissing(const Provider&)));
}

AtticaProvider::AtticaProvider(const Attica::Provider& provider, const QStringList& categories)
    : mEntryJob(0)
    , mInitialized(false)
{
    // init categories map with invalid categories
    foreach (const QString& category, categories) {
        mCategoryMap.insert(category, Attica::Category());
    }
    providerLoaded(provider);
}

QString AtticaProvider::id() const
{
    return m_provider.baseUrl().toString();
}

void AtticaProvider::authenticationCredentialsMissing(const KNS3::Provider& )
{
    kDebug() << "Authentication missing!";
    // FIXME Show autentication dialog
}

bool AtticaProvider::setProviderXML(const QDomElement & xmldata)
{
    if (xmldata.tagName() != "provider")
        return false;

    // FIXME this is quite ugly, repackaging the xml into a string
    QDomDocument doc("temp");
    kDebug(550) << "setting provider xml" << doc.toString();

    doc.appendChild(xmldata.cloneNode(true));
    m_providerManager.addProviderFromXml(doc.toString());

    if (!m_providerManager.providers().isEmpty()) {
        kDebug() << "base url of attica provider:" << m_providerManager.providers().last().baseUrl().toString();
    } else {
        kError() << "Could not load provider.";
        return false;
    }
    return true;
}

void AtticaProvider::setCachedEntries(const KNS3::EntryInternal::List& cachedEntries)
{
    mCachedEntries = cachedEntries;
}

void AtticaProvider::providerLoaded(const Attica::Provider& provider)
{
    mName = provider.name();
    kDebug() << "Added provider: " << provider.name();

    m_provider = provider;

    Attica::ListJob<Attica::Category>* job = m_provider.requestCategories();
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(listOfCategoriesLoaded(Attica::BaseJob*)));
    job->start();
}

void AtticaProvider::listOfCategoriesLoaded(Attica::BaseJob* listJob)
{
    if (!jobSuccess(listJob)) return;
    
    kDebug() << "loading categories: " << mCategoryMap.keys();

    Attica::ListJob<Attica::Category>* job = static_cast<Attica::ListJob<Attica::Category>*>(listJob);
    Category::List categoryList = job->itemList();

    foreach(const Category& category, categoryList) {
        if (mCategoryMap.contains(category.name())) {
            kDebug() << "Adding category: " << category.name();
            mCategoryMap[category.name()] = category;
        }
    }
    mInitialized = true;
    emit providerInitialized(this);
}

bool AtticaProvider::isInitialized() const
{
    return mInitialized;
}

void AtticaProvider::loadEntries(const KNS3::Provider::SearchRequest& request)
{
    if (mEntryJob) {
        mEntryJob->abort();
        mEntryJob = 0;
    }

    mCurrentRequest = request;
    if (request.sortMode == Installed) {
        if (request.page == 0) {
            emit loadingFinished(request, installedEntries());
        } else {
            emit loadingFinished(request, EntryInternal::List());
        }
        return;
    }
    
    if (request.sortMode == Updates) {
        checkForUpdates();
        return;
    }

    Attica::Provider::SortMode sorting = atticaSortMode(request.sortMode);
    Attica::Category::List categoriesToSearch;

    if (request.categories.isEmpty()) {
        // search in all categories
        categoriesToSearch = mCategoryMap.values();
    } else {
        foreach (const QString& categoryName, request.categories) {
            categoriesToSearch.append(mCategoryMap.value(categoryName));
        }
    }

    ListJob<Content>* job = m_provider.searchContents(categoriesToSearch, request.searchTerm, sorting, request.page, request.pageSize);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(categoryContentsLoaded(Attica::BaseJob*)));

    mEntryJob = job;
    job->start();
}

void AtticaProvider::checkForUpdates()
{
    foreach (const EntryInternal& e, mCachedEntries) {
        ItemJob<Content>* job = m_provider.requestContent(e.uniqueId());
        connect(job, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(detailsLoaded(Attica::BaseJob*)));
        m_updateJobs.insert(job);
        job->start();
        kDebug() << "Checking for update: " << e.name();
    }
}

void AtticaProvider::loadEntryDetails(const KNS3::EntryInternal& entry)
{
    ItemJob<Content>* job = m_provider.requestContent(entry.uniqueId());
    connect(job, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(detailsLoaded(Attica::BaseJob*)));
    job->start();
}

void AtticaProvider::detailsLoaded(BaseJob* job)
{
    if (jobSuccess(job)) {
        ItemJob<Content>* contentJob = static_cast<ItemJob<Content>*>(job);
        Content content = contentJob->result();
        EntryInternal entry = entryFromAtticaContent(content);
        emit entryDetailsLoaded(entry);
        kDebug() << "check update finished: " << entry.name();
    }

    if (m_updateJobs.remove(job) && m_updateJobs.isEmpty()) {
        kDebug() << "check update finished.";
        QList<EntryInternal> updatable;
        foreach(const EntryInternal& entry, mCachedEntries) {
            if (entry.status() == Entry::Updateable) {
                updatable.append(entry);
            }
        }
        emit loadingFinished(mCurrentRequest, updatable);
    }    
}

void AtticaProvider::categoryContentsLoaded(BaseJob* job)
{
    if (!jobSuccess(job)) return;

    ListJob<Content>* listJob = static_cast<ListJob<Content>*>(job);
    Content::List contents = listJob->itemList();

    EntryInternal::List entries;
    Q_FOREACH(const Content &content, contents) {
        mCachedContent.insert(content.id(), content);
        entries.append(entryFromAtticaContent(content));
    }

    kDebug() << "loaded: " << mCurrentRequest.hashForRequest() << " count: " << entries.size();
    emit loadingFinished(mCurrentRequest, entries);
    mEntryJob = 0;
}

Attica::Provider::SortMode AtticaProvider::atticaSortMode(const SortMode& sortMode)
{
    if (sortMode == Newest) {
        return Attica::Provider::Newest;
    }
    if (sortMode == Alphabetical) {
        return Attica::Provider::Alphabetical;
    }
    if (sortMode == Downloads) {
        return Attica::Provider::Downloads;
    }
    return Attica::Provider::Rating;
}

void AtticaProvider::loadPayloadLink(const KNS3::EntryInternal& entry, int linkId)
{
    Attica::Content content = mCachedContent.value(entry.uniqueId());
    DownloadDescription desc = content.downloadUrlDescription(linkId);

    if (desc.hasPrice()) {
        // Ask for balance, then show information...
        ItemJob<AccountBalance>* job = m_provider.requestAccountBalance();
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(accountBalanceLoaded(Attica::BaseJob*)));
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        kDebug() << "get account balance";
    } else {
        ItemJob<DownloadItem>* job = m_provider.downloadLink(entry.uniqueId(), QString::number(linkId));
        connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(downloadItemLoaded(Attica::BaseJob*)));
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        kDebug() << " link for " << entry.uniqueId();
    }
}

void AtticaProvider::accountBalanceLoaded(Attica::BaseJob* baseJob)
{
    if (!jobSuccess(baseJob)) return;

    ItemJob<AccountBalance>* job = static_cast<ItemJob<AccountBalance>*>(baseJob);
    AccountBalance item = job->result();

    QPair<EntryInternal, int> pair = mDownloadLinkJobs.take(job);
    EntryInternal entry(pair.first);
    Content content = mCachedContent.value(entry.uniqueId());
    if (content.downloadUrlDescription(pair.second).priceAmount() < item.balance()) {
        kDebug() << "Your balance is greather than the price."
                    << content.downloadUrlDescription(pair.second).priceAmount() << " balance: " << item.balance();
        if (KMessageBox::questionYesNo(0,
                i18nc("the price of a download item, parameter 1 is the currency, 2 is the price",
                      "This items costs %1 %2.\nDo you want to buy it?",
                      item.currency(), content.downloadUrlDescription(pair.second).priceAmount()
                )) == KMessageBox::Yes) {
            ItemJob<DownloadItem>* job = m_provider.downloadLink(entry.uniqueId(), QString::number(pair.second));
            connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(downloadItemLoaded(Attica::BaseJob*)));
            connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
            mDownloadLinkJobs[job] = qMakePair(entry, pair.second);
            job->start();
        } else {
            return;
        }
    } else {
        kDebug() << "You don't have enough money on your account!"
                << content.downloadUrlDescription(0).priceAmount() << " balance: " << item.balance();
        KMessageBox::information(0, i18n("Your account balance is too low:\nYour balance: %1\nPrice: %2",
                                         item.balance(),content.downloadUrlDescription(0).priceAmount()));
    }
}

void AtticaProvider::downloadItemLoaded(BaseJob* baseJob)
{
    if (!jobSuccess(baseJob)) return;

    ItemJob<DownloadItem>* job = static_cast<ItemJob<DownloadItem>*>(baseJob);
    DownloadItem item = job->result();

    EntryInternal entry = mDownloadLinkJobs.take(job).first;
    entry.setPayload(QString(item.url().toString()));
    emit payloadLinkLoaded(entry);
}

EntryInternal::List AtticaProvider::installedEntries() const
{
    EntryInternal::List entries;
    foreach (const EntryInternal& entry, mCachedEntries) {
        if (entry.status() == Entry::Installed || entry.status() == Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

void AtticaProvider::vote(const EntryInternal& entry, bool positiveVote)
{
    PostJob * job = m_provider.voteForContent(entry.uniqueId(), positiveVote);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(votingFinished(Attica::BaseJob*)));
    connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
    job->start();
}

void AtticaProvider::votingFinished(Attica::BaseJob* job)
{
    if (!jobSuccess(job)) return;
    emit signalInformation(i18nc("voting for an item (good/bad)", "Your vote was recorded."));
}

void AtticaProvider::becomeFan(const EntryInternal& entry)
{
    PostJob * job = m_provider.becomeFan(entry.uniqueId());
    connect(job, SIGNAL(finished(Attica::BaseJob*)), this, SLOT(becomeFanFinished(Attica::BaseJob*)));
    connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
    job->start();
}

void AtticaProvider::becomeFanFinished(Attica::BaseJob* job)
{
    if (!jobSuccess(job)) return;
    emit signalInformation(i18n("You are now a fan."));
}

bool AtticaProvider::jobSuccess(Attica::BaseJob* job) const
{
    if (job->metadata().error() == Attica::Metadata::NoError) {
        return true;
    }
    kDebug() << "job error: " << job->metadata().error() << " status code: " << job->metadata().statusCode() << job->metadata().message();

    if (job->metadata().error() == Attica::Metadata::NetworkError) {
        emit signalError(i18n("Network error. (%1)", job->metadata().statusCode()));
    }
    if (job->metadata().error() == Attica::Metadata::OcsError) {
        if (job->metadata().statusCode() == 200) {
            emit signalError(i18n("Too many requests to server. Please try again in a few minutes."));
        } else {
            emit signalError(i18n("Unknown Open Collaboration Service API error. (%1)", job->metadata().statusCode()));
        }
    }
    return false;
}

EntryInternal AtticaProvider::entryFromAtticaContent(const Attica::Content& content)
{
    EntryInternal entry;

    entry.setProviderId(id());
    entry.setUniqueId(content.id());
    entry.setStatus(KNS3::Entry::Downloadable);
    entry.setVersion(content.version());
    entry.setReleaseDate(content.updated().date());

    int index = mCachedEntries.indexOf(entry);
    if (index >= 0) {
        EntryInternal cacheEntry = mCachedEntries.at(index);
        // check if updateable
        if ((cacheEntry.status() == Entry::Installed) &&
            ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
            cacheEntry.setStatus(Entry::Updateable);
            cacheEntry.setUpdateVersion(entry.version());
            cacheEntry.setUpdateReleaseDate(entry.releaseDate());
        }
        entry = cacheEntry;
    } else {
        mCachedEntries.append(entry);
    }

    entry.setName(content.name());
    entry.setHomepage(content.detailpage());
    entry.setRating(content.rating());
    entry.setDownloadCount(content.downloads());
    entry.setNumberFans(content.attribute("fans").toInt());
    entry.setDonationLink(content.attribute("donationpage"));
    entry.setKnowledgebaseLink(content.attribute("knowledgebasepage"));
    entry.setNumberKnowledgebaseEntries(content.attribute("knowledgebaseentries").toInt());
    
    entry.setPreviewUrl(content.smallPreviewPicture("1"), EntryInternal::PreviewSmall1);
    entry.setPreviewUrl(content.smallPreviewPicture("2"), EntryInternal::PreviewSmall2);
    entry.setPreviewUrl(content.smallPreviewPicture("3"), EntryInternal::PreviewSmall3);

    entry.setPreviewUrl(content.previewPicture("1"), EntryInternal::PreviewBig1);
    entry.setPreviewUrl(content.previewPicture("2"), EntryInternal::PreviewBig2);
    entry.setPreviewUrl(content.previewPicture("3"), EntryInternal::PreviewBig3);

    entry.setLicense(content.license());
    Author author;
    author.setName(content.author());
    author.setHomepage(content.attribute("profilepage"));
    entry.setAuthor(author);

    entry.setSource(KNS3::EntryInternal::Online);
    entry.setSummary(content.description());
    entry.setChangelog(content.changelog());

    entry.clearDownloadLinkInformation();
    QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
    foreach (Attica::DownloadDescription desc, descs) {
        EntryInternal::DownloadLinkInformation info;
        info.name = desc.name();
        info.priceAmount = desc.priceAmount();
        info.distributionType = desc.distributionType();
        info.descriptionLink = desc.link();
        info.id = desc.id();
        info.isDownloadtypeLink = desc.isDownloadtypLink();
        entry.appendDownloadLinkInformation(info);
    }

    return entry;
}

} // namespace


#include "atticaprovider.moc"
