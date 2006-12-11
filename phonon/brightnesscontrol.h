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

#ifndef PHONON_BRIGHTNESSCONTROL_H
#define PHONON_BRIGHTNESSCONTROL_H

#include <kdelibs_export.h>
#include "videoeffect.h"

namespace Phonon
{
	class BrightnessControlPrivate;

	/**
	 * Video effect to control the brightness of the image.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT BrightnessControl : public VideoEffect
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( BrightnessControl )
		PHONON_HEIR( BrightnessControl )
		/**
		 * This property controls the brightness of the video signal.
		 *
		 * Depending on the backend and hardware this effect can be implemented
		 * in software or using hardware features like an Xv output would
		 * provide.
		 */
		Q_PROPERTY( int brightness READ brightness WRITE setBrightness )
		public:
			virtual QList<EffectParameter> parameterList() const;

			int brightness() const;
			/**
			 * The smallest value that is possible to set when calling
			 * setBrightness.
			 *
			 * Xv (often) returns -1000
			 */
			int lowerBound() const;
			/**
			 * The biggest value that is possible to set when calling
			 * setBrightness.
			 *
			 * Xv (often) returns 1000
			 */
			int upperBound() const;

		public Q_SLOTS:
			void setBrightness( int brightness );

		protected:
			virtual QVariant value( int parameterId ) const;
			virtual void setValue( int parameterId, QVariant newValue );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_BRIGHTNESSCONTROL_H
