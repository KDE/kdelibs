/* This is an experimental C api for aRts */

/*
 * warning: the aRts C API is not yet finished and WILL CHANGE - this 
 * is here for experimental purposes ...
 */
#include "capi.h"
#include "soundserver.h"
#include "stdsynthmodule.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <cassert>

using namespace std;

static class Sender *globalSender = 0;   /* TODO! */

class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule
{
	bool _finished;
	int pos;
	queue< DataPacket<mcopbyte>* > outqueue;
public:
	Sender() : _finished(false), pos(0)
	{
		globalSender = this;
	}
	~Sender()
	{
		globalSender = 0;
	}
	long samplingRate() { return 44100; }
	long channels()     { return 2; }
	long bits()         { return 16; }
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

	void write(mcopbyte *data, int size)
	{
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
	}
};

class ArtsCApi {
protected:
	static ArtsCApi *instance;
	int _lastResult;
	Dispatcher dispatcher;

	SimpleSoundServer server;

	ArtsCApi() :_lastResult(0),
		server(Reference("global:Arts_SimpleSoundServer"))
	{
		if(server.isNull())
			_lastResult = E_ARTS_NOSERVER;
	}
	void connect()
	{
		if(_lastResult == E_ARTS_NOSERVER) return;

		Sender *sender = new Sender();
		server.attach(sender);
		sender->start();
		// we don't hold a reference to the sender at all any more - the
		// soundserver should do so, as long as he wants -> FIXME
		sender->_release();

		_lastResult = 0;
	}
	void write(void *data, int size)
	{
		if(_lastResult == E_ARTS_NOSERVER) return;
		if(!globalSender)
		{
			_lastResult = E_ARTS_NOCONNECTION;
			return;
		}

		globalSender->write((mcopbyte *)data,size);

		_lastResult = 0;
	}

	int lastResult()
	{
		return _lastResult;
	}

public:
// CApi wrapping
	static int arts_init()
	{
		assert(!instance);
		instance = new ArtsCApi();
		return instance->lastResult();
	}
	static int arts_connect()
	{
		assert(instance);
		instance->connect();
		return instance->lastResult();
	}
	static int arts_write(void *data, int size)
	{
		assert(instance);
		instance->write(data,size);
		return instance->lastResult();
	}
};

ArtsCApi *ArtsCApi::instance = 0;

int arts_init()
{
	return ArtsCApi::arts_init();
}

int arts_connect()
{
	return ArtsCApi::arts_connect();
}

int arts_write(/* missing fd here, */ void *buffer,int size)
{
	return ArtsCApi::arts_write(buffer, size);
}
