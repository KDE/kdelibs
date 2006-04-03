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
#include <klocale.h>

namespace Phonon
{
namespace Fake
{
AudioEffect::AudioEffect( QObject* parent )
	: QObject( parent )
	, m_feedback( 0.5f )
	, m_level( 0.4f )
{
	for( int i = 0; i < 22127; ++i )
		m_delayBuffer.enqueue( 0.0f );
	addParameter( 1, i18n( "time" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
			static_cast<float>( m_delayBuffer.size() ) / 44.1f, 1.0f, 15000.0f,
			i18n( "Set's the delay in milliseconds" ) );
	addParameter( 2, i18n( "feedback" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
			m_feedback, 0.0f, 1.0f );
	addParameter( 3, i18n( "level" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
			m_level, 0.0f, 1.0f );
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

float AudioEffect::value( int parameterId ) const
{
	switch( parameterId )
	{
		case 1:
			return static_cast<float>( m_delayBuffer.size() ) / 44.1f;
		case 2:
			return m_feedback;
		case 3:
			return m_level;
	}
	return 0.0f;
}

void AudioEffect::setValue( int parameterId, float newValue )
{
	switch( parameterId )
	{
		case 1:
			{
				const int newsize = qRound( 44.1f * newValue );
				while( m_delayBuffer.size() < newsize )
					m_delayBuffer.enqueue( 0.0f );
				while( m_delayBuffer.size() > newsize )
					m_delayBuffer.dequeue();
			}
			break;
		case 2:
			m_feedback = newValue;
		case 3:
			m_level = newValue;
	}
}

inline float clamp( const float& min, const float& value, const float& max )
{
	return ( min > value ) ? min : ( max < value ) ? max : value;
}

void AudioEffect::processBuffer( QVector<float>& buffer )
{
	//int enqueue;
	for( int i = 0; i < buffer.size(); ++i )
	{
		m_delayBuffer.enqueue( buffer[ i ] * m_level + m_delayBuffer.head() * m_feedback );
		buffer[ i ] = clamp( -1.0, buffer[ i ] + m_delayBuffer.dequeue(), 1.0 );
		/*
		buffer[ i ] = clamp( -1.0, buffer[ i ] + m_delayBuffer.dequeue(), 1.0 );
		enqueue = rand() / ( RAND_MAX / 3 );
		if( enqueue > 0 || m_delayBuffer.isEmpty() )
		{
			m_delayBuffer.enqueue( buffer[ i ] * 0.11 );
			if( enqueue > 1 || m_delayBuffer.size() < 100 )
				m_delayBuffer.enqueue( buffer[ i ] * 0.2 );
		}
		*/
	}
}

}} //namespace Phonon::Fake

#include "audioeffect.moc"
// vim: sw=4 ts=4 noet
