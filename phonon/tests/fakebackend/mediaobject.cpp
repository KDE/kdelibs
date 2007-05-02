/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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
#include <QtCore/QTimer>
#include <kdebug.h>
#include "videopath.h"
#include "audiopath.h"
#include "stream.h"
#include "../../abstractmediastream_p.h"
#include <QtCore/QVector>
#include <cmath>
#include <QtCore/QFile>
#include <QtCore/QByteRef>
#include <QtCore/QStringList>

namespace Phonon
{
namespace Fake
{
static const int SAMPLE_RATE = 44100;
static const float SAMPLE_RATE_FLOAT = 44100.0f;
static const int FRAME_RATE = 25;
static const int SAMPLES_PER_FRAME = SAMPLE_RATE / FRAME_RATE;

MediaObject::MediaObject(QObject *parent)
    : QObject(parent)
    , m_state(Phonon::LoadingState)
    , m_tickTimer(new QTimer(this))
    , m_bufferSize(SAMPLES_PER_FRAME)//512)
    , m_lastSamplesMissing(0)
    , m_position(0.0f)
    , m_frequency(440.0f)
    , m_prefinishMarkReachedNotEmitted(true)
{
    //kDebug(604) << k_funcinfo << endl;
    connect(m_tickTimer, SIGNAL(timeout()), SLOT(emitTick()));
}

MediaObject::~MediaObject()
{
    //kDebug(604) << k_funcinfo << endl;
}

bool MediaObject::addVideoPath(QObject *videoPath)
{
    //kDebug(604) << k_funcinfo << endl;
    Q_ASSERT(videoPath);
    VideoPath *vp = qobject_cast<VideoPath *>(videoPath);
    Q_ASSERT(vp);
    Q_ASSERT(!m_videoPathList.contains(vp));
    m_videoPathList.append(vp);
    return true;
}

bool MediaObject::addAudioPath(QObject *audioPath)
{
    //kDebug(604) << k_funcinfo << endl;
    Q_ASSERT(audioPath);
    AudioPath *ap = qobject_cast<AudioPath *>(audioPath);
    Q_ASSERT(ap);
    Q_ASSERT(!m_audioPathList.contains(ap));
    m_audioPathList.append(ap);
    return true;
}

void MediaObject::removeVideoPath(QObject *videoPath)
{
    Q_ASSERT(videoPath);
    VideoPath *vp = qobject_cast<VideoPath *>(videoPath);
    Q_ASSERT(vp);
    Q_ASSERT(m_videoPathList.contains(vp));
    m_videoPathList.removeAll(vp);
}

void MediaObject::removeAudioPath(QObject *audioPath)
{
    Q_ASSERT(audioPath);
    AudioPath *ap = qobject_cast<AudioPath *>(audioPath);
    Q_ASSERT(ap);
    Q_ASSERT(m_audioPathList.contains(ap));
    m_audioPathList.removeAll(ap);
}

State MediaObject::state() const
{
    //kDebug(604) << k_funcinfo << endl;
    return m_state;
}

bool MediaObject::hasVideo() const
{
    //kDebug(604) << k_funcinfo << endl;
    return false;
}

bool MediaObject::isSeekable() const
{
    //kDebug(604) << k_funcinfo << endl;
    return true;
}

qint64 MediaObject::currentTime() const
{
    //kDebug(604) << k_funcinfo << endl;
    switch(state())
    {
    case Phonon::PausedState:
    case Phonon::BufferingState:
        return m_startTime.msecsTo(m_pauseTime);
    case Phonon::PlayingState:
        return m_startTime.elapsed();
    case Phonon::StoppedState:
    case Phonon::LoadingState:
        return 0;
    case Phonon::ErrorState:
        break;
    }
    return -1;
}

qint32 MediaObject::tickInterval() const
{
    //kDebug(604) << k_funcinfo << endl;
    return m_tickInterval;
}

void MediaObject::setTickInterval(qint32 newTickInterval)
{
    //kDebug(604) << k_funcinfo << endl;
    m_tickInterval = newTickInterval;
    if (m_tickInterval <= 0)
        m_tickTimer->setInterval(50);
    else
        m_tickTimer->setInterval(newTickInterval);
}

QStringList MediaObject::availableAudioStreams() const
{
    QStringList ret;
    ret << QLatin1String("en") << QLatin1String("de");
    return ret;
}

QStringList MediaObject::availableVideoStreams() const
{
    QStringList ret;
    ret << QLatin1String("en") << QLatin1String("de");
    return ret;
}

QStringList MediaObject::availableSubtitleStreams() const
{
    QStringList ret;
    ret << QLatin1String("en") << QLatin1String("de");
    return ret;
}

QString MediaObject::currentAudioStream(const QObject *audioPath) const
{
    return m_selectedAudioStream[audioPath];
}

QString MediaObject::currentVideoStream(const QObject *videoPath) const
{
    return m_selectedVideoStream[videoPath];
}

QString MediaObject::currentSubtitleStream(const QObject *videoPath) const
{
    return m_selectedSubtitleStream[videoPath];
}

void MediaObject::setCurrentAudioStream(const QString &streamName, const QObject *audioPath)
{
    if (availableAudioStreams().contains(streamName))
        m_selectedAudioStream[audioPath] = streamName;
}

void MediaObject::setCurrentVideoStream(const QString &streamName, const QObject *videoPath)
{
    if (availableVideoStreams().contains(streamName))
        m_selectedVideoStream[videoPath] = streamName;
}

void MediaObject::setCurrentSubtitleStream(const QString &streamName, const QObject *videoPath)
{
    if (availableSubtitleStreams().contains(streamName))
        m_selectedSubtitleStream[videoPath] = streamName;
}

void MediaObject::play()
{
    //kDebug(604) << k_funcinfo << endl;
    if (m_state == Phonon::LoadingState) {
        setState(Phonon::BufferingState);
    } else {
        setState(Phonon::PlayingState);
    }
}

QString MediaObject::errorString() const
{
    return QString();
}

Phonon::ErrorType MediaObject::errorType() const
{
    return Phonon::NoError;
}

void MediaObject::setState(State newstate)
{
    if (newstate == m_state)
        return;
    State oldstate = m_state;
    m_state = newstate;
    switch(newstate)
    {
    case Phonon::PausedState:
    case Phonon::BufferingState:
        m_pauseTime.start();
        break;
    case Phonon::PlayingState:
        m_tickTimer->start();
        if (oldstate == Phonon::PausedState || oldstate == Phonon::BufferingState)
            m_startTime = m_startTime.addMSecs(m_pauseTime.elapsed());
        else
            m_startTime.start();
        break;
    case Phonon::StoppedState:
    case Phonon::ErrorState:
    case Phonon::LoadingState:
        m_startTime = QTime();
        break;
    }
    //kDebug(604) << "emit stateChanged(" << newstate << ", " << oldstate << ")" << endl;
    emit stateChanged(newstate, oldstate);
}

static const float TWOPI = 6.28318530718f;
static const float maxFrequency = 1760.0f;
static const float minFrequency = 440.0f;
static const float frequencyToDelta = TWOPI / SAMPLE_RATE_FLOAT;

void MediaObject::fillBuffer(QVector<float> *buffer)
{
    //static QFile createdump("createdump");
    //if (!createdump.isOpen())
        //createdump.open(QIODevice::WriteOnly);

    m_frequency *= 1.059463094359f;
    if (m_frequency > maxFrequency)
        m_frequency = minFrequency;
    float delta = frequencyToDelta * m_frequency;

    float *data = buffer->data();
    const float * const end = data + m_bufferSize;

    while (data != end)
    {
        const float sample = sinf(m_position);
        //createdump.write(QByteArray::number(sample) + "\n");
         *(data++) = sample;
        m_position += delta;
        if (m_position > TWOPI)
            m_position -= TWOPI;
    }
}

void MediaObject::fillFrameData(Phonon::Experimental::VideoFrame *frame)
{
    static quint32 frameCount = 0;
    quint8 *dataPtr = reinterpret_cast<quint8 *>(frame->data.data());
    for (int y = 0; y < frame->height; ++y)
        for (int x = 0; x < frame->width; ++x)
        {
             *dataPtr++ = static_cast<quint8>(0xff);
             *dataPtr++ = static_cast<quint8>((x + frameCount) * 2 / 3); //red
             *dataPtr++ = static_cast<quint8>(y + frameCount); //green
             *dataPtr++ = static_cast<quint8>(frameCount / 2); //blue
        }
    ++frameCount;
}

qint64 MediaObject::totalTime() const
{
    //kDebug(604) << k_funcinfo << endl;
    return 1000 *60 *3; // 3 minutes
}

qint32 MediaObject::prefinishMark() const
{
    //kDebug(604) << k_funcinfo << endl;
    return m_prefinishMark;
}

qint32 MediaObject::transitionTime() const
{
    return m_transitionTime;
}

void MediaObject::setTransitionTime(qint32 time)
{
    m_transitionTime = time;
}

MediaSource MediaObject::source() const
{
    return m_source;
}

void MediaObject::setNextSource(const MediaSource &source)
{
    setSource(source);
}

void MediaObject::setSource(const MediaSource &source)
{
    //kDebug(604) << k_funcinfo << endl;
    m_prefinishMarkReachedNotEmitted = true;
    m_source = source;
    setState(Phonon::LoadingState);
    switch (m_source.type()) {
    case MediaSource::Invalid:
        return;
    case MediaSource::Url:
        if (!m_source.url().isValid()) {
            return;
        }
        break;
    case MediaSource::LocalFile:
        if (m_source.filename().isEmpty()) {
            return;
        }
        break;
    case MediaSource::Stream:
        Stream *s = new Stream(m_source, this);
        //Q_ASSERT(qobject_cast<Stream *>(m_source.stream()->d_ptr->streamInterface));
        break;
    }
    emit totalTimeChanged(totalTime());
    QMultiMap<QString, QString> metaData;
    metaData.insert("TITLE", "Fake video");
    metaData.insert("ARTIST", "Matthias Kretz");
    emit metaDataChanged(metaData);
    QTimer::singleShot(50, this, SLOT(loadingComplete()));
}

void MediaObject::loadingComplete()
{
    if (state() == Phonon::LoadingState) {
        setState(Phonon::StoppedState);
    } else if (state() == Phonon::BufferingState) {
        setState(Phonon::PlayingState);
    }
}

void MediaObject::setPrefinishMark(qint32 newPrefinishMark)
{
    //kDebug(604) << k_funcinfo << endl;
    m_prefinishMark = newPrefinishMark;
    if (currentTime() < totalTime() - m_prefinishMark) // not about to finish
        m_prefinishMarkReachedNotEmitted = true;
}

void MediaObject::pause()
{
    //kDebug(604) << k_funcinfo << endl;
    switch (state()) {
    case PlayingState:
    case BufferingState:
    case StoppedState:
        m_tickTimer->stop();
        setState(Phonon::PausedState);
        break;
    case PausedState:
    case LoadingState:
    case ErrorState:
        break;
    }
}

void MediaObject::stop()
{
    //kDebug(604) << k_funcinfo << endl;
    if (state() == Phonon::PlayingState || state() == Phonon::BufferingState || state() == Phonon::PausedState) {
        m_tickTimer->stop();
        setState(Phonon::StoppedState);
        m_position = 0.0f;
        m_frequency = 440.0f;
        m_prefinishMarkReachedNotEmitted = true;
    }
}

void MediaObject::seek(qint64 time)
{
    //kDebug(604) << k_funcinfo << endl;
    if (isSeekable())
    {
        switch(state())
        {
        case Phonon::PausedState:
        case Phonon::BufferingState:
            m_startTime = m_pauseTime;
            break;
        case Phonon::PlayingState:
            m_startTime.start();
            break;
        case Phonon::StoppedState:
        case Phonon::ErrorState:
        case Phonon::LoadingState:
            return; // cannot seek
        }
        m_startTime = m_startTime.addMSecs(-time);
    }

    if (currentTime() < totalTime() - m_prefinishMark) // not about to finish
        m_prefinishMarkReachedNotEmitted = true;
}

void MediaObject::emitTick()
{
    //kDebug(604) << "emit tick(" << currentTime() << ")" << endl;
    int tickInterval = 50;
    if (m_tickInterval > 0)
    {
        emit tick(currentTime());
        tickInterval = m_tickInterval;
    }
    QVector<float> buffer(m_bufferSize);
    Experimental::VideoFrame frame;
    frame.fourcc = 0x00000000;
    frame.width = 320;
    frame.height = 240;
    frame.depth = 24;
    frame.bpp = 8;
    frame.data.resize(frame.width * frame.height * 4);

    const int availableSamples = tickInterval * SAMPLE_RATE / 1000 + m_lastSamplesMissing;
    const int bufferCount = availableSamples / m_bufferSize;
    m_lastSamplesMissing = availableSamples - bufferCount * m_bufferSize;
    for (int i = 0; i < bufferCount; ++i)
    {
        fillBuffer(&buffer);
        foreach (AudioPath *ap, m_audioPathList)
            ap->processBuffer(buffer);
        fillFrameData(&frame);
        foreach (VideoPath *vp, m_videoPathList)
            vp->processFrame(frame);
    }
    if (currentTime() >= totalTime() - m_prefinishMark) { // about to finish
        if (m_prefinishMarkReachedNotEmitted) {
            m_prefinishMarkReachedNotEmitted = false;
            emit prefinishMarkReached(totalTime() - currentTime());
        }
    }
    if (currentTime() >= totalTime()) // finished
    {
        stop();
        emit finished();
    }
}

}}

#include "mediaobject.moc"
// vim: sw=4 ts=4
