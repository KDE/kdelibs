/*
    knewstuff3/provider.cpp
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>
    
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

#include "staticxmlprovider.h"

#include "core/xmlloader.h"
#include "core/provider_p.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

namespace KNS3
{

class StaticXmlProviderPrivate :public ProviderPrivate
{
public:
    StaticXmlProviderPrivate()
        :mInitialized(false)
    {}

    // map of download urls to their feed name
    QMap<QString, KUrl> mDownloadUrls;
    KUrl mUploadUrl;
    KUrl mNoUploadUrl;
    
    // cache of all entries known from this provider so far, mapped by their id
    Entry::List cachedEntries;
    QMap<Provider::SortMode, XmlLoader*> mFeedLoaders;
    QString searchTerm;
    QString mId;
    bool mInitialized;
};

StaticXmlProvider::StaticXmlProvider(   )
    : Provider(*new StaticXmlProviderPrivate)
{
}

StaticXmlProvider::~StaticXmlProvider()
{
    // d_ptr is deleted in base class!
}

QString StaticXmlProvider::id() const
{
    Q_D(const StaticXmlProvider);
    return d->mId;
    
}

bool StaticXmlProvider::setProviderXML(QDomElement & xmldata)
{
    Q_D(StaticXmlProvider);
    kDebug(550) << "setting provider xml";

    if (xmldata.tagName() != "provider")
        return false;

    d->mUploadUrl = xmldata.attribute("uploadurl");
    d->mNoUploadUrl = xmldata.attribute("nouploadurl");

    QString url = xmldata.attribute("downloadurl");
    if (!url.isEmpty()) {
        d->mDownloadUrls.insert(QString(), KUrl(url));
    }

    url = xmldata.attribute("downloadurl-latest");
    if (!url.isEmpty()) {
        d->mDownloadUrls.insert("latest", KUrl(url));
    }

    url = xmldata.attribute("downloadurl-score");
    if (!url.isEmpty()) {
        d->mDownloadUrls.insert("score", KUrl(url));
    }

    url = xmldata.attribute("downloadurl-downloads");
    if (!url.isEmpty()) {
        d->mDownloadUrls.insert("downloads", KUrl(url));
    }

    // FIXME: what exactly is the following condition supposed to do?
    // FIXME: make sure new KUrl in KDE 4 handles this right
    // FIXME: this depends on freedesktop.org icon naming... introduce 'desktopicon'?
    KUrl iconurl(xmldata.attribute("icon"));
    if (!iconurl.isValid()) 
        iconurl.setPath(xmldata.attribute("icon"));
    d->mIcon = iconurl;

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "title") {
            //QString lang = e.attribute("lang");
            d->mName = e.text().trimmed();
            kDebug() << "add name for provider ("<< this << "): " << e.text();
        }
    }

    // Validation

    if ((d->mNoUploadUrl.isValid()) && (d->mUploadUrl.isValid())) {
        kWarning(550) << "StaticXmlProvider: both uploadurl and nouploadurl given";
        return false;
    }

    if ((!d->mNoUploadUrl.isValid()) && (!d->mUploadUrl.isValid())) {
        kWarning(550) << "StaticXmlProvider: neither uploadurl nor nouploadurl given";
        return false;
    }

    d->mId = d->mDownloadUrls[QString()].url();
    if (d->mId.isEmpty()) {
        d->mId = d->mDownloadUrls[d->mDownloadUrls.keys().first()].url();
    }
    
    d->mInitialized = true;
    
    emit providerInitialized(this);
    return true;
}

QDomElement StaticXmlProvider::providerXML() const
{
    Q_D(const StaticXmlProvider);
    QDomDocument doc;

    QDomElement el = doc.createElement("provider");

    QDomElement e;
    e = addElement(doc, el, "title", d->mName);


    /*if(provider.downloadUrl().isValid())
    {
      el.setAttribute("downloadurl", provider.downloadUrl().url());
    }*/
    if (d->mUploadUrl.isValid()) {
        el.setAttribute("uploadurl", d->mUploadUrl.url());
    }
    if (d->mNoUploadUrl.isValid()) {
        el.setAttribute("nouploadurl", d->mNoUploadUrl.url());
    }
    if (d->mIcon.isValid()) {
        el.setAttribute("icon", d->mIcon.url());
    }

    const QStringList feeds = d->mDownloadUrls.keys();
    for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it) {
        if ((*it).isEmpty())
            el.setAttribute("downloadurl", d->mDownloadUrls.value((*it)).url());
        else
            el.setAttribute("downloadurl-" + (*it), d->mDownloadUrls.value((*it)).url());
    }

    return el;
}

bool StaticXmlProvider::isInitialized() const
{
    Q_D(const StaticXmlProvider);
    return d->mInitialized;
}

void StaticXmlProvider::setCachedEntries(const KNS3::Entry::List& cachedEntries)
{
    Q_D(StaticXmlProvider);
    kDebug() << "Set cached entries " << cachedEntries.size();
    d->cachedEntries.append(cachedEntries);
}

void StaticXmlProvider::loadEntries(SortMode sortMode, const QString& searchstring, int page, int pageSize)
{
    Q_D(StaticXmlProvider);
    Q_UNUSED(pageSize); // here we just get that one page, no matter what
    d->searchTerm = searchstring;

    if (sortMode == Installed) {
        kDebug() << installedEntries().size();
        emit loadingFinished(sortMode, searchstring, 0, 1, 1, installedEntries());
        return;
    }
    
    KUrl url = downloadUrl(sortMode);
    if (!url.isEmpty()) {
        if (page == 0) {
            // TODO first get the entries, then filter with searchString, finally emit the finished signal...
            XmlLoader * loader = new XmlLoader(this);
            connect(loader, SIGNAL(signalLoaded(const QDomDocument&)), SLOT(slotFeedFileLoaded(const QDomDocument&)));
            connect(loader, SIGNAL(signalFailed()), SLOT(slotFeedFailed()));
            d->mFeedLoaders.insert(sortMode, loader);

            loader->load(url);
            kDebug() << "Loader: " << loader;
        } else {
            // static providers only ever have one page of data
            emit loadingFinished(sortMode, searchstring, page, 0, 1, Entry::List());
        }
    } else {
        emit loadingFailed(sortMode, searchstring, page);
    }
}

KUrl StaticXmlProvider::downloadUrl(SortMode mode) const
{
    Q_D(const StaticXmlProvider);
    KUrl url;
    switch (mode) {
        case Installed: // should just query the registry and not end up here
        case Rating:
            url = d->mDownloadUrls.value("score");
            break;
        case Alphabetical:
            url = d->mDownloadUrls.value(QString());
            break;
        case Newest:
            url = d->mDownloadUrls.value("latest");
            break;
        case Downloads:
            url = d->mDownloadUrls.value("downloads");
            break;
    }
    if (url.isEmpty()) {
        url = d->mDownloadUrls.value(QString());
    }
    return url;
}

void StaticXmlProvider::slotFeedFileLoaded(const QDomDocument& doc)
{
    Q_D(StaticXmlProvider);
    XmlLoader * loader = qobject_cast<KNS3::XmlLoader*>(sender());
    if (!loader)
    {
        kWarning() << "Loader not found!";
        // FIXME emit something useful maybe? is this used?
        emit loadingFailed(Rating, QString(), 0);
        return;
    }

    // we have a loader, so see which sortmode it was used for
    QStringList::ConstIterator it;
    SortMode mode;
    
    foreach(SortMode sortMode, d->mFeedLoaders.keys()){
        if (loader == d->mFeedLoaders.value(sortMode))
        {
            mode = sortMode;
            break;
        }
    }

    // load all the entries from the domdocument given
    Entry::List entries;
    QDomElement element;

    element = doc.documentElement();
    QDomElement n;
    for (n = element.firstChildElement(); !n.isNull(); n = n.nextSiblingElement()) {
        Entry entry;
        entry.setEntryXML(n.toElement());
        entry.setProviderId(d->mId);
        
        // check to see if we already have this entry
        kDebug() << "Check: " << entry.providerId() << entry.uniqueId();

        int index = d->cachedEntries.indexOf(entry);
        if (index >= 0) {
            Entry cacheEntry = d->cachedEntries.takeAt(index);
            
            // check if updateable
            if ((cacheEntry.status() == Entry::Installed) &&
                 ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
                    entry.setStatus(Entry::Updateable);
            } else {
                entry.setStatus(cacheEntry.status());
            }
            cacheEntry = entry;
            entries << cacheEntry;
        } else {
            d->cachedEntries.append(entry);
            if (searchIncludesEntry(entry)) {
                entries << entry;
            }
        }
    }
    emit loadingFinished(mode, QString(), 0, entries.count(), 1, entries);
}

void StaticXmlProvider::slotFeedFailed()
{
	// TODO: get the sortmode, searchstring and page from the loader somehow so we can pass them on
	//emit loadingFailed();
}

bool StaticXmlProvider::searchIncludesEntry(const KNS3::Entry& entry) const
{
    Q_D(const StaticXmlProvider);

    if (d->searchTerm.isEmpty()) {
        return true;
    }
    QString search = d->searchTerm;
    if (entry.name().contains(search, Qt::CaseInsensitive) ||
        entry.summary().contains(search, Qt::CaseInsensitive) ||
        entry.author().name().contains(search, Qt::CaseInsensitive)
        ) {
        return true;
    }
    return false;
}

void StaticXmlProvider::loadPayloadLink(const KNS3::Entry& entry)
{
    emit payloadLinkLoaded(entry);
}


Entry::List StaticXmlProvider::installedEntries() const
{
    Q_D(const StaticXmlProvider);
    Entry::List entries;
    foreach (const Entry& entry, d->cachedEntries) {
        if (entry.status() == Entry::Installed || entry.status() == Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}


}

#include "staticxmlprovider.moc"
