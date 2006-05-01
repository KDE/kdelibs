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
#include "ifaces/audioeffect.h"
#include "audioeffectdescription.h"

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

Ifaces::AudioEffect* AudioEffect::iface()
{
	K_D( AudioEffect );
	if( !d->iface() )
		d->createIface();
	return d->iface();
}

void AudioEffectPrivate::createIface()
{
	if( iface_ptr )
		return;
	K_Q( AudioEffect );
	Ifaces::AudioEffect* iface = Factory::self()->createAudioEffect( type, q );
	if( iface )
	{
		setIface( iface );
		q->setupIface();
	}
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
	if( d->iface() )
	{
		ret = d->iface()->parameterList();
		for( int i = 0; i < ret.size(); ++i )
			ret[ i ].setEffect( const_cast<AudioEffect*>( this ) );
	}
	return ret;
}

float AudioEffect::value( int parameterId ) const
{
	K_D( const AudioEffect );
	return d->iface() ? d->iface()->value( parameterId ) : d->parameterValues[ parameterId ];
}

void AudioEffect::setValue( int parameterId, float newValue )
{
	K_D( AudioEffect );
	if( iface() )
		d->iface()->setValue( parameterId, newValue );
	else
		d->parameterValues[ parameterId ] = newValue;
}

bool AudioEffectPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		QList<EffectParameter> plist = iface()->parameterList();
		foreach( EffectParameter p, plist )
			parameterValues[ p.id() ] = p.value();
	}
	return true;
}

void AudioEffect::setupIface()
{
	K_D( AudioEffect );
	Q_ASSERT( d->iface() );

	// set up attributes
	QList<EffectParameter> plist = d->iface()->parameterList();
	foreach( EffectParameter p, plist )
		if( d->parameterValues.contains( p.id() ) )
			p.setValue( d->parameterValues[ p.id() ] );
}

} //namespace Phonon

#include "audioeffect.moc"

// vim: sw=4 ts=4 tw=80 noet
