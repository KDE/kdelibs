/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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
#include "mediaobject.h"
#include "mediaobject_p.h"
#include "factory.h"
#include "mediaobjectinterface.h"
#include "kiofallback.h"
#include "guiinterface.h"
#include "bytestreaminterface.h"

#include <kdebug.h>
#include <QCoreApplication>

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

namespace Phonon
{
PHONON_HEIR_IMPL(AbstractMediaProducer)

MediaObject::MediaObject(Phonon::MediaObjectPrivate &dd, QObject *parent)
    : AbstractMediaProducer(dd, parent)
{
}

KUrl MediaObject::url() const
{
    K_D(const MediaObject);
    MediaObjectInterface *iface = qobject_cast<MediaObjectInterface *>(d->m_backendObject);
    if (iface) {
        return iface->url();
    }
    return d->url;
}

PHONON_GETTER(qint32, aboutToFinishTime, d->aboutToFinishTime)

qint64 MediaObject::totalTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject)
        return -1;
    if (d->kiofallback) {
        return d->kiofallback->totalTime();
    }
    return INTERFACE_CALL(totalTime());
}

qint64 MediaObject::remainingTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject)
        return -1;
    qint64 ret;
    if (!BACKEND_GET(qint64, ret, "remainingTime"))
        ret = totalTime() - currentTime();
    if (ret < 0)
        return -1;
    return ret;
}

void MediaObject::setUrl(const KUrl &url)
{
    K_D(MediaObject);
    d->media = None;
    d->url = url;
    if (k_ptr->backendObject())
    {
        stop(); // first call stop as that often is the expected state
                // for setting a new URL
//X         if (url.scheme() == "http") {
//X             if (!d->kiofallback) {
//X                 d->kiofallback = GuiInterface::instance()->newKioFallback(this);
//X                 if (d->kiofallback) {
//X                     // k_ptr->backendObject() is a MediaObject, we want a ByteStream
//X                     d->deleteBackendObject();
//X                     if (d->state == PlayingState || d->state == PausedState || d->state == BufferingState) {
//X                         emit stateChanged(StoppedState, d->state);
//X                         d->state = StoppedState;
//X                     }
//X                     // catch URLs that we rather want KIO to handle
//X                     d->kiofallback->setupKioStreaming();
//X                     return;
//X                 }
//X             }
//X         }
        MediaObjectInterface *iface = qobject_cast<MediaObjectInterface *>(d->m_backendObject);
        if (iface)
        {
            iface->setUrl(url);
            // if the state changes to ErrorState it will be handled in
            // _k_stateChanged and a ByteStream will be used.
            return;
        }

        // we're using a ByteStream
        // first try to do with a proper MediaObject. By deleting the ByteStream
        // now we might end up with d->state == PlayingState because the stop
        // call above is async and so we might be deleting a playing ByteStream.
        // To fix this we change state to StoppedState manually.
        d->deleteBackendObject();
        if (d->state == PlayingState || d->state == PausedState || d->state == BufferingState) {
            emit stateChanged(StoppedState, d->state);
            d->state = StoppedState;
        }
        d->createBackendObject();
        // createBackendObject will set up a ByteStream (in setupIface) if needed
    }
}

MediaObject::Media MediaObject::media() const
{
    K_D(const MediaObject);
    return d->media;
}

void MediaObject::openMedia(Media media, const QString &mediaDevice)
{
    K_D(MediaObject);
    d->url.clear();
    d->media = media;
    d->mediaDevice = mediaDevice;
    if (k_ptr->backendObject()) {
        stop();
        if (!qobject_cast<MediaObjectInterface *>(d->m_backendObject)) {
            // m_backendObject is a ByteStream
            d->deleteBackendObject();
            if (d->state == PlayingState || d->state == PausedState || d->state == BufferingState) {
                emit stateChanged(StoppedState, d->state);
                d->state = StoppedState;
            }
            d->createBackendObject(); // calls openMedia in setupIface
        } else {
            INTERFACE_CALL(openMedia(media, mediaDevice));
        }
    }
}

PHONON_SETTER(setAboutToFinishTime, aboutToFinishTime, qint32)

bool MediaObjectPrivate::aboutToDeleteBackendObject()
{
    //kDebug(600) << k_funcinfo << endl;
    pBACKEND_GET(qint32, aboutToFinishTime, "aboutToFinishTime");
    if (AbstractMediaProducerPrivate::aboutToDeleteBackendObject()) {
        if (qobject_cast<ByteStreamInterface *>(m_backendObject)) {
            // the next backend object we'll handle is a MediaObject, so delete the
            // KioFallback object
            delete kiofallback;
            kiofallback = 0;
        }
        return true;
    }
    return false;
}

void MediaObjectPrivate::_k_stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    K_Q(MediaObject);

    // backend MediaObject reached ErrorState, try a ByteStream
    if (!url.isEmpty() && newstate == Phonon::ErrorState && !kiofallback) {
        Q_Q(MediaObject);
        kiofallback = GuiInterface::instance()->newKioFallback(q);
        if (!kiofallback) {
            kDebug(600) << "backend MediaObject reached ErrorState, no KIO fallback available" << endl;
            emit q->stateChanged(newstate, oldstate);
            return;
        }
        errorString = q->errorString();
        kDebug(600) << "backend MediaObject reached ErrorState, trying ByteStream now" << endl;
        deleteBackendObject();
        ignoreLoadingToBufferingStateChange = false;
        switch (oldstate) {
        case Phonon::BufferingState:
            // play() has already been called, we need to make sure it is called
            // on the backend ByteStream now, too
            //emit q->stateChanged(Phonon::LoadingState, oldstate);
            ignoreLoadingToBufferingStateChange = true;
            break;
        case Phonon::LoadingState:
            // no extras
            break;
        default:
            kError(600) << "backend MediaObject reached ErrorState after " << oldstate
                << ". It seems a KIO-ByteStream won't help here, trying anyway." << endl;
            emit q->stateChanged(Phonon::LoadingState, oldstate);
            break;
        }
        kiofallback->setupKioStreaming();
        if (oldstate == Phonon::BufferingState) {
            q->play();
        }
        return;
    } else if (ignoreLoadingToBufferingStateChange &&
            kiofallback &&
            oldstate == Phonon::LoadingState) {
        if (newstate != Phonon::BufferingState) {
            emit q->stateChanged(newstate, Phonon::BufferingState);
        }
        return;
    } else if (newstate == StoppedState && kiofallback) {
        switch (oldstate) {
        case PlayingState:
        case PausedState:
        case BufferingState:
            kiofallback->stopped();
            break;
        default:
            // nothing
            break;
        }
    }

    emit q->stateChanged(newstate, oldstate);
}

// setupIface is also called for ByteStream
void MediaObjectPrivate::setupBackendObject()
{
    Q_Q(MediaObject);
    Q_ASSERT(m_backendObject);
    //kDebug(600) << k_funcinfo << endl;
    AbstractMediaProducerPrivate::setupBackendObject();

    // disconnect what AbstractMediaProducer::setupIface connected to filter out
    // the LoadingState -> ErrorState change on setUrl
    QObject::disconnect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), q, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
    QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), q, SLOT(_k_stateChanged(Phonon::State, Phonon::State)));

    QObject::connect(m_backendObject, SIGNAL(finished()), q, SIGNAL(finished()));
    QObject::connect(m_backendObject, SIGNAL(aboutToFinish(qint32)), q, SIGNAL(aboutToFinish(qint32)));
    QObject::connect(m_backendObject, SIGNAL(length(qint64)), q, SIGNAL(length(qint64)));

    // set up attributes
    pBACKEND_CALL1("setAboutToFinishTime", qint32, aboutToFinishTime);
    if(!url.isEmpty() && !kiofallback) {
        pINTERFACE_CALL(setUrl(url));
        // if the state changes to ErrorState it will be handled in
        // _k_stateChanged and a ByteStream will be used.
    } else if (media != MediaObject::None) {
        pINTERFACE_CALL(openMedia(media, mediaDevice));
    }
}

} //namespace Phonon

#include "mediaobject.moc"

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
// vim: sw=4 ts=4 tw=80
