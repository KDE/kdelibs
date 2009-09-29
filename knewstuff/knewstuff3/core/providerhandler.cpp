/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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

#include "providerhandler.h"

//#include <kconfig.h>
#include <kdebug.h>
//#include <kio/job.h>
#include <kglobal.h>
//#include <kmessagebox.h>
#include <klocale.h>

#include "feed.h"

using namespace KNS;

ProviderHandler::ProviderHandler(const Provider& provider)
{
    mValid = false;
    mProvider = provider;
    mProviderXML = serializeElement(provider);
}

ProviderHandler::ProviderHandler(const QDomElement& providerxml)
    :mProviderXML(providerxml)
    ,mValid(false)
{
    mProvider = deserializeElement(providerxml);
}

bool ProviderHandler::isValid()
{
    return mValid;
}

QDomElement ProviderHandler::providerXML()
{
    return mProviderXML;
}

Provider *ProviderHandler::providerptr()
{
    Provider *provider = new Provider();
    provider->setName(mProvider.name());
    //provider->setDownloadUrl(mProvider.downloadUrl());
    provider->setUploadUrl(mProvider.uploadUrl());
    provider->setNoUploadUrl(mProvider.noUploadUrl());
    provider->setWebAccess(mProvider.webAccess());
    provider->setWebService(mProvider.webService());
    provider->setIcon(mProvider.icon());

    const QStringList feeds = mProvider.feeds();
    for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it) {
        Feed *feed = mProvider.downloadUrlFeed((*it));
        provider->addDownloadUrlFeed((*it), feed);
    }
    return provider;
}

Provider ProviderHandler::provider()
{
    return mProvider;
}

QDomElement ProviderHandler::serializeElement(const Provider& provider)
{
    QDomDocument doc;

    QDomElement el = doc.createElement("provider");

    KTranslatable name = provider.name();

    QStringList::ConstIterator it;
    QDomElement e;
    const QStringList langs = name.languages();
    for (it = langs.begin(); it != langs.end(); ++it) {
        e = addElement(doc, el, "title", name.translated(*it));
        e.setAttribute("lang", *it);
    }

    /*if(provider.downloadUrl().isValid())
    {
      el.setAttribute("downloadurl", provider.downloadUrl().url());
    }*/
    if (provider.uploadUrl().isValid()) {
        el.setAttribute("uploadurl", provider.uploadUrl().url());
    }
    if (provider.noUploadUrl().isValid()) {
        el.setAttribute("nouploadurl", provider.noUploadUrl().url());
    }
    if (provider.webAccess().isValid()) {
        el.setAttribute("webaccess", provider.webAccess().url());
    }
    if (provider.webService().isValid()) {
        el.setAttribute("webservice", provider.webService().url());
    }
    if (provider.icon().isValid()) {
        el.setAttribute("icon", provider.icon().url());
    }

    const QStringList feeds = provider.feeds();
    for (QStringList::ConstIterator it = feeds.begin(); it != feeds.end(); ++it) {
        Feed *feed = provider.downloadUrlFeed((*it));
        if ((*it).isEmpty())
            el.setAttribute("downloadurl", feed->feedUrl().url());
        else
            el.setAttribute("downloadurl-" + (*it), feed->feedUrl().url());
    }

    mValid = true;
    return el;
}

Provider ProviderHandler::deserializeElement(const QDomElement& providerxml)
{
    Provider provider;
    KTranslatable name;

    if (providerxml.tagName() != "provider") return provider;

    QString uploadurl = providerxml.attribute("uploadurl");
    QString nouploadurl = providerxml.attribute("nouploadurl");
    QString webservice = providerxml.attribute("webservice");
    QString webaccess = providerxml.attribute("webaccess");
    //provider.setDownloadUrl(KUrl(downloadurl));
    provider.setUploadUrl(KUrl(uploadurl));
    provider.setNoUploadUrl(KUrl(nouploadurl));
    provider.setWebService(KUrl(webservice));
    provider.setWebAccess(KUrl(webaccess));

    QString downloadurl = providerxml.attribute("downloadurl");
    QString downloadlatest = providerxml.attribute("downloadurl-latest");
    QString downloadscore = providerxml.attribute("downloadurl-score");
    QString downloaddownloads = providerxml.attribute("downloadurl-downloads");

    if (!downloadlatest.isEmpty()) {
        Feed *feedlatest = new Feed();
        feedlatest->setName(i18nc("describes the feed of the latest posted entries", "Latest"));
        feedlatest->setFeedUrl(downloadlatest);
        provider.addDownloadUrlFeed("latest", feedlatest);
    }
    if (!downloadscore.isEmpty()) {
        Feed *feedscore = new Feed();
        feedscore->setName(i18n("Highest Rated"));
        feedscore->setFeedUrl(downloadscore);
        provider.addDownloadUrlFeed("score", feedscore);
    }
    if (!downloaddownloads.isEmpty()) {
        Feed *feeddownloads = new Feed();
        feeddownloads->setName(i18n("Most Downloads"));
        feeddownloads->setFeedUrl(downloaddownloads);
        provider.addDownloadUrlFeed("downloads", feeddownloads);
    }
    if (!downloadurl.isEmpty()) {
        Feed *feedgeneric = new Feed();
        // feedgeneric->setName(i18n("Unsorted"));
        // Currently this is used for latest
        feedgeneric->setName(i18nc("describes the feed of the latest posted entries", "Latest"));
        feedgeneric->setFeedUrl(downloadurl);
        provider.addDownloadUrlFeed(QString(), feedgeneric);
    }

    // FIXME: what exactly is the following condition supposed to do?
    // FIXME: make sure new KUrl in KDE 4 handles this right
    // FIXME: this depends on freedesktop.org icon naming... introduce 'desktopicon'?
    KUrl iconurl(providerxml.attribute("icon"));
    if (!iconurl.isValid()) iconurl.setPath(providerxml.attribute("icon"));
    provider.setIcon(iconurl);

    QDomNode n;
    for (n = providerxml.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "title") {
            QString lang = e.attribute("lang");
            name.addString(lang, e.text().trimmed());
        }
    }

    provider.setName(name);

    // Validation

    if ((provider.noUploadUrl().isValid()) && (provider.uploadUrl().isValid())) {
        //kWarning(550) << "ProviderHandler: both uploadurl and nouploadurl given";
        return provider;
    }

    if ((!provider.noUploadUrl().isValid()) && (!provider.uploadUrl().isValid())) {
        if (!provider.webService().isValid()) {
            //kWarning(550) << "ProviderHandler: neither uploadurl nor nouploadurl nor DXS given";
            return provider;
        }
    }

    // Provider is valid

    mValid = true;
    return provider;
}

// FIXME: also used in EntryHandler - make common method?
QDomElement ProviderHandler::addElement(QDomDocument& doc, QDomElement& parent,
                                        const QString& tag, const QString& value)
{
    QDomElement n = doc.createElement(tag);
    n.appendChild(doc.createTextNode(value));
    parent.appendChild(n);

    return n;
}
