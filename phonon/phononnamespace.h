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

#include <kdelibs_export.h>

/**
 * \brief The KDE Multimedia classes
 *
 * In this Namespace you find the classes to access Multimedia functions for
 * audio and video playback. Those classes are not dependent
 * on any specific framework (like they were in pre KDE4 times) but rather use
 * exchangeable backends to do the "dirty" work.
 *
 * If you want to write a new backend you also find the necessary classes in
 * Phonon::Ifaces.
 *
 * \author Matthias Kretz <kretz@kde.org>
 *
 * \see Phonon::Ui
 */
namespace Phonon
{
	/**
	 * The state the media producing object is in at the moment.
	 *
	 * \see AbstractMediaProducer
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
		 * No application should use that category - it's the default, though.
		 */
		UnspecifiedCategory,
		/**
		 * If the sounds produced are notifications (bing, beep and such) you
		 * should use this category.
		 */
		NotificationCategory,
		/**
		 * If your application is a music player (like a jukebox or media player
		 * playing an audio file).
		 */
		MusicCategory,
		/**
		 * If the sound is the audio channel of a video.
		 */
		VideoCategory,
		/**
		 * If your applications produces sounds from communication with somebody
		 * else (VoIP, voice chat).
		 */
		CommunicationCategory,
		/**
		 * Sound produced by a computer game should go into this category.
		 */
		GameCategory
	};

	/**
	 * Returns a (translated) string to show to the user identifying the given
	 * Category.
	 */
	QString PHONONCORE_EXPORT categoryToString( Category c );
} //namespace Phonon

class kdbgstream;
#include <kdebug.h>
/**
 * Implements needed operator to use Phonon::State with kDebug
 */
inline PHONONCORE_EXPORT kdbgstream& operator<<( kdbgstream & stream, const Phonon::State state )
{
	switch( state )
	{
		case Phonon::ErrorState:
			stream << "Error";
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

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONONNAMESPACE_H
