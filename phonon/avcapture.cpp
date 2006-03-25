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
#include "avcapture.h"
#include "avcapture_p.h"
#include "ifaces/avcapture.h"
#include "factory.h"
#include "ifaces/backend.h"
#include "audiocapturedevice.h"
#include "videocapturedevice.h"

namespace Phonon
{
PHONON_HEIR_IMPL( AvCapture, AbstractMediaProducer )

AudioCaptureDevice AvCapture::audioCaptureDevice() const
{
	K_D( const AvCapture );
	return AudioCaptureDevice::fromIndex( d->iface() ? d->iface()->audioCaptureDevice() : d->audioCaptureDevice );
}

void AvCapture::setAudioCaptureDevice( const AudioCaptureDevice& audioCaptureDevice )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setAudioCaptureDevice( audioCaptureDevice.index() );
	else
		d->audioCaptureDevice = audioCaptureDevice.index();
}

void AvCapture::setAudioCaptureDevice( int audioCaptureDeviceIndex )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setAudioCaptureDevice( audioCaptureDeviceIndex );
	else
		d->audioCaptureDevice = audioCaptureDeviceIndex;
}

VideoCaptureDevice AvCapture::videoCaptureDevice() const
{
	K_D( const AvCapture );
	return VideoCaptureDevice::fromIndex( d->iface() ? d->iface()->videoCaptureDevice() : d->videoCaptureDevice );
}

void AvCapture::setVideoCaptureDevice( const VideoCaptureDevice& videoCaptureDevice )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setVideoCaptureDevice( videoCaptureDevice.index() );
	else
		d->videoCaptureDevice = videoCaptureDevice.index();
}

void AvCapture::setVideoCaptureDevice( int videoCaptureDeviceIndex )
{
	K_D( AvCapture );
	if( d->iface() )
		d->iface()->setVideoCaptureDevice( videoCaptureDeviceIndex );
	else
		d->videoCaptureDevice = videoCaptureDeviceIndex;
}

bool AvCapturePrivate::aboutToDeleteIface()
{
	audioCaptureDevice = iface()->audioCaptureDevice();
	videoCaptureDevice = iface()->videoCaptureDevice();
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

void AvCapture::setupIface()
{
	K_D( AvCapture );
	Q_ASSERT( d->iface() );
	AbstractMediaProducer::setupIface();

	// set up attributes
	d->iface()->setAudioCaptureDevice( d->audioCaptureDevice );
	d->iface()->setVideoCaptureDevice( d->videoCaptureDevice );
}

} //namespace Phonon

#include "avcapture.moc"

// vim: sw=4 ts=4 tw=80 noet
