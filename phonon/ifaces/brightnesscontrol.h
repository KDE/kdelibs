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

#ifndef PHONON_IFACES_BRIGHTNESSCONTROL_H
#define PHONON_IFACES_BRIGHTNESSCONTROL_H

#include "videoeffect.h"
#include "../brightnesscontrol.h"
#include <QVariant>

namespace Phonon
{
namespace Ifaces
{
	/**
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class BrightnessControl : virtual public VideoEffect
	{
		public:
			virtual int brightness() const = 0;
			virtual void setBrightness( int brightness ) = 0;

			virtual int lowerBound() const = 0;
			virtual int upperBound() const = 0;
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_IFACES_BRIGHTNESSCONTROL_H
