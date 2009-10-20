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
#include "staticxml/feed.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>

namespace KNS3
{

class StaticXmlProviderPrivate
{
public:
    StaticXmlProviderPrivate() {}

    KTranslatable mName;
    // map of download urls to their feed name
    QMap<QString, KUrl> mDownloadUrls;
    KUrl mUploadUrl;
    KUrl mNoUploadUrl;
    KUrl mIcon;
    // cache of all entries known from this provider so far, mapped by their id
    QMap<QString, Entry*> mEntries;
    QMap<QString, Feed*> mFeeds;

};

StaticXmlProvider::StaticXmlProvider()
        : d(new StaticXmlProviderPrivate)
{
}

StaticXmlProvider::~StaticXmlProvider()
{
    delete d;
}

bool StaticXmlProvider::setProviderXML(QDomElement & xmldata)
{
    kDebug(550) << "setting provider xml";

    if (xmldata.tagName() != "provider")
        return false;

    d->mUploadUrl = xmldata.attribute("uploadurl");
    d->mNoUploadUrl = xmldata.attribute("nouploadurl");

    QString downloadurl = xmldata.attribute("downloadurl");
    QString downloadlatest = xmldata.attribute("downloadurl-latest");
    QString downloadscore = xmldata.attribute("downloadurl-score");
    QString downloaddownloads = xmldata.attribute("downloadurl-downloads");

    if (!downloadlatest.isEmpty()) {
        Feed *feedlatest = new Feed();
        feedlatest->setName(i18nc("describes the feed of the latest posted entries", "Latest"));
        feedlatest->setFeedUrl(downloadlatest);
        d->mFeeds.insert("latest", feedlatest);
    }
    if (!downloadscore.isEmpty()) {
        Feed *feedscore = new Feed();
        feedscore->setName(i18n("Highest Rated"));
        feedscore->setFeedUrl(downloadscore);
        d->mFeeds.insert("score", feedscore);
    }
    if (!downloaddownloads.isEmpty()) {
        Feed *feeddownloads = new Feed();
        feeddownloads->setName(i18n("Most Downloads"));
        feeddownloads->setFeedUrl(downloaddownloads);
        d->mFeeds.insert("downloads", feeddownloads);
    }
    if (!downloadurl.isEmpty()) {
        Feed *feedgeneric = new Feed();
        // feedgeneric->setName(i18n("Unsorted"));
        // Currently this is used for latest
        feedgeneric->setName(i18nc("describes the feed of the latest posted entries", "Latest"));
        feedgeneric->setFeedUrl(downloadurl);
        d->mFeeds.insert(QString(), feedgeneric);
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
            QString lang = e.attribute("lang");
            d->mName.addString(lang, e.text().trimmed());
        }
    }

    // Validation

    if ((d->mNoUploadUrl.isValid()) && (d->mUploadUrl.isValid())) {
        kWarning(550) << "ProviderHandler: both uploadurl and nouploadurl given";
        return false;
    }

    if ((!d->mNoUploadUrl.isValid()) && (!d->mUploadUrl.isValid())) {
        kWarning(550) << "ProviderHandler: neither uploadurl nor nouploadurl given";
        return false;
    }

    return true;
}

QDomElement StaticXmlProvider::providerXML() const
{
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

    const QStringList feeds = d->mFeeds.keys();
    for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it) {
        Feed *feed = d->mFeeds.value((*it));
        if ((*it).isEmpty())
            el.setAttribute("downloadurl", feed->feedUrl().url());
        else
            el.setAttribute("downloadurl-" + (*it), feed->feedUrl().url());
    }

    return el;
}

KTranslatable StaticXmlProvider::name() const
{
    return d->mName;
}

KUrl StaticXmlProvider::icon() const
{
    return d->mIcon;
}

QStringList StaticXmlProvider::availableFeeds() const
{
    return d->mFeeds.keys();
}

void StaticXmlProvider::loadFeed(const QString & feedname, int page)
{
}

}

#include "staticxmlprovider.moc"
