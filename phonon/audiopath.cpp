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
#include "audiopath.h"
#include "audiopath_p.h"
#include "ifaces/audiopath.h"
#include "factory.h"

#include "audioeffect.h"
#include "abstractaudiooutput.h"

namespace Phonon
{
PHONON_OBJECT_IMPL( AudioPath )

AudioPath::~AudioPath()
{
	K_D( AudioPath );
	foreach( AbstractAudioOutput* ao, d->outputs )
		ao->removeDestructionHandler( this );
	foreach( AudioEffect* ae, d->effects )
		ae->removeDestructionHandler( this );
}

bool AudioPath::addOutput( AbstractAudioOutput* audioOutput )
{
	K_D( AudioPath );
	if( d->outputs.contains( audioOutput ) )
		return false;

	if( iface() && d->iface()->addOutput( audioOutput->iface() ) )
	{
		audioOutput->addDestructionHandler( this );
		d->outputs << audioOutput;
		return true;
	}
	return false;
}

bool AudioPath::removeOutput( AbstractAudioOutput* audioOutput )
{
	K_D( AudioPath );
	if( !d->outputs.contains( audioOutput ) )
		return false;

	if( iface()->removeOutput( audioOutput->iface() ) )
	{
		d->outputs.removeAll( audioOutput );
		return true;
	}
	return false;
}

const QList<AbstractAudioOutput*>& AudioPath::outputs() const
{
	K_D( const AudioPath );
	return d->outputs;
}

bool AudioPath::insertEffect( AudioEffect* newEffect, AudioEffect* insertBefore )
{
	// effects may be added multiple times, but that means the objects are
	// different (the class is still the same)
	K_D( AudioPath );
	if( d->effects.contains( newEffect ) )
		return false;

	if( iface() && d->iface()->insertEffect( newEffect->iface(), insertBefore ? insertBefore->iface() : 0 ) )
	{
		newEffect->addDestructionHandler( this );
		if( insertBefore )
			d->effects.insert( d->effects.indexOf( insertBefore ), newEffect );
		else
			d->effects << newEffect;
		return true;
	}
	return false;
}

bool AudioPath::removeEffect( AudioEffect* effect )
{
	K_D( AudioPath );
	if( !d->effects.contains( effect ) )
		return false;

	if( iface()->removeEffect( effect->iface() ) )
	{
		d->effects.removeAll( effect );
		return true;
	}
	return false;
}

const QList<AudioEffect*>& AudioPath::effects() const
{
	K_D( const AudioPath );
	return d->effects;
}

bool AudioPathPrivate::aboutToDeleteIface()
{
	return true;
}

void AudioPath::setupIface()
{
	K_D( AudioPath );
	Q_ASSERT( d->iface() );

	// set up attributes
	QList<AbstractAudioOutput*> outputList = d->outputs;
	foreach( AbstractAudioOutput* output, outputList )
		if( !d->iface()->addOutput( output->iface() ) )
			d->outputs.removeAll( output );

	QList<AudioEffect*> effectList = d->effects;
	foreach( AudioEffect* effect, effectList )
		if( !d->iface()->insertEffect( effect->iface() ) )
			d->effects.removeAll( effect );
}

void AudioPath::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioEffect
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	K_D( AudioPath );
	Q_ASSERT( o );
	AbstractAudioOutput* output = static_cast<AbstractAudioOutput*>( o );
	AudioEffect* audioEffect = static_cast<AudioEffect*>( o );
	if( d->outputs.contains( output ) )
	{
		if( d->iface() )
			d->iface()->removeOutput( output->iface() );
		d->outputs.removeAll( output );
	}
	else if( d->effects.contains( audioEffect ) )
	{
		if( d->iface() )
			d->iface()->removeEffect( audioEffect->iface() );
		d->effects.removeAll( audioEffect );
	}
}

} //namespace Phonon

#include "audiopath.moc"

// vim: sw=4 ts=4 tw=80 noet
