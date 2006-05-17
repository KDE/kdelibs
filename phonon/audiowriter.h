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

#ifndef PHONON_AUDIOWRITER_H
#define PHONON_AUDIOWRITER_H

#include "abstractaudiooutput.h"
#include "phonondefs.h"

namespace Phonon
{

class AudioWriterPrivate;
class PHONONCORE_EXPORT AudioWriter : public AbstractAudioOutput
{
	Q_OBJECT
	K_DECLARE_PRIVATE( AudioWriter )
	PHONON_HEIR( AudioWriter )
	/**
	 * This property defines the codec to be used for encoding the audio signal.
	 * Possible codecs can be retrieved from BackendCapabilities using the
	 * availableAudioCodecs() function.
	 *
	 * The default codec is an invalid codec (audioCodec().isValid() == false)
	 */
	Q_PROPERTY( AudioCodec audioCodec READ audioCodec WRITE setAudioCodec )
	public:
		AudioCodec audioCodec() const;
		
	public Q_SLOTS:
		setAudioCodec( AudioCodec audioCodec );
		// codec parameters? reuse EffectParameter class?
};

} // namespace Phonon

#endif // PHONON_AUDIOWRITER_H
// vim: sw=4 ts=4 noet tw=80
