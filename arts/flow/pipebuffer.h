    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef __PIPEBUFFER_H__
#define __PIPEBUFFER_H__

/*
 * BC - Status (2000-09-30): PipeSegment, PipeBuffer
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of libartsflow's
 * AudioSubSystem, and subject to change/disappearing due to optimization
 * work.
 */


#include <list>
namespace Arts {

class PipeSegment {
	long _remaining;
	char *currentpos, *buffer;
public:
	PipeSegment(long size, void *buffer);
	~PipeSegment();

	void *data();
	long remaining();
	void skip(long count);
};

class PipeBuffer {
protected:
	std::list<PipeSegment *> segments;
	long _size;

public:
	PipeBuffer();
	~PipeBuffer();

	// reading
	void *peek(long size);
	void skip(long size);
	long read(long size, void *buffer);
	void unRead(long size, void *buffer);

	// writing
	void write(long size, void *buffer);
	void clear();

	// status
	long size();
};
};

#endif
