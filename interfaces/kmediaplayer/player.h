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

namespace KMediaPlayer
{

class Player : public KParts::ReadOnlyPart, public PlayerDCOPObject
{
Q_OBJECT

public:
	// if useGUI is true, then this is a KMediaPlayer/Player (needs a widget)
	// else it is a KMediaPlayer/Engine (no widget needed)
	Player(QObject *parent, const char *name, bool useGUI = true);
	virtual ~Player(void);

	virtual View *view(void) = 0;

public slots:
	virtual void pause(void) = 0;
	virtual void play(void) = 0;
	virtual void stop(void) = 0;

	virtual void seek(unsigned long msec) = 0;
public:
	virtual bool isSeekable(void) const = 0;
	virtual unsigned long position(void) const = 0;
	virtual bool hasLength(void) const = 0;
	virtual unsigned long length(void) const = 0;

public slots:
	void setLooping(bool);
public:
	bool isLooping(void) const;
signals:
	void loopingChanged(bool);

public:
	enum State { Empty, Stop, Pause, Play };
	int state(void) const;
signals:
	void stateChanged(int);

protected slots:
	void setState(int);

private:
	bool currentLooping;
	State currentState;

	struct Data;
	Data *d;
};

}

#endif
