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

#include <iostream>
#include <math.h>
#include "audiosubsys.h"
#include "convert.h"
#include "dispatcher.h"

using namespace std;
using namespace Arts;

class SinTest : public ASProducer {
protected:
	int audiofd;
	int done;
	AudioSubSystem *as;
	bool attached;

public:
	SinTest();
	virtual ~SinTest();
	void needMore();
	void run(long samples);
};

SinTest::SinTest() : done(0), as(AudioSubSystem::the())
{
	// initialize and open audio subsystem (defaults to 44kHz, 16bit, stereo)
	attached = as->attachProducer(this);
	if(!attached)
	{
		cout << "audio subsystem is already used?" << endl;
	}
	else
	{
		if(as->open() == false)
		{
			cout << "audio subsystem init failed ("
			     << as->error() << ")" << endl;
			audiofd = -1;
		}
		else
			audiofd = as->selectWriteFD();
	}
}

SinTest::~SinTest()
{
	// deinitialize the audio subsystem
	if(attached)
		as->detachProducer();
}

void SinTest::run(long samples)
{
	// run this loop until enough samples are produced
	while(attached && audiofd >= 0 && (done < samples))
	{
		fd_set wfds, efds;
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(audiofd, &wfds);
		FD_SET(audiofd, &efds);

		// look if the audio filedescriptor becomes writeable
		int result = select(audiofd+1,0,&wfds,&efds,0);
		if(result > 0)
		{
			// if yes, tell the audio subsystem to do something about it
			if(FD_ISSET(audiofd, &wfds))
				as->handleIO(AudioSubSystem::ioWrite);
			if(FD_ISSET(audiofd, &efds))
				as->handleIO(AudioSubSystem::ioExcept);
		}
		else
		{
			cout << "select returned " << result << endl;
		}
	}
}

/*
 * needMore is a callback by the audio subsystem, which is made as soon as
 * the internal buffers need a refill
 */
void SinTest::needMore()
{
	float left[100], right[100];
	unsigned char out[400];

	// generate two waves, one 440 Hz (left), one 880Hz (right)
	for(int i=0;i<100;i++)
	{
		left[i] = sin((float)i*6.28/100.0);
		right[i] = sin((float)i*6.28/50.0);
	}
	convert_stereo_2float_i16le(100, left, right, out);

	// write them to the audio subsystem
	as->write(out,400);
	done += 100;
}

/*
 * This is intended to help testing the audio subsystem with different
 * hardware without needing to use the whole server.
 *
 * Warning: this is *not* an example how to use aRts (there are others),
 * but a simple test program for debugging purposes.
 */
int main()
{
	Dispatcher d;	// to get startup classes executed

	// the test, generate 88100 samples (2 seconds) of pure sine wave test data
	SinTest s;
	s.run(88100);
	return 0;
}
