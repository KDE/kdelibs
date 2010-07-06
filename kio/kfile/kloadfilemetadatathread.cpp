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

#include <resource.h>
#include <thing.h>
#include <resourcemanager.h>

#include "config-nepomuk.h"

#ifdef HAVE_NEPOMUK_WITH_SDO_0_5
#include <query/query.h>
#include <query/andterm.h>
#include <query/comparisonterm.h>
#include <query/resourceterm.h>
#include <query/resourcetypeterm.h>
#include <query/optionalterm.h>

#include "nfo.h"
#include "nuao.h"
#include "ndo.h"

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/NodeIterator>
#endif

#include <QMutexLocker>

KLoadFileMetaDataThread::KLoadFileMetaDataThread() :
    m_mutex(),
    m_data(),
    m_urls(),
    m_canceled(false)
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
                const QString uriString = prop.uri().toString();
                data.insert(uriString, formatValue(prop, it.value()));
                ++it;
            }

            if (variants.isEmpty()) {
                // the file has not been indexed, query the meta data
                // directly from the file
                const QString path = urls.first().toLocalFile();
                KFileMetaInfo metaInfo(path, QString(), KFileMetaInfo::Fastest);
                const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
                foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
                    const QString uriString = metaInfoItem.name();
                    const Nepomuk::Variant value(metaInfoItem.value());
                    data.insert(uriString, formatValue(Nepomuk::Types::Property(), value));
                }
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

QString  KLoadFileMetaDataThread::formatValue(const Nepomuk::Types::Property& prop, const Nepomuk::Variant& value)
{
    if (value.isDateTime()) {
        return KGlobal::locale()->formatDateTime(value.toDateTime(), KLocale::FancyLongDate);
    }

    else if(value.isDouble()) {
        return KGlobal::locale()->formatNumber(value.toDouble());
    }

    else if (value.isResource() || value.isResourceList()) {
#ifdef HAVE_NEPOMUK_WITH_SDO_0_5
        //
        // We handle the one special case of referrer URLs of downloads
        // TODO: put stuff like this in a generic rule-based framework
        //
        if(prop == Nepomuk::Vocabulary::NDO::copiedFrom()) {
            Nepomuk::Query::Query query(
                Nepomuk::Query::AndTerm(
                    Nepomuk::Query::ResourceTypeTerm(
                        Nepomuk::Vocabulary::NDO::DownloadEvent()
                        ),
                    Nepomuk::Query::ComparisonTerm(
                        Nepomuk::Vocabulary::NUAO::involves(),
                        Nepomuk::Query::ResourceTerm(m_urls.first())
                        )
                    )
                );
            query.setLimit(1);

            QList<Soprano::Node> results =
                Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
                    query.toSparqlQuery(),
                    Soprano::Query::QueryLanguageSparql).iterateBindings(0).allNodes();
            if(!results.isEmpty()) {
                Nepomuk::Resource dlRes(results.first().uri());
                KUrl url;
                QString label;
                if(dlRes.hasProperty(Nepomuk::Vocabulary::NDO::referrer())) {
                    url = dlRes.property(Nepomuk::Vocabulary::NDO::referrer()).toUrl();
                    KUrl referrerDomain(url);
                    referrerDomain.setPath(QString());
                    referrerDomain.setQuery(QString());
                    label = referrerDomain.prettyUrl();
                }
                else {
                    Nepomuk::Resource res(value.toResource());
                    url = res.resourceUri();
                    label = res.genericLabel();
                }

                return QString::fromLatin1("<a href=\"%1\">%2</a>")
                    .arg(url.url(), label);
            }
        }
#endif
        QStringList links;
        foreach(const Nepomuk::Resource& res, value.toResourceList()) {
            if (KProtocolInfo::isKnownProtocol(res.resourceUri())) {
                links << QString::fromLatin1("<a href=\"%1\">%2</a>")
                         .arg(KUrl(res.resourceUri()).url(),
                              res.genericLabel());
            } else {
                links << res.genericLabel();
            }
        }
        return links.join(QLatin1String(";\n"));
    }

    return value.toString();
}

#include "kloadfilemetadatathread_p.moc"
