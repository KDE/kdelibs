    /*

    Copyright (C) 2001 Robert Lunnon
                       bob@yarrabee.net.au
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

/**
 * only compile 'LibAudioIO' AudioIO class if libaudio was detected during
 * configure
 */
#ifdef HAVE_LIBAUDIOIO

#include <libaudioio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
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
#include "audioio.h"

namespace Arts {

class AudioIOLibAudioIO : public AudioIO {
protected:
	int audio_fd;
	int requestedFragmentSize;
	int requestedFragmentCount;
	SampleSpec_t spec;

public:
	AudioIOLibAudioIO();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOLibAudioIO,"libaudioio"," Portable Audio Library");
};

using namespace std;
using namespace Arts;

AudioIOLibAudioIO::AudioIOLibAudioIO()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = spec.rate=44100;
	paramStr(deviceName) = "/dev/Audio";
	requestedFragmentSize = param(fragmentSize) = 4096;
	requestedFragmentCount  =spec.max_blocks= param(fragmentCount) = 7;
	param(channels) = spec.channels=2;
	param(direction) = 2;
}

bool AudioIOLibAudioIO::open()
{
	string& _error = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);

	int mode;
	spec.channels=_channels;
	spec.max_blocks= param(fragmentCount);
	spec.rate= _samplingRate;
	spec.encoding= ENCODE_PCM;
	spec.precision=16 ;
	spec.endian=ENDIAN_NATURAL;
	spec.disable_threads=1;

	if(param(direction) == 3)
		mode = O_RDWR|O_NDELAY;
	else if(param(direction) == 2)
		mode = O_WRONLY|O_NDELAY;
	else
	{
		_error = "invalid direction";
		return false;
	}

	audio_fd = ::AudioIOOpenX( mode, &spec,&spec );
	if(audio_fd == -1)
	{
		_error = "device ";
		_error += _deviceName.c_str();
		_error += " can't be opened (";
		_error += strerror(errno);
		_error += ")";
		return false;
	}


	/*
	 * since we use spec.endian=ENDIAN_NATURAL we'll have little endian audio
	 * on little endian machines and big endian audio on big endian machines:
	 */
#ifdef WORDS_BIGENDIAN
	_format = 17;
#else
	_format = 16;
#endif

	spec.channels=_channels;

	spec.rate = _samplingRate;


	_fragmentSize = requestedFragmentSize;
	spec.max_blocks=_fragmentCount = requestedFragmentCount;


	artsdebug("buffering: %d fragments with %d bytes "
		"(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
		(float)(_fragmentSize*_fragmentCount) /
		(float)(2.0 * _samplingRate * _channels)*1000.0);

	return(true);
}

void AudioIOLibAudioIO::close()
{
	::AudioIOClose();
}

void AudioIOLibAudioIO::setParam(AudioParam p, int& value)
{
	switch(p)
	{
		case fragmentSize:
				param(p) = requestedFragmentSize = value;
			break;
		case fragmentCount:
				param(p) = requestedFragmentCount = value;
			break;
		default:
				param(p) = value;
			break;
	}
}

int AudioIOLibAudioIO::getParam(AudioParam p)
{
	switch(p)
	{
		case canRead:
				return AudioIOCheckRead();
			break;

		case canWrite:
				return (AudioIOCheckWriteReady()) ? (16*1024) : 0;
				// Assume if writable can write 16K
				// Arts Really doesn't care
			break;

		case selectReadFD:
				return (param(direction) & directionRead)?audio_fd:-1;
			break;

		case selectWriteFD:
				return (param(direction) & directionWrite)?audio_fd:-1;
			break;

		case autoDetect:
				/*
				 * if there is a "native" aRts driver, we'll rather use this
				 * than the generic libaudioio one, because the native one
				 * is likely to be better optimized to the needs of aRts than
				 * a generic driver, so keep the value small
				 */
				return 3;
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIOLibAudioIO::read(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::AudioIORead(buffer,size);
}

int AudioIOLibAudioIO::write(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::AudioIOWrite(buffer,size);
}

#endif
