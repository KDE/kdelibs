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

#include <kmediaplayer/player.h>

KMediaPlayer::PlayerDCOPObject::PlayerDCOPObject(void)
	: DCOPObject("KMediaPlayer")
{
}

KMediaPlayer::Player::Player(QWidget *, const char *, QObject *parent)
	: KParts::ReadOnlyPart(parent)
	, currentLooping(false)
	, currentState(Empty)
	, d(0)
{
}

KMediaPlayer::Player::Player(QObject *parent )
	: KParts::ReadOnlyPart(parent)
	, currentLooping(false)
	, currentState(Empty)
	, d(0)
{
}

KMediaPlayer::Player::~Player(void)
{
}

void KMediaPlayer::Player::setLooping(bool b)
{
	if(b != currentLooping)
	{
		currentLooping = b;
		emit loopingChanged(b);
	}
}

bool KMediaPlayer::Player::isLooping(void) const
{
	return currentLooping;
}

void KMediaPlayer::Player::setState(int s)
{
	if(s != currentState)
	{
		currentState = (State)s;
		emit stateChanged(s);
	}
}

int KMediaPlayer::Player::state(void) const
{
	return (int)currentState;
}

#include "player.moc"
