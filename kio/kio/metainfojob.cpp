// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "metainfojob.h"

#include <kfileitem.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kservicetypetrader.h>

#include <QtCore/QTimer>

#include "jobuidelegate.h"
#include "job_p.h"

using namespace KIO;

class KIO::MetaInfoJobPrivate: public KIO::JobPrivate
{
public:
    KFileItemList          items;       // all the items we got
    int                    currentItem;
    bool                   succeeded;   // if the current item is ok

    Q_DECLARE_PUBLIC(MetaInfoJob)
};

MetaInfoJob::MetaInfoJob(const KFileItemList& items, KFileMetaInfo::WhatFlags,
     int, int, const QStringList&, const QStringList&)
    : KIO::Job(*new MetaInfoJobPrivate)
{
    Q_D(MetaInfoJob);
    d->succeeded    = false;
    d->items        = items;
    d->currentItem  = 0;

    if (d->items.isEmpty())
    {
        kDebug(7007) << "nothing to do for the MetaInfoJob\n";
        emitResult();
        return;
    }

    kDebug(7007) << "starting MetaInfoJob\n";

    // Return to event loop first, determineNextFile() might delete this;
    // (no idea what that means, it comes from previewjob)
    QTimer::singleShot(0, this, SLOT(start()));
}

MetaInfoJob::~MetaInfoJob()
{
}

void MetaInfoJob::start()
{
    getMetaInfo();
}

void MetaInfoJob::removeItem(const KFileItem& item)
{
    Q_D(MetaInfoJob);
    if (d->items.at( d->currentItem ) == item)
    {
        KJob* job = subjobs().first();
        job->kill();
        removeSubjob( job );
        determineNextFile();
    }

    d->items.removeAll(item);
}

void MetaInfoJob::determineNextFile()
{
    Q_D(MetaInfoJob);
    if (d->currentItem >= d->items.count() - 1)
    {
        kDebug(7007) << "finished MetaInfoJob\n";
        emitResult();
        return;
    }

    ++d->currentItem;
    d->succeeded = false;

    // does the file item already have the needed info? Then shortcut
    KFileItem item = d->items.at( d->currentItem );
    if (item.metaInfo(false).isValid())
    {
//        kDebug(7007) << "Is already valid *************************\n";
        emit gotMetaInfo(item);
        determineNextFile();
        return;
    }

    getMetaInfo();
}

void MetaInfoJob::slotResult( KJob *job )
{
    removeSubjob(job);
    Q_ASSERT(!hasSubjobs()); // We should have only one job at a time ...

    determineNextFile();
}

void MetaInfoJob::getMetaInfo()
{
    Q_D(MetaInfoJob);
    KFileItem item = d->items.at( d->currentItem );
    Q_ASSERT(!item.isNull());

    KUrl URL;
    URL.setProtocol("metainfo");
    URL.setPath(item.url().path());

    KIO::TransferJob* job = KIO::get(URL, NoReload, HideProgressInfo);
    addSubjob(job);

    connect(job,  SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotMetaInfo(KIO::Job *, const QByteArray &)));

    job->addMetaData("mimeType", item.mimetype());
}


void MetaInfoJob::slotMetaInfo(KIO::Job*, const QByteArray &data)
{
    Q_D(MetaInfoJob);
    KFileMetaInfo info;
    QDataStream s(data);

    s >> info;

    KFileItem item = d->items.at( d->currentItem );
    item.setMetaInfo(info);
    emit gotMetaInfo(item);
    d->succeeded = true;
}

KIO_EXPORT MetaInfoJob *KIO::fileMetaInfo( const KFileItemList& items)
{
    return new MetaInfoJob(items);
}

KIO_EXPORT MetaInfoJob *KIO::fileMetaInfo( const KUrl::List &items)
{
    KFileItemList fileItems;
    foreach (const KUrl& url, items) {
        fileItems.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, url,
            true));
    }
    MetaInfoJob *job = new MetaInfoJob(fileItems);
    job->setUiDelegate(new JobUiDelegate());
    return job;
}

#include "metainfojob.moc"
