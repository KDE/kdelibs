    /*

    Copyright (C) 2001 Aaron Williams
                       aaronw@home.com
	          (C) 2001 Stefan Westerfeld
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

/*
 * Audio support for Sun Solaris, written by Aaron Williams
 *
 *
 * Please send comments to aaronw@home.com
 *
 * This code has been tested with Solaris 7 running on a Sun Ultra 5
 *
 * Note that in Solaris the select support appears to be broken.
 * Because of this, we use a timer and a dispatcher instead so we
 * arn't polling (and sucking up most of the CPU).
 *
 * Currently read support has not been tested and will likely break
 * other code
 *
 * 8-bit audio support also does not work (which I don't consider a 
 * big deal).
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * Only compile this AudioIO class if we're on Solaris
 */
#ifdef USE_SOLARIS

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <sys/audioio.h>
#include <stropts.h>
#include <sys/conf.h>

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

// This looks like the maximum buffer size according to the sys/audio*.h 
// files on Solaris7
#define SUN_MAX_BUFFER_SIZE		(65536)

namespace Arts {

    class AudioIOSun : public AudioIO, public TimeNotify {
    protected:
		uint_t bytesRead, bytesWritten, bytesPerSec;
		uint_t bytesPerSample;
		timeval start;
		int audio_fd;
		int requestedFragmentSize;
		int requestedFragmentCount;
		audio_info_t auinfo;
		
#ifdef WORDS_BIGENDIAN
		static const int defaultFormat = 17;
#else
		static const int defaultFormat = 16;
#endif
		
    public:
		AudioIOSun();

		// Timer callback
		void notifyTime();

		void setParam(AudioParam param, int& value);

		int getParam(AudioParam param);

		bool open();
		void close();
		int read(void *buffer, int size);
		int write(void *buffer, int size);
    };

    REGISTER_AUDIO_IO(AudioIOSun,"sun","Sun audio input/output");

};

using namespace std;
using namespace Arts;

AudioIOSun::AudioIOSun()
{
    /*
     * default parameters
     */
    param(samplingRate) = 44100;
	// solaris convention to support SunRays run out-of-the-box
	const char *audioDev = getenv("AUDIODEV");
    paramStr(deviceName) = (audioDev != 0)?audioDev:"/dev/audio";
    param(fragmentSize) = 1024;
    param(fragmentCount) = 7;
    param(channels) = 2;
    param(direction) = 2;
    param(format) = defaultFormat;
}

// Opens the audio device
bool AudioIOSun::open()
{
    string& _error = paramStr(lastError);
    string& _deviceName = paramStr(deviceName);
    int& _channels = param(channels);
    int& _fragmentSize = param(fragmentSize);
    int& _fragmentCount = param(fragmentCount);
    int& _samplingRate = param(samplingRate);
    int& _format = param(format);

    int mode;

    if (param(direction) == 3)
		mode = O_RDWR;
    else if (param(direction) == 2)
		mode = O_WRONLY;
    else
    {
		_error = "invalid direction";
		return false;
    }

    audio_fd = ::open(_deviceName.c_str(), mode, 0);

    if (audio_fd < 0)
    {
		_error = "device ";
		_error += _deviceName.c_str();
		_error += " can't be opened (";
		_error += strerror(errno);
		_error += ")";
		return false;
    }

    fcntl(audio_fd, F_SETFL, O_NDELAY);

    AUDIO_INITINFO(&auinfo);

    if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0)
    {
		_error = "device ";
		_error += _deviceName.c_str();
		_error += " AUDIO_GETINFO failed (";
		_error += strerror(errno);
		_error += ")";
		return false;
    }

    if(_format != 8) _format = defaultFormat;
#if 0
    printf("param(direction)=%d\n", param(direction));
    printf("format: %d\n", _format);
    printf("channels: %d\n", _channels);		
    printf("sampling rate: %d\n", _samplingRate);
#endif

    bytesPerSample = ((_format == 8) ? 8 : 16)/8 * _channels;

    auinfo.play.precision = (_format == 8) ? 8 : 16;
    if (param(direction) == 3)
		auinfo.record.precision = (_format == 8) ? 8 : 16;

    auinfo.play.encoding = AUDIO_ENCODING_LINEAR;
    if (param(direction) == 3)
		auinfo.record.encoding = AUDIO_ENCODING_LINEAR;

    auinfo.play.channels = _channels;
    if (param(direction) == 3)
		auinfo.record.channels = _channels;

    auinfo.play.sample_rate = _samplingRate;
    if (param(direction) == 3)
		auinfo.record.sample_rate = _samplingRate;

    if (ioctl(audio_fd, AUDIO_SETINFO, &auinfo) < 0)
    {
		_error = "AUDIO_SETINFO failed - ";
		_error += strerror(errno);

		close();
		return false;
    }

    
    if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0)
    {
		_error = "device ";
		_error += _deviceName.c_str();
		_error += " AUDIO_GETINFO failed (";
		_error += strerror(errno);
		_error += ")";
		return false;
    }

    if (auinfo.play.precision != (uint_t)((_format == 8) ? 8 : 16) ||
		(param(direction) == 3 && 
		 auinfo.record.precision != (uint_t)((_format == 8) ? 8 : 16)))
    {
		char play_details[80];
		char record_details[80];
		sprintf(play_details, 
				" (_format = %d, asked driver to give %d, got %d)",
				_format, _format, auinfo.play.precision);
		if (param(direction) == 3)
		    sprintf(record_details, 
				    " (_format = %d, asked driver to give %d, got %d)",
				    _format, _format, auinfo.record.precision);

		_error = "Can't set playback and/or record format  ";
		_error += "Play format: ";
		_error += play_details;
		if (param(direction) == 3) {
		    _error += "  Record format: ";
		    _error += record_details;
		}
		close();
		return false;
    }
    if ((auinfo.play.encoding != (uint_t)AUDIO_ENCODING_LINEAR) ||
		(param(direction) == 3 &&
		 auinfo.record.encoding != (uint_t)AUDIO_ENCODING_LINEAR))
    {
		char play_encoding[80], record_encoding[80];
		sprintf(play_encoding, "(%d bits, %d encoding)",
				auinfo.play.precision, auinfo.play.encoding);
		sprintf(record_encoding, "(%d bits, %d encoding)",
				auinfo.record.precision, auinfo.record.encoding);

		_error = "Can't set playback and/or record format";
		_error += "requested format was ";
		_error += (_format == 8) ? "8-bit AUDIO_ENCODING_LINEAR" :
		    "16-bit AUDIO_ENCODING_LINEAR";
		_error += ", got playback format ";
		_error += play_encoding;
		if (param(direction) == 3) {
		    _error += ", record format ";
		    _error += record_encoding;
		}

		close();
		return false;
    }

    if (auinfo.play.channels != (uint_t)_channels) {
		_error = "Audio device doesn't support number of ";
		_error += "requested playback channels";
		close();
		return false;
    }
    if (param(direction) == 3 && auinfo.record.channels != (uint_t)_channels) {
		_error = "Audio device doesn't support number of ";
		_error += "requested record channels";
		close();
		return false;
    }

    int tolerance = _samplingRate/10+1000;

    if (abs(int(auinfo.play.sample_rate - _samplingRate)) > tolerance)
    {
		_error = "can't set requested playback sampling rate";

		char details[80];
		sprintf(details," (requested rate %d, got rate %d)",
				_samplingRate, auinfo.play.sample_rate);
		_error += details;

		close();
		return false;
    }

    if (param(direction) == 3 &&
		abs(int(auinfo.record.sample_rate - _samplingRate)) > tolerance)
    {
		_error = "can't set requested record sampling rate";

		char details[80];
		sprintf(details," (requested rate %d, got rate %d)",
				_samplingRate, auinfo.play.sample_rate);
		_error += details;

		close();
		return false;
    }


    /*
     * don't allow unreasonable large fragmentSize/Count combinations,
     * because "real" hardware also doesn't
     */

    if(_fragmentSize > SUN_MAX_BUFFER_SIZE) 
		_fragmentSize = SUN_MAX_BUFFER_SIZE;

    while(_fragmentSize * _fragmentCount > SUN_MAX_BUFFER_SIZE)
		_fragmentCount--;

    bytesRead = bytesWritten = 0;
    bytesPerSec = _channels * 2 * _samplingRate;

    // Install the timer
    Dispatcher::the()->ioManager()->addTimer(10, this);

    gettimeofday(&start,0);

    return true;
}

void AudioIOSun::close()
{
    ::close(audio_fd);
    Dispatcher::the()->ioManager()->removeTimer(this);
}

// This is called on each timer tick
void AudioIOSun::notifyTime()
{
    int& _direction = param(direction);
    int& _fragmentSize = param(fragmentSize);

    for (;;) {
		int todo = 0;
		if ((_direction & directionRead) && getParam(canRead) > _fragmentSize)
		    todo |= AudioSubSystem::ioRead;

		if ((_direction & directionWrite) && getParam(canWrite) > _fragmentSize)
		    todo |= AudioSubSystem::ioWrite;

		if (!todo)
		    return;

		AudioSubSystem::the()->handleIO(todo);
    }
}

void AudioIOSun::setParam(AudioParam p, int& value)
{
    switch(p) {
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

int AudioIOSun::getParam(AudioParam p)
{
    int bytes;
    int count;

    switch(p)
    {
    case canRead:
		if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0)
		    return (0);
		bytes = (auinfo.record.samples * bytesPerSample) - bytesRead;
		if (bytes < 0) {
		    printf("Error: bytes %d < 0, samples=%u, bytesRead=%u\n",
				   bytes, auinfo.record.samples, bytesRead);
		    bytes = 0;
		}
		return bytes;

    case canWrite:
		if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0) 
		    return (0);
		count = SUN_MAX_BUFFER_SIZE - 
		    (bytesWritten - (auinfo.play.samples * bytesPerSample));
		return count;

	case autoDetect:
		/*
		 * If we're on Solaris, this driver is the one that will work,
		 * and if we're not on Solaris, it won't be compiled anyway.
		 */
		return 12;

    default:
		return param(p);
    }
}

int AudioIOSun::read(void *buffer, int size)
{
    size = ::read(audio_fd, buffer, size);
    if (size < 0)
		return 0;

    bytesRead += size;
    return size;
}

int AudioIOSun::write(void *buffer, int size)
{
    size = ::write(audio_fd, buffer, size);
    bytesWritten += size;
    return size;
}

#endif /* USE_SOLARIS */
