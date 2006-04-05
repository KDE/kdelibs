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
#include <klocale.h>
#include "delayaudioeffect.h"

namespace Phonon
{
namespace Fake
{
AudioEffect::AudioEffect( int effectId, QObject* parent )
	: QObject( parent )
	, m_effect( 0 )
{
	switch( effectId )
	{
		case 0x7F000001:
			m_effect = new DelayAudioEffect;
			addParameter( 1, i18n( "time" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
					m_effect->value( 1 ), 1.0f, 15000.0f,
					i18n( "Set's the delay in milliseconds" ) );
			addParameter( 2, i18n( "feedback" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
					m_effect->value( 2 ), 0.0f, 1.0f );
			addParameter( 3, i18n( "level" ), EffectParameter::BoundedBelowHint | EffectParameter::BoundedAboveHint,
					m_effect->value( 3 ), 0.0f, 1.0f );
	}
}

AudioEffect::~AudioEffect()
{
	delete m_effect;
	m_effect = 0;
}

float AudioEffect::value( int parameterId ) const
{
	if( m_effect )
		return m_effect->value( parameterId );
	return 0.0f;
}

void AudioEffect::setValue( int parameterId, float newValue )
{
	if( m_effect )
		m_effect->setValue( parameterId, newValue );
}

void AudioEffect::processBuffer( QVector<float>& buffer )
{
	if( m_effect )
		m_effect->processBuffer( buffer );
}

}} //namespace Phonon::Fake

#include "audioeffect.moc"
// vim: sw=4 ts=4 noet
