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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
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
    KFileItemListIterator* currentItem; // argh! No default constructor
    bool                   deleteItems; // Delete the KFileItems when done?
    bool                   succeeded;   // if the current item is ok
};

MetaInfoJob::MetaInfoJob(const KFileItemList &items, bool deleteItems)
    : KIO::Job(false /* no GUI */)
{
    d               = new MetaInfoJobPrivate;
    d->deleteItems  = deleteItems;
    d->succeeded    = false;
    d->items        = items;
    d->currentItem  = new KFileItemListIterator(d->items);
    
    d->items.setAutoDelete(deleteItems);
    
    if (d->currentItem->isEmpty())
    {
        kdDebug(7007) << "nothing to do for the MetaInfoJob\n";
        emitResult();
        return;
    }

    kdDebug(7007) << "starting MetaInfoJob\n";

    // Return to event loop first, determineNextFile() might delete this;
    // (no idea what that means, it comes from previewjob)
    QTimer::singleShot(0, this, SLOT(start()));
}

MetaInfoJob::~MetaInfoJob()
{
    delete d->currentItem;
    delete d;
}

void MetaInfoJob::start()
{
    getMetaInfo();
}

void MetaInfoJob::removeItem(const KFileItem* item)
{
    if (d->currentItem->current() == item)
    {
        subjobs.first()->kill();
        subjobs.removeFirst();
        determineNextFile();
    }

    d->items.remove(d->items.find(item));
}

void MetaInfoJob::determineNextFile()
{
    if (d->currentItem->atLast())
    {
        kdDebug(7007) << "finished MetaInfoJob\n";
        emitResult();
        return;
    }

    ++(*d->currentItem);
    d->succeeded = false;
    
    // does the file item already have the needed info? Then shortcut
    if (d->currentItem->current()->metaInfo(false).isValid())
    {
//        kdDebug(7007) << "Is already valid *************************\n";
        emit gotMetaInfo(d->currentItem->current());
        determineNextFile();
        return;
    }
   
    getMetaInfo();
}

void MetaInfoJob::slotResult( KIO::Job *job )
{
    subjobs.remove(job);
    Q_ASSERT(subjobs.isEmpty()); // We should have only one job at a time ...

    determineNextFile();
}

void MetaInfoJob::getMetaInfo()
{
    Q_ASSERT(!d->currentItem->isEmpty());

    KURL URL;
    URL.setProtocol("metainfo");
    URL.setPath(d->currentItem->current()->url().path());

    KIO::TransferJob* job = KIO::get(URL, false, false);
    addSubjob(job);
    
    connect(job,  SIGNAL(data(KIO::Job *, const QByteArray &)), 
            this, SLOT(slotMetaInfo(KIO::Job *, const QByteArray &)));

    job->addMetaData("mimeType", d->currentItem->current()->mimetype());
}


void MetaInfoJob::slotMetaInfo(KIO::Job* , const QByteArray &data)
{
    KFileMetaInfo info;
    QDataStream s(data, IO_ReadOnly);

    s >> info;
    
    // every file should get a valid (but maybe empty) item
    if (!info.isValid()) kdDebug(7007) << info.path() << " is not valid\n";
          
    d->currentItem->current()->setMetaInfo(info);
    emit gotMetaInfo(d->currentItem->current());
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

MetaInfoJob *KIO::fileMetaInfo( const KFileItemList &items)
{
    return new MetaInfoJob(items, false);
}

MetaInfoJob *KIO::fileMetaInfo( const KURL::List &items)
{
    KFileItemList fileItems;
    for (KURL::List::ConstIterator it = items.begin(); it != items.end(); ++it)
        fileItems.append(new KFileItem(-1, -1, *it, true));
    return new MetaInfoJob(fileItems, true);
}

