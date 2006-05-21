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

#include "backendcapabilities.h"
#include "ifaces/backend.h"
#include "factory.h"
#include "audiooutputdevice.h"
#include "videooutputdevice.h"
#include "audiocapturedevice.h"
#include "videocapturedevice.h"
#include "visualizationeffect.h"
#include "audioeffectdescription.h"
#include "videoeffectdescription.h"
#include "audiocodec.h"
#include "videocodec.h"
#include "containerformat.h"
#include <QList>
#include <QSet>
#include <QStringList>
#include <kservicetypetrader.h>

static KStaticDeleter<Phonon::BackendCapabilities> sd;

namespace Phonon
{

class BackendCapabilities::Private
{
};

BackendCapabilities* BackendCapabilities::m_self = 0;

BackendCapabilities* BackendCapabilities::self()
{
	if( !m_self )
		::sd.setObject( m_self, new BackendCapabilities() );
	return m_self;
}

BackendCapabilities::BackendCapabilities()
	: d( 0 ) //when changing this also uncomment the delete in the dtor
{
	m_self = this;
	connect( Factory::self(), SIGNAL( backendChanged() ), SLOT( slotBackendChanged() ) );
}

BackendCapabilities::~BackendCapabilities()
{
	// delete d;
	// d = 0;
}

bool BackendCapabilities::supportsVideo()
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	return backend ? backend->supportsVideo() : false;
}

bool BackendCapabilities::supportsOSD()
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	return backend ? backend->supportsOSD() : false;
}

bool BackendCapabilities::supportsSubtitles()
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	return backend ? backend->supportsSubtitles() : false;
}

QStringList BackendCapabilities::knownMimeTypes()
{
	const Ifaces::Backend* backend = Factory::self()->backend( false );
	if( backend )
		return backend->knownMimeTypes();
	else
	{
		const KService::List offers = KServiceTypeTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		if ( !offers.isEmpty() )
			return offers.first()->serviceTypes();
		return QStringList();
	}
}

bool BackendCapabilities::isMimeTypeKnown( QString mimeType )
{
	const Ifaces::Backend* backend = Factory::self()->backend( false );
	if( backend )
		return backend->knownMimeTypes().contains( mimeType );
	else
	{
		const KService::List offers = KServiceTypeTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		if ( !offers.isEmpty() )
			return offers.first()->hasServiceType( mimeType );
		return false;
	}
}

#define availableDevicesImpl( classname, indexesMethod ) \
QList<classname> BackendCapabilities::available ## classname ## s() \
{ \
	const Ifaces::Backend* backend = Factory::self()->backend(); \
	QList<classname> ret; \
	if( backend ) \
	{ \
		QSet<int> deviceIndexes = backend->indexesMethod(); \
		foreach( int i, deviceIndexes ) \
			ret.append( classname::fromIndex( i ) ); \
	} \
	return ret; \
}
availableDevicesImpl( AudioOutputDevice, audioOutputDeviceIndexes )
availableDevicesImpl( AudioCaptureDevice, audioCaptureDeviceIndexes )
availableDevicesImpl( VideoOutputDevice, videoOutputDeviceIndexes )
availableDevicesImpl( VideoCaptureDevice, videoCaptureDeviceIndexes )
availableDevicesImpl( VisualizationEffect, visualizationIndexes )
availableDevicesImpl( AudioCodec, audioCodecIndexes )
availableDevicesImpl( VideoCodec, videoCodecIndexes )
availableDevicesImpl( ContainerFormat, containerFormatIndexes )

QList<AudioEffectDescription> BackendCapabilities::availableAudioEffects()
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	QList<AudioEffectDescription> ret;
	if( backend )
	{
		QSet<int> deviceIndexes = backend->audioEffectIndexes();
		foreach( int i, deviceIndexes )
			ret.append( AudioEffectDescription::fromIndex( i ) );
	}
	return ret;
}

QList<VideoEffectDescription> BackendCapabilities::availableVideoEffects()
{
	const Ifaces::Backend* backend = Factory::self()->backend();
	QList<VideoEffectDescription> ret;
	if( backend )
	{
		QSet<int> deviceIndexes = backend->videoEffectIndexes();
		foreach( int i, deviceIndexes )
			ret.append( VideoEffectDescription::fromIndex( i ) );
	}
	return ret;
}

void BackendCapabilities::slotBackendChanged()
{
	emit capabilitiesChanged();
}

} // namespace Phonon
#include "backendcapabilities.moc"
// vim: sw=4 ts=4 noet
