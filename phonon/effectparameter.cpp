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
#include "effectparameter_p.h"
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

EffectParameter::EffectParameter( int parameterId, const QString& name, Hints hints,
		QVariant defaultValue, QVariant min, QVariant max,
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

EffectParameter::~EffectParameter()
{
}

EffectParameter::EffectParameter( const EffectParameter& rhs )
	: d( rhs.d )
{
}

EffectParameter& EffectParameter::operator=( const EffectParameter& rhs )
{
	d = rhs.d;
	return *this;
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

QVariant EffectParameter::minimumValue() const
{
	return d->min;
}

QVariant EffectParameter::maximumValue() const
{
	return d->max;
}

QVariant EffectParameter::defaultValue() const
{
	return d->defaultValue;
}

QVariant EffectParameter::value() const
{
	Q_ASSERT( d->effect );
	return d->effect->value( d->parameterId );
}

void EffectParameter::setValue( QVariant newValue )
{
	Q_ASSERT( d->effect );
	if( isIntegerControl() )
	{
		const int min = qvariant_cast<int>( d->min );
		const int max = qvariant_cast<int>( d->max );
		const int val = qvariant_cast<int>( newValue );
		newValue = qBound( min, val, max );
	}
	else if( !isToggleControl() ) // double
	{
		const double min = qvariant_cast<double>( d->min );
		const double max = qvariant_cast<double>( d->max );
		const double val = qvariant_cast<double>( newValue );
		newValue = qBound( min, val, max );
	} // bool doesn't need to be bounded :)
	d->effect->setValue( d->parameterId, newValue );
}

int EffectParameter::id() const
{
	return d->parameterId;
}

}

// vim: sw=4 ts=4
