/*

    Copyright (C) 2001 Carsten Griwodz
                       griff@ifi.uio.no

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

#ifdef _AIX

/*
 * The audio header files exist even if there is not soundcard the
 * the AIX machine. You won't be able to compile this code on AIX3
 * which had ACPA support, so /dev/acpa is not checked here.
 * I have no idea whether the Ultimedia Audio Adapter is actually
 * working or what it is right now.
 * For PCI machines including PowerSeries 850, baud or paud should
 * work. The DSP (MWave?) of the 850 laptops may need microcode
 * download. This is not implemented.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/machine.h>
#undef BIG_ENDIAN
#include <sys/audio.h>

#ifndef AUDIO_BIG_ENDIAN
#define AUDIO_BIG_ENDIAN BIG_ENDIAN
#endif

#include "debug.h"
#include "audioio.h"

namespace Arts {

class AudioIOAIX : public AudioIO {
    int openDevice();

protected:
    int audio_fd;

public:
    AudioIOAIX();

    void setParam(AudioParam param, int& value);
    int getParam(AudioParam param);

    bool open();
    void close();
    int read(void *buffer, int size);
    int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOAIX,"paud","Personal Audio Device");
};

using namespace std;
using namespace Arts;

int AudioIOAIX::openDevice()
{
	char devname[14];
	int  fd;
	for ( int dev=0; dev<4; dev++ )
	{
		for ( int chan=1; chan<8; chan++ )
		{
			sprintf(devname,"/dev/paud%d/%d",dev,chan);
			fd = ::open (devname, O_WRONLY, 0);
			if ( fd >= 0 )
			{
				paramStr(deviceName) = devname;
				return fd;
			}
			sprintf(devname,"/dev/baud%d/%d",dev,chan);
			fd = ::open (devname, O_WRONLY, 0);
			if ( fd >= 0 )
			{
				paramStr(deviceName) = devname;
				return fd;
			}
		}
	}
	return -1;
}

AudioIOAIX::AudioIOAIX()
{
	int fd = openDevice();
	if( fd >= 0 )
	{
		audio_status audioStatus;
		memset( &audioStatus, 0, sizeof(audio_status) );
		ioctl(fd, AUDIO_STATUS, &audioStatus);

		audio_buffer audioBuffer;
		memset( &audioBuffer, 0, sizeof(audio_buffer) );
		ioctl(fd, AUDIO_BUFFER, &audioBuffer);

		::close( fd );

		/*
		 * default parameters
		 */
		param(samplingRate)  = audioStatus.srate;
		param(fragmentSize)  = audioStatus.bsize;
		param(fragmentCount) = audioBuffer.write_buf_cap / audioStatus.bsize;
		param(channels)      = audioStatus.channels;
		param(direction)     = 2;

		param(format)        = ( audioStatus.bits_per_sample==8 ) ? 8
			: ( ( audioStatus.flags & AUDIO_BIG_ENDIAN ) ? 17 : 16 );
	}
}

bool AudioIOAIX::open()
{
	string& _error = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);

	int mode;

	switch( param(direction) )
	{
		case 1 : mode = O_RDONLY | O_NDELAY; break;
		case 2 : mode = O_WRONLY | O_NDELAY; break;
		case 3 :
				 _error = "open device twice to RDWR";
				 return false;
		default :
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

	if( (_channels!=1) && (_channels!=2) )
	{
		_error = "internal error; set channels to 1 (mono) or 2 (stereo)";

		close();
		return false;
	}

	// int requeststereo = stereo;

	// int speed = _samplingRate;

	audio_init audioInit;
	memset( &audioInit, 0, sizeof(audio_init) );
	audioInit.srate = _samplingRate;
	audioInit.bits_per_sample = ((_format==8)?8:16);
	audioInit.bsize = _fragmentSize;
	audioInit.mode = PCM;
	audioInit.channels = _channels;
	audioInit.flags = 0;
	audioInit.flags |= (_format==17) ? AUDIO_BIG_ENDIAN : 0;
	audioInit.flags |= (_format==8)  ? 0 : SIGNED;
	audioInit.operation = (param(direction)==1) ? RECORD : PLAY;

	if ( ioctl(audio_fd, AUDIO_INIT, &audioInit) < 0 )
	{
		_error = "AUDIO_INIT failed - ";
		_error += strerror(errno);
		switch ( audioInit.rc )
		{
			case 1 :
				_error += "Couldn't set audio format: DSP can't do play requests";
				break;
			case 2 :
				_error += "Couldn't set audio format: DSP can't do record requests";
				break;
			case 4 :
				_error += "Couldn't set audio format: request was invalid";
				break;
			case 5 :
				_error += "Couldn't set audio format: conflict with open's flags";
				break;
			case 6 :
				_error += "Couldn't set audio format: out of DSP MIPS or memory";
				break;
			default :
				_error += "Couldn't set audio format: not documented in sys/audio.h";
				break;
		}

		close();
		return false;
	}

	if (audioInit.channels != _channels)
	{
		_error = "audio device doesn't support number of requested channels";
		close();
		return false;
	}

	switch( _format )
	{
		case 8 :
			if (audioInit.flags&AUDIO_BIG_ENDIAN==1)
			{
				_error = "setting little endian format failed";
				close();
				return false;
			}
			if (audioInit.flags&SIGNED==1)
			{
				_error = "setting unsigned format failed";
				close();
				return false;
			}
			break;
		case 16 :
			if (audioInit.flags&AUDIO_BIG_ENDIAN==1)
			{
				_error = "setting little endian format failed";
				close();
				return false;
			}
			if (audioInit.flags&SIGNED==0)
			{
				_error = "setting signed format failed";
				close();
				return false;
			}
			break;
		case 17 :
			if (audioInit.flags&AUDIO_BIG_ENDIAN==0)
			{
				_error = "setting big endian format failed";
				close();
				return false;
			}
			if (audioInit.flags&SIGNED==0)
			{
				_error = "setting signed format failed";
				close();
				return false;
			}
			break;
		default :
			break;
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

	if (abs(audioInit.srate - _samplingRate) > tolerance)
	{  
		_error = "can't set requested samplingrate";

		char details[80];
		sprintf(details," (requested rate %d, got rate %ld)",
				_samplingRate, audioInit.srate);
		_error += details;

		close();
		return false;
	} 
	_samplingRate = audioInit.srate;

	_fragmentSize = audioInit.bsize;
	_fragmentCount = audioInit.bsize / audioInit.bits_per_sample;

	audio_buffer buffer_info;
	ioctl(audio_fd, AUDIO_BUFFER, &buffer_info);
	_fragmentCount = buffer_info.write_buf_cap / audioInit.bsize;


	artsdebug("buffering: %d fragments with %d bytes "
			"(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
			(float)(_fragmentSize*_fragmentCount) /
			(float)(2.0 * _samplingRate * _channels)*1000.0);

	return true;
}

void AudioIOAIX::close()
{
	::close(audio_fd);
}

void AudioIOAIX::setParam(AudioParam p, int& value)
{
	param(p) = value;
}

int AudioIOAIX::getParam(AudioParam p)
{
	audio_buffer info;
	switch(p)
	{
		case canRead:
			ioctl(audio_fd, AUDIO_BUFFER, &info);
			return (info.read_buf_cap - info.read_buf_size);
			break;

		case canWrite:
			ioctl(audio_fd, AUDIO_BUFFER, &info);
			return (info.write_buf_cap - info.write_buf_size);
			break;

		case selectReadFD:
			return (param(direction) & directionRead)?audio_fd:-1;
			break;

		case selectWriteFD:
			return (param(direction) & directionWrite)?audio_fd:-1;
			break;

		case autoDetect:
			/* You may prefer OSS if it works, e.g. on 43P 240
			 * or you may prefer UMS, if anyone bothers to write
			 * a module for it.
			 */
			return 2;
			break;

		default:
			return param(p);
			break;
	}
}

int AudioIOAIX::read(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::read(audio_fd,buffer,size);
}

int AudioIOAIX::write(void *buffer, int size)
{
	arts_assert(audio_fd != 0);
	return ::write(audio_fd,buffer,size);
}

#endif

