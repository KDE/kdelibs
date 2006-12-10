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
			m_parameterList.append( EffectParameter( 1, i18n( "time" ), 0,
						m_effect->value( 1 ), 1.0, 15000.0,
						i18n( "Set's the delay in milliseconds" ) ) );
			m_parameterList.append( EffectParameter( 2, i18n( "feedback" ), 0,
					m_effect->value( 2 ), 0.0, 1.0 ) );
			m_parameterList.append( EffectParameter( 3, i18n( "level" ), 0,
					m_effect->value( 3 ), 0.0, 1.0 ) );
	}
	qSort( m_parameterList );
}

AudioEffect::~AudioEffect()
{
	delete m_effect;
	m_effect = 0;
}

QList<Phonon::EffectParameter> AudioEffect::parameterList() const
{
	return m_parameterList;
}

QVariant AudioEffect::value( int parameterId ) const
{
	if( m_effect )
		return m_effect->value( parameterId );
	return QVariant(); // invalid
}

void AudioEffect::setValue( int parameterId, QVariant newValue )
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
// vim: sw=4 ts=4
