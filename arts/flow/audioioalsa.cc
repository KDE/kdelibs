    /*

    Copyright (C) 2000 Jozef Kosoru
                       jozef.kosoru@pobox.sk
			  (C) 2000 Stefan Westerfeld
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
 * only compile 'alsa' AudioIO class if configure things it is a good idea
 */
#ifdef HAVE_ALSA

#include <sys/asoundlib.h>
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

class AudioIOALSA : public AudioIO {
protected:
	int audio_fd;
	int requestedFragmentSize;
	int requestedFragmentCount;

	enum BufferMode{block, stream};
	int m_card;
	int m_device;
	int m_format;
	BufferMode m_bufferMode;

	snd_pcm_t *m_pcm_handle;
	struct snd_pcm_channel_info m_cinfo;
	snd_pcm_format_t m_cformat;
	struct snd_pcm_channel_params m_params;
	struct snd_pcm_channel_setup m_setup;

	int setPcmParams(const int channel);

public:
	AudioIOALSA();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOALSA,"alsa","Advanced Linux Sound Architecture");
};

using namespace std;
using namespace Arts;

AudioIOALSA::AudioIOALSA()
{
 	param(samplingRate) = 44100;
	paramStr(deviceName) = "/dev/dsp";
	requestedFragmentSize = param(fragmentSize) = 1024;
	requestedFragmentCount = param(fragmentCount) = 7;
	param(channels) = 2;
	param(direction) = 2;

	/*
	 * default parameters
	 */
    m_card = snd_defaults_pcm_card();  //!! need interface !!
    m_device = snd_defaults_pcm_device(); //!!
    m_format = SND_PCM_SFMT_S16_LE;
    m_bufferMode = block;  //block/stream

	if(m_card >= 0) {
    	char* cardname;
    	snd_card_get_name(m_card, &cardname);

		//!! thats not what devicename is intended to do
		//!! devicename is an input information into
		//!! the "driver", to select which card to use
		//!! not an output information
    	paramStr(deviceName) = cardname;
    	free(cardname);
	}
}

bool AudioIOALSA::open()
{
	string& _error = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _direction = param(direction);

	/* open pcm device */
	int mode = SND_PCM_OPEN_NONBLOCK;

	if(_direction == directionReadWrite)
    	mode |= SND_PCM_OPEN_DUPLEX;
	else if(_direction == directionWrite)
    	mode |= SND_PCM_OPEN_PLAYBACK;
	else
	{
		_error = "invalid direction";
		return false;
	}

	int err;
	if((err = snd_pcm_open(&m_pcm_handle, m_card, m_device, direction)) < 0) {
		_error = "device: ";
		_error += _deviceName.c_str();
		_error += " can't be opened (";
		_error += snd_strerror(err);
		_error += ")";
		return false;
	}
	else {
		artsdebug("ALSA driver: %s", _deviceName.c_str());
	}

	snd_pcm_nonblock_mode(m_pcm_handle, 0);

	/* flush buffers */
	(void)snd_pcm_capture_flush(m_pcm_handle);
	if(_direction & directionRead)
		(void)snd_pcm_channel_flush(m_pcm_handle, SND_PCM_CHANNEL_CAPTURE);
	if(_direction & directionWrite)
		(void)snd_pcm_channel_flush(m_pcm_handle, SND_PCM_CHANNEL_PLAYBACK);

	/* check device capabilities */
	//maybe later

	/* set the fragment settings to what the user requested */
	_fragmentSize = requestedFragmentSize;
	_fragmentCount = requestedFragmentCount;

  	/* set PCM communication parameters */
  	if((_direction & directionRead) && setPcmParams(SND_PCM_CHANNEL_CAPTURE))
		return false;
  	if((_direction & directionWrite) && setPcmParams(SND_PCM_CHANNEL_PLAYBACK))
		return false;

  	/* prepare channel */
    if((_direction & directionRead) &&
		snd_pcm_channel_prepare(m_pcm_handle, SND_PCM_CHANNEL_CAPTURE) < 0)
	{
      	_error = "Unable to prepare capture channel!";
		return false;
    }
    if((_direction & directionWrite) &&
    	snd_pcm_channel_prepare(m_pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0)
	{
    	_error = "Unable to prepare playback channel!";
	  	return false;
    }

	/* obtain current PCM setup (may differ from requested one) */
	(void)memset(&m_setup, 0, sizeof(m_setup));

	m_setup.channel = SND_PCM_CHANNEL_PLAYBACK;
	if(snd_pcm_channel_setup(m_pcm_handle, &m_setup) < 0) {
    	_error = "Unable to obtain channel setup!";
		return false;
	}

	/* check samplerate */
	const int tolerance = _samplingRate/10+1000;
	if(abs(m_setup.format.rate-_samplingRate) > tolerance)
	{
		_error = "Can't set requested sampling rate!";
		char details[80];
		sprintf(details," (requested rate %d, got rate %d)",
            _samplingRate, m_setup.format.rate);
    	_error += details;
		return false;
  	}
	_samplingRate = m_setup.format.rate;

	/* check format */
	if(m_setup.format.format != m_format) {
    	_error = "Can't set requested format:";
    	_error += snd_pcm_get_format_name(m_format);
		return false;
	}

	/* check voices */
	if(m_setup.format.voices != _channels) {
		_error = "Audio device doesn't support number of requested channels!";
		return false;
	}

	/* update fragment settings with what we got */
	switch(m_bufferMode) {
		case block:
      			_fragmentSize = m_setup.buf.block.frag_size;
      			_fragmentCount = m_setup.buf.block.frags_max-1;
      		break;
    	case stream:
      			_fragmentSize = m_setup.buf.stream.queue_size;
      			_fragmentCount = 1;
      		break;
  	}

  	artsdebug("buffering: %d fragments with %d bytes "
            "(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
            (float)(_fragmentSize*_fragmentCount) /
            (float)(2.0 * _samplingRate * _channels)*1000.0);

  	/* obtain PCM file descriptor */
  	audio_fd = snd_pcm_file_descriptor(m_pcm_handle,
		(mode & SND_PCM_OPEN_PLAYBACK)	? SND_PCM_CHANNEL_PLAYBACK
										: SND_PCM_CHANNEL_CAPTURE);

  	/* start recording */
  	if((_direction & directionRead) & snd_pcm_capture_go(m_pcm_handle)){
    	_error = "Can't start recording!";
		return false;
  	}

  	return true;
}

void AudioIOALSA::close()
{
	snd_pcm_close(m_pcm_handle);
}

void AudioIOALSA::setParam(AudioParam p, int& value)
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

int AudioIOALSA::getParam(AudioParam p)
{
    snd_pcm_channel_status_t status;
    (void)memset(&status, 0, sizeof(status));

	switch(p)
	{
		case canRead:
      			status.channel = SND_PCM_CHANNEL_CAPTURE;
      			if(snd_pcm_channel_status(m_pcm_handle, &status) < 0) {
        			arts_warning("Capture channel status error!");
        			return -1;
      			}
				return status.free;
			break;

		case canWrite:
      			status.channel = SND_PCM_CHANNEL_PLAYBACK;
      			if(snd_pcm_channel_status(m_pcm_handle, &status) < 0) {
        			arts_warning("Playback channel status error!");
        			return -1;
      			}
				return status.free;
			break;

		case selectFD:
				return audio_fd;
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIOALSA::read(void *buffer, int size)
{
	arts_assert(false);		/* unsupported */
	arts_assert(audio_fd != 0);
	return ::read(audio_fd,buffer,size);
}

int AudioIOALSA::write(void *buffer, int size)
{
    assert(m_bufferMode == block);

	while(snd_pcm_write(m_pcm_handle, buffer, size) != size) {
        snd_pcm_channel_status_t status;
        (void)memset(&status, 0, sizeof(status));
        status.channel = SND_PCM_CHANNEL_PLAYBACK;

        if(snd_pcm_channel_status(m_pcm_handle, &status) < 0) {
        	arts_warning("Playback channel status error!");
        	return -1;
        }
		else if(status.status == SND_PCM_STATUS_UNDERRUN) {
        	artsdebug("Underrun at position: %d", status.scount);
          	if(snd_pcm_channel_prepare(m_pcm_handle, SND_PCM_CHANNEL_PLAYBACK)
														< 0) {
            	arts_warning("Underrun: playback prepare error!");
            	return -1;
          	}
        }
        else {
        	arts_warning("Unknown playback error!");
        	return -1;
        }
    }
	return size;
}

int AudioIOALSA::setPcmParams(const int channel)
{
	int &_samplingRate = param(samplingRate);
	int &_channels = param(channels);
	int &_fragmentSize = param(fragmentSize);
	int &_fragmentCount = param(fragmentCount);

	(void)memset(&m_cformat, 0, sizeof(m_cformat));
	m_cformat.interleave = 1;
	m_cformat.format = m_format;
	m_cformat.rate = _samplingRate;
	m_cformat.voices = _channels;

	(void)memset(&m_params, 0, sizeof(m_params));
	switch(m_bufferMode){
		case stream:
				m_params.mode=SND_PCM_MODE_STREAM;
			break;
		case block:
				m_params.mode=SND_PCM_MODE_BLOCK;
			break;
	}
	m_params.channel=channel;
	(void)memcpy(&m_params.format, &m_cformat, sizeof(m_cformat));
	if(channel==SND_PCM_CHANNEL_CAPTURE){
		m_params.start_mode=SND_PCM_START_GO;
		m_params.stop_mode=SND_PCM_STOP_ROLLOVER;
	}
	else{	//SND_PCM_CHANNEL_PLAYBACK
		m_params.start_mode= (m_bufferMode==block) ? SND_PCM_START_FULL : SND_PCM_START_DATA;
		m_params.stop_mode=SND_PCM_STOP_ROLLOVER;	// SND_PCM_STOP_STOP
		//use this ^^^ if you want to track underruns
	}

	switch(m_bufferMode){
		case stream:
			m_params.buf.stream.queue_size=1024*1024; //_fragmentSize*_fragmentCount;
			m_params.buf.stream.fill=SND_PCM_FILL_SILENCE_WHOLE;
			m_params.buf.stream.max_fill=1024;
		break;
		case block:
			m_params.buf.block.frag_size=_fragmentSize;
			if(channel==SND_PCM_CHANNEL_CAPTURE){
				m_params.buf.block.frags_max=1;
				m_params.buf.block.frags_min=1;
			}
			else{	//SND_PCM_CHANNEL_PLAYBACK
				m_params.buf.block.frags_max=_fragmentCount+1;
				m_params.buf.block.frags_min=1;
			}
	}
	if(snd_pcm_channel_params(m_pcm_handle, &m_params)<0){
		paramStr(lastError) = "Unable to set channel params!";
		return 1;
	}
	else {
		return 0;
	}
}
#endif
