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

#include "brightnesscontrol.h"
#include "brightnesscontrol_p.h"
#include "factory.h"
#include "effectparameter.h"
#include <klocale.h>

namespace Phonon
{
PHONON_HEIR_IMPL( BrightnessControl, VideoEffect )

QList<EffectParameter> BrightnessControl::parameterList() const
{
	QList<EffectParameter> ret;
	EffectParameter par( 1, EffectParameter::IntegerHint, lowerBound(), upperBound(),
			0, i18n( "Brightness" ), i18n( "controls the brightness of the video images" ) );
	par.setEffect( const_cast<BrightnessControl*>( this ) );
	return ret;
}

QVariant BrightnessControl::value( int parameterId ) const
{
	if( parameterId == 1 )
		return brightness();
	return QVariant();
}

void BrightnessControl::setValue( int parameterId, QVariant newValue )
{
	if( parameterId == 1 )
		setBrightness( newValue.toInt() );
}

int BrightnessControl::brightness() const
{
	K_D( const BrightnessControl );
	return d->iface() ? d->iface()->brightness() : d->brightness;
}

int BrightnessControl::lowerBound() const
{
	K_D( const BrightnessControl );
	return d->iface() ? d->iface()->lowerBound() : -1000;
}

int BrightnessControl::upperBound() const
{
	K_D( const BrightnessControl );
	return d->iface() ? d->iface()->upperBound() : 1000;
}

void BrightnessControl::setBrightness( int brightness )
{
	K_D( BrightnessControl );
	if( iface() )
		d->iface()->setBrightness( brightness );
	else
		d->brightness = brightness;
}

bool BrightnessControlPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		brightness = iface()->brightness();
	}
	return true;
}

void BrightnessControl::setupIface()
{
	K_D( BrightnessControl );
	Q_ASSERT( d->iface() );

	// set up attributes
	d->iface()->setBrightness( d->brightness );
}
}

#include "brightnesscontrol.moc"
// vim: sw=4 ts=4 noet
