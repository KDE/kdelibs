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
#include "volumefadereffect_p.h"
#include "factory.h"

namespace Phonon
{
PHONON_HEIR_IMPL( VolumeFaderEffect, AudioEffect )

float VolumeFaderEffect::volume() const
{
	K_D( const VolumeFaderEffect );
	return d->iface() ? d->iface()->volume() : d->currentVolume;
}

void VolumeFaderEffect::setVolume( float volume )
{
	K_D( VolumeFaderEffect );
	if( iface() )
		d->iface()->setVolume( volume );
	else
		d->currentVolume = volume;
}

VolumeFaderEffect::FadeCurve VolumeFaderEffect::fadeCurve() const
{
	K_D( const VolumeFaderEffect );
	return d->iface() ? d->iface()->fadeCurve() : d->fadeCurve;
}

void VolumeFaderEffect::setFadeCurve( FadeCurve curve )
{
	K_D( VolumeFaderEffect );
	if( iface() )
		d->iface()->setFadeCurve( curve );
	else
		d->fadeCurve = curve;
}

void VolumeFaderEffect::fadeIn( int fadeTime )
{
	fadeTo( 1.0, fadeTime );
}

void VolumeFaderEffect::fadeOut( int fadeTime )
{
	fadeTo( 0.0, fadeTime );
}

void VolumeFaderEffect::fadeTo( float volume, int fadeTime )
{
	K_D( VolumeFaderEffect );
	if( iface() )
		d->iface()->fadeTo( volume, fadeTime );
	else
		d->currentVolume = volume;
}

bool VolumeFaderEffectPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		currentVolume = iface()->volume();
		fadeCurve = iface()->fadeCurve();
	}
	return true;
}

void VolumeFaderEffect::setupIface()
{
	K_D( VolumeFaderEffect );
	Q_ASSERT( d->iface() );

	// set up attributes
	d->iface()->setVolume( d->currentVolume );
	d->iface()->setFadeCurve( d->fadeCurve );
}
}

#include "volumefadereffect.moc"
// vim: sw=4 ts=4 noet
