/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_IFACES_BYTESTREAM_H
#define Phonon_IFACES_BYTESTREAM_H

#include "abstractmediaproducer.h"

class QString;
class QStringList;

namespace Phonon
{
namespace Ifaces
{
	class ByteStream : virtual public AbstractMediaProducer
	{
		public:
			virtual long totalTime() const = 0;
			virtual long remainingTime() const { return totalTime() - currentTime(); }
			virtual long aboutToFinishTime() const = 0;
			virtual long streamSize() const = 0;
			virtual bool streamSeekable() const = 0;

			virtual void setStreamSeekable( bool ) = 0;
			virtual void writeData( const QByteArray& data ) = 0;

			/**
			 * Sets the total number of bytes that will be streamed via
			 * writeData
			 */
			virtual void setStreamSize( long ) = 0;

			/**
			 * Called when there will be no more calls to writeBuffer
			 */
			virtual void endOfData() = 0;

			virtual void setAboutToFinishTime( long ) = 0;

		protected: //signals
			virtual void finished() = 0;
			virtual void aboutToFinish( long ) = 0;
			virtual void length( long ) = 0;
			virtual void needData() = 0;
			virtual void enoughData() = 0;
			virtual void seekStream( long ) = 0;
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_BYTESTREAM_H
