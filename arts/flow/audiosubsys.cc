    /*

    Copyright (C) 1999 Stefan Westerfeld
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "audiosubsys.h"

#define DEVICE_NAME "/dev/dsp"

using namespace std;

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
	_running = false;
	usageCount = 0;
	consumer = 0;
	producer = 0;
	fragment_buffer = 0;
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

int AudioSubSystem::open(int fragments,int size, int samplingrate, int channels,
                         bool wantfullduplex)
{
#ifdef HAVE_SYS_SOUNDCARD_H
	int mode;

	if(wantfullduplex)
		mode = O_RDWR|O_NDELAY;
	else
		mode = O_WRONLY|O_NDELAY;

	audio_fd = ::open(DEVICE_NAME, mode, 0);

	if(audio_fd == -1)
	{
		_error = "device ";
		_error += DEVICE_NAME;
		_error += " can't be opened (";
		_error += strerror(errno);
		_error += ")";
		return -1;
	}
	// this is required here since we'll use close to close down the
	// audio subsystem again if anything else goes wrong
	_running = true;

	int format = AFMT_S16_LE;  
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format)==-1)  
	{
		_error = "SNDCTL_DSP_SETFMT failed - ";
		_error += strerror(errno);

		close();
		return -1;
	}  

	if (format != AFMT_S16_LE)  
	{  
		_error = "Can't set 16bit (AFMT_S16_LE) playback";

		close();
		return -1;
	} 

	int stereo=-1;     /* 0=mono, 1=stereo */

	if(channels == 1)
	{
		stereo = 0;
	}
	if(channels == 2)
	{
		stereo = 1;
	}

	_channels = channels;

	if(stereo == -1)
	{
		_error = "internal error; set channels to 1 (mono) or 2 (stereo)";

		close();
		return -1;
	}

	int requeststereo = stereo;

	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo)==-1)
	{
		_error = "SNDCTL_DSP_STEREO failed - ";
		_error += strerror(errno);

		close();
		return -1;
	}

	if (requeststereo != stereo)
	{
		_error = "audio device doesn't support number of requested channels";

		close();
		return -1;
	}

	int speed = samplingrate;

	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1)  
	{
		_error = "SNDCTL_DSP_SPEED failed - ";
		_error += strerror(errno);

		close();
		return -1;
	}  

	if (speed != samplingrate)
	{  
		_error = "can't set requested samplingrate";

		close();
		return -1;
	} 

	// lower 16 bits are the fragment size (as 2^S)
	// higher 16 bits are the number of fragments
	int frag_arg = 0;

	fragment_size = size;

	// allocate global buffer to do I/O
	assert(fragment_buffer == 0);
	fragment_buffer = new char[fragment_size];

	while(size > 0) { size /= 2; frag_arg++; }
	frag_arg += (fragments << 16);
	if(ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag_arg) == -1)
	{
		char buffer[1024];
		_error = "can't set requested fragments settings";
		sprintf(buffer,"size%d:count%d\n",fragment_size,fragments);
		close();
		return -1;
	}

	// FIXME: check here if frag_arg changed

	if(wantfullduplex)
	{
		int enable_bits = ~(PCM_ENABLE_OUTPUT|PCM_ENABLE_INPUT);

		if(ioctl(audio_fd,SNDCTL_DSP_SETTRIGGER, &enable_bits) == -1)
		{
			_error = "can't request synchronous start of fullduplex operation";

			close();
			return -1;
		}

		char *zbuffer = (char *)calloc(sizeof(char), size);
		while(::write(audio_fd,zbuffer,size) > 0)
			/* fills up the audio buffer */;
		free(zbuffer);

		/*
	 	 * Go now, and hope! that the application does the select trick
	 	 * correctly, otherwise we'll get buffer over/underruns soon
	 	 */
		enable_bits = PCM_ENABLE_OUTPUT|PCM_ENABLE_INPUT;
		if(ioctl(audio_fd,SNDCTL_DSP_SETTRIGGER, &enable_bits) == -1)
		{
			_error = "can't start of fullduplex operation";

			close();
			return -1;
		}
	}

	fullDuplex = wantfullduplex;
	return audio_fd;
#else
	cerr << "Sorry: arts doesn't support sound I/O on non Voxware-esque systems, yet";
	return -1;
#endif
}

void AudioSubSystem::close()
{
	assert(_running);

	::close(audio_fd);

	wBuffer.clear();
	rBuffer.clear();

	_running = false;
	audio_fd = 0;

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
	if(type & ioRead)
	{
		int len = ::read(audio_fd,fragment_buffer,fragment_size);

		if(len > 0)
		{
			rBuffer.write(len,fragment_buffer);
		}
	}

	if(type & ioWrite)
	{
		while(wBuffer.size() < fragment_size)
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
				printf("FULL DUPLEX WARNING: client->needMore() failed; no more data available\n");
				return;
			}
		}
		int rSize = wBuffer.read(fragment_size,fragment_buffer);
		assert(rSize == fragment_size);

		int len = ::write(audio_fd,fragment_buffer,fragment_size);
		assert(len == fragment_size);
	}

	assert((type & ioExcept) == 0);
}

void AudioSubSystem::read(void *buffer, int size)
{
	while(rBuffer.size() < size)
	{
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(audio_fd,&readfds);

		//printf("must use select\n");
		int rc = select(audio_fd+1,&readfds,NULL,NULL,NULL);
		assert(rc > 0);

		handleIO(ioRead);
	}
	int rSize = rBuffer.read(size,buffer);
	assert(rSize == size);
}

void AudioSubSystem::write(void *buffer, int size)
{
	wBuffer.write(size,buffer);
}

int AudioSubSystem::channels()
{
	return _channels;
}
