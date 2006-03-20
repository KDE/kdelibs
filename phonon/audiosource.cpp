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

#include "audiosource.h"
#include <QString>
#include "audiosource_p.h"
#include "videosource.h"
#include "backendcapabilities.h"
#include "factory.h"
#include "ifaces/backend.h"

namespace Phonon
{

AudioSource::AudioSource()
	: NameDescriptionTuple( *new AudioSourcePrivate, -1, QString(), QString() )
{
}

AudioSource::AudioSource( const AudioSource& rhs )
	: NameDescriptionTuple( *new AudioSourcePrivate( rhs.d_func() ) )
{
}

AudioSource AudioSource::fromIndex( int index )
{
	const Ifaces::Backend* b = Factory::self()->backend();
	return AudioSource( index,
			b->audioSourceName( index ),
			b->audioSourceDescription( index ),
			b->audioSourceVideoIndex( index ) );
}

AudioSource::AudioSource( int index, const QString& name, const QString& description, int videoIndex )
	: NameDescriptionTuple( *new AudioSourcePrivate, index, name, description )
{
	Q_D( AudioSource );
	d->videoIndex = videoIndex;
}

AudioSource::~AudioSource()
{
}

const AudioSource& AudioSource::operator=( const AudioSource& rhs )
{
	Q_D( AudioSource );
	*d = *rhs.d_func();
	return *this;
}

bool AudioSource::operator==( const AudioSource& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

int AudioSource::indexOfAssociatedVideoSource() const
{
	return d_func()->videoIndex;
}

VideoSource AudioSource::associatedVideoSource() const
{
	return VideoSource::fromIndex( d_func()->videoIndex );
}

} //namespace Phonon
// vim: sw=4 ts=4 noet
