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
#include <assert.h>

#define arts_backend_debug(x) ;

using namespace std;
using namespace Arts;


class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule
{
	SoundServer server;
	float serverBufferTime;

	/*
	 * FIXME: bsWrapper is a more or less ugly trick to be able to use
	 * this object although not using smartwrappers to access it
	 */
	ByteSoundProducer bsWrapper;

	bool _finished, isAttached;
	int _samplingRate, _bits, _channels, pos;
	string _name;
	queue< DataPacket<mcopbyte>* > outqueue;

	int packetCount, packetCapacity;
	int blockingIO;

protected:
	/**
	 * returns the amount of bytes that will be played in a given amount of
	 * time in milliseconds
	 */
	int timeToBytes(float time)
	{
    	float playSpeed = channels() * samplingRate() * bits() / 8;
		return (int)(playSpeed * (time / 1000.0));
	}

	/**
	 * returns the time in milliseconds it takes with the current parameters
	 * to play a given amount of bytes
	 */
	float bytesToTime(int size)
	{
    	float playSpeed = channels() * samplingRate() * bits() / 8;
		return (1000.0 * ((float)size) / playSpeed);
	}

	int bufferSize() {
		return packetCount * packetCapacity;
	}

	float bufferTime() {
		return bytesToTime(bufferSize());
	}

	int bufferSpace() {
		int space = 0;

		attach();

		/* make sure that our information is up-to-date */
		Dispatcher::the()->ioManager()->processOneEvent(false);

		if(!outqueue.empty())
		{
			space += packetCapacity - pos;  /* the first, half filled packet */

			if(outqueue.size() > 1)			/* and the other, empty packets */
				space += (outqueue.size()-1)*packetCapacity;
		}
		return space;
	}

	int setBufferSize(int size)
	{
		/* don't change sizes when already streaming */
		if(isAttached)
			return ARTS_E_NOIMPL;

		/*
		 * these parameters are usually a bad idea ;-) however we have to start
		 * somewhere, and maybe in two years, with a highly optimized kernel
		 * this is possible - for now, don't request the impossible or don't
		 * complain if it doesn't work
		 */
		packetCount = 3;
		packetCapacity = 128;

		/*
		 * - do not configure stream buffers smaller than the server
		 *   recommended value
		 * - try to get more or less close to the value the application
		 *   wants
		 */
		int needSize = max(size, timeToBytes(server.minStreamBufferTime()));

		while(bufferSize() < needSize)
		{
			packetCount++;
			if(packetCount == 8)
			{
				packetCount /= 2;
				packetCapacity *= 2;
			}
		}

		return bufferSize();
	}

	int packetSettings()
	{
		int settings = 0;

		int cap = packetCapacity;
		while(cap > 1)
		{
			settings++;
			cap /= 2;
		}

		settings |= packetCount << 16;
		return settings;
	}

	int setPacketSettings(int settings)
	{
		/* don't change sizes when already streaming */
		if(isAttached)
			return ARTS_E_NOIMPL;

		packetCount = settings >> 16;
		
		packetCapacity = 1;
		int c = settings & 0xffff;
		while(c > 0) {
			packetCapacity *= 2;
			c--;
		}

		/*
		 * - do not configure stream buffers smaller than the server
		 *   recommended value
		 * - keep the packetSize the applications specified
		 */
		int needSize = timeToBytes(server.minStreamBufferTime());

		while(bufferSize() < needSize)
			packetCount++;

		return packetSettings();
	}


	void attach()
	{
		if(!isAttached)
		{
			isAttached = true;

			server.attach(bsWrapper);
			start();

            /*
             * TODO: this processOneEvent looks a bit strange here... it is
             * there since StdIOManager does block 5 seconds on the first
             * arts_write if it isn't - although notifications are pending
             *
             * Probably the real solution is to rewrite the
             * StdIOManager::processOneEvent function. (And maybe drop the
             * assumption that aRts will not block when an infinite amount
             * of notifications is pending - I mean: will it ever happen?)
             */
            Dispatcher::the()->ioManager()->processOneEvent(false);
		}
	}

public:
	Sender(SoundServer server, int rate, int bits, int channels,
		string name) : server(server), _finished(false), isAttached(false),
		_samplingRate(rate), _bits(bits), _channels(channels), pos(0),
		_name(name)
	{
		serverBufferTime = server.serverBufferTime();
		stream_set(ARTS_P_BUFFER_SIZE,64*1024);
		stream_set(ARTS_P_BLOCKING,1);
		bsWrapper = ByteSoundProducer::_from_base(this);
	}
	~Sender()
	{
		//
	}
	long samplingRate() { return _samplingRate; }
	long channels()     { return _channels; }
	long bits()         { return _bits; }
	bool finished()     { return _finished; }

	int stream_set(arts_parameter_t param, int value)
	{
		int result;

		switch(param) {
			case ARTS_P_BUFFER_SIZE:
				return setBufferSize(value);

			case ARTS_P_BUFFER_TIME:
				result = setBufferSize(timeToBytes(value));
				if(result < 0) return result;
				return (int)bufferTime();

			case ARTS_P_PACKET_SETTINGS:
				return setPacketSettings(value);

			case ARTS_P_BLOCKING:
				if(value != 0 && value != 1) return ARTS_E_NOIMPL;

				blockingIO = value;
				return blockingIO;
			/*
			 * maybe ARTS_P_TOTAL_LATENCY _could_ be made writeable, the
			 * others are of course useless
			 */
			case ARTS_P_BUFFER_SPACE:
			case ARTS_P_SERVER_LATENCY:
			case ARTS_P_TOTAL_LATENCY:
			case ARTS_P_PACKET_SIZE:
			case ARTS_P_PACKET_COUNT:
				return ARTS_E_NOIMPL;
		}
		return ARTS_E_NOIMPL;
	}

	int stream_get(arts_parameter_t param)
	{
		switch(param) {
			case ARTS_P_BUFFER_SIZE:
				return bufferSize();

			case ARTS_P_BUFFER_TIME:
				return (int)bufferTime();
			
			case ARTS_P_BUFFER_SPACE:
				return bufferSpace();

			case ARTS_P_PACKET_SETTINGS:
				return packetSettings();

			case ARTS_P_SERVER_LATENCY:
				return (int)serverBufferTime;

			case ARTS_P_TOTAL_LATENCY:
				return stream_get(ARTS_P_SERVER_LATENCY)
				     + stream_get(ARTS_P_BUFFER_TIME);

			case ARTS_P_BLOCKING:
				return blockingIO;

			case ARTS_P_PACKET_SIZE:
				return packetCapacity;

			case ARTS_P_PACKET_COUNT:
				return packetCount;
		}
		return ARTS_E_NOIMPL;
	}

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
				packet->size = pos;
				packet->send();
				outqueue.pop();
			}
			outdata.endPull();

			/* remove all packets from the outqueue */
			while(!outqueue.empty())
			{
				DataPacket<mcopbyte> *packet = outqueue.front();
				packet->size = 0;
				packet->send();
				outqueue.pop();
			}

			server.detach(bsWrapper);
		}
		// similar effect like "delete this;"		
		bsWrapper = ByteSoundProducer::null();
	}

	int suspend()
	{
		if(isAttached)
		{
			return server.suspend();
		}
		return 0;
	}

	int write(const mcopbyte *data, int size)
	{
		attach();

		int remaining = size;
		while(remaining)
		{
			if(blockingIO)
			{
				/* C API blocking style write */
				while(outqueue.empty())
					Dispatcher::the()->ioManager()->processOneEvent(true);
			}
			else
			{
				/* non blocking I/O */
				if(outqueue.empty())
					Dispatcher::the()->ioManager()->processOneEvent(false);

				/* still no more space to write? */
				if(outqueue.empty())
					return size - remaining;
			}

			/* get a packet */
			DataPacket<mcopbyte> *packet = outqueue.front();

			/* copy some data there */
			int tocopy = min(remaining,packetCapacity-pos);
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

		/* no possible error conditions */
		return size;
	}
};

class ArtsCApi {
protected:
	static ArtsCApi *instance;
	int refcnt;

	Dispatcher dispatcher;
	SoundServer server;

	ArtsCApi() : refcnt(1), server(Reference("global:Arts_SoundServer"))
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

	int suspend() {
		if(!server.isNull()) 
			return server.suspend()? 1:0;
		return ARTS_E_NOSERVER;
	}

	void free() {
		// nothing to do
	}

	arts_stream_t play_stream(int rate, int bits, int channels, const char *name)
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

	int write(arts_stream_t stream, const void *data, int size)
	{
		if(server.isNull())
			return ARTS_E_NOSERVER;

		if(!stream)
			return ARTS_E_NOSTREAM;

		Sender *sender = (Sender *)stream;
		return sender->write((const mcopbyte *)data,size);
	}

	int stream_set(arts_stream_t stream, arts_parameter_t param, int value)
	{
		if(server.isNull())
			return ARTS_E_NOSERVER;

		if(!stream)
			return ARTS_E_NOSTREAM;

		Sender *sender = (Sender *)stream;
		return sender->stream_set(param,value);
	}

	int stream_get(arts_stream_t stream, arts_parameter_t param)
	{
		if(server.isNull())
			return ARTS_E_NOSERVER;

		if(!stream)
			return ARTS_E_NOSTREAM;

		Sender *sender = (Sender *)stream;
		return sender->stream_get(param);
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

extern "C" int arts_backend_suspend()
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;
	arts_backend_debug("arts_backend_suspend");
	return ArtsCApi::the()->suspend();	
}

extern "C" void arts_backend_free()
{
	if(!ArtsCApi::the()) return;

	arts_backend_debug("arts_backend_free");
	ArtsCApi::the()->free();
	ArtsCApi::release();
}

extern "C" arts_stream_t arts_backend_play_stream(int rate, int bits, int channels, const char *name)
{
	if(!ArtsCApi::the()) return 0;

	arts_backend_debug("arts_backend_play_stream");
	return ArtsCApi::the()->play_stream(rate,bits,channels,name);
}

extern "C"arts_stream_t arts_backend_record_stream(int, int, int, const char *)
{
	if(!ArtsCApi::the()) return 0;

	arts_backend_debug("arts_backend_record_stream");
	return 0;
}

extern "C" void arts_backend_close_stream(arts_stream_t stream)
{
	if(!ArtsCApi::the()) return;

	arts_backend_debug("arts_backend_close_stream");
	ArtsCApi::the()->close_stream(stream);
}

extern "C"int arts_backend_read(arts_stream_t, void *, int)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_backend_read");
	return ARTS_E_NOIMPL;
}

extern "C" int arts_backend_write(arts_stream_t stream, const void *buffer,
						int count)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_backend_write");
	return ArtsCApi::the()->write(stream,buffer,count);
}

extern "C" int arts_backend_stream_set(arts_stream_t stream,
						arts_parameter_t param, int value)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_stream_set");
	return ArtsCApi::the()->stream_set(stream,param,value);
}

extern "C" int arts_backend_stream_get(arts_stream_t stream,
						arts_parameter_t param)
{
	if(!ArtsCApi::the()) return ARTS_E_NOINIT;

	arts_backend_debug("arts_stream_get");
	return ArtsCApi::the()->stream_get(stream,param);
}
