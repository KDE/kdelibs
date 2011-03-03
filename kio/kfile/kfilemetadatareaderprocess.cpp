/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
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

#include <iostream>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
#include <klocale.h>

#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QHash>
#include <QtCore/QString>

#define DISABLE_NEPOMUK_LEGACY
#include "config-nepomuk.h"

#include <nepomuk/query/filequery.h>
#include <nepomuk/query/comparisonterm.h>
#include <nepomuk/query/andterm.h>
#include <nepomuk/query/resourceterm.h>
#include <nepomuk/query/resourcetypeterm.h>
#include <nepomuk/query/optionalterm.h>
#include <nepomuk/utils/utils.h>
#include <nepomuk/property.h>
#include <nepomuk/tag.h>
#include <nepomuk/variant.h>
#include <nepomuk/resourcemanager.h>

using namespace std;

static void sendMetaData(const QHash<KUrl, Nepomuk::Variant>& data)
{
    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::WriteOnly);

    QHashIterator<KUrl, Nepomuk::Variant> it(data);
    while (it.hasNext()) {
        it.next();
        out << it.key().url() << it.value().variant();
    }

    cout << byteArray.toBase64().constData();
}

static QHash<KUrl, Nepomuk::Variant> readFileMetaData(const QList<KUrl>& urls)
{
    QHash<KUrl, Nepomuk::Variant> data;

    // Currently only the meta-data of one file is supported.
    // It might be an option to read all meta-data and show
    // ranges for each key.
    if (urls.count() == 1) {
        const QString path = urls.first().toLocalFile();
        KFileMetaInfo metaInfo(path, QString(), KFileMetaInfo::Fastest);
        const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
        foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
            const QString uriString = metaInfoItem.name();
            const Nepomuk::Variant value(metaInfoItem.value());
            data.insert(uriString,
                        Nepomuk::Utils::formatPropertyValue(Nepomuk::Types::Property(), value));
        }
    }

    return data;
}

static QHash<KUrl, Nepomuk::Variant> readFileAndContextMetaData(const QList<KUrl>& urls)
{
    QHash<KUrl, Nepomuk::Variant> metaData;

    unsigned int rating = 0;
    QString comment;
    QList<Nepomuk::Tag> tags;

    bool first = true;
    foreach (const KUrl& url, urls) {
        Nepomuk::Resource file(url);
        if (!file.isValid()) {
            continue;
        }

        if (!first && (rating != file.rating())) {
            rating = 0; // Reset rating
        } else if (first) {
            rating = file.rating();
        }

        if (!first && (comment != file.description())) {
            comment.clear(); // Reset comment
        } else if (first) {
            comment = file.description();
        }

        if (!first && (tags != file.tags())) {
            tags.clear(); // Reset tags
        } else if (first) {
            tags = file.tags();
        }

        if (first && (urls.count() == 1)) {
            // Get cached meta data by checking the indexed files
            QHash<QUrl, Nepomuk::Variant> variants = file.properties();
            QHash<QUrl, Nepomuk::Variant>::const_iterator it = variants.constBegin();
            while (it != variants.constEnd()) {
                Nepomuk::Types::Property prop(it.key());
                metaData.insert(prop.uri(), Nepomuk::Utils::formatPropertyValue(prop, it.value(),
                                                                            QList<Nepomuk::Resource>() << file,
                                                                            Nepomuk::Utils::WithKioLinks));
                ++it;
            }

            if (variants.isEmpty()) {
                // The file has not been indexed, query the meta data
                // directly from the file.
                metaData = readFileMetaData(QList<KUrl>() << urls.first());
            }
        }

        first = false;
    }

    if (Nepomuk::ResourceManager::instance()->initialized()) {
        metaData.insert(KUrl("kfileitem#rating"), rating);
        metaData.insert(KUrl("kfileitem#comment"), comment);

        QList<Nepomuk::Variant> tagVariants;
        foreach (const Nepomuk::Tag& tag, tags) {
            tagVariants.append(Nepomuk::Variant(tag));
        }
        metaData.insert(KUrl("kfileitem#tags"), tagVariants);
    }

    return metaData;
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("kfilemetadatareader", 0, ki18n("KFileMetaDataReader"),
                         "1.0",
                         ki18n("KFileMetaDataReader can be used to read metadata from a file"),
                         KAboutData::License_GPL,
                         ki18n("(C) 2011, Peter Penz"));
    aboutData.addAuthor(ki18n("Peter Penz"), ki18n("Current maintainer"), "peter.penz19@gmail.com");
    
    KCmdLineArgs::init(argc, argv, &aboutData);
    
    KCmdLineOptions options;
    options.add("file", ki18n("Only the meta data that is part of the file is read"));
    options.add("+[arg]", ki18n("List of URLs where the meta-data should be read from"));

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUrl::List urls;
    const int argsCount = args->count();
    for (int i = 0; i < argsCount; ++i) {
        urls.append(KUrl(args->arg(i)));
    }

    QHash<KUrl, Nepomuk::Variant> metaData;
    if (args->isSet("file")) {
        metaData = readFileMetaData(urls);
    } else {
        metaData = readFileAndContextMetaData(urls);
    }

    sendMetaData(metaData);
}
