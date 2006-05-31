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

#include "videooutputdevice.h"
#include <QString>
#include <QSet>
#include "videooutputdevice_p.h"
#include "factory.h"
#include "phonondefs.h"

namespace Phonon
{

VideoOutputDevice::VideoOutputDevice()
	: NameDescriptionTuple( *new VideoOutputDevicePrivate, -1, QString(), QString() )
{
}

VideoOutputDevice::VideoOutputDevice( const VideoOutputDevice& rhs )
	: NameDescriptionTuple( *new VideoOutputDevicePrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

NAMEDESCRIPTIONFROMINDEX( VideoOutputDevice, videoOutputDevice )

VideoOutputDevice::VideoOutputDevice( int index, const QString& name, const QString& description )
	: NameDescriptionTuple( *new VideoOutputDevicePrivate, index, name, description )
{
}

VideoOutputDevice::~VideoOutputDevice()
{
}

VideoOutputDevice& VideoOutputDevice::operator=( const VideoOutputDevice& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool VideoOutputDevice::operator==( const VideoOutputDevice& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
