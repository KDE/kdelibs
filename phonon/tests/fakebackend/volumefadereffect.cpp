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

#include "volumefadereffect.h"

namespace Phonon
{
namespace Fake
{
VolumeFaderEffect::VolumeFaderEffect( QObject* parent )
	: AudioEffect( -1, parent )
	, m_fadeTime( 0 )
{
}

VolumeFaderEffect::~VolumeFaderEffect()
{
}

float VolumeFaderEffect::volume() const
{
	if( m_fadeTime > 0 )
	{
		float v = m_volume + ( m_endvolume - m_volume ) / m_fadeTime * m_fadeStart.elapsed();
		if( ( m_volume < m_endvolume && v > m_endvolume ) || ( m_volume > m_endvolume && v < m_endvolume ) )
		{
			VolumeFaderEffect* t = const_cast<VolumeFaderEffect*>( this );
			t->m_volume = m_endvolume;
			t->m_fadeTime = 0;
		}
		else
			return v;
	}
	return m_volume;
}

void VolumeFaderEffect::setVolume( float volume )
{
	m_volume = volume;
}

Phonon::VolumeFaderEffect::FadeCurve VolumeFaderEffect::fadeCurve() const
{
	return m_fadeCurve;
}

void VolumeFaderEffect::setFadeCurve( Phonon::VolumeFaderEffect::FadeCurve curve )
{
	m_fadeCurve = curve;
}

void VolumeFaderEffect::fadeTo( float volume, int fadeTime )
{
	if( m_fadeTime > 0 )
		m_volume = this->volume();
	m_fadeStart.start();
	m_fadeTime = fadeTime;
	m_endvolume = volume;
}

}} //namespace Phonon::Fake

#include "volumefadereffect.moc"
// vim: sw=4 ts=4
