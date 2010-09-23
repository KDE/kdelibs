/*****************************************************************************
 * Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>                 *
 * Copyright (C) 2009-2010 by Sebastian Trueg <trueg@kde.org>                *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kloadfilemetadatathread_p.h"

#include <kfilemetainfo.h>
#include <kfilemetainfoitem.h>
#include <kglobal.h>
#include <klocale.h>
#include "kfilemetadataprovider_p.h"
#include <kprotocolinfo.h>

#include "resource.h"
#include "thing.h"
#include "variant.h"
#include "resourcemanager.h"

#include "config-nepomuk.h"

#include "query/filequery.h"
#include "query/comparisonterm.h"
#include "query/andterm.h"
#include "query/resourceterm.h"
#include "query/resourcetypeterm.h"
#include "query/optionalterm.h"
#include "utils/utils.h"

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/NodeIterator>

#include <QMutexLocker>

KLoadFileMetaDataThread::KLoadFileMetaDataThread()
    : m_canceled(false)
{
    connect(this, SIGNAL(finished()), this, SLOT(slotLoadingFinished()));
}

KLoadFileMetaDataThread::~KLoadFileMetaDataThread()
{
}

void KLoadFileMetaDataThread::load(const KUrl::List& urls)
{
    QMutexLocker locker(&m_mutex);
    m_urls = urls;
    m_canceled = false;
    start();
}

QHash<KUrl, Nepomuk::Variant> KLoadFileMetaDataThread::data() const
{
    QMutexLocker locker(&m_mutex);
    return m_data;
}

void KLoadFileMetaDataThread::cancel()
{
    // Setting m_canceled to true will cancel KLoadFileMetaDataThread::run()
    // as soon as run() gets the chance to check m_cancel.
    m_canceled = true;
}

void KLoadFileMetaDataThread::run()
{
    QMutexLocker locker(&m_mutex);
    const KUrl::List urls = m_urls;
    locker.unlock(); // no shared member is accessed until locker.relock()

    unsigned int rating = 0;
    QString comment;
    QList<Nepomuk::Tag> tags;
    QHash<KUrl, Nepomuk::Variant> data;

    bool first = true;
    foreach (const KUrl& url, urls) {
        if (m_canceled) {
            return;
        }

        Nepomuk::Resource file(url);
        if (!file.isValid()) {
            continue;
        }

        if (!first && (rating != file.rating())) {
            rating = 0; // reset rating
        } else if (first) {
            rating = file.rating();
        }

        if (!first && (comment != file.description())) {
            comment.clear(); // reset comment
        } else if (first) {
            comment = file.description();
        }

        if (!first && (tags != file.tags())) {
            tags.clear(); // reset tags
        } else if (first) {
            tags = file.tags();
        }

        if (first && (urls.count() == 1)) {
            // get cached meta data by checking the indexed files
            QHash<QUrl, Nepomuk::Variant> variants = file.properties();
            QHash<QUrl, Nepomuk::Variant>::const_iterator it = variants.constBegin();
            while (it != variants.constEnd()) {
                Nepomuk::Types::Property prop(it.key());
                data.insert(prop.uri(), Nepomuk::Utils::formatPropertyValue(prop, it.value(), QList<Nepomuk::Resource>() << file, Nepomuk::Utils::WithKioLinks));
                ++it;
            }

            if (variants.isEmpty()) {
                // The file has not been indexed, query the meta data
                // directly from the file

                // TODO: KFileMetaInfo (or one of it's used components) is not reentrant.
                // As temporary workaround the access is protected by a mutex.
                static QMutex metaInfoMutex;
                metaInfoMutex.lock();

                const QString path = urls.first().toLocalFile();
                KFileMetaInfo metaInfo(path, QString(), KFileMetaInfo::Fastest);
                const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
                foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
                    const QString uriString = metaInfoItem.name();
                    const Nepomuk::Variant value(metaInfoItem.value());
                    data.insert(uriString, Nepomuk::Utils::formatPropertyValue(Nepomuk::Types::Property(), value));
                }

                metaInfoMutex.unlock();
            }
        }

        first = false;
    }

    if (Nepomuk::ResourceManager::instance()->initialized()) {
        data.insert(KUrl("kfileitem#rating"), rating);
        data.insert(KUrl("kfileitem#comment"), comment);

        QList<Nepomuk::Variant> tagVariants;
        foreach (const Nepomuk::Tag& tag, tags) {
            tagVariants.append(Nepomuk::Variant(tag));
        }
        data.insert(KUrl("kfileitem#tags"), tagVariants);
    }

    locker.relock();
    m_data = data;
}

void KLoadFileMetaDataThread::slotLoadingFinished()
{
    emit finished(this);
}

#include "kloadfilemetadatathread_p.moc"
