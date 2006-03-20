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

#include "audioeffect.h"
#include <QVector>
#include <cstdlib>
#include <iostream>

namespace Phonon
{
namespace Fake
{
AudioEffect::AudioEffect( QObject* parent )
	: QObject( parent )
{
	for( int i = 0; i < 22127; ++i )
		m_delayBuffer.enqueue( 0.0f );
}

AudioEffect::~AudioEffect()
{
}

QString AudioEffect::type() const
{
	return m_type;
}

void AudioEffect::setType( const QString& type )
{
	m_type = type;
}

inline float clamp( const float& min, const float& value, const float& max )
{
	return ( min > value ) ? min : ( max < value ) ? max : value;
}

void AudioEffect::processBuffer( QVector<float>& buffer )
{
	int enqueue;
	for( int i = 0; i < buffer.size(); ++i )
	{
		buffer[ i ] = clamp( -1.0, buffer[ i ] + m_delayBuffer.dequeue(), 1.0 );
		enqueue = rand() / ( RAND_MAX / 3 );
		if( enqueue > 0 || m_delayBuffer.isEmpty() )
		{
			m_delayBuffer.enqueue( buffer[ i ] * 0.11 );
			if( enqueue > 1 || m_delayBuffer.size() < 100 )
				m_delayBuffer.enqueue( buffer[ i ] * 0.2 );
		}
	}
}

}} //namespace Phonon::Fake

#include "audioeffect.moc"
// vim: sw=4 ts=4 noet
