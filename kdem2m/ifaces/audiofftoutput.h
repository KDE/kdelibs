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
#ifndef KDEM2M_IFACES_AUDIOFFTOUTPUT_H
#define KDEM2M_IFACES_AUDIOFFTOUTPUT_H

#include "audiooutputbase.h"

template<class T> class QVector;

namespace KDEM2M
{
namespace Ifaces
{
	class AudioFFTOutput : virtual public AudioOutputBase
	{
		public:
			virtual ~AudioFFTOutput() {}

			// Operations:
			virtual void readFT( QVector<int>& buffer ) = 0;

			// Attributes Getters:
			virtual int width() const = 0;
			/** FTs per second */
			virtual int rate() const = 0;
			virtual int availableFTs() const = 0;

			// Attributes Setters:
			virtual int setWidth( int newWidth ) = 0;
			/** FTs per second */
			virtual int setRate( int newRate ) = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace KDEM2M::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_IFACES_AUDIOFFTOUTPUT_H
