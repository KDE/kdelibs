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

#define PHONON_INTERFACENAME ByteStreamInterface

namespace Phonon
{

KioFallback::KioFallback(MediaObject *parent)
    : QObject(parent),
    endOfDataSent(false),
    seeking(false),
    reading(false),
    m_open(false),
    m_seekPosition(0),
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
    setupKioJob();
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

    d->backendObject = Factory::createByteStream(q);
    if (!d->backendObject) {
        return;
    }

    connect(d->backendObject, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupByteStream()));
    setupKioJob();
    endOfDataSent = false;

    // reinitialize the KIO job when the playback has finished
    // if (do pre-buffering) {
    connect(d->backendObject, SIGNAL(finished()), this, SLOT(setupKioJob()));
    // }

    //setupIface for ByteStream
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
    Q_ASSERT(qobject_cast<ByteStreamInterface *>(d->backendObject));

    if (kiojob) {
        kiojob->kill();
    }

    if (KProtocolManager::supportsOpening(d->url)) {
        kiojob = KIO::open(d->url, 1 /*ReadOnly*/);
        if (!kiojob) {
            return;
        }
        m_open = false;

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
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
    if (filejob) {
        // KIO::FileJob
        // while seeking the backend won't get any data
        if (seeking || !m_open) {
            reading = true;
        } else if (!reading) {
            reading = true;
            filejob->read(32768);
        }
    } else {
        // KIO::TransferJob
        kiojob->resume();
    }
}

void KioFallback::bytestreamEnoughData()
{
    kDebug(600) << k_funcinfo << endl;
    // Don't suspend when using a FileJob. The FileJob is controlled by calls to
    // FileJob::read()
    if (kiojob && !qobject_cast<KIO::FileJob*>(kiojob) && !kiojob->isSuspended()) {
        kiojob->suspend();
    } else {
        reading = false;
    }
}

void KioFallback::bytestreamData(KIO::Job *, const QByteArray &data)
{
    if (seeking) {
        // seek doesn't block, so don't send data to the backend until it signals us
        // that the seek is done
        kDebug(600) << k_funcinfo << "seeking: do nothing" << endl;
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
        Q_ASSERT(filejob);
        filejob->read(32768);
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
        reading = false;
    }
}

void KioFallback::bytestreamSeekStream(qint64 position)
{
    kDebug(600) << k_funcinfo << position << " = " << qulonglong(position) << endl;
    seeking = true;
    if (m_open) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
        filejob->seek(position);
    } else {
        m_seekPosition = position;
    }
}

void KioFallback::bytestreamFileJobOpen(KIO::Job *)
{
    m_open = true;
    endOfDataSent = false;
    KIO::FileJob *filejob = static_cast<KIO::FileJob *>(kiojob);
    kDebug(600) << k_funcinfo << filejob->size() << endl;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    BACKEND_CALL1("setStreamSize", qint64, filejob->size());

    if (seeking) {
        filejob->seek(m_seekPosition);
    } else if (reading) {
        filejob->read(32768);
    }
}

void KioFallback::bytestreamSeekDone(KIO::Job *, KIO::filesize_t offset)
{
    kDebug(600) << k_funcinfo << offset << endl;
    seeking = false;
    endOfDataSent = false;
    if (reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(kiojob);
        Q_ASSERT(filejob);
        filejob->read(32768);
    }
}

} // namespace Phonon
#include "kiofallback.moc"
// vim: sw=4 sts=4 et tw=100
