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
#ifndef Phonon_FAKE_VIDEODATAOUTPUT_H
#define Phonon_FAKE_VIDEODATAOUTPUT_H

#include "abstractvideooutput.h"
#include <phonon/experimental/videoframe.h>
#include <QVector>
#include <QByteArray>
#include <QObject>
#include <QSize>

namespace Phonon
{
namespace Fake
{
	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class VideoDataOutput : public QObject, public AbstractVideoOutput
	{
		Q_OBJECT
		Q_INTERFACES( Phonon::Fake::AbstractVideoOutput )
		public:
			VideoDataOutput( QObject* parent );
			~VideoDataOutput();

		public Q_SLOTS:
			int frameRate() const;
			void setFrameRate( int frameRate );

			QSize naturalFrameSize() const;
			QSize frameSize() const;
			void setFrameSize( const QSize& frameSize );

			quint32 format() const;
			void setFormat( quint32 fourcc );

			//int displayLatency() const;
			//void setDisplayLatency( int milliseconds );

		public:
			virtual void* internal1( void* = 0 ) { return static_cast<Phonon::Fake::AbstractVideoOutput*>( this ); }

			// Fake specific:
            virtual void processFrame(Phonon::Experimental::VideoFrame &frame);

		signals:
            void frameReady(const Phonon::Experimental::VideoFrame &frame);
			void endOfMedia();

		private:
			quint32 m_fourcc;
			QByteArray m_pendingData;
			//int m_latency;
			int m_frameRate;
			QSize m_frameSize;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_VIDEODATAOUTPUT_H
