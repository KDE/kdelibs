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
#include <kio/scheduler.h>
#include <kparts/part.h>

#include <QtCore/QListIterator>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include <QtWebKit/QWebPluginFactory>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebView>

#define QL1S(x)  QLatin1String(x)

static bool excludedMimeType(const QString &type)
{
    if (type.startsWith(QL1S("inode/"), Qt::CaseInsensitive))
        return true;

    if (type.startsWith(QL1S("application/x-java"), Qt::CaseInsensitive))
        return true;

    if (type == QL1S("application/x-shockwave-flash") ||
        type == QL1S("application/futuresplash"))
      return true;

    return false;
}

class KWebPluginFactory::KWebPluginFactoryPrivate
{
public:
  KWebPluginFactoryPrivate() {}

  void _k_slotMimeType(KIO::Job *, const QString&);
  QString mimeType;
};

void KWebPluginFactory::KWebPluginFactoryPrivate::_k_slotMimeType(KIO::Job *kioJob, const QString& mimeType)
{
    kDebug(800) << "Got mimetype" << mimeType;
    this->mimeType = mimeType;
    KIO::TransferJob * job = qobject_cast<KIO::TransferJob*> (kioJob);
    if (job) {
        job->putOnHold();
        KIO::Scheduler::publishSlaveOnHold();
    }
}

KWebPluginFactory::KWebPluginFactory(QObject *parent)
                  :QWebPluginFactory(parent),
                   d(new KWebPluginFactoryPrivate)
{
}

KWebPluginFactory::~KWebPluginFactory()
{
}

QObject* KWebPluginFactory::create(const QString& _mimeType, const QUrl& url, const QStringList& argumentNames, const QStringList& argumentValues) const
{
    // Only attempt to find a KPart for the supported mime types...
    QVariantList arguments;
    const int count = argumentNames.count();

    for (int i = 0; i < count; ++i) {
        arguments << argumentNames.at(i) + "=\"" + argumentValues.at(i) + '\"';
    }

    QString mimeType (_mimeType.trimmed());
    // If no mimetype is provided, we do our best to correctly determine it here...
    if (mimeType.isEmpty()) {
        kDebug(800) << "Looking up missing mimetype for plugin resource:" << url;
        const KUrl reqUrl (url);
        KMimeType::Ptr ptr = KMimeType::findByUrl(reqUrl, 0, reqUrl.isLocalFile());
        // Stat the resource if we mimetype cannot be determined thru
        // KMimeType::findByUrl...
        if (ptr->isDefault()) {
            d->mimeType.clear();
            QEventLoop eventLoop;
            KIO::TransferJob *job = KIO::get(reqUrl, KIO::NoReload, KIO::HideProgressInfo);
            connect(job, SIGNAL(mimetype (KIO::Job *, const QString&)),
                    this, SLOT( _k_slotMimeType(KIO::Job *, const QString&)));
            connect (job, SIGNAL(finished (KJob *)), &eventLoop, SLOT(quit()));
            eventLoop.exec();
            mimeType = d->mimeType;
        } else {
            mimeType = ptr->name();
        }

       // Disregard inode/* mime-types...
       if (mimeType.startsWith(QLatin1String("inode/"), Qt::CaseInsensitive))
          mimeType.clear();
       kDebug(800) << "Updated mimetype to" << mimeType;
    }

    KParts::ReadOnlyPart* part = 0;

    // Defer handling of flash content to QtWebKit's builtin viewer.
    // If you want to use/test KDE's nspluginviewer, comment out the
    // if statement below.
    if (!mimeType.isEmpty() && !excludedMimeType(mimeType))
        part = KMimeTypeTrader::createPartInstanceFromQuery<KParts::ReadOnlyPart>(mimeType, 0, parent(), QString(), arguments);

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
            const QString scheme = page->mainFrame()->url().scheme();
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

#include "kwebpluginfactory.moc"
