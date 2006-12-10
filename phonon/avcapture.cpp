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
#include "factory.h"
#include "objectdescription.h"

#define PHONON_CLASSNAME AvCapture

namespace Phonon
{
PHONON_HEIR_IMPL( AbstractMediaProducer )

AudioCaptureDevice AvCapture::audioCaptureDevice() const
{
	K_D( const AvCapture );
	int index;
	if( d->backendObject )
		BACKEND_GET( int, index, "audioCaptureDevice" );
	else
		index = d->audioCaptureDevice;
	return AudioCaptureDevice::fromIndex( index );
}

void AvCapture::setAudioCaptureDevice( const AudioCaptureDevice& audioCaptureDevice )
{
	K_D( AvCapture );
	if( d->backendObject )
		BACKEND_CALL1( "setAudioCaptureDevice", int, audioCaptureDevice.index() );
	else
		d->audioCaptureDevice = audioCaptureDevice.index();
}

void AvCapture::setAudioCaptureDevice( int audioCaptureDeviceIndex )
{
	K_D( AvCapture );
	if( d->backendObject )
		BACKEND_CALL1( "setAudioCaptureDevice", int, audioCaptureDeviceIndex );
	else
		d->audioCaptureDevice = audioCaptureDeviceIndex;
}

VideoCaptureDevice AvCapture::videoCaptureDevice() const
{
	K_D( const AvCapture );
	int index;
	if( d->backendObject )
		BACKEND_GET( int, index, "videoCaptureDevice" );
	else
		index = d->videoCaptureDevice;
	return VideoCaptureDevice::fromIndex( index );
}

void AvCapture::setVideoCaptureDevice( const VideoCaptureDevice& videoCaptureDevice )
{
	K_D( AvCapture );
	if( d->backendObject )
		BACKEND_CALL1( "setVideoCaptureDevice", int, videoCaptureDevice.index() );
	else
		d->videoCaptureDevice = videoCaptureDevice.index();
}

void AvCapture::setVideoCaptureDevice( int videoCaptureDeviceIndex )
{
	K_D( AvCapture );
	if( d->backendObject )
		BACKEND_CALL1( "setVideoCaptureDevice", int, videoCaptureDeviceIndex );
	else
		d->videoCaptureDevice = videoCaptureDeviceIndex;
}

bool AvCapturePrivate::aboutToDeleteIface()
{
	pBACKEND_GET( int, audioCaptureDevice, "audioCaptureDevice" );
	pBACKEND_GET( int, videoCaptureDevice, "videoCaptureDevice" );
	return AbstractMediaProducerPrivate::aboutToDeleteIface();
}

void AvCapture::setupIface()
{
	K_D( AvCapture );
	Q_ASSERT( d->backendObject );
	AbstractMediaProducer::setupIface();

	// set up attributes
	BACKEND_CALL1( "setAudioCaptureDevice", int, d->audioCaptureDevice );
	BACKEND_CALL1( "setVideoCaptureDevice", int, d->videoCaptureDevice );
}

} //namespace Phonon

#include "avcapture.moc"

// vim: sw=4 ts=4 tw=80
