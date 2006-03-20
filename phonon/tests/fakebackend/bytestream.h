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

			virtual long currentTime() const;
			virtual long totalTime() const;
			virtual long aboutToFinishTime() const;
			virtual long streamSize() const;
			virtual bool streamSeekable() const;
			virtual bool seekable() const;

			virtual void setStreamSeekable( bool );
			virtual void writeData( const QByteArray& data );
			virtual void setStreamSize( long );
			virtual void endOfData();
			virtual void setAboutToFinishTime( long );

			virtual void play();
			virtual void pause();
			virtual void seek( long time );

		public Q_SLOTS:
			virtual void stop();

		Q_SIGNALS:
			void finished();
			void aboutToFinish( long );
			void length( long );
			void needData();
			void enoughData();
			void seekStream( long );

		private Q_SLOTS:
			void consumeStream();

		private:
			long m_aboutToFinishBytes;
			long m_streamSize;
			long m_bufferSize;
			long m_streamPosition;
			bool m_streamSeekable;
			bool m_eof;
			bool m_aboutToFinishEmitted;
			QTimer* m_streamConsumeTimer;
	};
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_FAKE_BYTESTREAM_H
