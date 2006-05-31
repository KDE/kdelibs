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

#include "videocapturedevice.h"
#include <QString>
#include "videocapturedevice_p.h"
#include "audiocapturedevice.h"
#include "backendcapabilities.h"
#include "factory.h"
#include <QSet>

namespace Phonon
{

VideoCaptureDevice::VideoCaptureDevice()
	: NameDescriptionTuple( *new VideoCaptureDevicePrivate, -1, QString(), QString() )
{
}

VideoCaptureDevice::VideoCaptureDevice( const VideoCaptureDevice& rhs )
	: NameDescriptionTuple( *new VideoCaptureDevicePrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

VideoCaptureDevice VideoCaptureDevice::fromIndex( int index )
{
	QObject* b = Factory::self()->backend();
	QSet<int> indexes;
	QMetaObject::invokeMethod( b, "videoCaptureDeviceIndexes", Qt::DirectConnection, Q_RETURN_ARG( QSet<int>, indexes ) );
	if( !indexes.contains( index ) )
		return VideoCaptureDevice(); //isValid() == false
	QString name, description;
	int videoIndex;
	QMetaObject::invokeMethod( b, "videoCaptureDeviceName", Qt::DirectConnection, Q_RETURN_ARG( QString, name ), Q_ARG( int, index ) );
	QMetaObject::invokeMethod( b, "videoCaptureDeviceDescription", Qt::DirectConnection, Q_RETURN_ARG( QString, description ), Q_ARG( int, index ) );
	QMetaObject::invokeMethod( b, "videoCaptureDeviceAudioIndex", Qt::DirectConnection, Q_RETURN_ARG( qint32, videoIndex ), Q_ARG( int, index ) );
	return VideoCaptureDevice( index, name, description, videoIndex );
}

VideoCaptureDevice::VideoCaptureDevice( int index, const QString& name, const QString& description, int audioIndex )
	: NameDescriptionTuple( *new VideoCaptureDevicePrivate, index, name, description )
{
	Q_D( VideoCaptureDevice );
	d->audioIndex = audioIndex;
}

VideoCaptureDevice::~VideoCaptureDevice()
{
}

VideoCaptureDevice& VideoCaptureDevice::operator=( const VideoCaptureDevice& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool VideoCaptureDevice::operator==( const VideoCaptureDevice& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

int VideoCaptureDevice::indexOfAssociatedAudioCaptureDevice() const
{
	return d_func()->audioIndex;
}

AudioCaptureDevice VideoCaptureDevice::associatedAudioCaptureDevice() const
{
	return AudioCaptureDevice::fromIndex( d_func()->audioIndex );
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
