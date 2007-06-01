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

#include "videopath.h"
#include "videopath_p.h"
#include "audiopath.h"
#include "audiopath_p.h"

#include "factory.h"
#include "mediaobjectinterface.h"
#include "frontendinterface_p.h"
#include "mediasource.h"
#include "abstractmediastream.h"
#include "abstractmediastream_p.h"

#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>

#include "phononnamespace_p.h"

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

namespace Phonon
{
PHONON_OBJECT_IMPL

MediaObject::~MediaObject()
{
    K_D(MediaObject);
    if (d->m_backendObject) {
        switch (state()) {
        case PlayingState:
        case BufferingState:
        case PausedState:
            stop();
            break;
        case ErrorState:
        case StoppedState:
        case LoadingState:
            break;
        }
    }
    foreach (VideoPath *vp, d->videoPaths) {
        vp->k_ptr->removeDestructionHandler(d);
    }
    foreach (AudioPath *ap, d->audioPaths) {
        ap->k_ptr->removeDestructionHandler(d);
    }
    delete k_ptr;
}

bool MediaObject::addVideoPath(VideoPath *videoPath)
{
    K_D(MediaObject);
    if (d->videoPaths.contains(videoPath)) {
        return false;
    }

    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        if (INTERFACE_CALL(addVideoPath(videoPath->k_ptr->backendObject()))) {
            videoPath->k_ptr->addDestructionHandler(d);
            d->videoPaths.append(videoPath);
            return true;
        }
    }
    return false;
}

bool MediaObject::addAudioPath(AudioPath *audioPath)
{
    K_D(MediaObject);
    if (d->audioPaths.contains(audioPath)) {
        return false;
    }

    if (k_ptr->backendObject() && audioPath->k_ptr->backendObject()) {
        if (INTERFACE_CALL(addAudioPath(audioPath->k_ptr->backendObject()))) {
            audioPath->k_ptr->addDestructionHandler(d);
            d->audioPaths.append(audioPath);
            return true;
        }
    }
    return false;
}

bool MediaObject::removeVideoPath(VideoPath *videoPath)
{
    K_D(MediaObject);
    if (!d->videoPaths.contains(videoPath) || !d->backendObject() || !videoPath->k_ptr->backendObject()) {
        return false;
    }

    INTERFACE_CALL(removeVideoPath(videoPath->k_ptr->backendObject()));
    d->videoPaths.removeAll(videoPath);
    return true;
}

bool MediaObject::removeAudioPath(AudioPath *audioPath)
{
    K_D(MediaObject);
    if (!d->audioPaths.contains(audioPath) || !d->backendObject() || !audioPath->k_ptr->backendObject()) {
        return false;
    }

    INTERFACE_CALL(removeAudioPath(audioPath->k_ptr->backendObject()));
    d->audioPaths.removeAll(audioPath);
    return true;
}

Phonon::State MediaObject::state() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject || d->errorOverride) {
        return d->state;
    }
    return INTERFACE_CALL(state());
}

PHONON_INTERFACE_SETTER(setTickInterval, tickInterval, qint32)
PHONON_INTERFACE_GETTER(qint32, tickInterval, d->tickInterval)
PHONON_INTERFACE_GETTER(bool, hasVideo, false)
PHONON_INTERFACE_GETTER(bool, isSeekable, false)
PHONON_INTERFACE_GETTER(qint64, currentTime, d->currentTime)
PHONON_INTERFACE_GETTER1(QString, currentAudioStream,    d->currentAudioStream[   audioPath], AudioPath *, audioPath)
PHONON_INTERFACE_GETTER1(QString, currentVideoStream,    d->currentVideoStream[   videoPath], VideoPath *, videoPath)
PHONON_INTERFACE_GETTER1(QString, currentSubtitleStream, d->currentSubtitleStream[videoPath], VideoPath *, videoPath)
PHONON_INTERFACE_GETTER(QStringList, availableAudioStreams, QStringList())
PHONON_INTERFACE_GETTER(QStringList, availableVideoStreams, QStringList())
PHONON_INTERFACE_GETTER(QStringList, availableSubtitleStreams, QStringList())

void MediaObject::setCurrentAudioStream(const QString &streamName, AudioPath *audioPath)
{
    K_D(MediaObject);
    if (k_ptr->backendObject() && audioPath->k_ptr->backendObject()) {
        INTERFACE_CALL(setCurrentAudioStream(streamName, audioPath->k_ptr->backendObject()));
    } else {
        d->currentAudioStream[audioPath] = streamName;
    }
}

void MediaObject::setCurrentVideoStream(const QString &streamName, VideoPath *videoPath)
{
    K_D(MediaObject);
    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        INTERFACE_CALL(setCurrentVideoStream(streamName, videoPath->k_ptr->backendObject()));
    } else {
        d->currentVideoStream[videoPath] = streamName;
    }
}

void MediaObject::setCurrentSubtitleStream(const QString &streamName, VideoPath *videoPath)
{
    K_D(MediaObject);
    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        INTERFACE_CALL(setCurrentSubtitleStream(streamName, videoPath->k_ptr->backendObject()));
    } else {
        d->currentSubtitleStream[videoPath] = streamName;
    }
}

QList<VideoPath *> MediaObject::videoPaths() const
{
    K_D(const MediaObject);
    return d->videoPaths;
}

QList<AudioPath *> MediaObject::audioPaths() const
{
    K_D(const MediaObject);
    return d->audioPaths;
}

void MediaObject::play()
{
    K_D(MediaObject);
    if (k_ptr->backendObject()) {
        INTERFACE_CALL(play());
    }
}

void MediaObject::pause()
{
    K_D(MediaObject);
    if (k_ptr->backendObject()) {
        INTERFACE_CALL(pause());
    }
}

void MediaObject::stop()
{
    K_D(MediaObject);
    if (k_ptr->backendObject()) {
        INTERFACE_CALL(stop());
    }
}

void MediaObject::seek(qint64 time)
{
    K_D(MediaObject);
    if (k_ptr->backendObject()) {
        INTERFACE_CALL(seek(time));
    }
}

QString MediaObject::errorString() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const MediaObject);
        if (d->errorOverride) {
            return d->errorString;
        }
        return INTERFACE_CALL(errorString());
    }
    return QString();
}

ErrorType MediaObject::errorType() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const MediaObject);
        if (d->errorOverride) {
            return d->errorType;
        }
        return INTERFACE_CALL(errorType());
    }
    return Phonon::NoError;
}

QStringList MediaObject::metaData(Phonon::MetaData f) const
{
    switch (f) {
    case ArtistMetaData:
        return metaData(QLatin1String("ARTIST"     ));
    case AlbumMetaData:
        return metaData(QLatin1String("ALBUM"      ));
    case TitleMetaData:
        return metaData(QLatin1String("TITLE"      ));
    case DateMetaData:
        return metaData(QLatin1String("DATE"       ));
    case GenreMetaData:
        return metaData(QLatin1String("GENRE"      ));
    case TracknumberMetaData:
        return metaData(QLatin1String("TRACKNUMBER"));
    case DescriptionMetaData:
        return metaData(QLatin1String("DESCRIPTION"));
    }
    return QStringList();
}

QStringList MediaObject::metaData(const QString &key) const
{
    K_D(const MediaObject);
    return d->metaData.values(key);
}

QMultiMap<QString, QString> MediaObject::metaData() const
{
    K_D(const MediaObject);
    return d->metaData;
}

PHONON_GETTER(qint32, prefinishMark, d->prefinishMark)
PHONON_SETTER(setPrefinishMark, prefinishMark, qint32)

PHONON_GETTER(qint32, transitionTime, d->transitionTime)
PHONON_SETTER(setTransitionTime, transitionTime, qint32)

qint64 MediaObject::totalTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject) {
        return -1;
    }
    return INTERFACE_CALL(totalTime());
}

qint64 MediaObject::remainingTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject) {
        return -1;
    }
    qint64 ret;
    if (!BACKEND_GET(qint64, ret, "remainingTime")) {
        ret = totalTime() - currentTime();
    }
    if (ret < 0) {
        return -1;
    }
    return ret;
}

MediaSource MediaObject::currentSource() const
{
    K_D(const MediaObject);
    return d->mediaSource;
}

void MediaObject::setCurrentSource(const MediaSource &newSource)
{
    K_D(MediaObject);
    MediaSource::Type oldSourceType = d->mediaSource.type();
    d->mediaSource = newSource;
    if (k_ptr->backendObject()) {
        stop(); // first call stop as that often is the expected state
                // for setting a new URL

//X         if (url.scheme() == "http") {
//X             if (!d->kiofallback) {
//X                 d->kiofallback = Factory::createKioMediaStream(url, this);
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

        if (d->mediaSource.type() == MediaSource::Stream) {
            Q_ASSERT(d->mediaSource.stream());
            d->mediaSource.stream()->d_func()->setMediaObjectPrivate(d);
        }
        if (d->mediaSource.type() == MediaSource::Url && oldSourceType != MediaSource::Url) {
            disconnect(d->m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
            connect(d->m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
        } else if (d->mediaSource.type() != MediaSource::Url && oldSourceType == MediaSource::Url) {
            disconnect(d->m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
            connect(d->m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
        }
        INTERFACE_CALL(setSource(d->mediaSource));
    }

//X         MediaObjectInterface *iface = qobject_cast<MediaObjectInterface *>(d->m_backendObject);
//X         if (iface) {
//X             iface->setUrl(url);
//X             // if the state changes to ErrorState it will be handled in
//X             // _k_stateChanged and a ByteStream will be used.
//X             return;
//X         }
//X 
//X         // we're using a ByteStream
//X         // first try to do with a proper MediaObject. By deleting the ByteStream
//X         // now we might end up with d->state == PlayingState because the stop
//X         // call above is async and so we might be deleting a playing ByteStream.
//X         // To fix this we change state to StoppedState manually.
//X         d->deleteBackendObject();
//X         if (d->state == PlayingState || d->state == PausedState || d->state == BufferingState) {
//X             emit stateChanged(StoppedState, d->state);
//X             d->state = StoppedState;
//X         }
//X         d->createBackendObject();
//X         // createBackendObject will set up a ByteStream (in setupIface) if needed
//X     }
}

QList<MediaSource> MediaObject::queue() const
{
    K_D(const MediaObject);
    return d->sourceQueue;
}

void MediaObject::setQueue(const QList<MediaSource> &sources)
{
    K_D(MediaObject);
    d->sourceQueue.clear();
    d->sourceQueue << sources;
}

void MediaObject::setQueue(const QList<QUrl> &urls)
{
    K_D(MediaObject);
    d->sourceQueue.clear();
    foreach (const QUrl &url, urls) {
        d->sourceQueue << url;
    }
}

void MediaObject::enqueue(const MediaSource &source)
{
    K_D(MediaObject);
    d->sourceQueue << source;
}

void MediaObject::enqueue(const QList<MediaSource> &sources)
{
    K_D(MediaObject);
    d->sourceQueue << sources;
}

void MediaObject::enqueue(const QList<QUrl> &urls)
{
    K_D(MediaObject);
    foreach (const QUrl &url, urls) {
        d->sourceQueue << url;
    }
}

void MediaObject::clearQueue()
{
    K_D(MediaObject);
    d->sourceQueue.clear();
}

bool MediaObjectPrivate::aboutToDeleteBackendObject()
{
    //pDebug() << Q_FUNC_INFO;
    pBACKEND_GET(qint32, prefinishMark, "prefinishMark");
    pBACKEND_GET(qint32, transitionTime, "transitionTime");
    //pDebug() << Q_FUNC_INFO;
    if (m_backendObject) {
        state = pINTERFACE_CALL(state());
        currentTime = pINTERFACE_CALL(currentTime());
        tickInterval = pINTERFACE_CALL(tickInterval());
    }
    return true;
}

void MediaObjectPrivate::streamError(Phonon::ErrorType type, const QString &text)
{
    Q_Q(MediaObject);
    State lastState = q->state();
    errorOverride = true;
    errorType = type;
    errorString = text;
    state = ErrorState;
    emit q->stateChanged(ErrorState, lastState);
}

void MediaObjectPrivate::_k_stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_Q(MediaObject);
    Q_ASSERT(mediaSource.type() == MediaSource::Url);

    // backend MediaObject reached ErrorState, try a KioMediaSource
    if (newstate == Phonon::ErrorState && !kiofallback) {
        kiofallback = Factory::createKioMediaStream(mediaSource.url(), q);
        if (!kiofallback) {
            pDebug() << "backend MediaObject reached ErrorState, no KIO fallback available";
            emit q->stateChanged(newstate, oldstate);
            return;
        }
        pDebug() << "backend MediaObject reached ErrorState, trying ByteStream now";
        ignoreLoadingToBufferingStateChange = false;
        switch (oldstate) {
        case Phonon::BufferingState:
            // play() has already been called, we need to make sure it is called
            // on the backend with the KioMediaStream MediaSource now, too
            ignoreLoadingToBufferingStateChange = true;
            break;
        case Phonon::LoadingState:
            // no extras
            break;
        default:
            pError() << "backend MediaObject reached ErrorState after " << oldstate
                << ". It seems a KioMediaStream won't help here, trying anyway." << endl;
            emit q->stateChanged(Phonon::LoadingState, oldstate);
            break;
        }
        kiofallback->d_func()->setMediaObjectPrivate(this);
        pINTERFACE_CALL(setSource(MediaSource(kiofallback)));
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
//X     } else if (newstate == StoppedState && kiofallback) {
//X         switch (oldstate) {
//X         case PlayingState:
//X         case PausedState:
//X         case BufferingState:
//X             kiofallback->stopped();
//X             break;
//X         default:
//X             // nothing
//X             break;
//X         }
    }

    emit q->stateChanged(newstate, oldstate);
}

void MediaObjectPrivate::_k_aboutToFinish()
{
    Q_Q(MediaObject);
    pDebug() << Q_FUNC_INFO;
    if (sourceQueue.isEmpty()) {
        emit q->aboutToFinish();
        return;
    }
    mediaSource = sourceQueue.dequeue();
    pINTERFACE_CALL(setNextSource(mediaSource));
    emit q->currentSourceChanged(mediaSource);
}

void MediaObjectPrivate::setupBackendObject()
{
    Q_Q(MediaObject);
    Q_ASSERT(m_backendObject);
    //pDebug() << Q_FUNC_INFO;

    if (mediaSource.type() == MediaSource::Url) {
        QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), q, SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
    } else {
        QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), q, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
    }
    QObject::connect(m_backendObject, SIGNAL(tick(qint64)),             q, SIGNAL(tick(qint64)));
    QObject::connect(m_backendObject, SIGNAL(seekableChanged(bool)),    q, SIGNAL(seekableChanged(bool)));
    QObject::connect(m_backendObject, SIGNAL(hasVideoChanged(bool)),    q, SIGNAL(hasVideoChanged(bool)));
    QObject::connect(m_backendObject, SIGNAL(bufferStatus(int)),        q, SIGNAL(bufferStatus(int)));
    QObject::connect(m_backendObject, SIGNAL(finished()),               q, SIGNAL(finished()));
    QObject::connect(m_backendObject, SIGNAL(aboutToFinish()),          q, SLOT(_k_aboutToFinish()));
    QObject::connect(m_backendObject, SIGNAL(prefinishMarkReached(qint32)), q, SIGNAL(prefinishMarkReached(qint32)));
    QObject::connect(m_backendObject, SIGNAL(totalTimeChanged(qint64)), q, SIGNAL(totalTimeChanged(qint64)));
    QObject::connect(m_backendObject, SIGNAL(metaDataChanged(const QMultiMap<QString, QString> &)),
            q, SLOT(_k_metaDataChanged(const QMultiMap<QString, QString> &)));

    // set up attributes
    pINTERFACE_CALL(setTickInterval(tickInterval));
    pBACKEND_CALL1("setPrefinishMark", qint32, prefinishMark);
    pBACKEND_CALL1("setTransitionTime", qint32, transitionTime);

    foreach (AudioPath *a, audioPaths) {
        if (!a->k_ptr->backendObject() || !pINTERFACE_CALL(addAudioPath(a->k_ptr->backendObject()))) {
            audioPaths.removeAll(a);
        }
    }
    foreach (VideoPath *v, videoPaths) {
        if (!v->k_ptr->backendObject() || !pINTERFACE_CALL(addVideoPath(v->k_ptr->backendObject()))) {
            videoPaths.removeAll(v);
        }
    }

    switch(state)
    {
    case LoadingState:
    case StoppedState:
    case ErrorState:
        break;
    case PlayingState:
    case BufferingState:
        QTimer::singleShot(0, q, SLOT(_k_resumePlay()));
        break;
    case PausedState:
        QTimer::singleShot(0, q, SLOT(_k_resumePause()));
        break;
    }
    const State backendState = pINTERFACE_CALL(state());
    if (state != backendState && state != ErrorState) {
        // careful: if state is ErrorState we might be switching from a
        // MediaObject to a ByteStream for KIO fallback. In that case the state
        // change to ErrorState was already suppressed.
        pDebug() << "emitting a state change because the backend object has been replaced";
        emit q->stateChanged(backendState, state);
        state = backendState;
    }

    foreach (FrontendInterfacePrivate *f, interfaceList) {
        f->_backendObjectChanged();
    }

    // set up attributes
    if (mediaSource.type() == MediaSource::Stream) {
        Q_ASSERT(mediaSource.stream());
        mediaSource.stream()->d_func()->setMediaObjectPrivate(this);
    }
    pINTERFACE_CALL(setSource(mediaSource));
}

void MediaObjectPrivate::_k_resumePlay()
{
    qobject_cast<MediaObjectInterface *>(m_backendObject)->play();
    if (currentTime > 0) {
        qobject_cast<MediaObjectInterface *>(m_backendObject)->seek(currentTime);
    }
}

void MediaObjectPrivate::_k_resumePause()
{
    qobject_cast<MediaObjectInterface *>(m_backendObject)->play();
    if (currentTime > 0) {
        qobject_cast<MediaObjectInterface *>(m_backendObject)->seek(currentTime);
    }
    qobject_cast<MediaObjectInterface *>(m_backendObject)->pause();
}

void MediaObjectPrivate::_k_metaDataChanged(const QMultiMap<QString, QString> &newMetaData)
{
    metaData = newMetaData;
    emit q_func()->metaDataChanged();
}

void MediaObjectPrivate::phononObjectDestroyed(BasePrivate *bp)
{
    // this method is called from Phonon::Base::~Base(), meaning the AudioPath
    // dtor has already been called, also virtual functions don't work anymore
    // (therefore qobject_cast can only downcast from Base)
    Q_ASSERT(bp);
    foreach (AudioPath *ap, audioPaths) {
        if (ap->k_ptr == bp) {
            if (m_backendObject && bp->backendObject()) {
                pINTERFACE_CALL(removeAudioPath(bp->backendObject()));
            }
            audioPaths.removeAll(ap);
        }
    }
    foreach (VideoPath *vp, videoPaths) {
        if (vp->k_ptr == bp) {
            if (m_backendObject && bp->backendObject()) {
                pINTERFACE_CALL(removeVideoPath(bp->backendObject()));
            }
            videoPaths.removeAll(vp);
        }
    }
}

} //namespace Phonon

#include "mediaobject.moc"

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
// vim: sw=4 tw=100 et
