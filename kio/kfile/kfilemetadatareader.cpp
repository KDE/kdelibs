/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz@gmx.at>                      *
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
#include <nepomuk/types/property.h>
#include <nepomuk/core/tag.h>
#include <nepomuk/core/variant.h>
#include <nepomuk/core/resourcemanager.h>

using namespace std;

static void sendMetaData(const QHash<KUrl, Nepomuk::Variant>& data)
{
    // Write the meta-data as sequence of the following properties:
    // 1. Metadata key
    // 2. Variant type
    // 3. Variant value
    QHashIterator<KUrl, Nepomuk::Variant> it(data);
    while (it.hasNext()) {
        it.next();
        const QString key = it.key().url();
        const Nepomuk::Variant& variant = it.value();

        const QString variantType = QString::number(variant.type());
        // TODO: serialize valuetypes like variant lists
        QString variantValue = variant.toString();
        // QChar::Other_Control acts as separator between the output-values.
        // Assure that this character is not already part of a value:
        variantValue.remove(QChar::Other_Control);

        cout << key.toLocal8Bit().data() << endl;
        cout << variantType.toLocal8Bit().data() << endl;
        cout << variantValue.toLocal8Bit().data() << endl;
    }
}

static int readMetaData(const KCmdLineArgs* args)
{
    KUrl::List urls;
    const int argsCount = args->count();
    for (int i = 0; i < argsCount; ++i) {
        urls.append(KUrl(args->arg(i)));
    }

    unsigned int rating = 0;
    QString comment;
    QList<Nepomuk::Tag> tags;
    QHash<KUrl, Nepomuk::Variant> data;

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
                data.insert(prop.uri(), Nepomuk::Utils::formatPropertyValue(prop, it.value(),
                                                                            QList<Nepomuk::Resource>() << file,
                                                                            Nepomuk::Utils::WithKioLinks));
                ++it;
            }

            if (variants.isEmpty()) {
                // The file has not been indexed, query the meta data
                // directly from the file.
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

    sendMetaData(data);
    return 0;
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
    options.add("+[arg]", ki18n("List of URLs where the meta-data should be read from"));

    KCmdLineArgs::addCmdLineOptions(options);
    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    return readMetaData(args);
}

