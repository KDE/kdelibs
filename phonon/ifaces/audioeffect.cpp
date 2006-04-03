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
#include "audioeffect_p.h"

namespace Phonon
{
namespace Ifaces
{

QList<Phonon::EffectParameter> AudioEffect::parameterList() const
{
	Q_D( const AudioEffect );
	return d->parameterList;
}

AudioEffect::AudioEffect()
	: Base( *new AudioEffectPrivate )
{
}

AudioEffect::AudioEffect( AudioEffectPrivate& dd )
	: Base( dd )
{
}

void AudioEffect::addParameter( int parameterId, Phonon::EffectParameter::Hints hints,
		float min, float max, float defaultValue, const QString& name,
		const QString& description )
{
	Q_D( AudioEffect );
	d->parameterList.append( EffectParameter( parameterId, hints, min, max, defaultValue, name, description ) );
	qSort( d->parameterList );
}

}} // namespace Phonon::Ifaces

// vim: sw=4 ts=4 noet
