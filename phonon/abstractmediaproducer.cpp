/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#include "abstractmediaproducer.h"
#include "abstractmediaproducer_p.h"
#include "factory.h"

#include "videopath.h"
#include "audiopath.h"

#include "mediaproducerinterface.h"
#include "addoninterface.h"
#include "trackinterface.h"
#include "chapterinterface.h"

#include <QTimer>

#include <kdebug.h>
#include <QStringList>

#include "frontendinterface_p.h"
#include "navigationinterface.h"
#include "angleinterface.h"

#define PHONON_CLASSNAME AbstractMediaProducer
#define PHONON_INTERFACENAME MediaProducerInterface

namespace Phonon
{
PHONON_ABSTRACTBASE_IMPL

AbstractMediaProducer::~AbstractMediaProducer()
{
    K_D(AbstractMediaProducer);
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
    foreach (VideoPath *vp, d->videoPaths)
        d->removeDestructionHandler(vp, d);
    foreach (AudioPath *ap, d->audioPaths)
        d->removeDestructionHandler(ap, d);
}

bool AbstractMediaProducer::addVideoPath(VideoPath *videoPath)
{
    K_D(AbstractMediaProducer);
    if (d->videoPaths.contains(videoPath))
        return false;

    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        if (qobject_cast<MediaProducerInterface *>(d->m_backendObject)->addVideoPath(videoPath->k_ptr->backendObject()))
        {
            d->addDestructionHandler(videoPath, d);
            d->videoPaths.append(videoPath);
            return true;
        }
    }
    return false;
}

bool AbstractMediaProducer::addAudioPath(AudioPath *audioPath)
{
    K_D(AbstractMediaProducer);
    if (d->audioPaths.contains(audioPath))
        return false;

    if (k_ptr->backendObject() && audioPath->k_ptr->backendObject()) {
        if (qobject_cast<MediaProducerInterface *>(d->m_backendObject)->addAudioPath(audioPath->k_ptr->backendObject()))
        {
            d->addDestructionHandler(audioPath, d);
            d->audioPaths.append(audioPath);
            return true;
        }
    }
    return false;
}

Phonon::State AbstractMediaProducer::state() const
{
    K_D(const AbstractMediaProducer);
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
PHONON_INTERFACE_GETTER1(QString, selectedAudioStream,    d->selectedAudioStream[   audioPath], AudioPath *, audioPath)
PHONON_INTERFACE_GETTER1(QString, selectedVideoStream,    d->selectedVideoStream[   videoPath], VideoPath *, videoPath)
PHONON_INTERFACE_GETTER1(QString, selectedSubtitleStream, d->selectedSubtitleStream[videoPath], VideoPath *, videoPath)
PHONON_INTERFACE_GETTER(QStringList, availableAudioStreams, QStringList())
PHONON_INTERFACE_GETTER(QStringList, availableVideoStreams, QStringList())
PHONON_INTERFACE_GETTER(QStringList, availableSubtitleStreams, QStringList())

void AbstractMediaProducer::selectAudioStream(const QString &streamName, AudioPath *audioPath)
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject() && audioPath->k_ptr->backendObject()) {
        INTERFACE_CALL(selectAudioStream(streamName, audioPath->k_ptr->backendObject()));
    } else {
        d->selectedAudioStream[audioPath] = streamName;
    }
}

void AbstractMediaProducer::selectVideoStream(const QString &streamName, VideoPath *videoPath)
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        INTERFACE_CALL(selectVideoStream(streamName, videoPath->k_ptr->backendObject()));
    } else {
        d->selectedVideoStream[videoPath] = streamName;
    }
}

void AbstractMediaProducer::selectSubtitleStream(const QString &streamName, VideoPath *videoPath)
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject() && videoPath->k_ptr->backendObject()) {
        INTERFACE_CALL(selectSubtitleStream(streamName, videoPath->k_ptr->backendObject()));
    } else {
        d->selectedSubtitleStream[videoPath] = streamName;
    }
}

QList<VideoPath *> AbstractMediaProducer::videoPaths() const
{
    K_D(const AbstractMediaProducer);
    return d->videoPaths;
}

QList<AudioPath *> AbstractMediaProducer::audioPaths() const
{
    K_D(const AbstractMediaProducer);
    return d->audioPaths;
}

void AbstractMediaProducer::play()
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject())
        INTERFACE_CALL(play());
}

void AbstractMediaProducer::pause()
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject())
        INTERFACE_CALL(pause());
}

void AbstractMediaProducer::stop()
{
    K_D(AbstractMediaProducer);
    if (k_ptr->backendObject())
    {
        INTERFACE_CALL(stop());
    }
}

void AbstractMediaProducer::seek(qint64 time)
{
    K_D(AbstractMediaProducer);
    State s = state();
    if (k_ptr->backendObject() && (s == Phonon::PlayingState || s == Phonon::BufferingState || s == Phonon::PausedState))
        INTERFACE_CALL(seek(time));
}

QString AbstractMediaProducer::errorString() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const AbstractMediaProducer);
        if (d->errorOverride) {
            return d->errorString;
        }
        return INTERFACE_CALL(errorString());
    }
    return QString();
}

ErrorType AbstractMediaProducer::errorType() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const AbstractMediaProducer);
        if (d->errorOverride) {
            return d->errorType;
        }
        return INTERFACE_CALL(errorType());
    }
    return Phonon::NoError;
}

template<>
PHONONCORE_EXPORT
bool AbstractMediaProducer::hasInterface<TrackInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->m_backendObject) {
        return qobject_cast<AddonInterface *>(d->m_backendObject)
            ->hasInterface(AddonInterface::TrackInterface);
    }
    return false;
}

template<>
PHONONCORE_EXPORT
bool AbstractMediaProducer::hasInterface<ChapterInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->m_backendObject) {
        return qobject_cast<AddonInterface *>(d->m_backendObject)
            ->hasInterface(AddonInterface::ChapterInterface);
    }
    return false;
}

template<>
PHONONCORE_EXPORT
bool AbstractMediaProducer::hasInterface<NavigationInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->m_backendObject) {
        return qobject_cast<AddonInterface *>(d->m_backendObject)
            ->hasInterface(AddonInterface::NavigationInterface);
    }
    return false;
}

template<>
PHONONCORE_EXPORT
bool AbstractMediaProducer::hasInterface<AngleInterface>() const
{
    K_D(const AbstractMediaProducer);
    if (d->m_backendObject) {
        return qobject_cast<AddonInterface *>(d->m_backendObject)
            ->hasInterface(AddonInterface::AngleInterface);
    }
    return false;
}

QStringList AbstractMediaProducer::metaDataKeys() const
{
    K_D(const AbstractMediaProducer);
    return d->metaData.keys();
}

QStringList AbstractMediaProducer::metaDataItems(const QString &key) const
{
    K_D(const AbstractMediaProducer);
    return d->metaData.values(key);
}

} //namespace Phonon

#include "abstractmediaproducer.moc"
#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
