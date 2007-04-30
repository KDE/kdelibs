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

#ifndef PHONONNAMESPACE_H
#define PHONONNAMESPACE_H

#include "phonon_export.h"

/**
 * \brief The %KDE Multimedia classes
 *
 * In this Namespace you find the classes to access Multimedia functions for
 * audio and video playback. Those classes are not dependent
 * on any specific framework (like they were in pre KDE4 times) but rather use
 * exchangeable backends to do the work.
 *
 * If you want to write a new backend take a look at \ref phonon_backend_development_page.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace Phonon
{
    enum DiscType {
        NoDisc,
        Cd,
        Dvd,
        Vcd
    };

    enum MetaData {
        ArtistMetaData,
        AlbumMetaData,
        TitleMetaData,
        DateMetaData,
        GenreMetaData,
        TracknumberMetaData,
        DescriptionMetaData
    };

    /**
     * The state the media producing object is in at the moment.
     *
     * \see MediaProducer
     */
    enum State
    {
        /**
         * After construction it might take a while before the Player is
         * ready to play(). Normally this doesn't happen for local
         * files, but can happen for remote files where the asynchronous
         * mimetype detection and prebuffering can take a while.
         */
        LoadingState,
        /**
         * The Player has a valid media file loaded and is ready for
         * playing.
         */
        StoppedState,
        /**
         * The Player is playing a media file.
         */
        PlayingState,
        /**
         * The Player is waiting for data to be able to continue
         * playing.
         */
        BufferingState,
        /**
         * The Player is currently paused.
         */
        PausedState,
        /**
         * An unrecoverable error occurred. The Object is unusable in this state.
         */
        ErrorState
    };

    /**
     * Set's the category your program should be listed in in the mixer.
     *
     * A Jukebox will set this to Music, a VoIP program to Communication, a
     * DVD player to video, and so on.
     *
     * \note These categories can also become useful for an application that
     * controls the volumes automatically, like turning down the music when a call
     * comes in, or turning down the notifications when the media player knows
     * it's playing classical music.
     *
     * \see AudioOutput::setCategory
     */
    enum Category
    {
        /**
         * If the sounds produced are notifications (bing, beep and such) you
         * should use this category.
         */
        NotificationCategory = 0,
        /**
         * If your application is a music player (like a jukebox or media player
         * playing an audio file).
         */
        MusicCategory = 1,
        /**
         * If the sound is the audio channel of a video.
         */
        VideoCategory = 2,
        /**
         * If your applications produces sounds from communication with somebody
         * else (VoIP, voice chat).
         */
        CommunicationCategory = 3,
        /**
         * Sound produced by a computer game should go into this category.
         */
        GameCategory = 4,
        /**
         * Sounds produced for accessibility (e.g. Text-To-Speech)
         */
        AccessibilityCategory = 5,
        /**
         * \internal
         * Holds the largest value of categories.
         */
        LastCategory = AccessibilityCategory
    };

    /**
     * Tells your program how to recover from an error.
     *
     * \see MediaProducer::errorType()
     */
    enum ErrorType {
        /**
         * No error. MediaProducer::errorType() returns this if
         * MediaProducer::state() != Phonon::ErrorState.
         */
        NoError = 0,
        /**
         * Playback should work, and trying with another URL should work.
         */
        NormalError = 1,
        /**
         * Something important does not work. Your program cannot continue
         * playback or capture or whatever it was trying to do
         * without help from the user.
         */
        FatalError = 2
    };

    /**
     * Returns a (translated) string to show to the user identifying the given
     * Category.
     */
    PHONON_EXPORT QString categoryToString(Category c);
} //namespace Phonon

class kdbgstream;
#include <kdebug.h>
/**
 * Implements needed operator to use Phonon::State with kDebug
 */
inline PHONON_EXPORT kdbgstream &operator<<(kdbgstream  & stream, const Phonon::State state)
{
    switch(state)
    {
    case Phonon::ErrorState:
        stream << "ErrorState";
        break;
    case Phonon::LoadingState:
        stream << "LoadingState";
        break;
    case Phonon::StoppedState:
        stream << "StoppedState";
        break;
    case Phonon::PlayingState:
        stream << "PlayingState";
        break;
    case Phonon::BufferingState:
        stream << "BufferingState";
        break;
    case Phonon::PausedState:
        stream << "PausedState";
        break;
    }
    return stream;
}

#include <QMetaType>

Q_DECLARE_METATYPE(Phonon::State)
Q_DECLARE_METATYPE(Phonon::ErrorType)
Q_DECLARE_METATYPE(Phonon::Category)

// vim: sw=4 ts=4 tw=80
#endif // PHONONNAMESPACE_H
