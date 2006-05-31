/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#include "audiooutput.h"
#include "audiooutput_p.h"
#include "factory.h"
#include "audiooutputdevice.h"

#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <cmath>

namespace Phonon
{
PHONON_HEIR_IMPL( AudioOutput, AbstractAudioOutput )

QString AudioOutput::name() const
{
	K_D( const AudioOutput );
	return d->name;
}

void AudioOutput::setName( const QString& newName )
{
	K_D( AudioOutput );
	d->name = newName;
}

PHONON_GETTER( AudioOutput, float, volume, d->volume )
PHONON_SETTER( AudioOutput, setVolume, volume, float )

static const double log10over20 = 0.1151292546497022842; // ln(10) / 20

double AudioOutput::volumeDecibel() const
{
	return -log( volume() ) / log10over20;
}

void AudioOutput::setVolumeDecibel( double newVolumeDecibel )
{
	setVolume( exp( -newVolumeDecibel * log10over20 ) );
}

Category AudioOutput::category() const
{
	K_D( const AudioOutput );
	return d->category;
}

QString AudioOutput::categoryName() const
{
	K_D( const AudioOutput );
	return Phonon::categoryToString( d->category );
}

void AudioOutput::setCategory( Category c )
{
	K_D( AudioOutput );
	d->category = c;
	//TODO: select the according AudioOutputDevice
}

AudioOutputDevice AudioOutput::outputDevice() const
{
	K_D( const AudioOutput );
	int index;
	if( d->backendObject )
		BACKEND_GET( int, index, "outputDevice" );
	else
		index = d->outputDeviceIndex;
	return AudioOutputDevice::fromIndex( index );
}

void AudioOutput::setOutputDevice( const AudioOutputDevice& newAudioOutputDevice )
{
	K_D( AudioOutput );
	if( iface() )
		BACKEND_CALL1( "setOutputDevice", int, newAudioOutputDevice.index() );
	else
		d->outputDeviceIndex = newAudioOutputDevice.index();
}

bool AudioOutputPrivate::aboutToDeleteIface()
{
	if( backendObject )
		pBACKEND_GET( float, volume, "volume" );
	return AbstractAudioOutputPrivate::aboutToDeleteIface();
}

void AudioOutput::setupIface()
{
	K_D( AudioOutput );
	Q_ASSERT( d->backendObject );
	AbstractAudioOutput::setupIface();

	connect( d->backendObject, SIGNAL( volumeChanged( float ) ), SIGNAL( volumeChanged( float ) ) );

	// set up attributes
	BACKEND_CALL1( "setVolume", float, d->volume );
}

} //namespace Phonon

#include "audiooutput.moc"

// vim: sw=4 ts=4 tw=80 noet
