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

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#define COMPILE_AUDIOIO_OSS 1
#endif

#ifdef HAVE_SOUNDCARD_H
#include <soundcard.h>
#define COMPILE_AUDIOIO_OSS 1
#endif

/**
 * only compile 'oss' AudioIO class if sys/soundcard.h or soundcard.h is present
 */
#ifdef COMPILE_AUDIOIO_OSS
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

class AudioIOOSS : public AudioIO {
protected:
	int audio_fd;
	int requestedFragmentSize;
	int requestedFragmentCount;

	std::string findDefaultDevice();
	int ossBits(int format);

public:
	AudioIOOSS();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOOSS,"oss","Open Sound System");
};

using namespace std;
using namespace Arts;

/*
 * Tries to figure out which is the OSS device we should write to
 */
string AudioIOOSS::findDefaultDevice()
{
	static const char *device[] = {
		"/dev/dsp",						/* Linux (and lots of others) */
		"/dev/sound/dsp",				/* Linux with devfs-only installation */
		"/dev/audio",					/* OpenBSD */
		0
	};

	for(int i = 0; device[i]; i++)
		if(access(device[i],F_OK) == 0)
			return device[i];

	return device[0];
}

int AudioIOOSS::ossBits(int format)
{
	arts_return_val_if_fail (format == AFMT_U8
			              || format == AFMT_S16_LE
						  || format == AFMT_S16_BE, 16);

	return (format == AFMT_U8)?8:16;
}

AudioIOOSS::AudioIOOSS()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 44100;
	paramStr(deviceName) = findDefaultDevice();
	requestedFragmentSize = param(fragmentSize) = 1024;
	requestedFragmentCount = param(fragmentCount) = 7;
	param(channels) = 2;
	param(direction) = 2;
}

bool AudioIOOSS::open()
{
	string& _error = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);

	int mode;

	if(param(direction) == 3)
		mode = O_RDWR|O_NDELAY;
	else if(param(direction) == 2)
		mode = O_WRONLY|O_NDELAY;
	else
	{
		_error = "invalid direction";
		return false;
	}

	audio_fd = ::open(_deviceName.c_str(), mode, 0);

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
	 * check device capabilities
	 */
	int device_caps;
	if(ioctl(audio_fd,SNDCTL_DSP_GETCAPS,&device_caps) == -1)
            device_caps=0;

	string caps = "";
	if(device_caps & DSP_CAP_DUPLEX) caps += "duplex ";
	if(device_caps & DSP_CAP_REALTIME) caps += "realtime ";
	if(device_caps & DSP_CAP_BATCH) caps += "batch ";
	if(device_caps & DSP_CAP_COPROC) caps += "coproc ";
	if(device_caps & DSP_CAP_TRIGGER) caps += "trigger ";
	if(device_caps & DSP_CAP_MMAP) caps += "mmap ";
	artsdebug("device capabilities: revision%d %s",
					device_caps & DSP_CAP_REVISION, caps.c_str());

	int requestedFormat = (_format == 8)?AFMT_U8:AFMT_S16_LE;
	int gotFormat = requestedFormat;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &gotFormat)==-1)  
	{
		_error = "SNDCTL_DSP_SETFMT failed - ";
		_error += strerror(errno);

		close();
		return false;
	}  

	if (_format && (ossBits(gotFormat) != ossBits(requestedFormat)))
	{  
		char details[80];
		sprintf(details," (_format = %d, asked driver to give %d, got %d)",
			_format, requestedFormat, gotFormat);

		_error = "Can't set playback format";
		_error += details;

		close();
		return false;
	}

	if(gotFormat == AFMT_U8)
		_format = 8;
	else if(gotFormat == AFMT_S16_LE)
		_format = 16;
	else if(gotFormat == AFMT_S16_BE)
		_format = 17;
	else
	{
		char details[80];
		sprintf(details," (_format = %d, asked driver to give %d, got %d)",
			_format, requestedFormat, gotFormat);

		_error = "unknown format given by driver";
		_error += details;

		close();
		return false;
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
		return false;
	}

	int requeststereo = stereo;

	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo)==-1)
	{
		_error = "SNDCTL_DSP_STEREO failed - ";
		_error += strerror(errno);

		close();
		return false;
	}

	if (requeststereo != stereo)
	{
		_error = "audio device doesn't support number of requested channels";

		close();
		return false;
	}

	int speed = _samplingRate;

	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1)  
	{
		_error = "SNDCTL_DSP_SPEED failed - ";
		_error += strerror(errno);

		close();
		return false;
	}  

    /*
	 * Some soundcards seem to be able to only supply "nearly" the requested
	 * sampling rate, especially PAS 16 cards seem to quite radical supplying
	 * something different than the requested sampling rate ;)
	 *
	 * So we have a quite large tolerance here (when requesting 44100 Hz, it
	 * will accept anything between 38690 Hz and 49510 Hz). Most parts of the
	 * aRts code will do resampling where appropriate, so it shouldn't affect
	 * sound quality.
	 */
    int tolerance = _samplingRate/10+1000;

	if (abs(speed-_samplingRate) > tolerance)
	{  
		_error = "can't set requested samplingrate";

		char details[80];
		sprintf(details," (requested rate %d, got rate %d)",
			_samplingRate, speed);
		_error += details;

		close();
		return false;
	} 
	_samplingRate = speed;

	/*
	 * set the fragment settings to what the user requested
	 */
	
	_fragmentSize = requestedFragmentSize;
	_fragmentCount = requestedFragmentCount;

	/*
	 * lower 16 bits are the fragment size (as 2^S)
	 * higher 16 bits are the number of fragments
	 */
	int frag_arg = 0;

	int size = _fragmentSize;
	while(size > 1) { size /= 2; frag_arg++; }
	frag_arg += (_fragmentCount << 16);
	if(ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag_arg) == -1)
	{
		char buffer[1024];
		_error = "can't set requested fragments settings";
		sprintf(buffer,"size%d:count%d\n",_fragmentSize,_fragmentCount);
		close();
		return false;
	}

	/*
	 * now see what we really got as cards aren't required to supply what
	 * we asked for
	 */
	audio_buf_info info;
	if(ioctl(audio_fd,SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		_error = "can't retrieve fragment settings";
		close();
		return false;
	}

	// update fragment settings with what we got
	_fragmentSize = info.fragsize;
	_fragmentCount = info.fragstotal;

	artsdebug("buffering: %d fragments with %d bytes "
		"(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
		(float)(_fragmentSize*_fragmentCount) /
		(float)(2.0 * _samplingRate * _channels)*1000.0);

	/*
	 * Workaround for broken kernel drivers: usually filling up the audio
	 * buffer is _only_ required if _fullDuplex is true. However, there
	 * are kernel drivers around (especially everything related to ES1370/1371)
	 * which will not trigger select()ing the file descriptor unless we have
	 * written something first.
	 */
	char *zbuffer = (char *)calloc(sizeof(char), _fragmentSize);
	if(_format == 8)
		for(int zpos = 0; zpos < _fragmentSize; zpos++)
			zbuffer[zpos] |= 0x80;
	
	for(int fill = 0; fill < _fragmentCount; fill++)
	{
		int len = ::write(audio_fd,zbuffer,_fragmentSize);
		if(len != _fragmentSize)
		{
			arts_debug("AudioIOOSS: failed prefilling audio buffer (might cause synchronization problems in conjunction with full duplex)");
			fill = _fragmentCount+1;
		}
	}
	free(zbuffer);

	/*
	 * Triggering - the original aRts code did this for full duplex:
	 *
	 *  - stop audio i/o using SETTRIGGER(~(PCM_ENABLE_INPUT|PCM_ENABLE_OUTPUT))
	 *  - fill buffer (see zbuffer code two lines above)
	 *  - start audio i/o using SETTRIGGER(PCM_ENABLE_INPUT|PCM_ENABLE_OUTPUT)
	 *
	 * this should guarantee synchronous start of input/output. Today, it
	 * seems there are too many broken drivers around for this.
	 */

	if(device_caps & DSP_CAP_TRIGGER)
	{
		int enable_bits = 0;

		if(param(direction) & 1) enable_bits |= PCM_ENABLE_INPUT;
		if(param(direction) & 2) enable_bits |= PCM_ENABLE_OUTPUT;

		if(ioctl(audio_fd,SNDCTL_DSP_SETTRIGGER, &enable_bits) == -1)
		{
			_error = "can't start sound i/o";

			close();
			return false;
		}
	}
	return true;
}

void AudioIOOSS::close()
{
	::close(audio_fd);
}

void AudioIOOSS::setParam(AudioParam p, int& value)
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

int AudioIOOSS::getParam(AudioParam p)
{
	audio_buf_info info;
	switch(p)
	{
		case canRead:
				ioctl(audio_fd, SNDCTL_DSP_GETISPACE, &info);
				return info.bytes;
			break;

		case canWrite:
				ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info);
				return info.bytes;
			break;

		case selectFD:
				return audio_fd;
			break;

		case autoDetect:
				/* OSS works reasonable almost everywhere where it compiles */
				return 10;	
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIOOSS::read(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::read(audio_fd,buffer,size);
}

int AudioIOOSS::write(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::write(audio_fd,buffer,size);
}

#endif
