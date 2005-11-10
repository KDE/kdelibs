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
#ifndef KDEM2M_IFACES_AUDIOOUTPUT_H
#define KDEM2M_IFACES_AUDIOOUTPUT_H

#include "audiooutputbase.h"

class QString;

namespace KDEM2M
{
namespace Ifaces
{
	class AudioOutput : virtual public AudioOutputBase
	{
		public:
			virtual ~AudioOutput() {}

			// Attributes Getters:
			virtual QString name() const = 0;
			virtual float volume() const = 0;

			// Attributes Setters:
			virtual QString setName( const QString& newName ) = 0;
			virtual float setVolume( float newVolume ) = 0;

		//signals:
		protected:
			virtual void volumeChanged( float newVolume ) = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace KDEM2M::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_IFACES_AUDIOOUTPUT_H
