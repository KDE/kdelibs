/*
    knewstuff3/provider.cpp
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
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

#include "staticxmlprovider.h"

#include "xmlloader.h"
#include "knewstuff3/core/provider_p.h"

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
    Entry::List mEntries;
    QMap<QString, QStringList> mFeedEntries;
	QMap<QString, XmlLoader*> mFeedLoaders;
    
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
    
    d->mInitialized = true;
    
    emit providerInitialized(this);
    return true;
}

QDomElement StaticXmlProvider::providerXML() const
{
    Q_D(const StaticXmlProvider);
    QDomDocument doc;

    QDomElement el = doc.createElement("provider");

    QStringList::ConstIterator it;
    QDomElement e;
    const QStringList langs = d->mName.languages();
    for (it = langs.begin(); it != langs.end(); ++it) {
        e = addElement(doc, el, "title", d->mName.translated(*it));
        e.setAttribute("lang", *it);
    }

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

QStringList StaticXmlProvider::availableSortingCriteria() const
{
    Q_D(const StaticXmlProvider);
    return d->mDownloadUrls.keys();
}

void StaticXmlProvider::loadEntries(const QString& sortMode, const QString& searchstring, int page, int pageSize)
{
    Q_D(StaticXmlProvider);
    if (d->mDownloadUrls.contains(sortMode)) {
        if (page == 0) {
            // TODO first get the entries, then filter with searchString, finally emit the finished signal...
            XmlLoader * loader = new XmlLoader(this);
            connect(loader, SIGNAL(signalLoaded(const QDomDocument&)), SLOT(slotFeedFileLoaded(const QDomDocument&)));
            connect(loader, SIGNAL(signalFailed()), SLOT(slotFeedFailed()));
            d->mFeedLoaders.insert(sortMode, loader);

            loader->load(d->mDownloadUrls.value(sortMode));
            kDebug() << "Loader: " << loader;
        } else {
            // static providers only ever have one page of data
            emit loadingFinished(sortMode, searchstring, page, 0, 1, Entry::List());
        }
    } else {
        emit loadingFailed(sortMode, searchstring, page);
    }
}

void StaticXmlProvider::slotFeedFileLoaded(const QDomDocument& doc)
{
    Q_D(StaticXmlProvider);
    XmlLoader * loader = qobject_cast<KNS3::XmlLoader*>(sender());
    if (!loader)
    {
        kWarning() << "Loader not found!";
        emit loadingFailed(QString(), QString(), 0);
        return;
    }


    // we have a loader, so see which sortmode it was used for
    QStringList::ConstIterator it;
    QString mode;
    const QStringList modes = d->mFeedLoaders.keys();
    for (it = modes.begin(); it != modes.end(); ++it) {
        if (loader == d->mFeedLoaders.value(*it))
        {
            mode = *it;
        }
    }


    // load all the entries from the domdocument given
    Entry::List entries;
    QDomElement element;

    element = doc.documentElement();
    kDebug() << "Document Element" << element.tagName();
    kDebug() << "  First Child" << element.firstChildElement().tagName();

    
    QDomElement n;

    for (n = element.firstChildElement(); !n.isNull(); n = n.nextSiblingElement()) {
        
        Entry entry;
        entry.setEntryXML(n.toElement());
        // check to see if we already have this entry
        if (d->mEntries.contains(entry)) {
            // if so, merge the two together
            
        }
        else {
            // add it to the list otherwise
            d->mEntries.append(entry);
            entries << entry;
        }
        d->mFeedEntries[mode].append(entry.uniqueId());
        // TODO: ask the engine if it knows about any cached/installed data, so we can merge that in too
    }
    
    // emit a the entry list
    emit loadingFinished(mode, QString(), 0, entries.count(), 1, entries);
}

void StaticXmlProvider::slotFeedFailed()
{
	// TODO: get the sortmode, searchstring and page from the loader somehow so we can pass them on
	//emit loadingFailed();
}

}

#include "staticxmlprovider.moc"
