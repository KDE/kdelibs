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
#include "../../ifaces/abstractmediaproducer.h"
#include <QTime>
#include <QList>
#include "audiopath.h"
#include "videopath.h"

class QTimer;

namespace Phonon
{
	class VideoFrame;

namespace Fake
{
	class KDE_EXPORT AbstractMediaProducer : public QObject, virtual public Ifaces::AbstractMediaProducer
	{
		Q_OBJECT
		public:
			AbstractMediaProducer( QObject* parent );
			virtual ~AbstractMediaProducer();
			virtual bool addVideoPath( Ifaces::VideoPath* videoPath );
			virtual bool addAudioPath( Ifaces::AudioPath* audioPath );
			virtual void removeVideoPath( Ifaces::VideoPath* videoPath );
			virtual void removeAudioPath( Ifaces::AudioPath* audioPath );
			virtual State state() const;
			virtual bool hasVideo() const;
			virtual bool seekable() const;
			virtual long currentTime() const;
			virtual long tickInterval() const;
			virtual long setTickInterval( long newTickInterval );
			virtual void play();
			virtual void pause();
			virtual void stop();
			virtual void seek( long time );

			void setBufferSize( int size );

		Q_SIGNALS:
			void stateChanged( Phonon::State newstate, Phonon::State oldstate );
			void tick( long time );

		public:
			virtual QObject* qobject() { return this; }
			virtual const QObject* qobject() const { return this; }

		protected:
			void fillBuffer( QVector<float>* buffer );
			void fillFrameData( Phonon::VideoFrame* frame );
			void setState( State );

		protected Q_SLOTS:
			virtual void emitTick();

		private:
			State m_state;
			QTimer* m_tickTimer;
			long m_tickInterval;
			QTime m_startTime, m_pauseTime;
			int m_bufferSize;
			QList<AudioPath*> m_audioPathList;
			QList<VideoPath*> m_videoPathList;
			int m_lastSamplesMissing;

			// for sound synthesis
			float m_position, m_frequency;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_ABSTRACTMEDIAPRODUCER_H
