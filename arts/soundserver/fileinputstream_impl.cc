    /*

    Copyright (C) 2001 Stefan Westerfeld
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

#include "kmedia2.h"
#include "stdsynthmodule.h"
#include "debug.h"

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

using namespace std;
using namespace Arts;

namespace Arts {
class FileInputStream_impl : virtual public FileInputStream_skel,
							 virtual public StdSynthModule
{
protected:
	string _filename;
	int age, fd;
	unsigned int _size, position;
	mcopbyte *data;
	queue< DataPacket<mcopbyte>* > wqueue;

public:
	static const unsigned int PACKET_COUNT;
	static const unsigned int PACKET_SIZE;

	FileInputStream_impl()
	{
		fd = -1;
		position = 0;
		data = 0;
		age = 0;
		_size = 0;
	}
	~FileInputStream_impl()
	{
		arts_assert(wqueue.size() == 0);

		close();
	}

	void close()
	{
		if(data != 0)
		{
			munmap((char*)data, _size);
			data = 0;
		}

		if(fd >= 0)
		{
			::close(fd);
			fd = -1;
		}
	}

	bool open(const string& filename)
	{
		close();

		fd = ::open(filename.c_str(), O_RDONLY);
		if(fd < 0)
			return false;

		_size = lseek(fd, 0, SEEK_END);
    	lseek(fd, 0, SEEK_SET);

    	data = (mcopbyte *)mmap(0, _size, PROT_READ, MAP_SHARED, fd, 0);
    	if(data == 0)
		{
			close();
			return false;
		}

		position = 0;
		if(_filename != filename)
		{
			_filename = filename;
			filename_changed(filename);
		}
		return true;
	}

	string filename() { return _filename; }
	void filename(const string& newfilename) { open(newfilename); }

	long size()				{ return _size; }
	bool eof()
	{
		return (fd < 0 || position >= _size)
			&& (wqueue.size() == PACKET_COUNT);
	}
	bool seekOk()			{ return true; }

	long seek(long newPosition)
	{
		arts_return_val_if_fail(fd < 0, -1);
		arts_return_val_if_fail(newPosition < 0, -1);
		arts_return_val_if_fail(newPosition > (long)_size, -1);

		long ageBeforeSeek = age;
		position = newPosition;

		processQueue();
		return ageBeforeSeek;
	}
	
	void processQueue()
	{
		unsigned int qsize = wqueue.size();

		for(unsigned int i = 0; i < qsize; i++)
		{
			if(position < _size)
			{
				DataPacket<mcopbyte> *packet = wqueue.front();
				wqueue.pop();

				packet->size = min(PACKET_SIZE, _size - position);
				memcpy(packet->contents, data+position, packet->size);
				age += packet->size;
				position += packet->size;
				packet->send();
			}
		}
	}

	void request_outdata(DataPacket<mcopbyte> *packet)
	{
		wqueue.push(packet);
		processQueue();
	}

	void streamStart()
	{
		/*
		 * start streaming
		 */
		outdata.setPull(PACKET_COUNT, PACKET_SIZE);
	}

	void streamEnd()
	{
		/*
		 * end streaming
		 */
		outdata.endPull();

		while(!wqueue.empty())
		{
			DataPacket<mcopbyte> *packet = wqueue.front();
			packet->size = 0;
			packet->send();
			wqueue.pop();
		}
	}
};

REGISTER_IMPLEMENTATION(FileInputStream_impl);

const unsigned int
FileInputStream_impl::PACKET_COUNT = 8;  

const unsigned int
FileInputStream_impl::PACKET_SIZE = 8192;

};
