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

#include <kdatastream.h> // Do not remove, needed for correct bool serialization
#include <kfileitem.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kio/kservice.h>
#include <kparts/componentfactory.h>

#include <qtimer.h>

#include "metainfojob.moc"

using namespace KIO;

struct KIO::MetaInfoJobPrivate
{
    KFileItemList          items;       // all the items we got
    int                    currentItem;
    bool                   deleteItems; // Delete the KFileItems when done?
    bool                   succeeded;   // if the current item is ok
};

MetaInfoJob::MetaInfoJob(const KFileItemList &items, bool deleteItems)
    : KIO::Job(false /* no GUI */),d(new MetaInfoJobPrivate)
{
    d->deleteItems  = deleteItems;
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
    if ( d->deleteItems )
        qDeleteAll( d->items );
    delete d;
}

void MetaInfoJob::start()
{
    getMetaInfo();
}

void MetaInfoJob::removeItem(const KFileItem* item)
{
    if (d->items.at( d->currentItem ) == item)
    {
        KIO::Job* job = subjobs().first();
        job->kill();
        removeSubjob( job );
        determineNextFile();
    }

    d->items.removeAll(const_cast<KFileItem *>(item));
}

void MetaInfoJob::determineNextFile()
{
    if (d->currentItem >= d->items.count() - 1)
    {
        kDebug(7007) << "finished MetaInfoJob\n";
        emitResult();
        return;
    }

    ++d->currentItem;
    d->succeeded = false;

    // does the file item already have the needed info? Then shortcut
    KFileItem* item = d->items.at( d->currentItem );
    if (item->metaInfo(false).isValid())
    {
//        kDebug(7007) << "Is already valid *************************\n";
        emit gotMetaInfo(item);
        determineNextFile();
        return;
    }

    getMetaInfo();
}

void MetaInfoJob::slotResult( KIO::Job *job )
{
    removeSubjob(job);
    Q_ASSERT(!hasSubjobs()); // We should have only one job at a time ...

    determineNextFile();
}

void MetaInfoJob::getMetaInfo()
{
    KFileItem* item = d->items.at( d->currentItem );
    Q_ASSERT(item);

    KUrl URL;
    URL.setProtocol("metainfo");
    URL.setPath(item->url().path());

    KIO::TransferJob* job = KIO::get(URL, false, false);
    addSubjob(job);

    connect(job,  SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotMetaInfo(KIO::Job *, const QByteArray &)));

    job->addMetaData("mimeType", item->mimetype());
}


void MetaInfoJob::slotMetaInfo(KIO::Job*, const QByteArray &data)
{
    KFileMetaInfo info;
    QDataStream s(data);

    s >> info;

    KFileItem* item = d->items.at( d->currentItem );
    item->setMetaInfo(info);
    emit gotMetaInfo(item);
    d->succeeded = true;
}

QStringList MetaInfoJob::availablePlugins()
{
    QStringList result;
    KTrader::OfferList plugins = KTrader::self()->query("KFilePlugin");
    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result.append((*it)->desktopEntryName());
    return result;
}

QStringList MetaInfoJob::supportedMimeTypes()
{
    QStringList result;
    KTrader::OfferList plugins = KTrader::self()->query("KFilePlugin");
    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result += (*it)->property("MimeTypes").toStringList();
    return result;
}

KIO_EXPORT MetaInfoJob *KIO::fileMetaInfo( const KFileItemList &items)
{
    return new MetaInfoJob(items, false);
}

KIO_EXPORT MetaInfoJob *KIO::fileMetaInfo( const KUrl::List &items)
{
    KFileItemList fileItems;
    for (KUrl::List::ConstIterator it = items.begin(); it != items.end(); ++it)
        fileItems.append(new KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    return new MetaInfoJob(fileItems, true);
}

