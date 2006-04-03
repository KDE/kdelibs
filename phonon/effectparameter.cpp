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

#include "effectparameter.h"
#include "effect.h"

namespace Phonon
{

EffectParameter::EffectParameter()
	: d( new EffectParameterPrivate )
{
	d->effect = 0;
}

bool EffectParameter::isValid() const
{
	return d->effect != 0;
}

EffectParameter::EffectParameter( int parameterId, Hints hints,
		float min, float max, float defaultValue, const QString& name,
		const QString& description )
	: d( new EffectParameterPrivate )
{
	d->effect = 0;
	d->parameterId = parameterId;
	d->min = min;
	d->max = max;
	d->defaultValue = defaultValue;
	d->name = name;
	d->description = description;
	d->hints = hints;
}

void EffectParameter::setEffect( Effect* effect )
{
	Q_ASSERT( effect );
	d->effect = effect;
}

bool EffectParameter::operator==( const EffectParameter& rhs ) const
{
	if( d == rhs.d )
		return true;
	return d->effect == rhs.d->effect
		&& d->parameterId == rhs.d->parameterId;
}

bool EffectParameter::operator<( const EffectParameter& rhs ) const
{
	return d->parameterId < rhs.d->parameterId;
}

bool EffectParameter::operator>( const EffectParameter& rhs ) const
{
	return d->parameterId > rhs.d->parameterId;
}

const QString& EffectParameter::name() const
{
	return d->name;
}

const QString& EffectParameter::description() const
{
	return d->description;
}

bool EffectParameter::isToggleControl() const
{
	return d->hints & ToggledHint;
}

bool EffectParameter::isLogarithmicControl() const
{
	return d->hints & LogarithmicHint;
}

bool EffectParameter::isIntegerControl() const
{
	return d->hints & IntegerHint;
}

bool EffectParameter::isBoundedBelow() const
{
	return d->hints & BoundedBelowHint;
}

bool EffectParameter::isBoundedAbove() const
{
	return d->hints & BoundedAboveHint;
}

float EffectParameter::minimumValue() const
{
	return d->min;
}

float EffectParameter::maximumValue() const
{
	return d->max;
}

float EffectParameter::defaultValue() const
{
	return d->defaultValue;
}

float EffectParameter::value() const
{
	Q_ASSERT( d->effect );
	return d->effect->value( d->parameterId );
}

void EffectParameter::setValue( float newValue ) const
{
	Q_ASSERT( d->effect );
	d->effect->setValue( d->parameterId, newValue );
}

int EffectParameter::id() const
{
	return d->parameterId;
}

}

// vim: sw=4 ts=4 noet
