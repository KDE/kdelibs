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
#include <QSet>

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
	QObject* b = Factory::self()->backend();
	QSet<int> indexes;
	QMetaObject::invokeMethod( b, "audioCaptureDeviceIndexes", Qt::DirectConnection, Q_RETURN_ARG( QSet<int>, indexes ) );
	if( !indexes.contains( index ) )
		return AudioCaptureDevice(); //isValid() == false
	QString name, description;
	int videoIndex;
	QMetaObject::invokeMethod( b, "audioCaptureDeviceName", Qt::DirectConnection, Q_RETURN_ARG( QString, name ), Q_ARG( int, index ) );
	QMetaObject::invokeMethod( b, "audioCaptureDeviceDescription", Qt::DirectConnection, Q_RETURN_ARG( QString, description ), Q_ARG( int, index ) );
	QMetaObject::invokeMethod( b, "audioCaptureDeviceVideoIndex", Qt::DirectConnection, Q_RETURN_ARG( qint32, videoIndex ), Q_ARG( int, index ) );
	return AudioCaptureDevice( index, name, description, videoIndex );
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

AudioCaptureDevice& AudioCaptureDevice::operator=( const AudioCaptureDevice& rhs )
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
