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

#include <iostream.h>
#include <algorithm>
#include "artsc.h"
#include "soundserver.h"
#include "stdsynthmodule.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <cassert>

#define arts_backend_debug(x) ;

using namespace std;


class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule
{
	SimpleSoundServer server;

	/*
	 * FIXME: bsWrapper is a more or less ugly trick to be able to use
	 * this object although not using smartwrappers to access it
	 */
	ByteSoundProducer bsWrapper;

	bool _finished, isAttached;
	int _samplingRate, _bits, _channels, pos;
	string _name;
	queue< DataPacket<mcopbyte>* > outqueue;
public:
	Sender(SimpleSoundServer server, int rate, int bits, int channels,
		string name) : server(server), _finished(false), isAttached(false),
		_samplingRate(rate), _bits(bits), _channels(channels), pos(0),
		_name(name)
	{
		bsWrapper = this;
	}
	~Sender()
	{
		//
	}
	long samplingRate() { return _samplingRate; }
	long channels()     { return _channels; }
	long bits()         { return _bits; }
	bool finished()     { return _finished; }

	static const int packetCount = 10, packetCapacity = 4096;
	void streamStart()
	{
		/*
		 * start streaming
		 */
		outdata.setPull(packetCount, packetCapacity);
	}

	void request_outdata(DataPacket<mcopbyte> *packet)
	{
		outqueue.push(packet);
	}

	void close()
	{
		if(isAttached)
		{
			if(pos != 0)
			{
				/* send the last half-filled packet */
				DataPacket<mcopbyte> *packet = outqueue.front();
				packet->size = packetCapacity;
				packet->send();
				outqueue.pop();
			}
			server.detach(bsWrapper);
		}
		bsWrapper = 0;		// similar effect like "delete this;"
	}

	int write(mcopbyte *data, int size)
	{
		if(!isAttached)
		{
			isAttached = true;

			server.attach(bsWrapper);
			start();
			// no blocking yet
			Dispatcher::the()->ioManager()->processOneEvent(false);
		}

		int remaining = size;
		while(remaining)
		{
			/* C API blocking style write */
			while(outqueue.empty())
				Dispatcher::the()->ioManager()->processOneEvent(true);

			/* get a packet */
			DataPacket<mcopbyte> *packet = outqueue.front();

			/* copy some data there */
			int tocopy = min(remaining,packetCapacity);
			memcpy(&packet->contents[pos],data,tocopy);
			pos += tocopy;
			data += tocopy;
			remaining -= tocopy;

			/* have we filled up the packet? then send it */
			if(pos == packetCapacity)
			{
				packet->size = packetCapacity;
				packet->send();
				outqueue.pop();
				pos = 0;
			}
		}

#if 0
		/* could certainly be optimized with memcpy and such */
		mcopbyte *enddata = data+size;
		while(data != enddata)
		{
			/* C API blocking style write */
			while(outqueue.empty())
				Dispatcher::the()->ioManager()->processOneEvent(true);

			DataPacket<mcopbyte> *packet = outqueue.front();
			packet->contents[pos++] = *data++;
			if(pos == packetCapacity)
			{
				packet->size = packetCapacity;
				packet->send();
				outqueue.pop();
				pos = 0;
			}
		}
#endif
		/* no possible error conditions */
		return size;
	}
};

class ArtsCApi {
protected:
	static ArtsCApi *instance;
	int refcnt;

	Dispatcher dispatcher;
	SimpleSoundServer server;

	ArtsCApi() : refcnt(1), server(Reference("global:Arts_SimpleSoundServer"))
	{
		//
	}

public:
// C Api commands
	int init() {
		if(server.isNull())
			return ARTS_E_NOSERVER;

		return 0;
	}

	void free() {
		// nothing to do
	}

	arts_stream_t play_stream(int rate, int bits, int channels, char *name)
	{
		if(server.isNull())
			return 0;

		return (arts_stream_t)new Sender(server,rate,bits,channels,name);
	}

	void close_stream(arts_stream_t stream)
	{
		if(server.isNull())
			return;

		Sender *sender = (Sender *)stream;
		if(!sender) return;

		sender->close();
	}

	int write(arts_stream_t stream, void *data, int size)
	{
		if(server.isNull())
			return ARTS_E_NOSERVER;

		if(!stream)
			return ARTS_E_NOSTREAM;

		Sender *sender = (Sender *)stream;
		return sender->write((mcopbyte *)data,size);
	}

// allocation and freeing of the class
	static ArtsCApi *the() {
		return instance;
	}
	static void ref() {
		if(!instance)
			instance = new ArtsCApi();
		else
			instance->refcnt++;
	}
	static void release() {
		assert(instance);
		assert(instance->refcnt > 0);
		instance->refcnt--;
		if(instance->refcnt == 0)
		{
			delete instance;
			instance = 0;
		}
	}
};

//----------------------------- static members -------------------------------

ArtsCApi *ArtsCApi::instance = 0;

//------------------ wrappers from C to C++ class ----------------------------

extern "C" int arts_backend_init()
{
	arts_backend_debug("arts_backend_init");
	ArtsCApi::ref();

	// if init fails, don't expect free, and don't expect that the user
	// continues using other API functions
	int rc = ArtsCApi::the()->init();
	if(rc < 0) ArtsCApi::release();
	return rc;
}

extern "C" void arts_backend_free()
{
	if(!ArtsCApi::the()) return;

	arts_backend_debug("arts_backend_free");
	ArtsCApi::the()->free();
	ArtsCApi::release();
}

extern "C" arts_stream_t arts_backend_play_stream(int rate, int bits, int channels, char *name)
{
	if(!ArtsCApi::the()) return 0;

	arts_backend_debug("arts_backend_play_stream");
	return ArtsCApi::the()->play_stream(rate,bits,channels,name);
}

extern "C"arts_stream_t arts_backend_record_stream(int, int, int, char *)
{
	if(!ArtsCApi::the()) return 0;

	arts_backend_debug("arts_backend_record_stream");
	return 0;
}

extern "C" void arts_backend_close_stream(arts_stream_t stream)
{
	if(!ArtsCApi::the()) return;

	arts_backend_debug("arts_backend_close_stream");
	return ArtsCApi::the()->close_stream(stream);
}

extern "C"int arts_backend_read(arts_stream_t, void *, int)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_backend_read");
	return ARTS_E_NOIMPL;
}

extern "C" int arts_backend_write(arts_stream_t stream, void *buffer, int count)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_backend_write");
	return ArtsCApi::the()->write(stream,buffer,count);
}
