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
#include "ifaces/audiooutput.h"
#include "factory.h"
#include "audiooutputdevice.h"

#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

namespace Phonon
{
PHONON_HEIR_IMPL( AudioOutput, AbstractAudioOutput )

QString AudioOutput::name() const
{
	K_D( const AudioOutput );
	return d->iface() ? d->iface()->name() : d->name;
}

void AudioOutput::setName( const QString& newName )
{
	K_D( AudioOutput );
	if( d->iface() )
		d->iface()->setName( newName );
	else
		d->name = newName;
}

float AudioOutput::volume() const
{
	K_D( const AudioOutput );
	return d->iface() ? d->iface()->volume() : d->volume;
}

void AudioOutput::setVolume( float newVolume )
{
	K_D( AudioOutput );
	if( d->iface() )
		d->iface()->setVolume( newVolume );
	else
		d->volume = newVolume;
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
	return AudioOutputDevice::fromIndex( d->iface() ? d->iface()->outputDevice() : d->outputDeviceIndex );
}

void AudioOutput::setOutputDevice( const AudioOutputDevice& newAudioOutputDevice )
{
	K_D( AudioOutput );
	if( iface() )
		d->iface()->setOutputDevice( newAudioOutputDevice.index() );
	else
		d->outputDeviceIndex = newAudioOutputDevice.index();
}

bool AudioOutputPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		name = iface()->name();
		volume = iface()->volume();
	}
	return AbstractAudioOutputPrivate::aboutToDeleteIface();
}

void AudioOutput::setupIface()
{
	K_D( AudioOutput );
	Q_ASSERT( d->iface() );
	AbstractAudioOutput::setupIface();

	connect( d->iface()->qobject(), SIGNAL( volumeChanged( float ) ), SIGNAL( volumeChanged( float ) ) );

	// set up attributes
	d->iface()->setName( d->name );
	d->iface()->setVolume( d->volume );
}

} //namespace Phonon

#include "audiooutput.moc"

// vim: sw=4 ts=4 tw=80 noet
