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
#ifndef Phonon_FAKE_MEDIAOBJECT_H
#define Phonon_FAKE_MEDIAOBJECT_H

#include <qurl.h>
#include <phonon/mediaobjectinterface.h>
#include <phonon/addoninterface.h>
#include <phonon/experimental/videoframe.h>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QObject>
#include <QtCore/QDate>

class QTimer;

namespace Phonon
{
namespace Fake
{
    class AudioPath;
    class VideoPath;
    class Stream;
    class AudioNode;
    class VideoNode;

    class MediaObject : public QObject, public MediaObjectInterface, public AddonInterface
    {
        friend class Stream;
        Q_OBJECT
        Q_INTERFACES(Phonon::MediaObjectInterface Phonon::AddonInterface)
        public:
            MediaObject(QObject *parent);
            ~MediaObject();
            Phonon::State state() const;
            bool hasVideo() const;
            bool isSeekable() const;
            qint64 currentTime() const;
            qint32 tickInterval() const;

            QStringList availableAudioStreams() const;
            QStringList availableVideoStreams() const;
            QStringList availableSubtitleStreams() const;

            void setTickInterval(qint32 newTickInterval);
            void play();
            void pause();
            void stop();
            void seek(qint64 time);

            QString errorString() const;
            Phonon::ErrorType errorType() const;

            bool hasInterface(Interface) const { return false; }
            QVariant interfaceCall(Interface, int, const QList<QVariant> &) { return QVariant(); }

            QUrl url() const;
            qint64 totalTime() const;
            qint32 prefinishMark() const;
            void setPrefinishMark(qint32 newPrefinishMark);

            qint32 transitionTime() const;
            void setTransitionTime(qint32);

            qint64 remainingTime() const;

            //void setUrl(const QUrl &url);
            //void openMedia(Phonon::MediaObject::Media media, const QString &mediaDevice);
            MediaSource source() const;
            void setSource(const MediaSource &source);
            void setNextSource(const MediaSource &source);

            // fake specific:
            bool wait();
            bool done();
            void addAudioNode(AudioNode *);
            void addVideoNode(VideoNode *);
            bool removeAudioNode(AudioNode *);
            bool removeVideoNode(VideoNode *);

        Q_SIGNALS:
            void stateChanged(Phonon::State newstate, Phonon::State oldstate);
            void tick(qint64 time);
            void metaDataChanged(QMultiMap<QString, QString>);
            void seekableChanged(bool);
            void hasVideoChanged(bool);
            void bufferStatus(int);

            // AddonInterface:
            void titleChanged(int);
            void availableTitlesChanged(int);
            void chapterChanged(int);
            void availableChaptersChanged(int);
            void angleChanged(int);
            void availableAnglesChanged(int);

            void finished();
            void prefinishMarkReached(qint32);
            void aboutToFinish();
            void totalTimeChanged(qint64 length);

        protected:
            void fillBuffer(QVector<float> *buffer);
            void fillFrameData(Phonon::Experimental::VideoFrame *frame);
            void setState(State);

        private Q_SLOTS:
            void loadingComplete();
            void emitTick();

        private:
            State m_state;
            QTimer *m_tickTimer;
            qint32 m_tickInterval;
            QTime m_startTime, m_pauseTime;
            int m_bufferSize;
            QList<AudioNode *> m_audioNodes;
            QList<VideoNode *> m_videoNodes;
            int m_lastSamplesMissing;

            // for sound synthesis
            float m_position, m_frequency;

            MediaSource m_source;
            MediaSource m_nextSource;
            qint32 m_prefinishMark;
            qint32 m_transitionTime;
            bool m_prefinishMarkReachedNotEmitted;
            int m_waiting;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_MEDIAOBJECT_H
