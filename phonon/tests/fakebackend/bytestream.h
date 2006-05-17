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
#ifndef Phonon_FAKE_BYTESTREAM_H
#define Phonon_FAKE_BYTESTREAM_H

#include "abstractmediaproducer.h"
#include "../../ifaces/bytestream.h"
class QTimer;

namespace Phonon
{
namespace Fake
{
	class ByteStream : public AbstractMediaProducer, virtual public Ifaces::ByteStream
	{
		Q_OBJECT
		public:
			ByteStream( QObject* parent );
			virtual ~ByteStream();

			virtual qint64 currentTime() const;
			virtual qint64 totalTime() const;
			virtual qint32 aboutToFinishTime() const;
			virtual qint64 streamSize() const;
			virtual bool streamSeekable() const;
			virtual bool seekable() const;

			virtual void setStreamSeekable( bool );
			virtual void writeData( const QByteArray& data );
			virtual void setStreamSize( qint64 );
			virtual void endOfData();
			virtual void setAboutToFinishTime( qint32 );

			virtual void play();
			virtual void pause();
			virtual void seek( qint64 time );

		public Q_SLOTS:
			virtual void stop();

		Q_SIGNALS:
			void finished();
			void aboutToFinish( qint32 );
			void length( qint64 );
			void needData();
			void enoughData();
			void seekStream( qint64 );

		private Q_SLOTS:
			void consumeStream();

		private:
			qint64 m_aboutToFinishBytes;
			qint64 m_streamSize;
			qint64 m_bufferSize;
			qint64 m_streamPosition;
			bool m_streamSeekable;
			bool m_eof;
			bool m_aboutToFinishEmitted;
			QTimer* m_streamConsumeTimer;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_BYTESTREAM_H
