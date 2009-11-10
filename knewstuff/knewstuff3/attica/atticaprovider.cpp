/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "xmlloader.h"
#include "core/provider_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/listjob.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <KMessageBox>

using namespace Attica;

namespace KNS3
{

class AtticaProviderPrivate :public ProviderPrivate
{
public:
    // List of categories that have to match exactly with those on the server
    QStringList categoryNameList;
    // the attica categories we are interested in (e.g. Wallpaper, Application, Vocabulary File...)
    Attica::Category::List categoryList;
    
    Attica::ProviderManager m_providerManager;
    Attica::Provider m_provider;

    KNS3::Entry::List installedEntries;

    QMap<BaseJob*, Entry> downloadLinkJobs;
    
    AtticaProviderPrivate()
    {
    }
};

AtticaProvider::AtticaProvider(const QStringList& categories)
    : Provider(*new AtticaProviderPrivate)
{
    Q_D(AtticaProvider);
    d->mName = KTranslatable("Attica");
    d->categoryNameList = categories;

    connect(&d->m_providerManager, SIGNAL(providersChanged()), SLOT(providerLoaded()));
}

AtticaProvider::~AtticaProvider()
{
    // d_ptr is deleted in base class!
}

QString AtticaProvider::id() const
{
    Q_D(const AtticaProvider);
    return d->m_provider.baseUrl().toString();
}

bool AtticaProvider::setProviderXML(QDomElement & xmldata)
{
    Q_D(AtticaProvider);
    kDebug(550) << "setting provider xml";

    if (xmldata.tagName() != "provider")
        return false;
    
    // FIXME this is quite ugly, repackaging the xml into a string
    QDomDocument doc("temp");
    doc.appendChild(xmldata);
    d->m_providerManager.addProviderFromXml(doc.toString());

    if (!d->m_providerManager.providers().isEmpty()) {
        kDebug() << "base url of attica provider:" << d->m_providerManager.providers().first().baseUrl().toString();
    }
    
    if (d->m_providerManager.providers().isEmpty()) {
        return false;
    }
    return true;
}

QDomElement AtticaProvider::providerXML() const
{
    QDomDocument doc;

    QDomElement el = doc.createElement("provider");
    // FIXME if needed, create xml
    return el;
}

void AtticaProvider::setCachedEntries(const KNS3::Entry::List& cachedEntries)
{
    Q_D(AtticaProvider);
    //d->
    
}

void AtticaProvider::providerLoaded()
{
    Q_D(AtticaProvider);
    if (d->m_providerManager.providers().isEmpty()) {
        return;
    }
    d->m_provider = d->m_providerManager.providers().first();

    Attica::ListJob<Attica::Category>* job = d->m_provider.requestCategories();
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(listOfCategoriesLoaded(Attica::BaseJob*)));
    job->start();
}

void AtticaProvider::listOfCategoriesLoaded(Attica::BaseJob* listJob)
{
    Q_D(AtticaProvider);
    kDebug() << "loading categories: " << d->categoryNameList;
    
    Attica::ListJob<Attica::Category>* job = static_cast<Attica::ListJob<Attica::Category>*>(listJob);
    Category::List categoryList = job->itemList();

    foreach(const Category& category, categoryList) {
        if (d->categoryNameList.contains(category.name())) {
            kDebug() << "Adding category: " << category.name();
            d->categoryList.append(category);
        }
    }
    emit providerInitialized(this);
}

bool AtticaProvider::isInitialized() const
{
    Q_D(const AtticaProvider);
    return !d->m_providerManager.providers().isEmpty();
}

void AtticaProvider::loadEntries(SortMode sortMode, const QString& searchString, int page, int pageSize)
{
    Q_D(AtticaProvider);
    Attica::Provider::SortMode sorting = atticaSortMode(sortMode);
    ListJob<Content>* job = d->m_provider.searchContents(d->categoryList, searchString, sorting, page, pageSize);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(categoryContentsLoaded(Attica::BaseJob*)));
    job->start();
}

void AtticaProvider::categoryContentsLoaded(BaseJob* job)
{
    ListJob<Content>* listJob = static_cast<ListJob<Content>*>(job);
    Content::List contents = listJob->itemList();

    Entry::List entries;
    
    Q_FOREACH(Content content, contents) {
        Entry entry;
        entry.setProviderId(id());
        entry.setName(content.name());
        entry.setUniqueId(content.id());
        entry.setRating(content.rating());
        entry.setDownloads(content.downloads());
        entry.setReleaseDate(content.updated().date());
        entry.setPreview(content.previewPicture("1"));
        entry.setLicense(content.license());
        //entry.setAuthor(content.author());
        entry.setSource(KNS3::Entry::Online);
        entry.setStatus(KNS3::Entry::Downloadable);
        entry.setSummary(content.description());
        entry.setVersion(content.version());

        entries.append(entry);
    }

    // FIXME page number and strings
    emit loadingFinished(Rating, "", 0, entries.count(), 10, entries);
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

void AtticaProvider::loadPayloadLink(const KNS3::Entry& entry)
{
    Q_D(AtticaProvider);
    ItemJob<DownloadItem>* job = d->m_provider.downloadLink(entry.uniqueId());
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(downloadItemLoaded(Attica::BaseJob*)));
    d->downloadLinkJobs[job] = entry;
    job->start();

    kDebug() << " link for " << entry.uniqueId();
}

void AtticaProvider::downloadItemLoaded(BaseJob* baseJob)
{
    Q_D(AtticaProvider);

    ItemJob<DownloadItem>* job = static_cast<ItemJob<DownloadItem>*>(baseJob);
    DownloadItem item = job->result();
    if (job->metadata().statusCode() != 100) {
        KMessageBox::error(0, "Could not get download link");
        return;
    }

    Entry entry = d->downloadLinkJobs.take(job);
    entry.setPayload(KTranslatable(item.url().toString()));
    emit payloadLinkLoaded(entry);
}

}

#include "atticaprovider.moc"
