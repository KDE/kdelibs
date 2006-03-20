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

#ifndef PHONON_VOLUMEFADEREFFECT_H
#define PHONON_VOLUMEFADEREFFECT_H

#include <kdelibs_export.h>
#include "audioeffect.h"

namespace Phonon
{
	class VolumeFaderEffectPrivate;
	namespace Ifaces
	{
		class VolumeFaderEffect;
	}

	/**
	 * Audio effect to gradually fade the audio volume.
	 *
	 * This effect differs from gradually changing the output volume in that
	 * a dedicated effect can change the volume in the smallest possible
	 * steps while every other volume control will make more or less
	 * noticable steps.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \see AudioOutput::setVolume
	 */
	class PHONON_EXPORT VolumeFaderEffect : public AudioEffect
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( VolumeFaderEffect )
		PHONON_HEIR( VolumeFaderEffect )
		public:
			/**
			 * Returns the current volume.
			 *
			 * \return The volume: 0.0 means 0%, 1.0 means 100%
			 *
			 * \see setVolume
			 */
			float volume() const;

		public Q_SLOTS:
			/**
			 * Tells the Fader to change the volume from the current volume to 100%
			 * in \p fadeTime milliseconds.
			 * Short for \c fadeTo( 1.0, fadeTime ).
			 *
			 * \param fadeTime the fade duration in milliseconds
			 *
			 * \see fadeTo
			 * \see setVolume
			 */
			void fadeIn( int fadeTime );

			/**
			 * Tells the Fader to change the volume from the current volume to 0%
			 * in \p fadeTime milliseconds.
			 * Short for \c fadeTo( 0.0, fadeTime ).
			 *
			 * \param fadeTime the fade duration in milliseconds
			 *
			 * \see fadeTo
			 */
			void fadeOut( int fadeTime );

			/**
			 * Sets the volume of the fader immediately to the
			 * desired value.
			 *
			 * \param volume The volume: 0.0 means 0%, 1.0 means
			 * 100%
			 */
			void setVolume( float volume );

			/**
			 * Tells the Fader to change the volume from the current value to
			 * \p volume in \p fadeTime milliseconds
			 *
			 * \see fadeIn
			 * \see fadeOut
			 */
			void fadeTo( float volume, int fadeTime );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_VOLUMEFADEREFFECT_H
