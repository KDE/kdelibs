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

#include "videocodec.h"
#include "videocodec_p.h"
#include "factory.h"
#include "phonondefs.h"

#include <QString>
#include <QSet>

namespace Phonon
{

VideoCodec::VideoCodec()
	: NameDescriptionTuple( *new VideoCodecPrivate, -1, QString(), QString() )
{
}

VideoCodec::VideoCodec( const VideoCodec& rhs )
	: NameDescriptionTuple( *new VideoCodecPrivate, rhs.index(), rhs.name(), rhs.description() )
{
}

VideoCodec::VideoCodec( int index, const QString& name, const QString& description )
	: NameDescriptionTuple( *new VideoCodecPrivate, index, name, description )
{
}

VideoCodec& VideoCodec::operator=( const VideoCodec& rhs )
{
	*d_func() = *rhs.d_func();
	return *this;
}

bool VideoCodec::operator==( const VideoCodec& rhs ) const
{
	return *d_func() == *rhs.d_func();
}

NAMEDESCRIPTIONFROMINDEX( VideoCodec, videoCodec )

} // namespace Phonon

// vim: sw=4 ts=4 noet
