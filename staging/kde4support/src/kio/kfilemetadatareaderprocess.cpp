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

#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QTimer>

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

class KFileMetaDataReaderApplication : public QCoreApplication
{
    Q_OBJECT

public:
    KFileMetaDataReaderApplication(int& argc, char** argv);

private Q_SLOTS:
    void readAndSendMetaData();

private:
    void sendMetaData(const QHash<QUrl, Nepomuk::Variant>& data);
    QHash<QUrl, Nepomuk::Variant> readFileMetaData(const QList<QUrl>& urls) const;
    QHash<QUrl, Nepomuk::Variant> readFileAndContextMetaData(const QList<QUrl>& urls) const;
};



KFileMetaDataReaderApplication::KFileMetaDataReaderApplication(int& argc, char** argv) :
    QCoreApplication(argc, argv)
{
    QTimer::singleShot(0, this, SLOT(readAndSendMetaData()));
}

void KFileMetaDataReaderApplication::readAndSendMetaData()
{
    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QList<QUrl> urls;
    for (int i = 0; i < args->count(); ++i) {
        urls.append(args->url(i));
    }

    QHash<QUrl, Nepomuk::Variant> metaData;
    if (args->isSet("file")) {
        metaData = readFileMetaData(urls);
    } else {
        metaData = readFileAndContextMetaData(urls);
    }

    sendMetaData(metaData);

    quit();
}

void KFileMetaDataReaderApplication::sendMetaData(const QHash<QUrl, Nepomuk::Variant>& data)
{
    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::WriteOnly);

    QHashIterator<QUrl, Nepomuk::Variant> it(data);
    while (it.hasNext()) {
        it.next();

        out << it.key();

        // Unlike QVariant no streaming operators are implemented for Nepomuk::Variant.
        // So it is required to manually encode the variant for the stream.
        // The decoding counterpart is located in KFileMetaDataReader.
        const Nepomuk::Variant& variant = it.value();
        if (variant.isList()) {
            out << 0 << variant.toStringList();
        } else if (variant.isResource()) {
            out << 1 << variant.toString();
        } else {
            out << 2 << variant.variant();
        }
    }

    cout << byteArray.toBase64().constData();
}

QHash<QUrl, Nepomuk::Variant> KFileMetaDataReaderApplication::readFileMetaData(const QList<QUrl>& urls) const
{
    QHash<QUrl, Nepomuk::Variant> data;

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

QHash<QUrl, Nepomuk::Variant> KFileMetaDataReaderApplication::readFileAndContextMetaData(const QList<QUrl>& urls) const
{
    QHash<QUrl, Nepomuk::Variant> metaData;

    bool isNepomukIndexerActive = false;
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        KConfig config("nepomukserverrc");
        isNepomukIndexerActive = config.group("Service-nepomukfileindexer").readEntry("autostart", false);
    } else {
        // No context meta data can be read without enabled Nepomuk
        return readFileMetaData(urls);
    }

    unsigned int rating = 0;
    QString comment;
    QList<Nepomuk::Tag> tags;

    if (urls.count() == 1) {
        // Read the metadata of the file that are provided as properties
        // (e.g. image-size, artist, album, ...)
        bool useReadFromFileFallback = true;

        Nepomuk::Resource file(urls.first());
        if (file.isValid() && !file.resourceUri().isEmpty()) {
            QHash<QUrl, Nepomuk::Variant> variants = file.properties();
            QHash<QUrl, Nepomuk::Variant>::const_iterator it = variants.constBegin();
            while (it != variants.constEnd()) {
                Nepomuk::Types::Property prop(it.key());
                metaData.insert(prop.uri(), Nepomuk::Utils::formatPropertyValue(prop, it.value(),
                                                                                QList<Nepomuk::Resource>() << file,
                                                                                Nepomuk::Utils::WithKioLinks));
                ++it;
            }
            useReadFromFileFallback = !isNepomukIndexerActive || variants.isEmpty();

            rating = file.rating();
            comment = file.description();
            tags = file.tags();
        }

        if (useReadFromFileFallback) {
            // No metadata could be received with Nepomuk. Parse the file
            // itself as fallback to extract metadata.
            metaData = readFileMetaData(QList<QUrl>() << urls.first());
        }
    } else {
        // Read the data for rating, comment and tags
        bool first = true;
        foreach (const QUrl& url, urls) {
            Nepomuk::Resource file(url);
            if (!file.isValid()) {
                continue;
            }

            if (!first && rating != file.rating()) {
                rating = 0; // Reset rating
            } else if (first) {
                rating = file.rating();
            }

            if (!first && comment != file.description()) {
                comment.clear(); // Reset comment
            } else if (first) {
                comment = file.description();
            }

            if (!first && tags != file.tags()) {
                tags.clear(); // Reset tags
            } else if (first) {
                tags = file.tags();
            }

            first = false;
        }
    }

    metaData.insert(QUrl("kfileitem#rating"), rating);
    metaData.insert(QUrl("kfileitem#comment"), comment);

    QList<Nepomuk::Variant> tagVariants;
    foreach (const Nepomuk::Tag& tag, tags) {
        tagVariants.append(Nepomuk::Variant(tag));
    }
    metaData.insert(QUrl("kfileitem#tags"), tagVariants);

    return metaData;
}

int main(int argc, char *argv[])
{
    K4AboutData aboutData("kfilemetadatareader", "kio4", ki18n("KFileMetaDataReader"),
                         "1.0",
                         ki18n("KFileMetaDataReader can be used to read metadata from a file"),
                         K4AboutData::License_GPL,
                         ki18n("(C) 2011, Peter Penz"));
    aboutData.addAuthor(ki18n("Peter Penz"), ki18n("Current maintainer"), "peter.penz19@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("file", ki18n("Only the meta data that is part of the file is read"));
    options.add("+[arg]", ki18n("List of URLs where the meta-data should be read from"));

    KCmdLineArgs::addCmdLineOptions(options);

    KFileMetaDataReaderApplication app(argc, argv);
    return app.exec();
}

#include "kfilemetadatareaderprocess.moc"
