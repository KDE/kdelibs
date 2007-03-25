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
#include <phonon/mediaproducerinterface.h>
#include <QMultiMap>

class QTimer;

namespace Phonon
{
	class VideoFrame;

namespace Fake
{
	class KDE_EXPORT AbstractMediaProducer : public QObject, public Phonon::MediaProducerInterface
	{
		Q_OBJECT
		Q_INTERFACES( Phonon::MediaProducerInterface )
		public:
			AbstractMediaProducer( QObject* parent );
			~AbstractMediaProducer();
			bool addVideoPath( QObject* videoPath );
			bool addAudioPath( QObject* audioPath );
			void removeVideoPath( QObject* videoPath );
			void removeAudioPath( QObject* audioPath );
			Phonon::State state() const;
			bool hasVideo() const;
			bool isSeekable() const;
			qint64 currentTime() const;
			qint32 tickInterval() const;

			QStringList availableAudioStreams() const;
			QStringList availableVideoStreams() const;
			QStringList availableSubtitleStreams() const;

			QString selectedAudioStream( const QObject* audioPath ) const;
			QString selectedVideoStream( const QObject* videoPath ) const;
			QString selectedSubtitleStream( const QObject* videoPath ) const;

			void selectAudioStream( const QString& streamName, const QObject* audioPath );
			void selectVideoStream( const QString& streamName, const QObject* videoPath );
			void selectSubtitleStream( const QString& streamName, const QObject* videoPath );

			void setTickInterval( qint32 newTickInterval );
			void play();
			void pause();
			void stop();
			void seek( qint64 time );

            QString errorString() const;
            Phonon::ErrorType errorType() const;

			void setBufferSize( int size );

		Q_SIGNALS:
			void stateChanged( Phonon::State newstate, Phonon::State oldstate );
			void tick( qint64 time );
			void metaDataChanged( QMultiMap<QString, QString> );
            void seekableChanged(bool);
            void hasVideoChanged(bool);
            void bufferStatus(int);

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

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_ABSTRACTMEDIAPRODUCER_H
