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
#ifndef Phonon_MEDIAOBJECT_H
#define Phonon_MEDIAOBJECT_H

#include "phonon_export.h"
#include "phonondefs.h"
#include "phononnamespace.h"
#include "mediasource.h"

namespace Solid
{
    class Device;
} // namespace Solid

namespace Phonon
{
    class VideoPath;
    class AudioPath;
    class MediaObjectPrivate;
    class KioFallbackImpl;
    class MediaSource;

    /**
     * @short Common interface for all media sources.
     *
     * This class holds common functionality of all objects that produce a media
     * stream, like MediaObject or SoundcardCapture.
     *
     * Some functions of this class (and its subclasses) are asynchronous.
     * That means if you call play() the object only starts playing when the
     * stateChanged() signal tells you that the object changed into \ref PlayingState.
     * The states you can expect are documented for those methods.
     *
     * @author Matthias Kretz <kretz@kde.org>
     * @see MediaObject
     */
    /**
     * \short Interface for media playback of a given URL.
     *
     * This class is the most important class for most mediaplayback tasks. With
     * this class you open a URL, play, pause, stop, seek and get a lot of
     * information about the media data.
     *
     * A common usage example is the following:
     * \code
     * media = new MediaObject(this);
     * connect(media, SIGNAL(finished()), SLOT(slotFinished());
     * media->setUrl(KUrl("/home/username/music/filename.ogg"));
     * media->play();
     * \endcode
     *
     * \ingroup Playback
     * \author Matthias Kretz <kretz@kde.org>
     */
    class PHONON_EXPORT MediaObject : public QObject
    {
        friend class MediaQueue;
        friend class KioFallbackImpl;
        friend class FrontendInterfacePrivate;
        Q_OBJECT
        K_DECLARE_PRIVATE(MediaObject)
        PHONON_OBJECT(MediaObject)
        Q_PROPERTY(qint32 prefinishMark READ prefinishMark WRITE setPrefinishMark)
        /**
         * \brief The time interval in milliseconds between two ticks.
         *
         * The %tick interval is the time that elapses between the emission of two tick signals.
         * If you set the interval to @c 0 the tick signal gets disabled. The %tick
         * interval defaults to @c 0 (disabled).
         *
         * @warning The back-end is free to choose a different tick interval close
         * to what you asked for. This means that the following code @em may @em fail:
         * \code
         * int x = 200;
         * producer->setTickInterval(x);
         * Q_ASSERT(x == producer->tickInterval());
         * \endcode
         * On the other hand the following is guaranteed:
         * \code
         * int x = 200;
         * producer->setTickInterval(x);
         * Q_ASSERT(x >= producer->tickInterval() && x <= 2 *producer->tickInterval());
         * \endcode
         *
         * @see tick
         */
        Q_PROPERTY(qint32 tickInterval READ tickInterval WRITE setTickInterval)
        public:
            /**
             * Destroys the MediaObject.
             */
            ~MediaObject();

            /**
             * Add a VideoPath to process the video data of this media.
             *
             * The interface allows for a VideoPath to be added to multiple
             * MediaObject objects and multiple VideoPath objects to
             * be added to an MediaObject object.
             *
             * @warning Backends don't have to be able to add multiple
             * VideoPaths or use a VideoPath for multiple MediaObject
             * objects. If in doubt take care of the return value.
             *
             * @param videoPath The VideoPath that should process the video
             * data.
             *
             * @return @c true if the VideoPath was successfully added
             * @return @c false if the VideoPath could not be added. This can
             * happen if the backend does not support multiple paths per
             * MediaObject or VideoPath for multiple
             * MediaObject.
             *
             * @see videoPaths
             */
            bool addVideoPath(VideoPath *videoPath);

            /**
             * Add an AudioPath to process the audio data of this media.
             *
             * The interface allows for an AudioPath to be added to multiple
             * MediaObject objects and multiple AudioPath objects to
             * be added to an MediaObject object.
             *
             * @warning Backends don't have to be able to add multiple
             * AudioPaths or use a AudioPath for multiple MediaObject
             * objects. If in doubt take care of the return value.
             *
             * @param audioPath The AudioPath that should process the audio
             * data.
             *
             * @return @c true if the AudioPath was successfully added
             * @return @c false if the AudioPath could not be added. This can
             * happen if the backend does not support multiple paths per
             * MediaObject or AudioPath for multiple
             * MediaObject.
             *
             * @see audioPaths
             */
            bool addAudioPath(AudioPath *audioPath);

            bool removeVideoPath(VideoPath *videoPath);
            bool removeAudioPath(AudioPath *audioPath);

            /**
             * Get the current state.
             *
             * @return The state of the object.
             *
             * @see State
             */
            State state() const;

            /**
             * Check whether the media data includes a video stream.
             *
             * @return Returns @c true if the media contains video data.
             */
            bool hasVideo() const;

            /**
             * If the current media may be seeked returns true.
             *
             * @return Whether the current media may be seeked.
             *
             * @see seekableChanged()
             */
            bool isSeekable() const;

            qint32 tickInterval() const;

            /**
             * Returns the list of all connected VideoPath objects.
             *
             * @return A list of all connected VideoPath objects.
             *
             * @see addVideoPath
             */
            QList<VideoPath *> videoPaths() const;

            /**
             * Returns the list of all connected AudioPath objects.
             *
             * @return A list of all connected AudioPath objects.
             *
             * @see addAudioPath
             */
            QList<AudioPath *> audioPaths() const;

            /**
             * Returns the selected audio stream for the given AudioPath object.
             *
             * \param audioPath If 0 the default audio stream is returned. Else
             * the audio stream for the given AudioPath object is returned.
             *
             * \see availableAudioStreams
             * \see selectAudioStream
             */
            QString currentAudioStream(AudioPath *audioPath = 0) const;

            /**
             * Returns the selected video stream for the given VideoPath object.
             *
             * \param videoPath If 0 the default video stream is returned. Else
             * the video stream for the given VideoPath object is returned.
             *
             * \see availableVideoStreams
             * \see selectVideoStream
             */
            QString currentVideoStream(VideoPath *videoPath = 0) const;

            /**
             * Returns the selected subtitle stream for the given VideoPath object.
             *
             * \param videoPath If 0 the default subtitle stream is returned. Else
             * the subtitle stream for the given VideoPath object is returned.
             *
             * \see availableSubtitleStreams
             * \see selectSubtitleStream
             */
            QString currentSubtitleStream(VideoPath *videoPath = 0) const;

            /**
             * Returns the audio streams that can be selected by the user. The
             * strings can directly be used in the user interface.
             *
             * \see selectedAudioStream
             * \see selectAudioStream
             */
            QStringList availableAudioStreams() const;

            /**
             * Returns the video streams that can be selected by the user. The
             * strings can directly be used in the user interface.
             *
             * \see selectedVideoStream
             * \see selectVideoStream
             */
            QStringList availableVideoStreams() const;

            /**
             * Returns the subtitle streams that can be selected by the user. The
             * strings can directly be used in the user interface.
             *
             * \see selectedSubtitleStream
             * \see selectSubtitleStream
             */
            QStringList availableSubtitleStreams() const;

            /**
             * Returns the strings associated with the given \p key.
             *
             * Backends should use the keys specified in the Ogg Vorbis
             * documentation: http://xiph.org/vorbis/doc/v-comment.html
             *
             * Therefore the following should work with every backend:
             *
             * A typical usage looks like this:
             * \code
             * setMetaArtist (media->metaData("ARTIST"    ));
             * setMetaAlbum  (media->metaData("ALBUM"     ));
             * setMetaTitle  (media->metaData("TITLE"     ));
             * setMetaDate   (media->metaData("DATE"      ));
             * setMetaGenre  (media->metaData("GENRE"     ));
             * setMetaTrack  (media->metaData("TRACKNUMBER"));
             * setMetaComment(media->metaData("DESCRIPTION"));
             * \endcode
             */
            QStringList metaData(const QString &key) const;

            QStringList metaData(Phonon::MetaData) const;

            /**
             * Returns all meta data.
             */
            QMultiMap<QString, QString> metaData() const;

            /**
             * A translated string describing the error.
             */
            QString errorString() const;

            /**
             * Tells your program what to do about the error.
             *
             * \see Phonon::ErrorType
             */
            ErrorType errorType() const;

            /**
             * Returns the current media source.
             *
             * \see setCurrentSource
             */
            MediaSource currentSource() const;

            /**
             * Set the media source the MediaObject should use.
             *
             * \param source The MediaSource object to the media data. You can
             * just as well use a QUrl or QString (for a local file) here.
             *
             * \code
             * QUrl url("http://www.example.com/music.ogg");
             * mediaObject->setCurrentSource(url);
             * \endcode
             *
             * \see currentSource
             */
            void setCurrentSource(const MediaSource &source);

            /**
             * Returns the queued media sources. This does list does not include
             * the current source (returned by \ref currentSource).
             */
            QList<MediaSource> queue() const;

            void setQueue(const QList<MediaSource> &sources);
            void setQueue(const QList<QUrl> &urls);
            void enqueue(const MediaSource &source);
            void enqueue(const QList<MediaSource> &sources);
            void enqueue(const QList<QUrl> &urls);
            void clearQueue();


            /**
             * Get the current time (in milliseconds) of the file currently being played.
             *
             * @return The current time (in milliseconds)
             */
            qint64 currentTime() const;
            /**
             * Get the total time (in milliseconds) of the file currently being played.
             *
             * \see totalTimeChanged
             */
            qint64 totalTime() const;
            /**
             * Get the remaining time (in milliseconds) of the file currently being played.
             */
            qint64 remainingTime() const;

            /**
             * Returns the time in milliseconds the aboutToFinish signal is
             * emitted before the playback finishes. Defaults to 0.
             *
             * \return The aboutToFinish time in milliseconds.
             *
             * \see setPrefinishMark
             * \see prefinishMarkReached
             */
            qint32 prefinishMark() const;

            qint32 transitionTime() const;
            /**
             * positive: gap
             * 0: gapless
             * negative: crossfade
             *
             * in msec
             */
            void setTransitionTime(qint32 msec);

        public Q_SLOTS:
            /**
             * Selects an audio stream from the media.
             *
             * Some media formats allow multiple audio streams to be stored in
             * the same file. Normally only one should be played back. You can
             * select the stream for all connected AudioPath objects or per
             * AudioPath object. The latter allows to play, two or more
             * different audio streams simultaneously.
             *
             * \param streamName one string out of the list returned by availableAudioStreams()
             * \param audioPath the AudioPath object for which the audio stream
             * selection should be used
             *
             * \see availableAudioStreams()
             * \see selectedAudioStream()
             */
            void setCurrentAudioStream(const QString &streamName, AudioPath *audioPath = 0);

            /**
             * Selects a video stream from the media.
             *
             * Some media formats allow multiple video streams to be stored in
             * the same file. Normally only one should be played back. You can
             * select the stream for all connected VideoPath objects or per
             * VideoPath object. The latter allows to play, two or more
             * different video streams simultaneously.
             *
             * \param streamName one string out of the list returned by availableVideoStreams()
             * \param videoPath the VideoPath object for which the video stream
             * selection should be used
             *
             * \see availableVideoStreams()
             * \see selectedVideoStream()
             */
            void setCurrentVideoStream(const QString &streamName, VideoPath *videoPath = 0);

            /**
             * Selects a subtitle stream from the media.
             *
             * Some media formats allow multiple subtitle streams to be stored in
             * the same file. Normally only one should be displayed. You can
             * select the stream for all connected VideoPath objects or per
             * VideoPath object. The latter allows to display, two or more
             * different subtitle streams simultaneously.
             *
             * \param streamName One string out of the list returned by
             * availableSubtitleStreams(); if a \c null QString is passed no
             * subtitles will be displayed.
             * \param videoPath the VideoPath object for which the subtitle stream
             * selection should be used
             *
             * \see availableSubtitleStreams()
             * \see selectedSubtitleStream()
             */
            void setCurrentSubtitleStream(const QString &streamName, VideoPath *videoPath = 0);

            void setTickInterval(qint32 newTickInterval);

            /**
             * Requests playback of the media data to start. Playback only
             * starts when stateChanged() signals that it goes into \ref PlayingState,
             * though.
             *
             * \par Possible states right after this call:
             * \li \ref BufferingState
             * \li \ref PlayingState
             * \li (\ref ErrorState)
             */
            void play();

            /**
             * Requests playback to pause. If it was paused before nothing changes.
             *
             * \par Possible states right after this call:
             * \li \ref PlayingState
             * \li \ref PausedState
             * \li (\ref ErrorState)
             */
            void pause();

            /**
             * Requests playback to stop. If it was stopped before nothing changes.
             *
             * \par Possible states right after this call:
             * \li the state it was in before (e.g. \ref PlayingState)
             * \li \ref StoppedState
             * \li (\ref ErrorState)
             */
            void stop();

            /**
             * Requests a seek to the time indicated.
             *
             * You can only seek if state() == \ref PlayingState, \ref BufferingState or \ref PausedState.
             *
             * The call is asynchronous, so currentTime can still be the old
             * value right after this method was called. If all you need is a
             * slider that shows the current position and allows the user to
             * seek use the class SeekSlider.
             *
             * @param time The time in milliseconds where to continue playing.
             *
             * \par Possible states right after this call:
             * \li \ref BufferingState
             * \li \ref PlayingState
             * \li (\ref ErrorState)
             *
             * \see SeekSlider
             */
            void seek(qint64 time);

            /**
             * Sets the time in milliseconds the aboutToFinish signal is emitted
             * before the playback finishes. Defaults to 0.
             *
             * \param newAboutToFinishTime The new aboutToFinishTime in
             * milliseconds
             *
             * \see aboutToFinishTime
             * \see aboutToFinish
             */
            void setPrefinishMark(qint32 msecToEnd);

        Q_SIGNALS:
            /**
             * Emitted when the state of the MediaObject has changed.
             * In case you're not interested in the old state you can also
             * connect to a slot that only has one \ref State argument.
             *
             * @param newstate The state the Player is in now.
             * @param oldstate The state the Player was in before.
             */
            void stateChanged(Phonon::State newstate, Phonon::State oldstate);

            /**
             * This signal gets emitted every tickInterval milliseconds.
             *
             * @param time The position of the media file in milliseconds.
             *
             * @see setTickInterval, tickInterval
             */
            void tick(qint64 time);

            /**
             * This signal is emitted whenever the audio/video data that is
             * being played is associated with new meta data. E.g. for radio
             * streams this happens when the next song is played.
             *
             * You can get the new meta data with the metaData methods.
             */
            void metaDataChanged();

            /**
             * Emitted whenever the return value of isSeekable() changes.
             *
             * Normally you'll check isSeekable() first and then let this signal
             * tell you whether seeking is possible now or not. That way you
             * don't have to poll isSeekable().
             *
             * \param isSeekable \p true  if the stream is seekable (i.e. calling
             *                            seek() works)
             *                   \p false if the stream is not seekable (i.e.
             *                            all calls to seek() will be ignored)
             */
            void seekableChanged(bool isSeekable);

            /**
             * Emitted whenever the return value of hasVideo() changes.
             *
             * Normally you'll check hasVideo() first and then let this signal
             * tell you whether video is available now or not. That way you
             * don't have to poll hasVideo().
             *
             * \param hasVideo \p true  The stream contains video and adding a
             *                          VideoWidget will show a video.
             *                 \p false There is no video data in the stream and
             *                          adding a VideoWidget will show an empty (black)
             *                          VideoWidget.
             */
            void hasVideoChanged(bool hasVideo);

            /**
             * Tells about the status of the buffer.
             *
             * You can use this signal to show a progress bar to the user when
             * in BufferingState:
             *
             * \code
             * progressBar->setRange(0, 100); // this is the default
             * connect(mediaObject, SIGNAL(bufferStatus(int)), progressBar, SLOT(setValue(int)));
             * \endcode
             *
             * \param percentFilled A number between 0 and 100 telling you how
             *                      much the buffer is filled.
             */ // other names: bufferingProgress
            void bufferStatus(int percentFilled);

            /**
             * Emitted when the object has finished playback.
             * It is not emitted if you call stop(), pause() or
             * load(), but only on end-of-file or a critical error.
             *
             * \see currentSourceChanged
             * \see aboutToFinish
             * \see prefinishMarkReached
             */
            void finished();

            /**
             * Emitted when the MediaObject makes a transition to the next
             * MediaSource in the \ref queue.
             *
             * In other words, it is emitted when an individual MediaSource is
             * finished.
             *
             * \param newSource The source that starts to play at the time the
             * signal is emitted.
             */
            void currentSourceChanged(const MediaSource &newSource);

            /**
             * Emitted before the playback of the whole queue stops. When this
             * signal is emitted you still have time to provide the next
             * MediaSource so that playback continues.
             *
             * This signal can be used to provide the next MediaSource just in
             * time for the transition still to work.
             */
            void aboutToFinish(); // aboutToStopConcurrentPlaybackButYouStillHaveTimeToProvideANewSource();

            /**
             * Emitted when there are only \p msecToEnd milliseconds left
             * for playback.
             *
             * \param msecToEnd The remaining time until the playback queue finishes.
             *
             * \see setPrefinishMark
             * \see prefinishMark
             * \see aboutToFinish
             * \see finished
             */
            void prefinishMarkReached(qint32 msecToEnd);

            /**
             * This signal is emitted as soon as the total time of the media file is
             * known or has changed. For most non-local media data the total
             * time of the media can only be known after some time. At that time the
             * totalTime function can not return useful information. You have
             * to wait for this signal to know the real total time.
             *
             * \param newTotalTime The length of the media file in milliseconds.
             *
             * \see totalTime
             */
            void totalTimeChanged(qint64 newTotalTime);

        protected:
            //MediaObject(Phonon::MediaObjectPrivate &dd, QObject *parent);

        private:
            Q_PRIVATE_SLOT(k_func(), void _k_resumePlay())
            Q_PRIVATE_SLOT(k_func(), void _k_resumePause())
            Q_PRIVATE_SLOT(k_func(), void _k_metaDataChanged(const QMultiMap<QString, QString> &))
            Q_PRIVATE_SLOT(k_func(), void _k_stateChanged(Phonon::State, Phonon::State))
            Q_PRIVATE_SLOT(k_func(), void _k_aboutToFinish())
    };
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_MEDIAOBJECT_H
