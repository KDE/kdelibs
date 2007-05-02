/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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

#include "phonon_export.h"
#include "phonondefs.h"
#include <QtCore/QObject>

namespace Phonon
{
    class VideoPathPrivate;
    class Effect;
    class AbstractVideoOutput;

    /**
     * \short A class to route and process video data.
     *
     * This class is used to define where the video data is sent to, what
     * effects are applied and what video channel (angle) is to be used.
     *
     * For simple usage an instance of this class is added to a MediaObject
     * and an AbstractVideoOutput subclass (such as VideoWidget) added with addOutput.
     * @code
     * VideoPath *path = new VideoPath(this);
     * media->addVideoPath(path);
     * path->addOutput(videoOutput);
     * @endcode
     *
     * It is best to add the path to a MediaObject after
     * construction, before calling any other methods.
     *
     * \author Matthias Kretz <kretz@kde.org>
     * \see AudioPath
     */
    class PHONON_EXPORT VideoPath : public QObject
    {
        friend class MediaObject;
        friend class MediaObjectPrivate;
        K_DECLARE_PRIVATE(VideoPath)
        Q_OBJECT
        PHONON_OBJECT(VideoPath)
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
            bool addOutput(AbstractVideoOutput *videoOutput);

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
            bool removeOutput(AbstractVideoOutput *videoOutput);

            /**
             * Returns a list of AbstractVideoOutput objects that are currently
             * used as outputs.
             *
             * \return A list with all current outputs.
             *
             * \see addOutput
             * \see removeOutput
             */
            const QList<AbstractVideoOutput *> &outputs() const;

            /**
             * Inserts an effect into the video path.
             *
             * You may insert effects of the same class as often as you like,
             * but if you insert the same object, the call will fail.
             *
             * \param newEffect An Effect object.
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
             * \see Effect
             */
            bool insertEffect(Effect *newEffect, Effect *insertBefore = 0);

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
            bool removeEffect(Effect *effect);

            /**
             * Returns a list of Effect objects that are currently
             * used as effects. The order in the list determines the order the
             * video signal is sent through the effects.
             *
             * \return A list with all current effects.
             *
             * \see insertEffect
             * \see removeEffect
             */
            const QList<Effect *> &effects() const;
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_VIDEOPATH_H
