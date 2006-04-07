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
#include "audiocapturedevice.h"
#include "videocapturedevice.h"
#include "audiooutputdevice.h"
#include "audioeffectdescription.h"
#include <QList>
#include <QSet>
#include "videoeffectdescription.h"
#include <QStringList>
#include <ktrader.h>
#include <kservice.h>

static KStaticDeleter<Phonon::BackendCapabilities> sd;

namespace Phonon
{

class BackendCapabilities::Private
{
	public:
		const Ifaces::Backend* backend;
};

BackendCapabilities* BackendCapabilities::m_self = 0;

BackendCapabilities* BackendCapabilities::self()
{
	if( !m_self )
		::sd.setObject( m_self, new BackendCapabilities() );
	return m_self;
}

BackendCapabilities::BackendCapabilities()
	: d( new Private() )
{
	m_self = this;
	d->backend = Factory::self()->backend();
	connect( Factory::self(), SIGNAL( backendChanged() ), SLOT( slotBackendChanged() ) );
}

BackendCapabilities::~BackendCapabilities()
{
}

bool BackendCapabilities::supportsVideo()
{
	const BackendCapabilities::Private* d = self()->d;
	return d->backend ? d->backend->supportsVideo() : false;
}

bool BackendCapabilities::supportsOSD()
{
	const BackendCapabilities::Private* d = self()->d;
	return d->backend ? d->backend->supportsOSD() : false;
}

bool BackendCapabilities::supportsSubtitles()
{
	const BackendCapabilities::Private* d = self()->d;
	return d->backend ? d->backend->supportsSubtitles() : false;
}

QStringList BackendCapabilities::knownMimeTypes()
{
	const BackendCapabilities::Private* d = self()->d;
	if( d->backend )
		return d->backend->knownMimeTypes();
	else
	{
		KTrader::OfferList offers = KTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		KService::Ptr service = offers.first();
		return service->serviceTypes();
	}
}

bool BackendCapabilities::isMimeTypeKnown( QString mimeType )
{
	const BackendCapabilities::Private* d = self()->d;
	if( d->backend )
		return d->backend->knownMimeTypes().contains( mimeType );
	else
	{
		KTrader::OfferList offers = KTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		KService::Ptr service = offers.first();
		return service->hasServiceType( mimeType );
	}
}

QList<AudioOutputDevice> BackendCapabilities::availableAudioOutputDevices()
{
	const BackendCapabilities::Private* d = self()->d;
	QList<AudioOutputDevice> ret;
	if( d->backend )
	{
		QSet<int> deviceIndexes = d->backend->audioOutputDeviceIndexes();
		foreach( int i, deviceIndexes )
			ret.append( AudioOutputDevice::fromIndex( i ) );
	}
	return ret;
}

QList<AudioCaptureDevice> BackendCapabilities::availableAudioCaptureDevices()
{
	const BackendCapabilities::Private* d = self()->d;
	QList<AudioCaptureDevice> ret;
	if( d->backend )
	{
		QSet<int> deviceIndexes = d->backend->audioCaptureDeviceIndexes();
		foreach( int i, deviceIndexes )
			ret.append( AudioCaptureDevice::fromIndex( i ) );
	}
	return ret;
}

QList<VideoCaptureDevice> BackendCapabilities::availableVideoCaptureDevices()
{
	const BackendCapabilities::Private* d = self()->d;
	QList<VideoCaptureDevice> ret;
	if( d->backend )
	{
		QSet<int> deviceIndexes = d->backend->videoCaptureDeviceIndexes();
		foreach( int i, deviceIndexes )
			ret.append( VideoCaptureDevice::fromIndex( i ) );
	}
	return ret;
}

QList<AudioEffectDescription> BackendCapabilities::availableAudioEffects()
{
	const BackendCapabilities::Private* d = self()->d;
	QList<AudioEffectDescription> ret;
	if( d->backend )
	{
		QSet<int> deviceIndexes = d->backend->audioEffectIndexes();
		foreach( int i, deviceIndexes )
			ret.append( AudioEffectDescription::fromIndex( i ) );
	}
	return ret;
}

QList<VideoEffectDescription> BackendCapabilities::availableVideoEffects()
{
	const BackendCapabilities::Private* d = self()->d;
	QList<VideoEffectDescription> ret;
	if( d->backend )
	{
		QSet<int> deviceIndexes = d->backend->videoEffectIndexes();
		foreach( int i, deviceIndexes )
			ret.append( VideoEffectDescription::fromIndex( i ) );
	}
	return ret;
}

void BackendCapabilities::slotBackendChanged()
{
	d->backend = Factory::self()->backend();
	emit capabilitiesChanged();
}

} // namespace Phonon
#include "backendcapabilities.moc"
// vim: sw=4 ts=4 noet
