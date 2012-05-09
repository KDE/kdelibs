/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kwebpluginfactory.h"
#include "kwebpage.h"
#include "kwebview.h"

#include <kmimetypetrader.h>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include <kdebug.h>

#include <kio/job.h>
#include <kparts/part.h>

#include <QtCore/QListIterator>
#include <QtCore/QStringList>
#include <QtCore/QList>

#include <QtWebKit/QWebPluginFactory>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebView>

#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)


KWebPluginFactory::KWebPluginFactory(QObject *parent)
                  :QWebPluginFactory(parent),d(0)
{
}

KWebPluginFactory::~KWebPluginFactory()
{
}

QObject* KWebPluginFactory::create(const QString& _mimeType, const QUrl& url, const QStringList& argumentNames, const QStringList& argumentValues) const
{
    QString mimeType (_mimeType.trimmed());
    // If no mimetype is provided, we do our best to correctly determine it here...
    if (mimeType.isEmpty()) {
        kDebug(800) << "Looking up missing mimetype for plugin resource:" << url;
        extractGuessedMimeType(url, &mimeType);
        kDebug(800) << "Updated mimetype to" << mimeType;
    }

    // Defer handling of flash content to QtWebKit's builtin viewer.
    // If you want to use/test KDE's nspluginviewer, comment out the
    // if statement below.
    KParts::ReadOnlyPart* part = (excludedMimeType(mimeType) ? 0 : createPartInstanceFrom(mimeType, argumentNames, argumentValues, 0, parent()));

    kDebug(800) << "Asked for" << mimeType << "plugin, got" << part;

    if (part) {
        QMap<QString, QString> metaData = part->arguments().metaData();
        QString urlStr = url.toString(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
        metaData.insert("PropagateHttpHeader", "true");
        metaData.insert("referrer", urlStr);
        metaData.insert("cross-domain", urlStr);
        metaData.insert("main_frame_request", "TRUE");
        metaData.insert("ssl_activate_warnings", "TRUE");

        KWebPage *page = qobject_cast<KWebPage *>(parent());

        if (page) {
            const QString scheme = page->currentFrame()->url().scheme();
            if (page && (QString::compare(scheme, QL1S("https"), Qt::CaseInsensitive) == 0 ||
                         QString::compare(scheme, QL1S("webdavs"), Qt::CaseInsensitive) == 0))
              metaData.insert("ssl_was_in_use", "TRUE");
            else
              metaData.insert("ssl_was_in_use", "FALSE");
        }

        KParts::OpenUrlArguments openUrlArgs = part->arguments();
        openUrlArgs.metaData() = metaData;
        openUrlArgs.setMimeType(mimeType);
        part->setArguments(openUrlArgs);
        part->openUrl(url);
        return part->widget();
    }

    return 0;
}

QList<KWebPluginFactory::Plugin> KWebPluginFactory::plugins() const
{
    QList<Plugin> plugins;
    return plugins;
}

static bool isHttpProtocol(const QUrl& url)
{
    const QString scheme (url.scheme());
    return (scheme.startsWith(QL1S("http"), Qt::CaseInsensitive)
         || scheme.startsWith(QL1S("webdav"), Qt::CaseInsensitive));
}

void KWebPluginFactory::extractGuessedMimeType (const QUrl& url, QString* mimeType) const
{
    if (mimeType) {
        const KUrl reqUrl ((isHttpProtocol(url) ? url.path() : url));
        KMimeType::Ptr ptr = KMimeType::findByUrl(reqUrl, 0, reqUrl.isLocalFile(), true);
        if (!ptr->isDefault() && !ptr->name().startsWith(QL1S("inode/"), Qt::CaseInsensitive)) {
            *mimeType = ptr->name();
        }
    }
}

KParts::ReadOnlyPart* KWebPluginFactory::createPartInstanceFrom(const QString& mimeType,
                                                                const QStringList& argumentNames,
                                                                const QStringList& argumentValues,
                                                                QWidget* parentWidget,
                                                                QObject* parentObj) const
{
    KParts::ReadOnlyPart* part = 0;

    if (!mimeType.isEmpty()) {
        // Only attempt to find a KPart for the supported mime types...
        QVariantList arguments;
        const int count = argumentNames.count();

        for (int i = 0; i < count; ++i) {
            arguments << QString(argumentNames.at(i) + QL1S("=\"") + argumentValues.at(i) + QL1C('\"'));
        }
        part = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(mimeType, parentWidget, parentObj, QString(), arguments);
    }

    return part;
}

bool KWebPluginFactory::excludedMimeType (const QString& mimeType) const
{
    if (mimeType.startsWith(QL1S("inode/"), Qt::CaseInsensitive))
        return true;

    if (mimeType.startsWith(QL1S("application/x-java"), Qt::CaseInsensitive))
        return true;

    if (mimeType == QL1S("application/x-shockwave-flash") ||
        mimeType == QL1S("application/futuresplash"))
      return true;

    return false;
}

#include "kwebpluginfactory.moc"
