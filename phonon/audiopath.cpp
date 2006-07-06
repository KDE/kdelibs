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
		ao->removeDestructionHandler( d );
	foreach( AudioEffect* ae, d->effects )
		ae->removeDestructionHandler( d );
}

bool AudioPath::addOutput( AbstractAudioOutput* audioOutput )
{
	K_D( AudioPath );
	if( d->outputs.contains( audioOutput ) )
		return false;

	if( iface() )
	{
		bool success;
		BACKEND_GET1( bool, success, "addOutput", QObject*, audioOutput->iface() );
		if( success )
		{
			audioOutput->addDestructionHandler( d );
			d->outputs << audioOutput;
			return true;
		}
	}
	return false;
}

bool AudioPath::removeOutput( AbstractAudioOutput* audioOutput )
{
	K_D( AudioPath );
	if( !d->outputs.contains( audioOutput ) )
		return false;

	if( d->backendObject )
	{
		bool success;
		BACKEND_GET1( bool, success, "removeOutput", QObject*, audioOutput->iface() );
		if( success )
		{
			d->outputs.removeAll( audioOutput );
			return true;
		}
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

	if( iface() )
	{
		bool success;
		BACKEND_GET2( bool, success, "insertEffect", QObject*, newEffect->iface(), QObject*, insertBefore ? insertBefore->iface() : 0 );
		if( success )
		{
			newEffect->addDestructionHandler( d );
			if( insertBefore )
				d->effects.insert( d->effects.indexOf( insertBefore ), newEffect );
			else
				d->effects << newEffect;
			return true;
		}
	}
	return false;
}

bool AudioPath::removeEffect( AudioEffect* effect )
{
	K_D( AudioPath );
	if( !d->effects.contains( effect ) )
		return false;

	if( d->backendObject )
	{
		bool success;
		BACKEND_GET1( bool, success, "removeEffect", QObject*, effect->iface() );
		if( success )
		{
			d->effects.removeAll( effect );
			return true;
		}
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
	Q_ASSERT( d->backendObject );

	// set up attributes
	bool success;
	QList<AbstractAudioOutput*> outputList = d->outputs;
	foreach( AbstractAudioOutput* output, outputList )
	{
		BACKEND_GET1( bool, success, "addOutput", QObject*, output->iface() );
		if( !success )
			d->outputs.removeAll( output );
	}

	QList<AudioEffect*> effectList = d->effects;
	foreach( AudioEffect* effect, effectList )
	{
		BACKEND_GET1( bool, success, "insertEffect", QObject*, effect->iface() );
		if( !success )
			d->effects.removeAll( effect );
	}
}

void AudioPathPrivate::phononObjectDestroyed( Base* o )
{
	// this method is called from Phonon::Base::~Base(), meaning the AudioEffect
	// dtor has already been called, also virtual functions don't work anymore
	// (therefore qobject_cast can only downcast from Base)
	Q_ASSERT( o );
	AbstractAudioOutput* output = static_cast<AbstractAudioOutput*>( o );
	AudioEffect* audioEffect = static_cast<AudioEffect*>( o );
	if( outputs.contains( output ) )
	{
		if( backendObject )
			pBACKEND_CALL1( "removeOutput", QObject*, output->iface() );
		outputs.removeAll( output );
	}
	else if( effects.contains( audioEffect ) )
	{
		if( backendObject )
			pBACKEND_CALL1( "removeEffect", QObject*, audioEffect->iface() );
		effects.removeAll( audioEffect );
	}
}

} //namespace Phonon

#include "audiopath.moc"

// vim: sw=4 ts=4 tw=80 noet
