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

#include "core/xmlloader.h"
#include "core/provider_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
//#include "kutils/kcmultidialog.h"

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/listjob.h>
#include <attica/content.h>
#include <attica/downloaditem.h>

using namespace Attica;

namespace KNS3
{

class AtticaProviderPrivate :public ProviderPrivate
{
public:
    // List of categories that have to match exactly with those on the server
    QStringList categoryNameList;
    // List of patterns that are matched against categories
    // (so that Wallpaper returns KDE Wallpaper 640x480 for example)
    QStringList categoryPatternList;

    // the attica categories we are interested in (e.g. Wallpaper, Application, Vocabulary File...)
    Attica::Category::List categoryList;

    Attica::ProviderManager m_providerManager;
    Attica::Provider m_provider;

    KNS3::Entry::List cachedEntries;

    QHash<BaseJob*, Entry> downloadLinkJobs;

    // keep track of the pages we requested
    QHash<BaseJob*, int> entryJobs;
    
    AtticaProviderPrivate()
    {
    }
};

AtticaProvider::AtticaProvider(const QStringList& categories, const QStringList& categoriesPatterns)
    : Provider(*new AtticaProviderPrivate)
{
    Q_D(AtticaProvider);
    d->mName = QString("Attica");
    d->categoryNameList = categories;
    d->categoryPatternList = categoriesPatterns;

    connect(&d->m_providerManager, SIGNAL(providersChanged()), SLOT(providerLoaded()));
    connect(&d->m_providerManager, SIGNAL(authenticationCredentialsMissing(const Provider&)), 
            SLOT(authenticationCredentialsMissing(const Provider&)));
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

void AtticaProvider::authenticationCredentialsMissing(const KNS3::Provider& )
{
    kDebug() << "Authentication missing!";
// FIXME Show autentication dialog
    /*
    KCMultiDialog* KCM = new KCMultiDialog();
    KCM->setWindowTitle( i18n( "Open Collaboration Providers" ) );
    KCM->addModule( "attica" );
    KCM->exec();
    KCM->deleteLater();
    */
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

    // FIXME I don't think the last is a good idea...
    if (!d->m_providerManager.providers().isEmpty()) {
        kDebug() << "base url of attica provider:" << d->m_providerManager.providers().last().baseUrl().toString();
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
    d->cachedEntries = cachedEntries;
}

void AtticaProvider::providerLoaded()
{
    Q_D(AtticaProvider);
    if (d->m_providerManager.providers().isEmpty()) {
        kDebug() << "No valid provider found!";
        return;
    }
    d->m_provider = d->m_providerManager.providers().last();

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
        foreach(const QString& pattern, d->categoryPatternList) {
            if (category.name().contains(pattern)) {
                kDebug() << "Adding category (pattern): " << category.name();
                d->categoryList.append(category);
            }
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

    if (sortMode == Installed) {
        emit loadingFinished(sortMode, searchString, 0, 1, 10000, installedEntries());
        return;
    }
    
    Attica::Provider::SortMode sorting = atticaSortMode(sortMode);
    ListJob<Content>* job = d->m_provider.searchContents(d->categoryList, searchString, sorting, page, pageSize);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(categoryContentsLoaded(Attica::BaseJob*)));

    d->entryJobs[job] = page;
    
    job->start();
}

void AtticaProvider::categoryContentsLoaded(BaseJob* job)
{
    Q_D(AtticaProvider);
    
    ListJob<Content>* listJob = static_cast<ListJob<Content>*>(job);
    Content::List contents = listJob->itemList();

    Entry::List entries;
    
    Q_FOREACH(Content content, contents) {
        Entry entry;
        entry.setProviderId(id());
        entry.setUniqueId(content.id());
        entry.setStatus(KNS3::Entry::Downloadable);
        entry.setVersion(content.version());
        entry.setReleaseDate(content.updated().date());

        int index = d->cachedEntries.indexOf(entry);
        
        if (index >= 0) {
            Entry cacheEntry = d->cachedEntries.at(index);
            // check if updateable
            if ((cacheEntry.status() == Entry::Installed) &&
                ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
                cacheEntry.setStatus(Entry::Updateable);
            }
            entry = cacheEntry;
        } else {
            d->cachedEntries.append(entry);
        }

        entry.setName(content.name());
        entry.setHomepage(content.detailpage());
        entry.setRating(content.rating());
        entry.setDownloads(content.downloads());
        //entry.setPreview(content.previewPicture("1"));
        entry.setPreviewSmall(content.smallPreviewPicture("1"));
        entry.setPreviewBig(content.previewPicture("1"));
        entry.setLicense(content.license());
        Author author;
        author.setName(content.author());
        entry.setAuthor(author);
        
        entry.setSource(KNS3::Entry::Online);
        entry.setSummary(content.description());

        entries.append(entry);
    }

    // FIXME page number and strings
    emit loadingFinished(Rating, "", d->entryJobs.value(job), entries.count(), 20, entries);
    kDebug() << "loading finished page " << d->entryJobs.value(job);
    d->entryJobs.remove(job);
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
    if (job->metadata().error() != Metadata::NoError) {
        kDebug() << job->metadata().error() << job->metadata().statusCode();
        KMessageBox::error(0, "Could not get download link.");
        return;
    }

    Entry entry = d->downloadLinkJobs.take(job);
    entry.setPayload(QString(item.url().toString()));
    emit payloadLinkLoaded(entry);
}

Entry::List AtticaProvider::installedEntries() const
{
    Q_D(const AtticaProvider);
    Entry::List entries;
    foreach (const Entry& entry, d->cachedEntries) {
        if (entry.status() == Entry::Installed || entry.status() == Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

}

#include "atticaprovider.moc"
