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

#include "audiosubsys.h"

#define DEVICE_NAME "/dev/dsp"

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

//--- AudioSubSystem implementation

AudioSubSystem *AudioSubSystem::the()
{
	return aStart.the();
}

const char *AudioSubSystem::error()
{
	return _error.c_str();
}

AudioSubSystem::AudioSubSystem() :_fragmentCount(7), _fragmentSize(1024),
                                  _samplingRate(44100), _channels(2),
								  _fullDuplex(false)
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

void AudioSubSystem::fragmentCount(int fragmentCount)
{
	_fragmentCount = fragmentCount;
}

int AudioSubSystem::fragmentCount()
{
	return _fragmentCount;
}

void AudioSubSystem::fragmentSize(int fragmentSize)
{
	_fragmentSize = fragmentSize;
}

int AudioSubSystem::fragmentSize()
{
	return _fragmentSize;
}

void AudioSubSystem::samplingRate(int samplingRate)
{
	_samplingRate = samplingRate;
}

int AudioSubSystem::samplingRate()
{
	return _samplingRate;
}

void AudioSubSystem::channels(int channels)
{
	_channels = channels;
}

int AudioSubSystem::channels()
{
	return _channels;
}

void AudioSubSystem::fullDuplex(bool fullDuplex)
{
	_fullDuplex = fullDuplex;
}

bool AudioSubSystem::fullDuplex()
{
	return _fullDuplex;
}


int AudioSubSystem::open()
{
#ifdef HAVE_SYS_SOUNDCARD_H
	int mode;

	if(_fullDuplex)
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

	if(_channels == 1)
	{
		stereo = 0;
	}
	if(_channels == 2)
	{
		stereo = 1;
	}

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

	int speed = _samplingRate;

	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1)  
	{
		_error = "SNDCTL_DSP_SPEED failed - ";
		_error += strerror(errno);

		close();
		return -1;
	}  

    /*
	 * Some soundcards seem to be able to only supply "nearly" the requested
	 * sampling rate - so allow a tolerance of 2%. This will result in slight
	 * detuning, but at least it will work.
	 */
    int tolerance = _samplingRate/50+1;

	if (abs(speed-_samplingRate) > tolerance)
	{  
		_error = "can't set requested samplingrate";

		close();
		return -1;
	} 

	// lower 16 bits are the fragment size (as 2^S)
	// higher 16 bits are the number of fragments
	int frag_arg = 0;

	// allocate global buffer to do I/O
	assert(fragment_buffer == 0);
	fragment_buffer = new char[_fragmentSize];

	int size = _fragmentSize;
	while(size > 1) { size /= 2; frag_arg++; }
	frag_arg += (_fragmentCount << 16);
	if(ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag_arg) == -1)
	{
		char buffer[1024];
		_error = "can't set requested fragments settings";
		sprintf(buffer,"size%d:count%d\n",_fragmentSize,_fragmentCount);
		close();
		return -1;
	}

	// FIXME: check here if frag_arg changed

	int enable_bits = ~(PCM_ENABLE_OUTPUT|PCM_ENABLE_INPUT);
	if(_fullDuplex)
	{
		if(ioctl(audio_fd,SNDCTL_DSP_SETTRIGGER, &enable_bits) == -1)
		{
			_error = "can't request synchronous start of fullduplex operation";

			close();
			return -1;
		}
	}

	/*
	 * Workaround for broken kernel drivers: usually filling up the audio
	 * buffer is _only_ required if _fullDuplex is true. However, there
	 * are kernel drivers around (especially everything related to ES1370/1371)
	 * which will not trigger select()ing the file descriptor unless we have
	 * written something first.
	 */
	char *zbuffer = (char *)calloc(sizeof(char), size);
	while(::write(audio_fd,zbuffer,size) > 0)
		/* fills up the audio buffer */;
	free(zbuffer);

	if(_fullDuplex)
	{
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
#ifdef HAVE_SYS_SOUNDCARD_H
	if(type & ioRead)
	{
		int len = ::read(audio_fd,fragment_buffer,_fragmentSize);

		if(len > 0)
		{
			rBuffer.write(len,fragment_buffer);
		}
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
				printf("FULL DUPLEX WARNING: client->needMore() failed; no more data available\n");
				return;
			}
		}
		/*
		 * look how much we really can write without blocking
		 */
		audio_buf_info info;
		ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info);

		int can_write = min(info.bytes, _fragmentSize);

		/*
		 * ok, so write it (as we checked that our buffer has enough data
		 * to do so and the soundcardbuffer has enough data to handle this
		 * write, nothing can go wrong here)
		 */
		int rSize = wBuffer.read(can_write,fragment_buffer);
		assert(rSize == can_write);

		int len = ::write(audio_fd,fragment_buffer,can_write);
		assert(len == can_write);
	}

	assert((type & ioExcept) == 0);
#endif
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
