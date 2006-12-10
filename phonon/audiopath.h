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
#ifndef Phonon_AUDIOPATH_H
#define Phonon_AUDIOPATH_H

#include "base.h"
#include "phonondefs.h"
#include <QObject>
#include "basedestructionhandler.h"

namespace Phonon
{
	class AudioPathPrivate;
	class AudioEffect;
	class AbstractAudioOutput;

	/**
	 * \short A class to route and process audio data.
	 *
	 * This class is used to define where the audio data is sent to, what
	 * effects are applied and what audio channels is to be used.
	 *
	 * For simple usage an instance of this class is added to an AbstractMediaProducer
	 * and an AbstractAudioOutput subclass (such as AudioOutput) added with
	 * addOutput.
	 * \code
	 * AudioPath* path = new AudioPath( this );
	 * mediaProducer->addAudioPath( path );
	 * path->addOutput( audioOutput );
	 * \endcode
	 *
	 * It is best to add the path to an AbstractMediaProducer after
	 * construction, before calling any other methods.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \see VideoPath
	 */
	class PHONONCORE_EXPORT AudioPath : public QObject, public Base
	{
		friend class AbstractMediaProducer;
		friend class AbstractMediaProducerPrivate;
		friend class Visualization;
		K_DECLARE_PRIVATE( AudioPath )
		Q_OBJECT
		PHONON_OBJECT( AudioPath )

		public:
			~AudioPath();

			/**
			 * Adds an audio output at the "end" of the audio path. This sends
			 * all audio data from the selected channel, after they are
			 * processed in the effects, to the given audio output object.
			 *
			 * You may add outputs of the same class as often as you like,
			 * but if you insert the same object, the call will fail.
			 *
			 * \param audioOutput An object of a subclass of AbstractAudioOutput.
			 * This can be standard soundcard outputs or special outputs that
			 * give your program access to the audio data (or frequency spectrum).
			 *
			 * \return Returns whether the call was successful.
			 *
			 * \see removeOutput
			 * \see AudioOutput
			 * \see AudioFftOutput
			 * \see AudioDataOutput
			 * \see outputs
			 */
			bool addOutput( AbstractAudioOutput* audioOutput );

			/**
			 * Removes the audio output, thereby disconnecting the audio data
			 * flow.
			 *
			 * If the audio output object gets deleted while it is still
			 * connected the output will be removed automatically.
			 *
			 * \param audioOutput your audio output object
			 *
			 * \return Returns whether the call was successful. If it returns
			 * \c false the audio output could not be found in the path, meaning
			 * it was not connected at all.
			 *
			 * \see addOutput
			 * \see outputs
			 */
			bool removeOutput( AbstractAudioOutput* audioOutput );

			/**
			 * Returns a list of AbstractAudioOutput objects that are currently
			 * used as outputs.
			 *
			 * \return A list with all current outputs.
			 *
			 * \see addOutput
			 * \see removeOutput
			 */
			const QList<AbstractAudioOutput*>& outputs() const;

			/**
			 * Inserts an effect into the audio path.
			 *
			 * You may insert effects of the same class as often as you like,
			 * but if you insert the same object, the call will fail.
			 *
			 * \param newEffect An object of a subclass of AudioEffect.
			 *
			 * \param insertBefore If you already inserted an effect you can
			 * tell with this parameter in which order the audio data gets
			 * processed. If this is \c 0 the effect is appended at the end of
			 * the processing list. If the effect has not been inserted before
			 * the method will do nothing and return \c false.
			 *
			 * \return Returns whether the effect could be inserted at the
			 * specified position. If \c false is returned the effect was not
			 * inserted.
			 *
			 * \see removeEffect
			 * \see effects
			 */
			bool insertEffect( AudioEffect* newEffect, AudioEffect* insertBefore = 0 );

			/**
			 * Removes an effect from the audio path.
			 *
			 * If the effect gets deleted while it is still connected the effect
			 * will be removed automatically.
			 *
			 * \param effect The effect to be removed.
			 *
			 * \return Returns whether the call was successful. If it returns
			 * \c false the effect could not be found in the path, meaning it
			 * has not been inserted before.
			 *
			 * \see insertEffect
			 * \see effects
			 */
			bool removeEffect( AudioEffect* effect );

			/**
			 * Returns a list of AudioEffect objects that are currently
			 * used as effects. The order in the list determines the order the
			 * audio signal is sent through the effects.
			 *
			 * \return A list with all current effects.
			 *
			 * \see insertEffect
			 * \see removeEffect
			 */
			const QList<AudioEffect*>& effects() const;
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIOPATH_H
