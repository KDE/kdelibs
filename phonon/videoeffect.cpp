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
#include "videoeffect.h"
#include "videoeffect_p.h"
#include "effectparameter.h"
#include "factory.h"
#include "ifaces/videoeffect.h"
#include "videoeffectdescription.h"

namespace Phonon
{

VideoEffect::VideoEffect( const VideoEffectDescription& type, QObject* parent )
	: QObject( parent )
	, Base( *new VideoEffectPrivate )
{
	K_D( VideoEffect );
	d->type = type.index();
	d->createIface();
}

VideoEffect::VideoEffect( VideoEffectPrivate& dd, QObject* parent, const VideoEffectDescription& type )
	: QObject( parent )
	, Base( dd )
{
	K_D( VideoEffect );
	d->type = type.index();
}

Ifaces::VideoEffect* VideoEffect::iface()
{
	K_D( VideoEffect );
	if( !d->iface() )
		d->createIface();
	return d->iface();
}

void VideoEffectPrivate::createIface()
{
	if( iface_ptr )
		return;
	K_Q( VideoEffect );
	Ifaces::VideoEffect* iface = Factory::self()->createVideoEffect( type, q );
	if( iface )
	{
		setIface( iface );
		q->setupIface();
	}
}

VideoEffectDescription VideoEffect::type() const
{
	K_D( const VideoEffect );
	return VideoEffectDescription::fromIndex( d->type );
}

QList<EffectParameter> VideoEffect::parameterList() const
{
	K_D( const VideoEffect );
	QList<EffectParameter> ret;
	// there should be an iface object, but better be safe for those backend
	// switching corner-cases: when the backend switches the new backend might
	// not support this effect -> no iface object
	if( d->iface() )
	{
		ret = d->iface()->parameterList();
		for( int i = 0; i < ret.size(); ++i )
			ret[ i ].setEffect( const_cast<VideoEffect*>( this ) );
	}
	return ret;
}

QVariant VideoEffect::value( int parameterId ) const
{
	K_D( const VideoEffect );
	return d->iface() ? d->iface()->value( parameterId ) : d->parameterValues[ parameterId ];
}

void VideoEffect::setValue( int parameterId, QVariant newValue )
{
	K_D( VideoEffect );
	if( iface() )
		d->iface()->setValue( parameterId, newValue );
	else
		d->parameterValues[ parameterId ] = newValue;
}

bool VideoEffectPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		QList<EffectParameter> plist = iface()->parameterList();
		foreach( EffectParameter p, plist )
			parameterValues[ p.id() ] = p.value();
	}
	return true;
}

void VideoEffect::setupIface()
{
	K_D( VideoEffect );
	Q_ASSERT( d->iface() );

	// set up attributes
	QList<EffectParameter> plist = d->iface()->parameterList();
	foreach( EffectParameter p, plist )
		if( d->parameterValues.contains( p.id() ) )
			p.setValue( d->parameterValues[ p.id() ] );
}

} //namespace Phonon

#include "videoeffect.moc"

// vim: sw=4 ts=4 tw=80 noet
