/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#ifndef Kdem2m_IFACES_BYTESTREAM_H
#define Kdem2m_IFACES_BYTESTREAM_H

#include "mediaproducer.h"

class QString;
class QStringList;

namespace Kdem2m
{
namespace Ifaces
{
	class ByteStream : virtual public MediaProducer
	{
		public:
			virtual ~ByteStream() {}

			virtual void writeBuffer( const QByteArray& buffer ) = 0;

			/**
			 * Called when there will be no more calls to writeBuffer
			 */
			virtual void endOfData() = 0;

			/**
			 * Sets the total number of bytes that will be streamed via
			 * writeBuffer
			 */
			virtual void toBeWritten() = 0;

		protected: //signals
			virtual void bufferUnderrun() = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace Kdem2m::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_IFACES_BYTESTREAM_H
