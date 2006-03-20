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

#include "videosource.h"
#include <QString>
#include "videosource_p.h"
#include "audiosource.h"
#include "backendcapabilities.h"
#include "factory.h"
#include "ifaces/backend.h"

namespace Phonon
{

VideoSource::VideoSource()
	: NameDescriptionTuple( *new VideoSourcePrivate, -1, QString(), QString() )
{
}

VideoSource::VideoSource( const VideoSource& rhs )
	: NameDescriptionTuple( *new VideoSourcePrivate( rhs.d_func() ) )
{
}

VideoSource VideoSource::fromIndex( int index )
{
	const Ifaces::Backend* b = Factory::self()->backend();
	return VideoSource( index,
			b->audioSourceName( index ),
			b->audioSourceDescription( index ),
			b->audioSourceVideoIndex( index ) );
}

VideoSource::VideoSource( int index, const QString& name, const QString& description, int audioIndex )
	: NameDescriptionTuple( *new VideoSourcePrivate, index, name, description )
{
	Q_D( VideoSource );
	d->audioIndex = audioIndex;
}

VideoSource::~VideoSource()
{
}

const VideoSource& VideoSource::operator=( const VideoSource& rhs )
{
	Q_D( VideoSource );
	*d = *rhs.d_func();
	return *this;
}

bool VideoSource::operator==( const VideoSource& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

int VideoSource::indexOfAssociatedAudioSource() const
{
	return d_func()->audioIndex;
}

AudioSource VideoSource::associatedAudioSource() const
{
	return AudioSource::fromIndex( d_func()->audioIndex );
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
