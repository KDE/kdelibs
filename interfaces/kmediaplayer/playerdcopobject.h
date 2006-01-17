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

#ifndef KMEDIAPLAYERPLAYERDCOPOBJECT_H
#define KMEDIAPLAYERPLAYERDCOPOBJECT_H

#include <dcopobject.h>
#include <kurl.h>

namespace KMediaPlayer
{

class KDE_EXPORT PlayerDCOPObject : public DCOPObject
{
K_DCOP

public:
	PlayerDCOPObject(void);

k_dcop:
	virtual bool openURL(const KUrl &file) = 0;
	virtual void pause() = 0;
	virtual void play() = 0;
	virtual void stop() = 0;

	virtual void seek(unsigned long msec) = 0;
	virtual bool isSeekable() const = 0;
	virtual unsigned long position() const = 0;

	virtual bool hasLength() const = 0;
	virtual unsigned long length() const = 0;

	virtual void setLooping(bool) = 0;
	virtual bool isLooping() const = 0;

	virtual int state() const = 0;
	virtual void setState(int) = 0;
};

}

#endif
