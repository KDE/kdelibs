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
#ifndef Phonon_VIDEOPATH_H
#define Phonon_VIDEOPATH_H

#include "base.h"
#include "phonondefs.h"
#include <QObject>
#include "basedestructionhandler.h"

namespace Phonon
{
	class VideoPathPrivate;
	class VideoEffect;
	class AbstractVideoOutput;

	/**
	 * \short A class to route and process video data.
	 *
	 * This class is used to define where the video data is sent to, what
	 * effects are applied and what video channel (angle) is to be used.
	 *
	 * For simple usage an instance of this class is added to an AbstractMediaProducer
	 * and an AbstractAudioOutput subclass (such as AudioOutput) added with addOutput.
	 * @code
	 * VideoPath* path = new VideoPath( this );
	 * mediaProducer->addVideoPath( path );
	 * path->addOutput( videoOutput );
	 * @endcode
	 *
	 * It is best to add the path to an AbstractMediaProducer after
	 * construction, before calling any other methods.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \see AudioPath
	 */
	class PHONONCORE_EXPORT VideoPath : public QObject, public Base
	{
		friend class AbstractMediaProducer;
		friend class AbstractMediaProducerPrivate;
		K_DECLARE_PRIVATE( VideoPath )
		Q_OBJECT
		PHONON_OBJECT( VideoPath )
		public:
			~VideoPath();

			/**
			 * Adds a video output at the "end" of the video path. This sends
			 * all video data, after it is processed in the effects, to the
			 * given video output object.
			 *
			 * You may add outputs of the same class as often as you like,
			 * but if you insert the same object, the call will fail.
			 *
			 * \param videoOutput An object of a subclass of AbstractVideoOutput.
			 * This can be standard soundcard outputs or special outputs that
			 * give your program access to the video data (or frequency spectrum).
			 *
			 * \return Returns whether the call was successful.
			 *
			 * \see removeOutput
			 * \see VideoWidget
			 */
			bool addOutput( AbstractVideoOutput* videoOutput );

			/**
			 * Removes the video output, thereby disconnecting the video data
			 * flow.
			 *
			 * If the video output object gets deleted while it is still
			 * connected the output will be removed automatically.
			 *
			 * \param videoOutput your video output object
			 *
			 * \return Returns whether the call was successful. If it returns
			 * \c false the video output could not be found in the path, meaning
			 * it was not connected at all.
			 *
			 * \see addOutput
			 */
			bool removeOutput( AbstractVideoOutput* videoOutput );

			/**
			 * Returns a list of AbstractVideoOutput objects that are currently
			 * used as outputs.
			 *
			 * \return A list with all current outputs.
			 *
			 * \see addOutput
			 * \see removeOutput
			 */
			const QList<AbstractVideoOutput*>& outputs() const;

			/**
			 * Inserts an effect into the video path.
			 *
			 * You may insert effects of the same class as often as you like,
			 * but if you insert the same object, the call will fail.
			 *
			 * \param newEffect An object of a subclass of VideoEffect.
			 *
			 * \param insertBefore If you already inserted an effect you can
			 * tell with this parameter in which order the video data gets
			 * processed. If this is \c 0 the effect is appended at the end of
			 * the processing list. If the effect has not been inserted before
			 * the method will do nothing and return \c false.
			 *
			 * \return Returns whether the effect could be inserted at the
			 * specified position. If \c false is returned the effect was not
			 * inserted.
			 *
			 * \see removeEffect
			 * \see VideoEffect
			 */
			bool insertEffect( VideoEffect* newEffect, VideoEffect* insertBefore = 0 );

			/**
			 * Removes an effect from the video path.
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
			 */
			bool removeEffect( VideoEffect* effect );

			/**
			 * Returns a list of VideoEffect objects that are currently
			 * used as effects. The order in the list determines the order the
			 * video signal is sent through the effects.
			 *
			 * \return A list with all current effects.
			 *
			 * \see insertEffect
			 * \see removeEffect
			 */
			const QList<VideoEffect*>& effects() const;
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_VIDEOPATH_H
