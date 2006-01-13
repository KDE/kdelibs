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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kjavadownloader.h"
#include "kjavaappletserver.h"

#include <kurl.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdebug.h>
#include <qfile.h>

static const int DATA = 0;
static const int FINISHED = 1;
static const int ERRORCODE = 2;
static const int HEADERS = 3;
static const int REDIRECT = 4;
static const int MIMETYPE = 5;
static const int CONNECTED = 6;
static const int REQUESTDATA = 7;

static const int KJAS_STOP = 0;
static const int KJAS_HOLD = 1;
static const int KJAS_RESUME = 2;

KJavaKIOJob::~KJavaKIOJob() {}

void KJavaKIOJob::data( const QByteArray& )
{
    kdError(6100) << "Job id mixup" << endl;
}

//-----------------------------------------------------------------------------

class KJavaDownloaderPrivate
{
friend class KJavaDownloader;
public:
    KJavaDownloaderPrivate() : responseCode(0), isfirstdata(true) {}
    ~KJavaDownloaderPrivate()
    {
        delete url;
        if (job) job->kill(); // KIO::Job::kill deletes itself
    }
private:
    int               loaderID;
    KURL*             url;
    QByteArray        file;
    KIO::TransferJob* job;
    int               responseCode;
    bool              isfirstdata;
};


KJavaDownloader::KJavaDownloader( int ID, const QString& url )
	:d(new KJavaDownloaderPrivate)
{
    kdDebug(6100) << "KJavaDownloader(" << ID << ") = " << url << endl;


    d->loaderID = ID;
    d->url = new KURL( url );

    d->job = KIO::get( *d->url, false, false );
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
    //kdDebug(6100) << "slotData(" << d->loaderID << ")" << endl;

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->isfirstdata) {
        QString headers = d->job->queryMetaData("HTTP-Headers");
        if (!headers.isEmpty()) {
            d->file.resize( headers.length() );
            memcpy( d->file.data(), headers.ascii(), headers.length() );
            server->sendURLData( d->loaderID, HEADERS, d->file );
            d->file.resize( 0 );
        }
        d->isfirstdata = false;
    }
    if ( qb.size() )
        server->sendURLData( d->loaderID, DATA, qb );
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
    kdDebug(6100) << "slotResult(" << d->loaderID << ")" << endl;

    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if( d->job->error())
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
        d->file.resize( 0 );
    }
    else
    {
        server->sendURLData( d->loaderID, FINISHED, d->file );
    }
    d->job = 0L; // signal KIO::Job::result deletes itself
    server->removeDataJob( d->loaderID ); // will delete this
    KJavaAppletServer::freeJavaServer();
}

void KJavaDownloader::jobCommand( int cmd )
{
    if (!d->job) return;
    switch (cmd) {
        case KJAS_STOP: {
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") stop" << endl;
            d->job->kill();
            d->job = 0L; // KIO::Job::kill deletes itself
            KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
            server->removeDataJob( d->loaderID ); // will delete this
            KJavaAppletServer::freeJavaServer();
            break;
        }
        case KJAS_HOLD:
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") hold" << endl;
            d->job->suspend();
            break;
        case KJAS_RESUME:
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") resume" << endl;
            d->job->resume();
            break;
    }
}

//-----------------------------------------------------------------------------

class KJavaUploaderPrivate
{
public:
    KJavaUploaderPrivate() {}
    ~KJavaUploaderPrivate()
    {
        delete url;
        if (job) job->kill(); // KIO::Job::kill deletes itself
    }
    int               loaderID;
    KURL*             url;
    QByteArray        file;
    KIO::TransferJob* job;
    bool              finished;
};

KJavaUploader::KJavaUploader( int ID, const QString& url )
	:d(new KJavaUploaderPrivate)
{
    kdDebug(6100) << "KJavaUploader(" << ID << ") = " << url << endl;


    d->loaderID = ID;
    d->url = new KURL( url );
    d->job = 0L;
    d->finished = false;
}

void KJavaUploader::start()
{
    kdDebug(6100) << "KJavaUploader::start(" << d->loaderID << ")" << endl;
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    // create a suspended job
    d->job = KIO::put( *d->url, -1, false, false, false );
    d->job->suspend();
    connect( d->job, SIGNAL(dataReq( KIO::Job*, QByteArray& )),
            this,   SLOT(slotDataRequest( KIO::Job*, QByteArray& )) );
    connect( d->job, SIGNAL(result(KIO::Job*)),
            this,   SLOT(slotResult(KIO::Job*)) );
    server->sendURLData( d->loaderID, CONNECTED, d->file );
    KJavaAppletServer::freeJavaServer();
}

KJavaUploader::~KJavaUploader()
{
    delete d;
}

void KJavaUploader::slotDataRequest( KIO::Job*, QByteArray& qb )
{
    // send our data and suspend
    kdDebug(6100) << "slotDataRequest(" << d->loaderID << ") finished:" << d->finished << endl;
    qb.resize( d->file.size() );
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->file.size() == 0) {
        d->job = 0L; // eof, job deletes itself
        server->removeDataJob( d->loaderID ); // will delete this
    } else {
        memcpy( qb.data(), d->file.data(), d->file.size() );
        d->file.resize( 0 );
	if (!d->finished) {
            server->sendURLData( d->loaderID, REQUESTDATA, d->file );
            d->job->suspend();
        }
    }
    KJavaAppletServer::freeJavaServer();
}

void KJavaUploader::data( const QByteArray& qb )
{
    kdDebug(6100) << "KJavaUploader::data(" << d->loaderID << ")" << endl;
    d->file.resize( qb.size() );
    memcpy( d->file.data(), qb.data(), qb.size() );
    d->job->resume();
}

void KJavaUploader::slotResult( KIO::Job* )
{
    kdDebug(6100) << "slotResult(" << d->loaderID << ") job:" << d->job << endl;

    if (!d->job)
        return;
    KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
    if (d->job->error())
    {
        int code = d->job->error();
        QString codestr = QString::number(code);
        d->file.resize(codestr.length());
        memcpy( d->file.data(), codestr.ascii(), codestr.length() );
        kdDebug(6100) << "slave had an error " << code <<  ": " << d->job->errorString() << endl;

        server->sendURLData( d->loaderID, ERRORCODE, d->file );
        d->file.resize( 0 );
    }
    else // shouldn't come here
        kdError(6100) << "slotResult(" << d->loaderID << ") job:" << d->job << endl;
    d->job = 0L; // signal KIO::Job::result deletes itself
    server->removeDataJob( d->loaderID ); // will delete this
    KJavaAppletServer::freeJavaServer();
}

void KJavaUploader::jobCommand( int cmd )
{
    if (!d->job) return;
    switch (cmd) {
        case KJAS_STOP: {
            kdDebug(6100) << "jobCommand(" << d->loaderID << ") stop" << endl;
	    d->finished = true;
            if (d->job->isSuspended())
                d->job->resume();
            break;
        }
    }
}

#include "kjavadownloader.moc"
