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

#include "pipebuffer.h"
#include <assert.h>

using namespace std;
using namespace Arts;

PipeSegment::PipeSegment(long size, void *buffer)
{
	this->buffer = new char[size];
	this->currentpos = this->buffer;
	memcpy(this->buffer,buffer,size);

	_remaining = size;
}

PipeSegment::~PipeSegment()
{
	delete[] buffer;
}

void *PipeSegment::data()
{
	return currentpos;
}

long PipeSegment::remaining()
{
	return _remaining;
}

void PipeSegment::skip(long count)
{
	_remaining -= count;
	currentpos += count;
	assert(_remaining >= 0);
}

PipeBuffer::PipeBuffer()
{
	_size = 0;
}

PipeBuffer::~PipeBuffer()
{
	clear();
}

void PipeBuffer::clear()
{
	while(segments.size() != 0)
	{
		PipeSegment *first = *segments.begin();
		delete first;

		segments.pop_front();
	}
	_size = 0;
}

void *PipeBuffer::peek(long size)
{
	while(!segments.empty())
	{
		PipeSegment *first = *segments.begin();
		if(size <= first->remaining())
			return first->data();
	}
	return 0;
}

void PipeBuffer::skip(long size)
{
	while(!segments.empty() && size > 0)
	{
		PipeSegment *first = *segments.begin();

		// if we have less data to skip than the first segment contains
		if(size < first->remaining())
		{
			// skip the data inside the segment

			_size -= size;
			first->skip(size);
			return;
		}
		else
		{
			// otherwise erase the first segment

			_size -= first->remaining();
			size -= first->remaining();

			delete first;
			segments.pop_front();
		}
	}
}

long PipeBuffer::read(long size, void *buffer)
{
	long readbytes = 0;
	char *bptr = (char *)buffer;

	while(!segments.empty() && size > 0)
	{
		PipeSegment *first = *segments.begin();
		long readCnt = size;
		if(readCnt > first->remaining()) readCnt = first->remaining();

		memcpy(bptr,first->data(),readCnt);

		first->skip(readCnt);
		size -= readCnt;
		bptr += readCnt;
		readbytes += readCnt;

		if(first->remaining() == 0)
		{
			delete first;
			segments.pop_front();
		}
	}

	_size -= readbytes;
	return readbytes;
}

long PipeBuffer::size()
{
	return _size;
}

void PipeBuffer::write(long size, void *buffer)
{
	segments.push_back(new PipeSegment(size,buffer));
	_size += size;
}

void PipeBuffer::unRead(long size, void *buffer)
{
	segments.push_front(new PipeSegment(size,buffer));
	_size += size;
}
