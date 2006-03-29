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

#ifndef PHONON_AUDIOOUTPUTDEVICE_H
#define PHONON_AUDIOOUTPUTDEVICE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class AudioOutputDevicePrivate;

/**
 * \short Description for possible AudioOutput device choices.
 *
 * There often are multiple choices of audio devices from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AudioOutput
 */
class PHONONCORE_EXPORT AudioOutputDevice : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( AudioOutputDevice )
	friend class BackendCapabilities;
	public:
		/**
		 * Constructs an invalid AudioOutputDevice
		 *
		 * \see isValid
		 */
		AudioOutputDevice();

		/**
		 * Constructs a copy of \p audioOutputDevice.
		 *
		 * \see isValid
		 */
		AudioOutputDevice( const AudioOutputDevice& audioOutputDevice );

		~AudioOutputDevice();

		/**
		 * Assigns a copy of the AudioOutputDevice \p audioOutputDevice to this
		 * AudioOutputDevice and returns a reference to it.
		 */
		AudioOutputDevice& operator=( const AudioOutputDevice& audioOutputDevice );

		/**
		 * Returns \c true if this AudioOutputDevice describes the same source
		 * as \p audioOutputDevice; otherwise returns \c false.
		 */
		bool operator==( const AudioOutputDevice& audioOutputDevice ) const;

		/**
		 * Returns a new AudioOutputDevice object that describes the source
		 * with the given \p index.
		 */
		static AudioOutputDevice fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		AudioOutputDevice( int index, const QString& name, const QString& description );
};
} //namespace Phonon

#endif // PHONON_OUTPUTDEVICE_H
