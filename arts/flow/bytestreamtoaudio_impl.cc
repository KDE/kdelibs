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

#include "artsflow.h"
#include "stdsynthmodule.h"
#include "resample.h"
#include <iostream>

using namespace std;
using namespace Arts;

namespace Arts {

class PacketRefiller : public Refiller {
protected:
	queue< DataPacket<mcopbyte>* > inqueue;
	int pos;

public:
	PacketRefiller() : pos(0)
	{
	}
	void process(DataPacket<mcopbyte>* packet)
	{
		inqueue.push(packet);
	}
	unsigned long read(unsigned char *buffer, unsigned long bytes)
	{
		unsigned long done = 0;
		while(!inqueue.empty())
		{
			long tocopy = bytes - done;
			if(tocopy == 0) return bytes;			/* complete? */

			DataPacket<mcopbyte> *packet = inqueue.front();
			if(tocopy > (packet->size - pos))
				tocopy = (packet->size - pos);

			memcpy(&buffer[done],&packet->contents[pos],tocopy);
			pos += tocopy;
			done += tocopy;

			if(pos == packet->size) {
				packet->processed();
				pos = 0;
				inqueue.pop();
			}
		}
		return done;
	}
};

class ByteStreamToAudio_impl : public ByteStreamToAudio_skel,
                               public StdSynthModule
{
	PacketRefiller refiller;
	Resampler resampler;
	long _samplingRate, _channels, _bits;
public:
	ByteStreamToAudio_impl() :resampler(&refiller),
			_samplingRate(44100), _channels(2), _bits(16)
	{
		//
	}

	long samplingRate() { return _samplingRate; }
	void samplingRate(long newRate) {
		_samplingRate = newRate;
		resampler.setStep((float)_samplingRate / samplingRateFloat);
	}

	long channels() { return _channels; }
	void channels(long newChannels) {
		_channels = newChannels;
		resampler.setChannels(_channels);
	}

	long bits() { return _bits; }
	void bits(long newBits) {
		_bits = newBits;
		resampler.setBits(_bits);
	}

	bool running() { return !resampler.underrun(); }

	void process_indata(DataPacket<mcopbyte> *packet)
	{
		refiller.process(packet);
	}

	void calculateBlock(unsigned long samples)
	{
		resampler.run(left,right,samples);
	}
};

REGISTER_IMPLEMENTATION(ByteStreamToAudio_impl);

};
