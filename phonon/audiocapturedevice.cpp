/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "audiocapturedevice.h"
#include <QString>
#include "audiocapturedevice_p.h"
#include "videocapturedevice.h"
#include "backendcapabilities.h"
#include "factory.h"
#include "ifaces/backend.h"

namespace Phonon
{

AudioCaptureDevice::AudioCaptureDevice()
	: NameDescriptionTuple( *new AudioCaptureDevicePrivate, -1, QString(), QString() )
{
}

AudioCaptureDevice::AudioCaptureDevice( const AudioCaptureDevice& rhs )
	: NameDescriptionTuple( *new AudioCaptureDevicePrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

AudioCaptureDevice AudioCaptureDevice::fromIndex( int index )
{
	const Ifaces::Backend* b = Factory::self()->backend();
	return AudioCaptureDevice( index,
			b->audioCaptureDeviceName( index ),
			b->audioCaptureDeviceDescription( index ),
			b->audioCaptureDeviceVideoIndex( index ) );
}

AudioCaptureDevice::AudioCaptureDevice( int index, const QString& name, const QString& description, int videoIndex )
	: NameDescriptionTuple( *new AudioCaptureDevicePrivate, index, name, description )
{
	Q_D( AudioCaptureDevice );
	d->videoIndex = videoIndex;
}

AudioCaptureDevice::~AudioCaptureDevice()
{
}

const AudioCaptureDevice& AudioCaptureDevice::operator=( const AudioCaptureDevice& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool AudioCaptureDevice::operator==( const AudioCaptureDevice& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

int AudioCaptureDevice::indexOfAssociatedVideoCaptureDevice() const
{
	return d_func()->videoIndex;
}

VideoCaptureDevice AudioCaptureDevice::associatedVideoCaptureDevice() const
{
	return VideoCaptureDevice::fromIndex( d_func()->videoIndex );
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
