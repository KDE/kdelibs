/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "entryloader.h"

#include "entryhandler.h"
#include "feed.h"

#include <QtCore/QByteArray>

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>

using namespace KNS;

EntryLoader::EntryLoader(QObject* parent)
        : QObject(parent)
{
    m_feed = 0;
    m_provider = 0;
}

void EntryLoader::load(const Provider *provider, const Feed *feed)
{
    //kDebug(550) << "EntryLoader::load()";

    m_provider = provider;
    m_feed = feed;

    m_entries.clear();
    m_jobdata.clear();

    KUrl stuffurl = feed->feedUrl();
    //kDebug(550) << "EntryLoader::load(): stuffUrl: " << stuffurl.url();

    KIO::TransferJob *job = KIO::get(stuffurl, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob *)),
            SLOT(slotJobResult(KJob *)));
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            SLOT(slotJobData(KIO::Job *, const QByteArray &)));
}

const Feed *EntryLoader::feed() const
{
    return m_feed;
}

const Provider *EntryLoader::provider() const
{
    return m_provider;
}

void EntryLoader::slotJobData(KIO::Job *, const QByteArray &data)
{
    //kDebug(550) << "EntryLoader::slotJobData()";

    m_jobdata.append(data);
}

void EntryLoader::slotJobResult(KJob *job)
{
    if (job->error()) {
        emit signalEntriesFailed();
        return;
    }

    //QString contents = QString::fromUtf8(m_jobdata);

    //kDebug(550) << "--ENTRIES-START--";
    //kDebug(550) << QString::fromUtf8(m_jobdata);
    //kDebug(550) << "--ENTRIES-END--";

    QDomDocument doc;
    if (!doc.setContent(m_jobdata)) {
        emit signalEntriesFailed();
        return;
    }

    QDomElement entries = doc.documentElement();

    if (entries.isNull()) {
        kWarning(550) << "No document in stuff.xml.";
    }

    QDomNode n;
    for (n = entries.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();

        if (e.tagName() == "stuff") {
            EntryHandler handler(e);
            m_entries.append(handler.entryptr());
        }
    }

    emit signalEntriesLoaded(m_entries);
}

#include "entryloader.moc"
