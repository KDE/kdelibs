    /*

    Copyright (C) 2001 Carsten Kroll
                       ckroll@pinnaclesys.com

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
 * only compile 'sgi' AudioIO class if compiled under IRIX
 */
#ifdef HAVE_IRIX

#include <dmedia/audio.h>
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

class AudioIOSGI : public AudioIO {
protected:
	int requestedFragmentSize;
	int requestedFragmentCount;
	ALport audio_port,audio_port1;
	ALconfig audioconfig;
	int framesz;

public:
	AudioIOSGI();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};


REGISTER_AUDIO_IO(AudioIOSGI,"sgi","SGI dmedia audio i/o");
};

using namespace std;
using namespace Arts;

AudioIOSGI::AudioIOSGI()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 44100;
	paramStr(deviceName) = "audioio";
	requestedFragmentSize = param(fragmentSize) = 4096;
	requestedFragmentCount = param(fragmentCount) = 10;
	audio_port=0;
	audio_port1=0;
	param(format)=16;
	param(channels) = 2;
	param(direction) = 2;
}

bool AudioIOSGI::open()
{
	string& _error = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);
	int& _direction = param(direction);
	int err;

	if(_direction != 3  && _direction != 2){
		_error = "invalid direction";
		return false;
	}
	framesz=(_format >> 3) * _channels;

	audioconfig = alNewConfig();
	alSetSampFmt(audioconfig,AL_SAMPFMT_TWOSCOMP);

	alSetWidth(audioconfig, _format==8 ? AL_SAMPLE_8 : _format==16 ? AL_SAMPLE_16 : AL_SAMPLE_24);
	alSetQueueSize(audioconfig,(requestedFragmentSize * requestedFragmentCount) / framesz);
	alSetChannels(audioconfig,_channels);

	audio_port = alOpenPort("out","w",audioconfig);


	if (audio_port == (ALport) 0 ) {
		err = oserror();
		if (err == AL_BAD_NO_PORTS) {
			_error = "System is out of audio ports";
		} else if (err == AL_BAD_DEVICE_ACCESS) {
			_error = "Couldn't access audio device";
		} else if (err == AL_BAD_OUT_OF_MEM) {
			_error = "Out of memory";
		}
		close();
		return false;
	}
	if (_direction == 3){
		audio_port1  = alOpenPort("in","r",audioconfig);
		if (audio_port1 == (ALport) 0 ) {
		err = oserror();
		if (err == AL_BAD_NO_PORTS) {
			_error = "System is out of audio ports";
		} else if (err == AL_BAD_DEVICE_ACCESS) {
			_error = "Couldn't access audio device";
		} else if (err == AL_BAD_OUT_OF_MEM) {
			_error = "Out of memory";
		}
		close();
		return false;
	}
	}
	/*
	* Attempt to set a crystal-based sample-rate on the
	* given device.
	*/
	ALpv x[2];
	x[0].param = AL_MASTER_CLOCK;
	x[0].value.i = AL_CRYSTAL_MCLK_TYPE;
	x[1].param = AL_RATE;
	x[1].value.ll = alDoubleToFixed(double(_samplingRate));
	if (alSetParams(alGetResource(audio_port),x, 2)<0) {
		_error="setparams failed: ";
		_error+=alGetErrorString(oserror());
		close();
		return false;
	}
	if (_direction == 3)
		if (alSetParams(alGetResource(audio_port1),x, 2)<0) {
			_error="setparams failed: ";
			_error+=alGetErrorString(oserror());
			close();
			return false;
		}
	if (x[1].sizeOut < 0) {
		_error="rate was invalid";
		close();
		return false;
	}

	alSetFillPoint(audio_port,(alGetQueueSize(audioconfig)*5)/10) ;//50 %
	if (_direction == 3)
		alSetFillPoint(audio_port1,(alGetQueueSize(audioconfig)*4)/10) ;//40 %
	/*
	 * set the fragment settings to what the user requested
	 */

	_fragmentSize = requestedFragmentSize;
	_fragmentCount = requestedFragmentCount;


	artsdebug("buffering: %d fragments with %d bytes "
		"(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
		(float)(_fragmentSize*_fragmentCount) /
		(float)(2.0 * _samplingRate * _channels)*1000.0);


	return true;
}

void AudioIOSGI::close()
{
	alFreeConfig(audioconfig);
	alClosePort(audio_port);
	audio_port=0;
	if (param(direction) == 3) {
		alClosePort(audio_port1);
		audio_port1=0;
	}
}

void AudioIOSGI::setParam(AudioParam p, int& value)
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

int AudioIOSGI::getParam(AudioParam p)
{
	int frames;
	switch(p)
	{
		case canRead:
				frames=alGetFilled(audio_port);
				return frames*framesz;
			break;

		case canWrite:
				frames=alGetFillable(audio_port);
				return frames*framesz;
			break;

		case selectReadFD:
				return (param(direction) & directionRead)?
						alGetFD(audio_port1):-1;
			break;

		case selectWriteFD:
				return (param(direction) & directionWrite)?
						alGetFD(audio_port):-1;
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIOSGI::read(void *buffer, int size)
{
	arts_assert(audio_port1 != 0);
	::alReadFrames(audio_port1,buffer,size/framesz);
	return size;
}

int AudioIOSGI::write(void *buffer, int size)
{
	arts_assert(audio_port != 0);
	::alWriteFrames(audio_port,buffer,size/framesz);
	return size;
}

#endif
