/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kjavadownloader.h"
#include "kjavaappletserver.h"

#include <kurl.h>
#include <kio/job.h>
#include <kdebug.h>
#include <qfile.h>

class KJavaDownloaderPrivate
{
friend class KJavaDownloader;
public:
    ~KJavaDownloaderPrivate()
    {
        if( url )
        delete url;
    }
private:
    QString           loaderID;
    KURL*             url;
    QByteArray        file;
    KIO::TransferJob* job;
};


KJavaDownloader::KJavaDownloader( QString& ID, QString& url )
{
    kdDebug(6100) << "KJavaDownloader for ID = " << ID << " and url = " << url << endl;

    d = new KJavaDownloaderPrivate;

    d->loaderID = ID;
    d->url = new KURL( url );

    d->job = KIO::get( url, false, false );
    connect( d->job,  SIGNAL(data( KIO::Job*, const QByteArray& )),
             this,    SLOT(slotData( KIO::Job*, const QByteArray& )) );
    connect( d->job, SIGNAL(result(KIO::Job*)),
             this,   SLOT(slotResult(KIO::Job*)) );
}

KJavaDownloader::~KJavaDownloader()
{
    delete d;
}

void KJavaDownloader::slotData( KIO::Job*, const QByteArray& qb )
{
    kdDebug(6100) << "slotData for url = " << d->url->url() << endl;

    int cur_size = d->file.size();
    int qb_size = qb.size();
    d->file.resize( cur_size + qb_size );
    memcpy( d->file.data() + cur_size, qb.data(), qb_size );


}

void KJavaDownloader::slotResult( KIO::Job* )
{
    kdDebug(6100) << "slotResult for url = " << d->url->url() << endl;

    if( d->job->error() )
    {
        kdDebug(6100) << "slave had an error = " << d->job->errorString() << endl;
        KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
        d->file.resize(0);
        server->sendURLData( d->loaderID, d->url->url(), d->file );
        KJavaAppletServer::freeJavaServer();
    }
    else
    {
        kdDebug(6100) << "slave got all its data, sending to KJAS" << endl;
        kdDebug(6100) << "size of data = " << d->file.size() << endl;
        KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
        server->sendURLData( d->loaderID, d->url->url(), d->file );
        KJavaAppletServer::freeJavaServer();
    }

    delete this;
}

#include "kjavadownloader.moc"

