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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

#include "debug.h"
#include "audioio.h"
#include "audiosubsys.h"
#include "iomanager.h"
#include "dispatcher.h"

namespace Arts {

class AudioIONull : public AudioIO, public TimeNotify {
protected:
	timeval start;
	double samplesRead, samplesWritten, bytesPerSec;

public:
	AudioIONull();

	void notifyTime();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIONull,"null","No audio input/output");
};

using namespace std;
using namespace Arts;

AudioIONull::AudioIONull()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 44100;
	paramStr(deviceName) = "null";
	param(fragmentSize) = 1024;
	param(fragmentCount) = 7;
	param(channels) = 2;
	param(direction) = 2;
}

bool AudioIONull::open()
{
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);
	_format = 16;

	/*
	 * don't allow unreasonable large fragmentSize/Count combinations,
	 * because "real" hardware also doesn't
	 */

	if(_fragmentSize > 1024*128) _fragmentSize = 1024*128;

	while(_fragmentSize * _fragmentCount > 1024*128)
		_fragmentCount--;

	Dispatcher::the()->ioManager()->addTimer(10, this);

	samplesRead = samplesWritten = 0.0;
	bytesPerSec = _channels * 2 * _samplingRate;
	gettimeofday(&start,0);

	return true;
}

void AudioIONull::close()
{
	Dispatcher::the()->ioManager()->removeTimer(this);
}

void AudioIONull::notifyTime()
{
	int& _direction = param(direction);
	int& _fragmentSize = param(fragmentSize);

	for(;;)
	{
		int todo = 0;

		if((_direction & directionRead) && getParam(canRead) >= _fragmentSize)
			todo |= AudioSubSystem::ioRead;

		if((_direction & directionWrite) && getParam(canWrite) >= _fragmentSize)
			todo |= AudioSubSystem::ioWrite;

		if(!todo)
			return;

		AudioSubSystem::the()->handleIO(todo);
	}
}

void AudioIONull::setParam(AudioParam p, int& value)
{
	param(p) = value;
}

int AudioIONull::getParam(AudioParam p)
{
	timeval now;
	double delta;
	int bytes;

	switch(p)
	{
		case canRead:
		case canWrite:
				gettimeofday(&now,0);
				delta = (double)now.tv_sec + (double)now.tv_usec/1000000.0;
				delta -= (double)start.tv_sec + (double)start.tv_usec/1000000.0;
				bytes = (int)( (delta * bytesPerSec)
								 -  ((p == canRead)?samplesRead:samplesWritten)
								  );
				return bytes;
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIONull::read(void *buffer, int size)
{
	samplesRead += size;
	memset(buffer, 0, size);
	return size;
}

int AudioIONull::write(void *, int size)
{
	samplesWritten += size;
	return size;
}
