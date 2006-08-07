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

#ifndef BYTESTREAM_P_H
#define BYTESTREAM_P_H

#include "bytestream.h"
#include "abstractmediaproducer_p.h"

namespace Phonon
{
class ByteStreamPrivate : public AbstractMediaProducerPrivate
{
	K_DECLARE_PUBLIC( ByteStream )
	PHONON_PRIVATECLASS( AbstractMediaProducer )
	protected:
		ByteStreamPrivate()
			: aboutToFinishTime( 0 )
			, streamSize( -1 )
			, streamSeekable( false )
		{
		}

		qint32 aboutToFinishTime;
		qint64 streamSize;
		bool streamSeekable;
};
}

#endif // BYTESTREAM_P_H
// vim: sw=4 ts=4 tw=80
