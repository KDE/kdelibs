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

#include "kiofallbackimpl.h"
#include "../factory.h"
#include "../phonondefs_p.h"
#include "../mediaobject.h"
#include "../mediaobject_p.h"
#include "../bytestreaminterface.h"

#include <kio/jobclasses.h>
#include <kio/filejob.h>
#include <kio/job.h>
#include <kprotocolmanager.h>
#include <klocale.h>

#define PHONON_INTERFACENAME ByteStreamInterface

namespace Phonon
{

KioFallbackImpl::KioFallbackImpl(MediaObject *parent)
    : KioFallback(parent),
    m_endOfDataSent(false),
    m_seeking(false),
    m_reading(false),
    m_open(false),
    m_seekPosition(0),
    m_kiojob(0)
{
}

KioFallbackImpl::~KioFallbackImpl()
{
    if (m_kiojob) {
        m_kiojob->kill();
        m_kiojob = 0;
    }
}

void KioFallbackImpl::stopped()
{
    // if (do pre-buffering) {
    setupKioJob();
    // }
}

qint64 KioFallbackImpl::totalTime() const
{
    const MediaObject *q = static_cast<const MediaObject *>(parent());
    const MediaObjectPrivate *d = q->k_func();
    return INTERFACE_CALL(totalTime());
}

void KioFallbackImpl::setupKioStreaming()
{
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->m_backendObject == 0);

    d->m_backendObject = Factory::createByteStream(q);
    if (!d->m_backendObject) {
        return;
    }

    connect(d->m_backendObject, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupByteStream()));
    setupKioJob();
    m_endOfDataSent = false;

    // reinitialize the KIO job when the playback has finished
    // if (do pre-buffering) {
    connect(d->m_backendObject, SIGNAL(finished()), this, SLOT(setupKioJob()));
    // }

    //setupIface for ByteStream
    connect(d->m_backendObject, SIGNAL(needData()), this, SLOT(bytestreamNeedData()));
    connect(d->m_backendObject, SIGNAL(enoughData()), this, SLOT(bytestreamEnoughData()));
    connect(d->m_backendObject, SIGNAL(seekStream(qint64)), this, SLOT(bytestreamSeekStream(qint64)));

    //TODO handle redirection ...

    d->setupBackendObject();
}

void KioFallbackImpl::setupKioJob()
{
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->m_backendObject);
    Q_ASSERT(qobject_cast<ByteStreamInterface *>(d->m_backendObject));

    if (m_kiojob) {
        m_kiojob->kill();
    }

    if (KProtocolManager::supportsOpening(d->url)) {
        m_kiojob = KIO::open(d->url, QIODevice::ReadOnly);
        if (!m_kiojob) {
            return;
        }
        m_open = false;

        BACKEND_CALL1("setStreamSeekable", bool, true);
        connect(m_kiojob, SIGNAL(open(KIO::Job *)), this, SLOT(bytestreamFileJobOpen(KIO::Job *)));
        connect(m_kiojob, SIGNAL(position(KIO::Job *, KIO::filesize_t)),
                this, SLOT(bytestreamSeekDone(KIO::Job *, KIO::filesize_t)));
    } else {
        m_kiojob = KIO::get(d->url, false, false);
        if (!m_kiojob) {
            return;
        }

        BACKEND_CALL1("setStreamSeekable", bool, false);
        connect(m_kiojob, SIGNAL(totalSize(KJob *, qulonglong)),
                this, SLOT(bytestreamTotalSize(KJob *,qulonglong)));
    }

    m_kiojob->addMetaData("UserAgent", QLatin1String("KDE Phonon"));
    connect(m_kiojob, SIGNAL(data(KIO::Job *,const QByteArray &)),
            this, SLOT(bytestreamData(KIO::Job *,const QByteArray &)));
    connect(m_kiojob, SIGNAL(result(KJob *)), this, SLOT(bytestreamResult(KJob *)));
}

void KioFallbackImpl::bytestreamNeedData()
{
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_kiojob);
    if (filejob) {
        // KIO::FileJob
        // while m_seeking the backend won't get any data
        if (m_seeking || !m_open) {
            m_reading = true;
        } else if (!m_reading) {
            m_reading = true;
            filejob->read(32768);
        }
    } else {
        // KIO::TransferJob
        m_kiojob->resume();
    }
}

void KioFallbackImpl::bytestreamEnoughData()
{
    kDebug(600) << k_funcinfo << endl;
    // Don't suspend when using a FileJob. The FileJob is controlled by calls to
    // FileJob::read()
    if (m_kiojob && !qobject_cast<KIO::FileJob *>(m_kiojob) && !m_kiojob->isSuspended()) {
        m_kiojob->suspend();
    } else {
        m_reading = false;
    }
}

void KioFallbackImpl::bytestreamData(KIO::Job *, const QByteArray &data)
{
    if (m_seeking) {
        // seek doesn't block, so don't send data to the backend until it signals us
        // that the seek is done
        kDebug(600) << k_funcinfo << "m_seeking: do nothing" << endl;
        return;
    }

    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    Q_ASSERT(d->m_backendObject);

    if (data.isEmpty()) {
        m_reading = false;
        if (!m_endOfDataSent) {
            kDebug(600) << k_funcinfo << "empty data: stopping the stream" << endl;
            m_endOfDataSent = true;
            INTERFACE_CALL(endOfData());
        }
        return;
    }

    //kDebug(600) << k_funcinfo << "calling writeData on the Backend ByteStream " << data.size() << endl;
    INTERFACE_CALL(writeData(data));
    if (m_reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_kiojob);
        Q_ASSERT(filejob);
        filejob->read(32768);
    }
}

void KioFallbackImpl::bytestreamResult(KJob *job)
{
    if (job->error()) {
        QString kioErrorString = job->errorString();
        kDebug(600) << "KIO Job error: " << kioErrorString << endl;
        disconnect(m_kiojob, SIGNAL(data(KIO::Job *,const QByteArray &)),
                this, SLOT(bytestreamData(KIO::Job *,const QByteArray &)));
        disconnect(m_kiojob, SIGNAL(result(KJob *)),
                this, SLOT(bytestreamResult(KJob *)));
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_kiojob);
        if (filejob) {
            disconnect(m_kiojob, SIGNAL(open(KIO::Job *)),
                    this, SLOT(bytestreamFileJobOpen(KIO::Job *)));
            disconnect(m_kiojob, SIGNAL(position(KIO::Job *, KIO::filesize_t)),
                    this, SLOT(bytestreamSeekDone(KIO::Job *, KIO::filesize_t)));
        } else {
            disconnect(m_kiojob, SIGNAL(totalSize(KJob *, qulonglong)),
                    this, SLOT(bytestreamTotalSize(KJob *,qulonglong)));
        }
        // go to ErrorState - NormalError
        MediaObject *q = static_cast<MediaObject *>(parent());
        MediaObjectPrivate *d = q->k_func();
        State lastState = q->state();
        d->errorOverride = true;
        d->errorType = NormalError;
        d->errorString =
            i18n("<html>%1 reported the error:<blockquote>%2</blockquote>"
                    "<hr/>Falling back to KIO to read the media data failed with the error:"
                    "<blockquote>%3</blockquote></html>",
                    Factory::backendName(),
                    d->errorString, kioErrorString);
        d->state = ErrorState;
        d->_k_stateChanged(ErrorState, lastState);
    }
    m_kiojob = 0;
    m_endOfDataSent = true;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    INTERFACE_CALL(endOfData());
    m_reading = false;
}

void KioFallbackImpl::bytestreamTotalSize(KJob *, qulonglong size)
{
    kDebug(600) << k_funcinfo << size << endl;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    BACKEND_CALL1("setStreamSize", qint64, size);
}

void KioFallbackImpl::cleanupByteStream()
{
    if (m_kiojob) {
        m_kiojob->kill();
        m_kiojob = 0;
        m_reading = false;
    }
}

void KioFallbackImpl::bytestreamSeekStream(qint64 position)
{
    kDebug(600) << k_funcinfo << position << " = " << qulonglong(position) << endl;
    m_seeking = true;
    if (m_open) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_kiojob);
        filejob->seek(position);
    } else {
        m_seekPosition = position;
    }
}

void KioFallbackImpl::bytestreamFileJobOpen(KIO::Job *)
{
    m_open = true;
    m_endOfDataSent = false;
    KIO::FileJob *filejob = static_cast<KIO::FileJob *>(m_kiojob);
    kDebug(600) << k_funcinfo << filejob->size() << endl;
    MediaObject *q = static_cast<MediaObject *>(parent());
    MediaObjectPrivate *d = q->k_func();
    BACKEND_CALL1("setStreamSize", qint64, filejob->size());

    if (m_seeking) {
        filejob->seek(m_seekPosition);
    } else if (m_reading) {
        filejob->read(32768);
    }
}

void KioFallbackImpl::bytestreamSeekDone(KIO::Job *, KIO::filesize_t offset)
{
    kDebug(600) << k_funcinfo << offset << endl;
    m_seeking = false;
    m_endOfDataSent = false;
    if (m_reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob *>(m_kiojob);
        Q_ASSERT(filejob);
        filejob->read(32768);
    }
}

} // namespace Phonon
#include "kiofallbackimpl.moc"
// vim: sw=4 sts=4 et tw=100
