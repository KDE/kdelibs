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
#include <phonon/ifaces/videodataoutput.h>
#include <phonon/videoframe.h>
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
	class VideoDataOutput : public QObject, virtual public Ifaces::VideoDataOutput, public Phonon::Fake::AbstractVideoOutput
	{
		Q_OBJECT
		public:
			VideoDataOutput( QObject* parent );
			~VideoDataOutput();

			virtual int frameRate() const;
			virtual void setFrameRate( int frameRate );

			virtual QSize naturalFrameSize() const;
			virtual QSize frameSize() const;
			virtual void setFrameSize( const QSize& frameSize );

			virtual quint32 format() const;
			virtual void setFormat( quint32 fourcc );

			//virtual int displayLatency() const;
			//virtual void setDisplayLatency( int milliseconds );

			virtual void* internal1( void* = 0 ) { return static_cast<Phonon::Fake::AbstractVideoOutput*>( this ); }

			// Fake specific:
			virtual void processFrame( Phonon::VideoFrame& frame );

		signals:
			void frameReady( const Phonon::VideoFrame& frame );
			void endOfMedia();

		public:
			virtual QObject* qobject() { return this; }
			virtual const QObject* qobject() const { return this; }

		private:
			quint32 m_fourcc;
			QByteArray m_pendingData;
			//int m_latency;
			int m_frameRate;
			QSize m_frameSize;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_VIDEODATAOUTPUT_H
