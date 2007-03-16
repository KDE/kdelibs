/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kiofallback.h"
#include "factory.h"
#include "phonondefs_p.h"
#include "mediaobject.h"
#include "mediaobject_p.h"
#include "bytestreaminterface.h"

#include <kio/jobclasses.h>
#include <kio/filejob.h>
#include <kio/job.h>
#include <kprotocolmanager.h>

//#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME ByteStreamInterface

namespace Phonon
{

KioFallback::KioFallback(MediaObject *parent)
    : QObject(parent),
    endOfDataSent(false),
    seeking(false),
    reading(false),
    kiojob(0)
{
}

KioFallback::~KioFallback()
{
    if (kiojob) {
        kiojob->kill();
        kiojob = 0;
    }
}

void KioFallback::stopped()
{
    // if (do pre-buffering) {
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    ByteStreamInterface *bs = qobject_cast<ByteStreamInterface *>(d->backendObject);
    if (bs) {
        setupKioJob();
    }
    // }
}

qint64 KioFallback::totalTime() const
{
    const MediaObject *q = static_cast<const MediaObject *>(parent());
    const MediaObjectPrivate *d = q->k_func();
    return INTERFACE_CALL(totalTime());
}

void KioFallback::setupKioStreaming()
{
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->backendObject == 0);

    d->backendObject = Factory::createByteStream(this);
    if (!d->backendObject) {
        return;
    }

    connect(d->backendObject, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupByteStream()));
    setupKioJob();
    endOfDataSent = false;

    //setupIface for ByteStream
    //connect(d->backendObject, SIGNAL(finished()), q, SIGNAL(finished()));
    connect(d->backendObject, SIGNAL(finished()), this, SLOT(setupKioJob()));
    //connect(d->backendObject, SIGNAL(aboutToFinish(qint32)), q, SIGNAL(aboutToFinish(qint32)));
    //connect(d->backendObject, SIGNAL(length(qint64)), q, SIGNAL(length(qint64)));

    connect(d->backendObject, SIGNAL(needData()), this, SLOT(bytestreamNeedData()));
    connect(d->backendObject, SIGNAL(enoughData()), this, SLOT(bytestreamEnoughData()));
    connect(d->backendObject, SIGNAL(seekStream(qint64)), this, SLOT(bytestreamSeekStream(qint64)));

    //TODO handle redirection ...

    q->setupIface();
}

void KioFallback::setupKioJob()
{
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->backendObject);

    if (kiojob) {
        kiojob->kill();
    }

    if (KProtocolManager::supportsOpening(d->url)) {
        kiojob = KIO::open(d->url, 1 /*ReadOnly*/);
        if (!kiojob) {
            return;
        }

        BACKEND_CALL1("setStreamSeekable", bool, true);
        connect(kiojob, SIGNAL(open(KIO::Job*)), this, SLOT(bytestreamFileJobOpen(KIO::Job*)));
        connect(kiojob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
                this, SLOT(bytestreamSeekDone(KIO::Job*, KIO::filesize_t)));
    } else {
        kiojob = KIO::get(d->url, false, false);
        if (!kiojob) {
            return;
        }

        BACKEND_CALL1("setStreamSeekable", bool, false);
        connect(kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
                this, SLOT(bytestreamTotalSize(KJob*,qulonglong)));
    }

    kiojob->addMetaData("UserAgent", QLatin1String("KDE Phonon"));
    connect(kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
            this, SLOT(bytestreamData(KIO::Job*,const QByteArray&)));
    connect(kiojob, SIGNAL(result(KJob*)), this, SLOT(bytestreamResult(KJob*)));
}

void KioFallback::bytestreamNeedData()
{
    // while seeking the backend won't get any data
    if (seeking) {
        return;
    }

    if (kiojob && kiojob->isSuspended()) {
        kiojob->resume();
    }
    if (!reading) {
        reading = true;
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
        if (filejob) {
            filejob->read( 32768 );
        }
    }
}

void KioFallback::bytestreamEnoughData()
{
    kDebug(600) << k_funcinfo << endl;
    // Don't suspend when using a FileJob. The FileJob is controlled by calls to
    // FileJob::read()
    if (kiojob && !qobject_cast<KIO::FileJob*>(kiojob) && !kiojob->isSuspended()) {
        kiojob->suspend();
    }
    reading = false;
}

void KioFallback::bytestreamData(KIO::Job *, const QByteArray &data)
{
    if (seeking) {
        kDebug(600) << k_funcinfo << "seeking: do nothing" << endl;
        reading = false;
        return;
    }

    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->backendObject);

    if (data.isEmpty()) {
        reading = false;
        if (!endOfDataSent) {
            kDebug(600) << k_funcinfo << "empty data: stopping the stream" << endl;
            endOfDataSent = true;
            INTERFACE_CALL(endOfData());
        }
        return;
    }

    //kDebug(600) << k_funcinfo << "calling writeData on the Backend ByteStream " << data.size() << endl;
    INTERFACE_CALL(writeData(data));
    if (reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
        if (filejob) {
            filejob->read(32768);
        }
    }
}

void KioFallback::bytestreamResult(KJob *job)
{
    if (job->error()) {
        kDebug(600) << "KIO Job error: " << job->errorString() << endl;
        disconnect(kiojob, SIGNAL(data(KIO::Job*,const QByteArray&)),
                this, SLOT(bytestreamData(KIO::Job*,const QByteArray&)));
        disconnect(kiojob, SIGNAL(result(KJob*)),
                this, SLOT(bytestreamResult(KJob*)));
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
        if (filejob) {
            disconnect(kiojob, SIGNAL(open(KIO::Job*)),
                    this, SLOT(bytestreamFileJobOpen(KIO::Job*)));
            disconnect(kiojob, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
                    this, SLOT(bytestreamSeekDone(KIO::Job*, KIO::filesize_t)));
        } else {
            disconnect(kiojob, SIGNAL(totalSize(KJob*, qulonglong)),
                    this, SLOT(bytestreamTotalSize(KJob*,qulonglong)));
        }
    }
    kiojob = 0;
    endOfDataSent = true;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    INTERFACE_CALL(endOfData());
    reading = false;
}

void KioFallback::bytestreamTotalSize(KJob *, qulonglong size)
{
    kDebug(600) << k_funcinfo << size << endl;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    BACKEND_CALL1("setStreamSize", qint64, size);
}

void KioFallback::cleanupByteStream()
{
    if (kiojob) {
        kiojob->kill();
        kiojob = 0;
    }
}

void KioFallback::bytestreamSeekStream(qint64 position)
{
    kDebug(600) << k_funcinfo << position << " = " << qulonglong(position) << endl;
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
    seeking = true;
    reading = false;

    // don't suspend when seeking as that will make the application hang,
    // waiting for the FileJob::position signal
    if (kiojob->isSuspended()) {
        kiojob->resume();
    }

    filejob->seek(position);
    // seek doesn't block, so don't send data to the backend until it signals us
    // that the seek is done
}

void KioFallback::bytestreamFileJobOpen(KIO::Job *)
{
    endOfDataSent = false;
    KIO::FileJob *filejob = static_cast<KIO::FileJob *>(kiojob);
    kDebug(600) << k_funcinfo << filejob->size() << endl;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    BACKEND_CALL1("setStreamSize", qint64, filejob->size());

    // start streaming data to the Backend until it signals enoughData
    reading = true;
    filejob->read(32768);
}

void KioFallback::bytestreamSeekDone(KIO::Job *, KIO::filesize_t offset)
{
    kDebug(600) << k_funcinfo << offset << endl;
    seeking = false;
    endOfDataSent = false;
    reading = true;
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
    Q_ASSERT(filejob);
    filejob->read(32768);
}

} // namespace Phonon
#include "kiofallback.moc"
// vim: sw=4 sts=4 et tw=100
