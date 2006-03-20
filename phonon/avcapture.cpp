/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#include "avcapture.h"
#include "avcapture_p.h"
#include "ifaces/avcapture.h"
#include "factory.h"
#include "ifaces/backend.h"

namespace Phonon
{
PHONON_HEIR_IMPL( AvCapture, AbstractMediaProducer )

const AudioSource& AvCapture::audioSource() const
{
	K_D( const AvCapture );
	if( d->iface() && d->iface()->audioSource() != d->audioSource.index() )
	{
		int index = d->iface()->audioSource();
		const Ifaces::Backend* backend = Factory::self()->backend();
		const_cast<AvCapturePrivate*>( d )->audioSource = AudioSource( index,
				backend->audioSourceName( index ),
				backend->audioSourceDescription( index ) );
	}
	return d->audioSource;
}

void AvCapture::setAudioSource( const AudioSource& audioSource )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setAudioSource( audioSource.index() );
	else
		d->audioSource = audioSource;
}

void AvCapture::setAudioSource( int audioSourceIndex )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setAudioSource( audioSourceIndex );
	else
	{
		const Ifaces::Backend* backend = Factory::self()->backend();
		if( backend )
			d->audioSource = AudioSource( audioSourceIndex,
					backend->audioSourceName( audioSourceIndex ),
					backend->audioSourceDescription( audioSourceIndex ) );
	}
}

const VideoSource& AvCapture::videoSource() const
{
	K_D( const AvCapture );
	if( d->iface() && d->iface()->videoSource() != d->videoSource.index() )
	{
		int index = d->iface()->videoSource();
		const Ifaces::Backend* backend = Factory::self()->backend();
		const_cast<AvCapturePrivate*>( d )->videoSource = VideoSource( index,
				backend->videoSourceName( index ),
				backend->videoSourceDescription( index ) );
	}
	return d->videoSource;
}

void AvCapture::setVideoSource( const VideoSource& videoSource )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setVideoSource( videoSource.index() );
	else
		d->videoSource = videoSource;
}

void AvCapture::setVideoSource( int videoSourceIndex )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setVideoSource( videoSourceIndex );
	else
	{
		const Ifaces::Backend* backend = Factory::self()->backend();
		if( backend )
			d->videoSource = VideoSource( videoSourceIndex,
					backend->videoSourceName( videoSourceIndex ),
					backend->videoSourceDescription( videoSourceIndex ) );
	}
}

bool AvCapturePrivate::aboutToDeleteIface()
{
	int index = iface()->audioSource();
	const Ifaces::Backend* backend = Factory::self()->backend();
	audioSource = AudioSource( index,
			backend->audioSourceName( index ),
			backend->audioSourceDescription( index ) );
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

void AvCapture::setupIface()
{
	K_D( AvCapture );
	Q_ASSERT( d->iface() );
	AbstractMediaProducer::setupIface();

	// set up attributes
	d->iface()->setAudioSource( d->audioSource.index() );
}

} //namespace Phonon

#include "avcapture.moc"

// vim: sw=4 ts=4 tw=80 noet
