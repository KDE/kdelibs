    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "soundserver.h"
#include "stdsynthmodule.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace Arts;

class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule
{
	bool _finished;
	int pos;
public:
	Sender() : _finished(false), pos(0)
	{
	}
	long samplingRate() { return 44100; }
	long channels()     { return 2; }
	long bits()         { return 16; }
	bool finished()     { return _finished; }

	static const int packetCount = 10, packetCapacity = 1024;
	void streamStart()
	{
		/*
		 * start streaming
		 */
		outdata.setPull(packetCount, packetCapacity);
	}

	void request_outdata(DataPacket<mcopbyte> *packet)
	{
		/*
		 * fill the packet with new data
		 */
		// ----------------- audio fill up start -----------------

		int j;
		mcopbyte *to = &packet->contents[0];
		for(j=0;j<packetCapacity/4;j++)
		{
			// generate two sin waves
			float fpos = ((float)pos)/44100.0;
			long left  = (long)(sin(fpos*6.28*440.0)*30000.0);
			long right = (long)(sin(fpos*6.28*880.0)*30000.0);
			pos++;

			// put the samples in the packet
		    *to++ = left & 0xff;
			*to++ = (left >> 8) & 0xff;                                              
		    *to++ = right & 0xff;
			*to++ = (right >> 8) & 0xff;
		}

		// ------------------ audio fill up end ------------------
		packet->send();
	}
};

int main(int argc, char **argv)
{
	Dispatcher dispatcher;
	SimpleSoundServer server = Reference("global:Arts_SimpleSoundServer");

	if(server.isNull())
	{
		cerr << "Can't connect to sound server" << endl;
		return 1;
	}

	ByteSoundProducer sender = ByteSoundProducer::_from_base(new Sender());
	server.attach(sender);
	sender.start();
	dispatcher.run();
	server.detach(sender);		// in this example: not reached
}
