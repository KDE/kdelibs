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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

#include "debug.h"
#include "audiosubsys.h"
#include "audioio.h"

#define DEFAULT_DEVICE_NAME "/dev/dsp"

using namespace std;
using namespace Arts;

//--- automatic startup class

static AudioSubSystemStart aStart;

void AudioSubSystemStart::startup()
{
	_instance = new AudioSubSystem();
}

void AudioSubSystemStart::shutdown()
{
	delete _instance;
}

//--- AudioSubSystemPrivate data

class Arts::AudioSubSystemPrivate
{
public:
	AudioIO *audioIO;
	string audioIOName;
	bool audioIOInit;
};

//--- AudioSubSystem implementation

AudioSubSystem *AudioSubSystem::the()
{
	return aStart.the();
}

const char *AudioSubSystem::error()
{
	return _error.c_str();
}

AudioSubSystem::AudioSubSystem()
{
	d = new AudioSubSystemPrivate;
	d->audioIO = 0;
	d->audioIOInit = false;
	
	_running = false;
	usageCount = 0;
	consumer = 0;
	producer = 0;
	fragment_buffer = 0;
}

AudioSubSystem::~AudioSubSystem()
{
	delete d;
}

bool AudioSubSystem::attachProducer(ASProducer *producer)
{
	assert(producer);
	if(this->producer) return false;

	this->producer = producer;
	return true;
}

bool AudioSubSystem::attachConsumer(ASConsumer *consumer)
{
	assert(consumer);
	if(this->consumer) return false;

	this->consumer = consumer;
	return true;
}

void AudioSubSystem::detachProducer()
{
	assert(producer);
	producer = 0;

	if(_running) close();
}

void AudioSubSystem::detachConsumer()
{
	assert(consumer);
	consumer = 0;

	if(_running) close();
}

/* initially creates default AudioIO */
void AudioSubSystem::initAudioIO()
{
	/* auto detect */
	if(!d->audioIOInit)
	{
		string bestName;
		int bestValue = 0;

		arts_debug("autodetecting driver: ");
		for(int i = 0; i < AudioIO::queryAudioIOCount(); i++)
		{
			string name = AudioIO::queryAudioIOParamStr(i, AudioIO::name);
			AudioIO *aio = AudioIO::createAudioIO(name.c_str());
			int value = aio->getParam(AudioIO::autoDetect);

			arts_debug(" - %s: %d", name.c_str(), value);
			if(value > bestValue)
			{
				bestName = name;
				bestValue = value;
			}
			delete aio;
		}
		if(bestValue)
		{
			arts_debug("... which means we'll default to %s", bestName.c_str());
			audioIO(bestName);
		}
		else
		{
			arts_debug("... nothing we could use as default found");
		}
	}
}

void AudioSubSystem::audioIO(const string& audioIO)
{
	if(d->audioIO)
		delete d->audioIO;

	d->audioIOName = audioIO;
	d->audioIO = AudioIO::createAudioIO(audioIO.c_str());
	d->audioIOInit = true;
}

string AudioSubSystem::audioIO()
{
	initAudioIO();

	return d->audioIOName;
}

void AudioSubSystem::deviceName(const string& deviceName)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParamStr(AudioIO::deviceName, deviceName.c_str());
}

string AudioSubSystem::deviceName()
{
	initAudioIO();
	if(!d->audioIO) return "";

	return d->audioIO->getParamStr(AudioIO::deviceName);
}

void AudioSubSystem::fragmentCount(int fragmentCount)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParam(AudioIO::fragmentCount, fragmentCount);
}

int AudioSubSystem::fragmentCount()
{
	initAudioIO();
	if(!d->audioIO) return 0;

	return d->audioIO->getParam(AudioIO::fragmentCount);
}

void AudioSubSystem::fragmentSize(int fragmentSize)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParam(AudioIO::fragmentSize, fragmentSize);
}

int AudioSubSystem::fragmentSize()
{
	initAudioIO();
	if(!d->audioIO) return 0;

	return d->audioIO->getParam(AudioIO::fragmentSize);
}

void AudioSubSystem::samplingRate(int samplingRate)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParam(AudioIO::samplingRate, samplingRate);
}

int AudioSubSystem::samplingRate()
{
	initAudioIO();
	if(!d->audioIO) return 0;

	return d->audioIO->getParam(AudioIO::samplingRate);
}

void AudioSubSystem::channels(int channels)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParam(AudioIO::channels, channels);
}

int AudioSubSystem::channels()
{
	initAudioIO();
	if(!d->audioIO) return 0;

	return d->audioIO->getParam(AudioIO::channels);
}

void AudioSubSystem::format(int format)
{
	initAudioIO();
	if(!d->audioIO) return;

	d->audioIO->setParam(AudioIO::format, format);
}

int AudioSubSystem::format()
{
	initAudioIO();
	if(!d->audioIO) return 0;

	return d->audioIO->getParam(AudioIO::format);
}

int AudioSubSystem::bits()
{
	int _format = format();
	arts_assert(_format == 0 || _format == 8 || _format == 16 || _format == 17);
	return (_format & (16 | 8));
}

void AudioSubSystem::fullDuplex(bool fullDuplex)
{
	initAudioIO();
	if(!d->audioIO) return;

	int direction = fullDuplex?3:2;
	d->audioIO->setParam(AudioIO::direction, direction);
}

bool AudioSubSystem::fullDuplex()
{
	initAudioIO();
	if(!d->audioIO) return false;

	return d->audioIO->getParam(AudioIO::direction) == 3;
}


bool AudioSubSystem::check()
{
	int fd;
	bool ok = open(fd);

	if(ok) close();
	return ok;
}

bool AudioSubSystem::open(int& fd)
{
	assert(!_running);

	initAudioIO();
	if(!d->audioIO)
	{
		if(d->audioIOName == "")
			_error = "couldn't auto detect which audio I/O method to use";
		else	
			_error = "unable to select '"+d->audioIOName+"' style audio I/O";
		audio_fd = fd = -1;
		return false;
	}

	if(d->audioIO->open())
	{
		_running = true;

		_fragmentSize = d->audioIO->getParam(AudioIO::fragmentSize);
		_fragmentCount = d->audioIO->getParam(AudioIO::fragmentCount);

		// allocate global buffer to do I/O
		assert(fragment_buffer == 0);
		fragment_buffer = new char[_fragmentSize];

		audio_fd = fd = d->audioIO->getParam(AudioIO::selectFD);
		return true;
	}
	else
	{
		_error = d->audioIO->getParamStr(AudioIO::lastError);
		audio_fd = fd = -1;
		return false;
	}
}

int AudioSubSystem::open()
{
	int fd = -1;

	if(!open(fd)) return -1;

	if(fd < 0) {
		_error = "using obsolete AudioSubSystem::open() interface doesn't work "
				 "with " + d->audioIOName + " driver";
		close();
		return -1;
	}
	return fd;
}

void AudioSubSystem::close()
{
	assert(_running);
	assert(d->audioIO);

	d->audioIO->close();

	wBuffer.clear();
	rBuffer.clear();

	_running = false;
	audio_fd = -1;

	if(fragment_buffer)
	{
		delete[] fragment_buffer;
		fragment_buffer = 0;
	}
}

bool AudioSubSystem::running()
{
	return _running;
}

void AudioSubSystem::handleIO(int type)
{
	assert(d->audioIO);

	if(type & ioRead)
	{
		int len = d->audioIO->read(fragment_buffer,_fragmentSize);

		if(len > 0)
			rBuffer.write(len,fragment_buffer);
	}

	if(type & ioWrite)
	{
		/*
		 * make sure that we have a fragment full of data at least
		 */
		while(wBuffer.size() < _fragmentSize)
		{
			long wbsz = wBuffer.size();
			producer->needMore();

			if(wbsz == wBuffer.size())
			{
				/*
				 * Even though we asked the client to supply more
				 * data, he didn't give us more. So we can't supply
				 * output data as well. Bad luck. Might produce a
				 * buffer underrun - but we can't help here.
				 */
				arts_info("full duplex: no more data available (underrun)");
				return;
			}
		}

		/*
		 * look how much we really can write without blocking
		 */
		int space = d->audioIO->getParam(AudioIO::canWrite);
		int can_write = min(space, _fragmentSize);

		if(can_write > 0)
		{
			/*
			 * ok, so write it (as we checked that our buffer has enough data
			 * to do so and the soundcardbuffer has enough data to handle this
			 * write, nothing can go wrong here)
			 */
			int rSize = wBuffer.read(can_write,fragment_buffer);
			assert(rSize == can_write);
	
			int len = d->audioIO->write(fragment_buffer,can_write);
			assert(len == can_write);
		}
	}

	assert((type & ioExcept) == 0);
}

void AudioSubSystem::read(void *buffer, int size)
{
	/*
	 * try the peaceful method first: read something, and hope that there
	 * will be something to read
	 */
	while(rBuffer.size() < size)
	{
		int s = rBuffer.size();
		handleIO(ioRead);
		if(s == rBuffer.size()) break;
	}
	/*
	 * if this doesn't help, select until there is something to read, and
	 * then read
	 */
	while(rBuffer.size() < size)
	{
		int rc;
		do
		{
			fd_set readfds;
			timeval select_timeout;
			long selectabs = 50000;	// 50 ms
			select_timeout.tv_sec = selectabs / 1000000;
			select_timeout.tv_usec = selectabs % 1000000;

			FD_ZERO(&readfds);

			// if no audio_fd (-1) given, do a pure 50ms sleep
			if(audio_fd >= 0)
				FD_SET(audio_fd,&readfds);

			rc = select(audio_fd+1,&readfds,NULL,NULL,&select_timeout);
		} while(rc == -1 && errno == EINTR);

		if(rc <= 0)
		{
			arts_info("full duplex: timeout occured (ospace=%d, ispace=%d)",
				d->audioIO->getParam(AudioIO::canWrite),
				d->audioIO->getParam(AudioIO::canRead));
		}
		handleIO(ioRead);
	}
	int rSize = rBuffer.read(size,buffer);
	assert(rSize == size);
}

void AudioSubSystem::write(void *buffer, int size)
{
	wBuffer.write(size,buffer);
}
