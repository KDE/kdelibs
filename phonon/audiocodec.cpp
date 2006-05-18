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

#include "audiocodec.h"
#include "audiocodec_p.h"
#include "factory.h"
#include "ifaces/backend.h"

#include <QString>
#include <QSet>

namespace Phonon
{

AudioCodec::AudioCodec()
	: NameDescriptionTuple( *new AudioCodecPrivate, -1, QString(), QString() )
{
}

AudioCodec::AudioCodec( const AudioCodec& rhs )
	: NameDescriptionTuple( *new AudioCodecPrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

AudioCodec::AudioCodec( int index, const QString& name, const QString& description )
	: NameDescriptionTuple( *new AudioCodecPrivate, index, name, description )
{
}

AudioCodec& AudioCodec::operator=( const AudioCodec& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool AudioCodec::operator==( const AudioCodec& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

AudioCodec AudioCodec::fromIndex( int index )
{
	const Ifaces::Backend* b = Factory::self()->backend();
	if( b->visualizationIndexes().contains( index ) )
		return AudioCodec( index,
				b->visualizationName( index ),
				b->visualizationDescription( index ) );
	else
		return AudioCodec(); //isValid() == false
}

} // namespace Phonon

// vim: sw=4 ts=4 noet
