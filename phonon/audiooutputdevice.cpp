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

#include "audiooutputdevice.h"
#include <QString>
#include "audiooutputdevice_p.h"
#include "factory.h"
#include "ifaces/backend.h"

namespace Phonon
{

AudioOutputDevice::AudioOutputDevice()
	: NameDescriptionTuple( *new AudioOutputDevicePrivate, -1, QString(), QString() )
{
}

AudioOutputDevice::AudioOutputDevice( const AudioOutputDevice& rhs )
	: NameDescriptionTuple( *new AudioOutputDevicePrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

AudioOutputDevice AudioOutputDevice::fromIndex( int index )
{
	const Ifaces::Backend* b = Factory::self()->backend();
	return AudioOutputDevice( index,
			b->audioOutputDeviceName( index ),
			b->audioOutputDeviceDescription( index ) );
}

AudioOutputDevice::AudioOutputDevice( int index, const QString& name, const QString& description )
	: NameDescriptionTuple( *new AudioOutputDevicePrivate, index, name, description )
{
}

AudioOutputDevice::~AudioOutputDevice()
{
}

AudioOutputDevice& AudioOutputDevice::operator=( const AudioOutputDevice& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool AudioOutputDevice::operator==( const AudioOutputDevice& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
