/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Copyright (c) 1998 Sirtaj Singh Kang <taj@kde.org>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include "kimageio.h"

#include "qmimedatabase.h"
#include <QDebug>
#include <kservicetypetrader.h>
#include <klocalizedstring.h>

QString
KImageIO::pattern(Mode mode)
{
    QStringList patterns;
    QString allPatterns;
    QString separator("|");
    QMimeDatabase db;

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services)
    {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            QString mimeType = service->property("X-KDE-MimeType").toString();
            if ( mimeType.isEmpty() ) continue;
            QMimeType mime = db.mimeTypeForName(mimeType);
            if (!mime.isValid()) {
                qWarning() << service->entryPath() << " specifies unknown mimetype " << mimeType;
            } else {
                QString pattern = mime.globPatterns().join(" ");
                patterns.append( pattern + separator + mime.comment() );
                if (!allPatterns.isEmpty() )
                    allPatterns += ' ';
                allPatterns += pattern;
            }
        }
    }

    allPatterns = allPatterns + separator + i18n("All Pictures");
    patterns.sort();
    patterns.prepend(allPatterns);

    QString pattern = patterns.join(QLatin1String("\n"));
    return pattern;
}

QStringList KImageIO::typeForMime(const QString& mimeType)
{
    if ( mimeType.isEmpty() )
        return QStringList();

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( mimeType == service->property("X-KDE-MimeType").toString() )
            return ( service->property("X-KDE-ImageFormat").toStringList() );
    }
    return QStringList();
}

QStringList KImageIO::mimeTypes( Mode mode )
{
    QStringList mimeList, allFormats;

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            const QString mime = service->property("X-KDE-MimeType").toString();
            if ( !mime.isEmpty() )
                mimeList.append( mime );
        }
    }

    return mimeList;
}

QStringList KImageIO::types( Mode mode )
{
    QStringList imagetypes;
    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
             (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

            imagetypes += service->property("X-KDE-ImageFormat").toStringList();
        }
    }
    return imagetypes;
}

bool KImageIO::isSupported( const QString& mimeType, Mode mode )
{
    if (mimeType.isEmpty() )
        return false;

    const KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    foreach(const KService::Ptr &service, services) {
        if ( mimeType == service->property("X-KDE-MimeType").toString() ) {

            if ( (service->property("X-KDE-Read").toBool() && mode == Reading) ||
                 (service->property("X-KDE-Write").toBool() && mode == Writing ) ) {

                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}
