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

#include "providerloader.h"

#include "providerhandler.h"

#include <QtCore/QByteArray>

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>

using namespace KNS;

ProviderLoader::ProviderLoader(QObject* parent)
        : QObject(parent)
{
}

void ProviderLoader::load(const QString &providersurl)
{
    //kDebug(550) << "ProviderLoader::load()";

    m_providers.clear();
    m_jobdata.clear();

    //kDebug(550) << "ProviderLoader::load(): providersUrl: " << providersurl;

    KIO::TransferJob *job = KIO::get(KUrl(providersurl), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob *)),
            SLOT(slotJobResult(KJob *)));
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
            SLOT(slotJobData(KIO::Job *, const QByteArray &)));
}

void ProviderLoader::slotJobData(KIO::Job *, const QByteArray &data)
{
    //kDebug(550) << "ProviderLoader::slotJobData()";

    m_jobdata.append(data);
}

void ProviderLoader::slotJobResult(KJob *job)
{
    if (job->error()) {
        emit signalProvidersFailed();
        return;
    }

    //kDebug(550) << "--PROVIDERS-START--";
    //kDebug(550) << QString::fromUtf8(m_jobdata);
    //kDebug(550) << "--PROVIDERS-END--";

    QDomDocument doc;
    if (!doc.setContent(m_jobdata)) {
        emit signalProvidersFailed();
        return;
    }

    QDomElement providers = doc.documentElement();

    if (providers.tagName() != "ghnsproviders" &&
            providers.tagName() != "knewstuffproviders") {
        kWarning(550) << "No document in providers.xml.";
        emit signalProvidersFailed();
        return;
    }

    QDomNode n;
    for (n = providers.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement p = n.toElement();

        if (p.tagName() == "provider") {
            ProviderHandler handler(p);
            m_providers.append(handler.providerptr());
        }
    }

    emit signalProvidersLoaded(m_providers);
}

#include "providerloader.moc"
