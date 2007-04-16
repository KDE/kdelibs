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

#include "abstractmediaproducer_p.h"

#include "mediaproducerinterface.h"
#include "audiopath.h"
#include "videopath.h"
#include "frontendinterface_p.h"

#include <QTimer>

#define PHONON_CLASSNAME AbstractMediaProducer
#define PHONON_INTERFACENAME MediaProducerInterface

namespace Phonon
{
bool AbstractMediaProducerPrivate::aboutToDeleteBackendObject()
{
    //kDebug(600) << k_funcinfo << endl;
    if (m_backendObject) {
        state = pINTERFACE_CALL(state());
        currentTime = pINTERFACE_CALL(currentTime());
        tickInterval = pINTERFACE_CALL(tickInterval());
    }
    return true;
}

void AbstractMediaProducerPrivate::setupBackendObject()
{
    Q_Q(AbstractMediaProducer);
    Q_ASSERT(m_backendObject);
    //kDebug(600) << k_funcinfo << endl;

    QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), q, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
    QObject::connect(m_backendObject, SIGNAL(tick(qint64)), q, SIGNAL(tick(qint64)));
    QObject::connect(m_backendObject, SIGNAL(metaDataChanged(const QMultiMap<QString, QString> &)), q, SLOT(_k_metaDataChanged(const QMultiMap<QString, QString> &)));
    QObject::connect(m_backendObject, SIGNAL(seekableChanged(bool)), q, SIGNAL(seekableChanged(bool)));
    QObject::connect(m_backendObject, SIGNAL(hasVideoChanged(bool)), q, SIGNAL(hasVideoChanged(bool)));
    QObject::connect(m_backendObject, SIGNAL(bufferStatus(int)), q, SIGNAL(bufferStatus(int)));

    // set up attributes
    pINTERFACE_CALL(setTickInterval(tickInterval));

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
        kDebug(600) << "emitting a state change because the backend object has been replaced" << endl;
        emit q->stateChanged(backendState, state);
        state = backendState;
    }

    foreach (FrontendInterfacePrivate *f, interfaceList) {
        f->_backendObjectChanged();
    }
}

void AbstractMediaProducerPrivate::_k_resumePlay()
{
    qobject_cast<MediaProducerInterface *>(m_backendObject)->play();
    if (currentTime > 0) {
        qobject_cast<MediaProducerInterface *>(m_backendObject)->seek(currentTime);
    }
}

void AbstractMediaProducerPrivate::_k_resumePause()
{
    qobject_cast<MediaProducerInterface *>(m_backendObject)->play();
    if (currentTime > 0) {
        qobject_cast<MediaProducerInterface *>(m_backendObject)->seek(currentTime);
    }
    qobject_cast<MediaProducerInterface *>(m_backendObject)->pause();
}

void AbstractMediaProducerPrivate::_k_metaDataChanged(const QMultiMap<QString, QString> &newMetaData)
{
    metaData = newMetaData;
    emit q_func()->metaDataChanged();
}

void AbstractMediaProducerPrivate::phononObjectDestroyed(Base *o)
{
    // this method is called from Phonon::Base::~Base(), meaning the AudioPath
    // dtor has already been called, also virtual functions don't work anymore
    // (therefore qobject_cast can only downcast from Base)
    Q_ASSERT(o);
    AudioPath *audioPath = static_cast<AudioPath *>(o);
    VideoPath *videoPath = static_cast<VideoPath *>(o);
    if (audioPaths.contains(audioPath)) {
        if (m_backendObject && audioPath->k_ptr->backendObject()) {
            pINTERFACE_CALL(removeAudioPath(audioPath->k_ptr->backendObject()));
        }
        audioPaths.removeAll(audioPath);
    } else if (videoPaths.contains(videoPath)) {
        if (m_backendObject && videoPath->k_ptr->backendObject()) {
            pINTERFACE_CALL(removeVideoPath(videoPath->k_ptr->backendObject()));
        }
        videoPaths.removeAll(videoPath);
    }
}

} // namespace Phonon

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
