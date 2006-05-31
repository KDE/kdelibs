/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_FAKE_ABSTRACTMEDIAPRODUCER_H
#define Phonon_FAKE_ABSTRACTMEDIAPRODUCER_H

#include <QObject>
#include <QTime>
#include <QList>
#include <phonon/phononnamespace.h>
#include "audiopath.h"
#include "videopath.h"
#include <QHash>

class QTimer;

namespace Phonon
{
	class VideoFrame;

namespace Fake
{
	class KDE_EXPORT AbstractMediaProducer : public QObject
	{
		Q_OBJECT
		public:
			AbstractMediaProducer( QObject* parent );
			~AbstractMediaProducer();
			Q_INVOKABLE bool addVideoPath( QObject* videoPath );
			Q_INVOKABLE bool addAudioPath( QObject* audioPath );
			Q_INVOKABLE void removeVideoPath( QObject* videoPath );
			Q_INVOKABLE void removeAudioPath( QObject* audioPath );
			Q_INVOKABLE Phonon::State state() const;
			Q_INVOKABLE bool hasVideo() const;
			Q_INVOKABLE bool seekable() const;
			Q_INVOKABLE qint64 currentTime() const;
			Q_INVOKABLE qint32 tickInterval() const;

			Q_INVOKABLE QStringList availableAudioStreams() const;
			Q_INVOKABLE QStringList availableVideoStreams() const;
			Q_INVOKABLE QStringList availableSubtitleStreams() const;

			Q_INVOKABLE QString selectedAudioStream( const QObject* audioPath ) const;
			Q_INVOKABLE QString selectedVideoStream( const QObject* videoPath ) const;
			Q_INVOKABLE QString selectedSubtitleStream( const QObject* videoPath ) const;

			Q_INVOKABLE void selectAudioStream( const QString& streamName, const QObject* audioPath );
			Q_INVOKABLE void selectVideoStream( const QString& streamName, const QObject* videoPath );
			Q_INVOKABLE void selectSubtitleStream( const QString& streamName, const QObject* videoPath );

			Q_INVOKABLE void setTickInterval( qint32 newTickInterval );
			Q_INVOKABLE void play();
			Q_INVOKABLE void pause();
			Q_INVOKABLE void stop();
			Q_INVOKABLE void seek( qint64 time );

			void setBufferSize( int size );

		Q_SIGNALS:
			void stateChanged( Phonon::State newstate, Phonon::State oldstate );
			void tick( qint64 time );

		protected:
			void fillBuffer( QVector<float>* buffer );
			void fillFrameData( Phonon::VideoFrame* frame );
			void setState( State );

		protected Q_SLOTS:
			virtual void emitTick();

		private:
			State m_state;
			QTimer* m_tickTimer;
			qint32 m_tickInterval;
			QTime m_startTime, m_pauseTime;
			int m_bufferSize;
			QList<AudioPath*> m_audioPathList;
			QList<VideoPath*> m_videoPathList;
			int m_lastSamplesMissing;

			// for sound synthesis
			float m_position, m_frequency;

			QHash<const QObject*, QString> m_selectedAudioStream;
			QHash<const QObject*, QString> m_selectedVideoStream;
			QHash<const QObject*, QString> m_selectedSubtitleStream;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_ABSTRACTMEDIAPRODUCER_H
