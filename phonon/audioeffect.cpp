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
#include "audioeffect.h"
#include "audioeffect_p.h"
#include "effectparameter.h"
#include "factory.h"

namespace Phonon
{
AudioEffect::AudioEffect( const AudioEffectDescription& type, QObject* parent )
	: QObject( parent )
	, Base( *new AudioEffectPrivate )
{
	K_D( AudioEffect );
	d->type = type.index();
	d->createIface();
}

AudioEffect::AudioEffect( AudioEffectPrivate& dd, QObject* parent, const AudioEffectDescription& type )
	: QObject( parent )
	, Base( dd )
{
	K_D( AudioEffect );
	d->type = type.index();
}

void AudioEffectPrivate::createIface()
{
	if( backendObject )
		return;
	K_Q( AudioEffect );
	backendObject = Factory::self()->createAudioEffect( type, q );
	if( backendObject )
		q->setupIface();
}

AudioEffectDescription AudioEffect::type() const
{
	K_D( const AudioEffect );
	return AudioEffectDescription::fromIndex( d->type );
}

QList<EffectParameter> AudioEffect::parameterList() const
{
	K_D( const AudioEffect );
	QList<EffectParameter> ret;
	// there should be an iface object, but better be safe for those backend
	// switching corner-cases: when the backend switches the new backend might
	// not support this effect -> no iface object
	if( d->backendObject )
	{
		BACKEND_GET( QList<EffectParameter>, ret, "parameterList" );
		for( int i = 0; i < ret.size(); ++i )
			ret[ i ].setEffect( const_cast<AudioEffect*>( this ) );
	}
	return ret;
}

QVariant AudioEffect::value( int parameterId ) const
{
	K_D( const AudioEffect );
	if( !d->backendObject )
		return d->parameterValues[ parameterId ];
	QVariant ret;
	BACKEND_GET1( QVariant, ret, "value", int, parameterId );
	return ret;
}

void AudioEffect::setValue( int parameterId, QVariant newValue )
{
	K_D( AudioEffect );
	if( iface() )
		BACKEND_CALL2( "setValue", int, parameterId, QVariant, newValue );
	else
		d->parameterValues[ parameterId ] = newValue;
}

bool AudioEffectPrivate::aboutToDeleteIface()
{
	if( backendObject )
	{
		QList<EffectParameter> plist;
		pBACKEND_GET( QList<EffectParameter>, plist, "parameterList" );
		foreach( EffectParameter p, plist )
			parameterValues[ p.id() ] = p.value();
	}
	return true;
}

void AudioEffect::setupIface()
{
	K_D( AudioEffect );
	Q_ASSERT( d->backendObject );

	// set up attributes
	QList<EffectParameter> plist;
	BACKEND_GET( QList<EffectParameter>, plist, "parameterList" );
	foreach( EffectParameter p, plist )
		if( d->parameterValues.contains( p.id() ) )
			p.setValue( d->parameterValues[ p.id() ] );
}

} //namespace Phonon

#include "audioeffect.moc"

// vim: sw=4 ts=4 tw=80
