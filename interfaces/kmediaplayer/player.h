// Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Except as contained in this notice, the name(s) of the author(s) shall not be
// used in advertising or otherwise to promote the sale, use or other dealings
// in this Software without prior written authorization from the author(s).

#ifndef KMEDIAPLAYERPLAYER_H
#define KMEDIAPLAYERPLAYER_H

#include <kparts/part.h>
#include <kmediaplayer/playerdcopobject.h>
#include <kmediaplayer/view.h>

/** KMediaPlayer contains an interface to reusable media player components.
*/
namespace KMediaPlayer
{

/** Player is the center of the KMediaPlayer interface.   It provides all of
 * the necessary media player operations, and optionally provides the GUI to
 * control them.
 *
 * There are two servicetypes for Player:  KMediaPlayer/Player and
 * KMediaPlayer/Engine.  KMediaPlayer/Player provides a widget (accessable
 * through @ref view as well as XML GUI KActions.  KMediaPlayer/Engine omits
 * the user interface facets, for those who wish to provide their own
 * interface.
 */
class Player : public KParts::ReadOnlyPart, public PlayerDCOPObject
{
Q_OBJECT

public:
	/** This constructor is what to use when no GUI is required, as in the
	 * case of a KMediaPlayer/Engine.
	 */
	Player(QObject *parent, const char *name);

	/** This constructor is what to use when a GUI is required, as in the
	 * case of a KMediaPlayer/Player.
	 */
	Player(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name);

	virtual ~Player(void);

	/** A convenience function returning a pointer to the @ref View for this
	 * Player, or 0 if this Player has no GUI.
	 */
	virtual View *view(void) = 0;

public slots:
	/** Pause playback of the media track.*/
	virtual void pause(void) = 0;

	/** Begin playing the media track.*/
	virtual void play(void) = 0;

	/** Stop playback of the media track and return to the beginning.*/
	virtual void stop(void) = 0;

	/** Move the current playback position to the specified time in
	 * milliseconds, if the track is seekable.  Some streams may not be
	 * seeked.
	 */
	virtual void seek(unsigned long msec) = 0;
public:
	/** Returns whether the current track honors seek requests.*/
	virtual bool isSeekable(void) const = 0;

	/** Returns the current playback position in the track.*/
	virtual unsigned long position(void) const = 0;

	/** Returns whether the current track has a length.  Some streams are
	 * endless, and do not have one. */
	virtual bool hasLength(void) const = 0;

	/** Returns the length of the current track.*/
	virtual unsigned long length(void) const = 0;

public slots:
	/** Set whether the Player should continue playing at the beginning of
	 * the track when the end of the track is reached.
	 */
	void setLooping(bool);
public:
	/** Return the current looping state. */
	bool isLooping(void) const;
signals:
	/** Emitted when the looping state is changed. */
	void loopingChanged(bool);

public:
	/** The possible states of the Player */
	enum State
	{
		/** No track is loaded. */
		Empty,
		/** Not playing. */
		Stop,
		/** Playing is temporarily suspended. */
		Pause,
		/** The media is currently being output. */
		Play
	};
	/** Return the current state of the player. */
	int state(void) const;
signals:
	/** Emitted when the state changes. */
	void stateChanged(int);

protected slots:
	/** Implementers use this to control what users see as the current
	 * state.*/
	void setState(int);

private:
	bool currentLooping;
	State currentState;

	struct Data;
	Data *d;
};

}

#endif
