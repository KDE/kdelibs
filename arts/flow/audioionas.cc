    /*

    Copyright (C) 2001 Jochen Hoenicke
                       jochen@gnu.org

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
 * only compile nas AudioIO class if libaudio was detected during
 * configure
 */
#ifdef HAVE_LIBAUDIONAS

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

#include "audioio.h"
#include "audiosubsys.h"
#include "iomanager.h"
#include "dispatcher.h"

#include <audio/audiolib.h>

namespace Arts {

static AuBool
eventHandler(AuServer *, AuEvent *ev, AuEventHandlerRec *handler);

class AudioIONAS : public AudioIO, public IONotify {
protected:
	AuServer *aud;
	AuDeviceID device;
	AuFlowID flow;
	AuElement elements[2];

	int freeBytes;

public:
	AudioIONAS();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	void run();
	void notifyIO(int, int);
	int read(void *buffer, int size);
	int write(void *buffer, int size);

	friend AuBool 
	eventHandler(AuServer *, AuEvent *ev, AuEventHandlerRec *handler);
};

REGISTER_AUDIO_IO(AudioIONAS,"nas","Network Audio System");
};

using namespace std;
using namespace Arts;

static AuBool
Arts::eventHandler(AuServer *, AuEvent *ev, AuEventHandlerRec *handler)
{
	AudioIONAS *nas = (AudioIONAS *) handler->data;
	if (ev->type == AuEventTypeElementNotify)
	{
		AuElementNotifyEvent *event = (AuElementNotifyEvent *) ev;
		
		switch (event->kind)
		{
		case AuElementNotifyKindLowWater:
			nas->freeBytes += event->num_bytes;
			AudioSubSystem::the()->handleIO
				(AudioSubSystem::ioWrite);
			break;
		case AuElementNotifyKindState:
			if (event->cur_state ==  AuStatePause
			    && event->reason != AuReasonUser)
			{
				nas->freeBytes += event->num_bytes;
				AudioSubSystem::the()->handleIO
					(AudioSubSystem::ioWrite);
			}
			break;
		}
	}
	return true;
}

AudioIONAS::AudioIONAS()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 11025;
	paramStr(deviceName) = "null";
	param(fragmentSize) = 1024;
	param(fragmentCount) = 7;
	param(format) = 16;
	param(channels) = 2;
	param(direction) = 2;
}

bool AudioIONAS::open()
{
	char *server_msg;
	
	int& _channels = param(channels);
	int& _direction = param(direction);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);
	string& _device = paramStr(deviceName);
	string& _error = paramStr(lastError);
	int  _buf_samples, i;

	if((_direction & directionRead))
	{
		_error = "no record audio device";
		return false;
	}

	aud = AuOpenServer(_device.compare("null") == 0 
			   ? NULL : _device.c_str(), 
			   0, NULL, 0, NULL, &server_msg);
	if(aud == NULL)
	{
		_error = "device ";
		_error += _device;
		_error += " can't be opened (";
		_error += server_msg;
		_error += ")";
		return false;
	}

	device = AuNone;
	for (i = 0; i < AuServerNumDevices(aud); i++) 
	{
		AuDeviceAttributes *devattr = AuServerDevice(aud, i);
		if (AuDeviceKind(devattr) == AuComponentKindPhysicalOutput
		    && AuDeviceNumTracks(devattr) == _channels)
		{
			device = AuDeviceIdentifier(devattr);
			break;
		}
	}
				  
	if (device == AuNone)
	{
		_error = "Couldn't find an output device";
		return false;
	}

	if (!(flow = AuCreateFlow(aud, NULL)))
	{
		_error = "Couldn't create flow";
		return false;
	}

	_buf_samples = _fragmentSize;
	AuMakeElementImportClient(&elements[0], _samplingRate,
				  _format == 8 ? AuFormatLinearUnsigned8
				  : AuFormatLinearSigned16LSB,
				  _channels, AuTrue,
				  _buf_samples * _fragmentCount,
				  _buf_samples * (_fragmentCount)/2,
				  0, NULL);
	AuMakeElementExportDevice(&elements[1], 0, device, _samplingRate,
				  AuUnlimitedSamples, 0, NULL);
	AuSetElements(aud, flow, AuTrue, 2, elements, NULL);
	AuRegisterEventHandler(aud, AuEventHandlerIDMask, 0, flow,
			       eventHandler, (AuPointer) this);

	freeBytes = 0;
	AuStartFlow(aud, flow, NULL);

	Dispatcher::the()->ioManager()->watchFD(aud->fd, IOType::read, this);

	AuHandleEvents(aud);
	return true;
}

void AudioIONAS::close()
{
	Dispatcher::the()->ioManager()->remove(this, IOType::all);
	AuWriteElement(aud, flow, 0, 0, NULL, AuTrue, NULL);
	AuCloseServer(aud);
	aud = NULL;
}

void AudioIONAS::setParam(AudioParam p, int& value)
{
	param(p) = value;
}

int AudioIONAS::getParam(AudioParam p)
{
	switch(p)
	{
	case canWrite:
		return freeBytes;
		
	default:
		return param(p);
	}
}

void AudioIONAS::notifyIO(int, int)
{
	AuHandleEvents(aud);
}

int AudioIONAS::read(void *, int )
{
	return 0;
}

int AudioIONAS::write(void *buffer, int size)
{
	if (size > freeBytes)
		size = freeBytes;
	if (size > 0)
		AuWriteElement(aud, flow, 0, size, buffer, AuFalse, NULL);
	freeBytes -= size;
	if (freeBytes > 0)
		AudioSubSystem::the()->handleIO(AudioSubSystem::ioWrite);
	return size;
}

#endif
