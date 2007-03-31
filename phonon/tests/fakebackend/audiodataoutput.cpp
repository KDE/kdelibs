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

#include "audiodataoutput.h"
#include <QVector>
#include <QMap>

namespace Phonon
{
namespace Fake
{
AudioDataOutput::AudioDataOutput( QObject* parent )
	: AbstractAudioOutput( parent )
{
}

AudioDataOutput::~AudioDataOutput()
{
}

Phonon::Experimental::AudioDataOutput::Format AudioDataOutput::format() const
{
	return m_format;
}

int AudioDataOutput::dataSize() const
{
	return m_dataSize;
}

int AudioDataOutput::sampleRate() const
{
	return 44100;
}

void AudioDataOutput::setFormat(Phonon::Experimental::AudioDataOutput::Format format)
{
	m_format = format;
}

void AudioDataOutput::setDataSize( int size )
{
	m_dataSize = size;
}

typedef QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<float> > FloatMap;
typedef QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<qint16> > IntMap;

inline void AudioDataOutput::convertAndEmit( const QVector<float>& buffer )
{
	if( m_format == Phonon::Experimental::AudioDataOutput::FloatFormat )
	{
		FloatMap map;
		map.insert( Phonon::Experimental::AudioDataOutput::LeftChannel, buffer );
		map.insert( Phonon::Experimental::AudioDataOutput::RightChannel, buffer );
		emit dataReady( map );
	}
	else
	{
		IntMap map;
		QVector<qint16> intBuffer( m_dataSize );
		for( int i = 0; i < m_dataSize; ++i )
			intBuffer[ i ] = static_cast<qint16>( buffer[ i ] * static_cast<float>( 0x7FFF ) );
		map.insert( Phonon::Experimental::AudioDataOutput::LeftChannel, intBuffer );
		map.insert( Phonon::Experimental::AudioDataOutput::RightChannel, intBuffer );
		emit dataReady( map );
	}
}

void AudioDataOutput::processBuffer( const QVector<float>& buffer )
{
	// TODO emit endOfMedia
	m_pendingData += buffer;
	if( m_pendingData.size() < m_dataSize )
		return;

	if( m_pendingData.size() == m_dataSize )
		convertAndEmit( buffer );
	else
	{
		QVector<float> floatBuffer( m_dataSize );
		while( m_pendingData.size() >= m_dataSize )
		{
			memcpy( floatBuffer.data(), m_pendingData.constData(), m_dataSize * sizeof( float ) );
			convertAndEmit( floatBuffer );
			int newSize = m_pendingData.size() - m_dataSize;
			memmove( m_pendingData.data(), m_pendingData.constData() + m_dataSize, newSize * sizeof( float ) );
			m_pendingData.resize( newSize );
		}
	}
}

}} //namespace Phonon::Fake

#include "audiodataoutput.moc"
// vim: sw=4 ts=4
