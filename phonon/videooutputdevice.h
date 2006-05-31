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

#ifndef PHONON_VIDEOOUTPUTDEVICE_H
#define PHONON_VIDEOOUTPUTDEVICE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class VideoOutputDevicePrivate;

/**
 * \short Description for possible VideoOutput device choices.
 *
 * There often are multiple choices of video devices from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see VideoOutput
 */
class PHONONCORE_EXPORT VideoOutputDevice : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( VideoOutputDevice )
	public:
		/**
		 * Constructs an invalid VideoOutputDevice
		 *
		 * \see isValid
		 */
		VideoOutputDevice();

		/**
		 * Constructs a copy of \p videoOutputDevice.
		 *
		 * \see isValid
		 */
		VideoOutputDevice( const VideoOutputDevice& videoOutputDevice );

		~VideoOutputDevice();

		/**
		 * Assigns a copy of the VideoOutputDevice \p videoOutputDevice to this
		 * VideoOutputDevice and returns a reference to it.
		 */
		VideoOutputDevice& operator=( const VideoOutputDevice& videoOutputDevice );

		/**
		 * Returns \c true if this VideoOutputDevice describes the same source
		 * as \p videoOutputDevice; otherwise returns \c false.
		 */
		bool operator==( const VideoOutputDevice& videoOutputDevice ) const;

		/**
		 * Returns a new VideoOutputDevice object that describes the source
		 * with the given \p index.
		 */
		static VideoOutputDevice fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		VideoOutputDevice( int index, const QString& name, const QString& description );
};
} //namespace Phonon

#endif // PHONON_OUTPUTDEVICE_H
