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

static const int DATA = 0;
static const int FINISHED = 1;
static const int ERRORCODE = 2;
static const int HEADERS = 3;
static const int REDIRECT = 4;
static const int MIMETYPE = 5;
                
class KJavaDownloaderPrivate
{
friend class KJavaDownloader;
public:
    KJavaDownloaderPrivate() : responseCode(0), isfirstdata(true) {}
    ~KJavaDownloaderPrivate()
    {
        if( url )
        delete url;
    }
private:
    int               loaderID;
    KURL*             url;
    QByteArray        file;
    KIO::TransferJob* job;
    int               responseCode;
    bool              isfirstdata;
};


/* KDE 4: Make them const QString & */
KJavaDownloader::KJavaDownloader( int ID, const QString& url )
{
    kdDebug(6100) << "KJavaDownloader for ID = " << ID << " and url = " << url << endl;

    d = new KJavaDownloaderPrivate;

    d->loaderID = ID;
    d->url = new KURL( url );

    d->job = KIO::get( url, false, false );
    d->job->addMetaData("PropagateHttpHeader", "true");
    connect( d->job,  SIGNAL(data( KIO::Job*, const QByteArray& )),
             this,    SLOT(slotData( KIO::Job*, const QByteArray& )) );
    connect( d->job, SIGNAL(connected(KIO::Job*)),
             this, SLOT(slotConnected(KIO::Job*)));
    connect( d->job, SIGNAL(mimetype(KIO::Job*, const QString&)),
             this, SLOT(slotMimetype(KIO::Job*, const QString&)));
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

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->isfirstdata) {
        QString headers = d->job->queryMetaData("HTTP-Headers");
        if (!headers.isEmpty()) {
            d->file.resize( headers.length() );
            memcpy( d->file.data(), headers.ascii(), headers.length() );
            server->sendURLData( d->loaderID, HEADERS, d->file );
        }
        d->isfirstdata = false;
        KIO::MetaData md = d->job->metaData();
        KIO::MetaData::ConstIterator it;
        for( it = md.begin(); it !=  md.end(); ++it)
            kdDebug(6100)<< "metadata " << it.key() << "=" << it.data() << endl;
    }
    int qb_size = qb.size();
    if (qb_size) {
        d->file.resize( qb_size );
        memcpy( d->file.data(), qb.data(), qb_size );
        server->sendURLData( d->loaderID, DATA, d->file );
        d->file.resize( 0 );
    }
    KJavaAppletServer::freeJavaServer();
}

void KJavaDownloader::slotConnected(KIO::Job*)
{
    kdDebug(6100) << "slave connected" << endl;
    d->responseCode = d->job->error();
}

void KJavaDownloader::slotMimetype(KIO::Job*, const QString & type) {
    kdDebug(6100) << "slave mimetype " << type << endl;
}

void KJavaDownloader::slotResult( KIO::Job* )
{
    kdDebug(6100) << "slotResult for url = " << d->url->url() << endl;

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if( d->job->error())
    //if( d->job->error() || d->job->isErrorPage())
    {
        kdDebug(6100) << "slave had an error = " << d->job->errorString() << endl;
        int code = d->job->error();
        if (!code)
            code = 404;
        QString codestr = QString::number(code);
        d->file.resize(codestr.length());
        memcpy( d->file.data(), codestr.ascii(), codestr.length() );
        kdDebug(6100) << "slave had an error = " << code << endl;

        server->sendURLData( d->loaderID, ERRORCODE, d->file );
    }
    else
    {
        kdDebug(6100) << "slave got all its data, sending to KJAS" << endl;
        kdDebug(6100) << "size of data = " << d->file.size() << endl;
        server->sendURLData( d->loaderID, FINISHED, d->file );
    }
    KJavaAppletServer::freeJavaServer();

    delete this;
}

#include "kjavadownloader.moc"

