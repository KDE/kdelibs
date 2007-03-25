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

#include "bytestream.h"
#include <QTimer>

/* Fake bytestream:
 * Tries to reach a ratio of 4MB = 4min
 * => 4 000 000 Byte = 240 000 msec
 * => 50 Byte = 3 msec
 * make that 51 Byte = 3 msec => 17 Byte = 1 msec
 */

namespace Phonon
{
namespace Fake
{
ByteStream::ByteStream( QObject* parent )
	: AbstractMediaProducer( parent )
	, m_bufferSize( 0 )
	, m_streamPosition( 0 )
	, m_eof( false )
	, m_aboutToFinishEmitted( false )
	, m_streamConsumeTimer( new QTimer( this ) )
{
	connect( m_streamConsumeTimer, SIGNAL( timeout() ), SLOT( consumeStream() ) );
	setState( Phonon::LoadingState );
}

ByteStream::~ByteStream()
{
}

qint64 ByteStream::currentTime() const
{
	return m_streamPosition / 17;
}

qint64 ByteStream::totalTime() const
{
	if( m_streamSize >= 0 )
		return m_streamSize / 17;
    return -1;
	//return 1000*60*3; // 3 minutes
}

qint32 ByteStream::aboutToFinishTime() const
{
	return m_aboutToFinishBytes / 17;
}

qint64 ByteStream::streamSize() const
{
	return m_streamSize;
}

bool ByteStream::streamSeekable() const
{
	return m_streamSeekable;
}

void ByteStream::setStreamSeekable( bool s )
{
	m_streamSeekable = s;
}

void ByteStream::writeData( const QByteArray& data )
{
	Q_ASSERT( ! m_eof );
	m_bufferSize += data.size();
    if (m_bufferSize > 17 * 1000) {
		if( state() == Phonon::BufferingState )
			setState( Phonon::PlayingState );
		else if( state() == Phonon::LoadingState )
			setState( Phonon::StoppedState );
    } else {
        emit bufferStatus(m_bufferSize / 17000);
    }
}

void ByteStream::setStreamSize( qint64 s )
{
	m_streamSize = s;
	emit length( totalTime() );
}

void ByteStream::endOfData()
{
	m_eof = true;
	if( state() == Phonon::BufferingState )
		setState( Phonon::PlayingState );
	else if( state() == Phonon::LoadingState )
		setState( Phonon::StoppedState );
}

void ByteStream::setAboutToFinishTime( qint32 t )
{
	m_aboutToFinishBytes = t * 17;
}

void ByteStream::play()
{
	AbstractMediaProducer::play();
	m_streamConsumeTimer->start( 300 );
	if( state() == Phonon::LoadingState )
	{
		setState( Phonon::BufferingState );
		return;
	}
}

void ByteStream::pause()
{
    if (state() == Phonon::PlayingState || state() == Phonon::BufferingState) {
        AbstractMediaProducer::pause();
        m_streamConsumeTimer->stop();
    }
}

void ByteStream::stop()
{
	if( state() == Phonon::LoadingState )
		return;

	AbstractMediaProducer::stop();
	m_streamConsumeTimer->stop();
}

bool ByteStream::isSeekable() const
{
	return m_streamSeekable;
}

void ByteStream::seek( qint64 time )
{
	if( ! isSeekable() )
		return;

	const qint64 dataStart = m_streamPosition;
	const qint64 dataEnd = dataStart + m_bufferSize;
	qint64 newDataPosition = time * 17;
	m_streamPosition = newDataPosition;
	if( newDataPosition < dataStart || newDataPosition > dataEnd )
	{
		m_bufferSize = 0;
		//setState( Phonon::BufferingState );
		emit seekStream( newDataPosition );
	}
	else
		m_bufferSize = dataEnd - newDataPosition;
	m_aboutToFinishEmitted = false;

	AbstractMediaProducer::seek( currentTime() );
}

void ByteStream::consumeStream()
{
	switch( state() )
	{
		case Phonon::LoadingState:
		case Phonon::BufferingState:
		case Phonon::ErrorState:
		case Phonon::PausedState:
		case Phonon::StoppedState:
			return;
		case Phonon::PlayingState:
			break;
	}
	qint64 bytes = m_streamConsumeTimer->interval() * 17;
	if( m_bufferSize < bytes )
	{
		m_streamPosition += m_bufferSize;
		m_bufferSize = 0;
	}
	else
	{
		m_streamPosition += bytes;
		m_bufferSize -= bytes;
	}
	if( m_eof )
	{
		if( m_bufferSize == 0 )
		{
			emit finished();
			stop();
		}
		else if( !m_aboutToFinishEmitted && m_bufferSize <= m_aboutToFinishBytes )
		{
			m_aboutToFinishEmitted = true;
			emit aboutToFinish( totalTime() - currentTime() );
		}
	}
	else
	{
		if( m_streamSize >= 0 && !m_aboutToFinishEmitted
				&& m_streamSize - m_streamPosition <= m_aboutToFinishBytes )
		{
			m_aboutToFinishEmitted = true;
			emit aboutToFinish( totalTime() - currentTime() );
		}
		if( m_bufferSize < 17 * 5000 ) // try to keep a buffer of more than 5s
			emit needData();
		else if( m_bufferSize > 17 * 10000 ) // and don't let it grow too big (max 10s)
			emit enoughData();
	}
}

}} //namespace Phonon::Fake

#include "bytestream.moc"
// vim: sw=4 ts=4
