/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_MEDIAOBJECTINTERFACE_H
#define PHONON_MEDIAOBJECTINTERFACE_H

#include "mediaobject.h"
#include <QtCore/QObject>

namespace Phonon
{
class StreamInterface;

/** \class MediaObjectInterface mediaobjectinterface.h Phonon/MediaObjectInterface
 * \short Backend interface for media sources.
 *
 * The backend implementation has to provide two signals, that are not defined
 * in this interface:
 * <ul>
 * <li>\anchor phonon_MediaObjectInterface_stateChanged
 * <b>void stateChanged(\ref Phonon::State newstate, \ref Phonon::State oldstate)</b>
 *
 * Emitted when the state of the MediaObject has changed.
 * In case you're not interested in the old state you can also
 * connect to a slot that only has one State argument.
 *
 * \param newstate The state the Player is in now.
 * \param oldstate The state the Player was in before.
 * </li>
 * <li>\anchor phonon_MediaObjectInterface_tick
 * <b>void tick(qint64 time)</b>
 *
 * This signal gets emitted every tickInterval milliseconds.
 *
 * \param time The position of the media file in milliseconds.
 *
 * \see setTickInterval()
 * \see tickInterval()
 * </li>
 * </ul>
 *
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 * \see MediaObject
 */
class MediaObjectInterface
{
    public:
        virtual ~MediaObjectInterface() {}

        /**
         * Lists the available audio streams the media provides. The strings are
         * what is shown to the user and often just consist of the stream name
         * encoded into the media file.
         *
         * Be prepared to change the audio stream when \ref
         * setCurrentAudioStream is called with one of the
         * strings of this list. If the media has multiple audio streams but the
         * backend cannot switch then the list of available audio streams should be
         * empty.
         *
         * \returns A list of names for the available audio streams.
         */
        //virtual QList<AudioStreamDescription> availableAudioStreams() const = 0;
        /**
         * Lists the available video streams the media provides. The strings are
         * what is shown to the user and often just consist of the stream name
         * encoded into the media file.
         *
         * Be prepared to change the video stream when \ref
         * setCurrentVideoStream is called with one of the
         * strings of this list. If the media has multiple video streams but the
         * backend cannot switch then the list of available video streams should be
         * empty.
         *
         * \returns A list of names for the available video streams.
         */
        //virtual QList<VideoStreamDescription> availableVideoStreams() const = 0;
        /**
         * Lists the available subtitle streams the media provides. The strings are
         * what is shown to the user and often just consist of the stream name
         * encoded into the media file.
         *
         * Be prepared to change the subtitle stream when \ref
         * setCurrentSubtitleStream is called with one of the
         * strings of this list. If the media has multiple subtitle streams but the
         * backend cannot switch then the list of available subtitle streams should be
         * empty.
         *
         * \returns A list of names for the available subtitle streams.
         */
        //virtual QList<SubtitleStreamDescription> availableSubtitleStreams() const = 0;

        /**
         * Returns the selected audio stream.
         */
        //virtual AudioStreamDescription currentAudioStream() const = 0;
        /**
         * Returns the selected video stream.
         */
        //virtual VideoStreamDescription currentVideoStream() const = 0;
        /**
         * Returns the selected subtitle stream.
         */
        //virtual SubtitleStreamDescription currentSubtitleStream() const = 0;

        /**
         * Selects one audio stream.
         *
         * \param stream description of the audio stream.
         *
         */
        //virtual void setCurrentAudioStream(const AudioStreamDescription &stream) = 0;
        /**
         * Selects one video stream.
         *
         * \param stream description of the video stream.
         */
        //virtual void setCurrentVideoStream(const VideoStreamDescription &stream) = 0;
        /**
         * Selects one subtitle stream. By default normally no subtitle is selected,
         * but depending on the media this may be different.
         *
         * \param stream description of the subtitle stream.
         */
        //virtual void setCurrentSubtitleStream(const SubtitleStreamDescription &stream) = 0;

        /**
         * Requests the playback to start.
         *
         * This method is only called if the state transition to \ref PlayingState is possible.
         *
         * The backend should react immediately
         * by either going into \ref PlayingState or \ref BufferingState if the
         * former is not possible.
         */
        virtual void play() = 0;

        /**
         * Requests the playback to pause.
         *
         * This method is only called if the state transition to \ref PausedState is possible.
         *
         * The backend should react as fast as possible. Go to \ref PausedState
         * as soon as playback is paused.
         */
        virtual void pause() = 0;

        /**
         * Requests the playback to be stopped.
         *
         * This method is only called if the state transition to \ref StoppedState is possible.
         *
         * The backend should react as fast as possible. Go to \ref StoppedState
         * as soon as playback is stopped.
         *
         * A subsequent call to play() will start playback at the beginning of
         * the media.
         */
        virtual void stop() = 0;

        /**
         * Requests the playback to be seeked to the given time.
         *
         * The backend does not have to finish seeking while in this function
         * (i.e. the backend does not need to block the thread until the seek is
         * finished; even worse it might lead to deadlocks when using a
         * ByteStream which gets its data from the thread this function would
         * block).
         *
         * As soon as the seek is done the currentTime() function and
         * the tick() signal will report it.
         *
         * \param milliseconds The time where playback should seek to in
         * milliseconds.
         */
        virtual void seek(qint64 milliseconds) = 0;

        /**
         * Return the time interval in milliseconds between two ticks.
         *
         * \returns Returns the tick interval that it was set to (might not
         * be the same as you asked for).
         */
        virtual qint32 tickInterval() const = 0;
        /**
         * Change the interval the tick signal is emitted. If you set \p
         * interval to 0 the signal gets disabled.
         *
         * \param interval tick interval in milliseconds
         *
         * \returns Returns the tick interval that it was set to (might not
         *          be the same as you asked for).
         */
        virtual void setTickInterval(qint32 interval) = 0;

        /**
         * Check whether the media data includes a video stream.
         *
         * \return returns \p true if the media contains video data
         */
        virtual bool hasVideo() const = 0;
        /**
         * If the current media may be seeked returns true.
         *
         * \returns whether the current media may be seeked.
         */
        virtual bool isSeekable() const = 0;
        /**
         * Get the current time (in milliseconds) of the file currently being played.
         */
        virtual qint64 currentTime() const = 0;
        /**
         * Get the current state.
         */
        virtual Phonon::State state() const = 0;

        /**
         * A translated string describing the error.
         */
        virtual QString errorString() const = 0;

        /**
         * Tells your program what to do about the error.
         *
         * \see Phonon::ErrorType
         */
        virtual Phonon::ErrorType errorType() const = 0;

        /**
         * Returns the total time of the media in milliseconds.
         *
         * If the total time is not know return -1. Do not block until it is
         * known, instead emit the totalTimeChanged signal as soon as the total
         * time is known or changes.
         */
        virtual qint64 totalTime() const = 0;

        /**
         * Returns the current source.
         */
        virtual MediaSource source() const = 0;

        /**
         * Sets the current source. When this function is called the MediaObject is
         * expected to stop all current activity and start loading the new
         * source (i.e. go into LoadingState).
         *
         * It is expected that the
         * backend now starts preloading the media data, filling the audio
         * and video buffers and making all media meta data available. It
         * will also trigger the totalTimeChanged signal.
         *
         * If the backend does not know how to handle the source it needs to
         * change state to Phonon::ErrorState. Don't bother about handling KIO
         * URLs. It is enough to handle AbstractMediaStream sources correctly.
         */
        virtual void setSource(const MediaSource &) = 0;

        /**
         * Sets the next source to be used for transitions. When a next source
         * is set playback should continue with the new source. In that case
         * finished and prefinishMarkReached are not emitted.
         *
         * \param source The source to transition to (crossfade/gapless/gap). If
         * \p source is an invalid MediaSource object then the queue is empty
         * and the playback should stop normally.
         */
        virtual void setNextSource(const MediaSource &source) = 0;

        virtual qint64 remainingTime() const { return totalTime() - currentTime(); }
        virtual qint32 prefinishMark() const = 0;
        virtual void setPrefinishMark(qint32) = 0;

        virtual qint32 transitionTime() const = 0;
        virtual void setTransitionTime(qint32) = 0;
};
}

Q_DECLARE_INTERFACE(Phonon::MediaObjectInterface, "MediaObjectInterface3.phonon.kde.org")

#endif // PHONON_MEDIAOBJECTINTERFACE_H
// vim: sw=4 ts=4 tw=80
