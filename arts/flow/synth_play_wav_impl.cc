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

#include "config.h"
#ifdef HAVE_LIBAUDIOFILE
#include "artsflow.h"
#include "stdsynthmodule.h"
#include "debug.h"
#include "cachedwav.h"
#include "convert.h"
#include <stdio.h>
#include <iostream>

extern "C" {
/* Some versions of libaudiofile seem to lack the extern "C" declaration,
 * so you you may need that extra one.
 *
 * Other versions of libaudiofile seem to have two closing "}" in aupvlist.h,
 * so if you can't compile, this, check that /usr/include/aupvlist.h contains
 * something like that
 *
 * #ifdef __cplusplus
 * }
 * #endif 
 *
 * only once not twice.
 */
#include "audiofile.h"
}

#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Arts;


CachedWav *CachedWav::load(Cache *cache, string filename)
{
	CachedWav *wav;

	wav = (CachedWav *)cache->get(string("CachedWav:")+filename);
	if(!wav) {
		wav = new CachedWav(cache,filename);

		if(!wav->initOk)		// loading failed
		{
			wav->decRef();
			return 0;
		}
	}

	return(wav);
}

bool CachedWav::isValid()
{
	if(!initOk)
		return false;

	struct stat newstat;

	lstat(filename.c_str(),&newstat);
	return(newstat.st_mtime == oldstat.st_mtime);
}

int CachedWav::memoryUsage()
{
	return(bufferSize);
}

CachedWav::CachedWav(Cache *cache, string filename) : CachedObject(cache),
								 filename(filename),initOk(false), buffer(0)
{
	int sampleFormat;
	AFframecount	frameCount;
	AFfilehandle	file;

	setKey(string("CachedWav:")+filename);

	if(lstat(filename.c_str(),&oldstat) == -1)
	{
		arts_info("CachedWav: Can't stat file '%s'", filename.c_str());
		return;
	}

	file = afOpenFile(filename.c_str(), "r", NULL);
	if(!file)
	{
		arts_info("CachedWav: Can't read file '%s'", filename.c_str());
		return;
	}

	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);
	if(frameCount <= 0 || frameCount >= INT_MAX)
	{
		arts_info("CachedWav: Invalid length for '%s'", filename.c_str());
		afCloseFile(file);
		return;
	}

	channelCount = afGetChannels(file, AF_DEFAULT_TRACK);
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);

	// we want everything converted to little endian unconditionally
	afSetVirtualByteOrder(file,AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);

	arts_debug("loaded wav %s",filename.c_str());
	arts_debug("  sample format: %d, sample width: %d",
		sampleFormat,sampleWidth);
	arts_debug("   channelCount: %d",channelCount);
	arts_debug("     frameCount: %d",frameCount);

	// different handling required for other sample widths
	assert(sampleWidth == 16 || sampleWidth == 8);

	long frameSize = (sampleWidth/8)*channelCount;
	samplingRate = afGetRate(file, AF_DEFAULT_TRACK);

	/*
	 * if we don't know the track bytes, we'll have to figure out ourselves
	 * how many frames are stored here - it would be nicer if libaudiofile
	 * let us know somehow whether the value returned for getFrameCount
	 * means "don't know" or is really the correct length
	 */
	int trackBytes = afGetTrackBytes(file, AF_DEFAULT_TRACK);
	if(trackBytes == -1)
	{
		arts_debug("unknown length");
		long fcount = 0, f = 0;

		list<void *> blocks;
		do
		{
			void *block = malloc(1024 * frameSize);

			f = afReadFrames(file, AF_DEFAULT_TRACK,block,1024);
			if(f > 0)
			{
				fcount += f;
				blocks.push_back(block);
			}
			else
			{
				free(block);
			}
		} while(f > 0);

		frameCount = fcount;
		arts_debug("figured out frameCount = %ld", fcount);

		bufferSize = frameCount * frameSize;
		buffer = new uchar[bufferSize];
		assert(buffer);

		// reassemble and free the blocks
		while(!blocks.empty())
		{
			void *block = blocks.front();
			blocks.pop_front();

			f = (fcount>1024)?1024:fcount;
			memcpy(&buffer[(frameCount-fcount)*frameSize],block,f*frameSize);
			fcount -= f;
		}
		assert(fcount == 0);
	}
	else
	{
		bufferSize = frameCount * frameSize;
		buffer = new uchar[bufferSize];
		assert(buffer);

		afReadFrames(file, AF_DEFAULT_TRACK,buffer,frameCount);
	}

	afCloseFile(file);
	initOk = true;
}

CachedWav::~CachedWav()
{
	if(buffer)
		delete[] buffer;
}

namespace Arts {

class Synth_PLAY_WAV_impl : public Synth_PLAY_WAV_skel, public StdSynthModule {
protected:
	double flpos;

	float _speed;
	string _filename;
	bool _finished;
	CachedWav *cachedwav;

	void unload()
	{
		if(cachedwav)
		{
			cachedwav->decRef();
			cachedwav = 0;
		}
	}

	void load()
	{
		// unload the old file if necessary
		unload();

		// load the new (which will reset the position)
		cachedwav = CachedWav::load(Cache::the(), _filename);
		flpos = 0.0;
	}

public:
	float speed() { return _speed; }
	void speed(float newSpeed) { _speed = newSpeed; }

	string filename() { return _filename; }
	void filename(const string& filename) { _filename = filename; load(); }

	void finished(bool f)
	{
		if(_finished != f)
		{
			_finished = f;
			finished_changed(f);
		}
	}
	bool finished() { return _finished; }

	Synth_PLAY_WAV_impl();
	~Synth_PLAY_WAV_impl();

	void streamInit();
	void calculateBlock(unsigned long samples);
};

REGISTER_IMPLEMENTATION(Synth_PLAY_WAV_impl);

};

Synth_PLAY_WAV_impl::Synth_PLAY_WAV_impl()
{
	cachedwav = 0;
	_speed = 1.0;
	_filename = "";
	_finished = false;
}

Synth_PLAY_WAV_impl::~Synth_PLAY_WAV_impl()
{
	unload();
}

void Synth_PLAY_WAV_impl::streamInit()
{
	finished(false);
}

void Synth_PLAY_WAV_impl::calculateBlock(unsigned long samples)
{
	unsigned long haveSamples = 0;

	if(cachedwav)
	{
		double speed = cachedwav->samplingRate / samplingRateFloat * _speed;

		haveSamples = uni_convert_stereo_2float(samples, cachedwav->buffer,
		   cachedwav->bufferSize,cachedwav->channelCount,cachedwav->sampleWidth,
		   left,right,speed,flpos);

		flpos += (double)haveSamples * speed;
	}

	if(haveSamples != samples)
	{
		unsigned long i;

		for(i=haveSamples;i<samples;i++)
			left[i] = right[i] = 0.0;

		finished(true);
	}

/*
	float speed = 0.0;
	unsigned long haveSamples = 0;

	if(cachedwav)
	{
		float allSamples = cachedwav->bufferSize*8 /
	    				   cachedwav->sampleWidth/cachedwav->channelCount;
		float fHaveSamples = allSamples - flpos;

		speed = cachedwav->samplingRate / (float)samplingRate * _speed;

		fHaveSamples /= speed;
		fHaveSamples -= 2.0;	// one due to interpolation and another against
								// rounding errors
		if(fHaveSamples > 0)
		{
			haveSamples = (int)fHaveSamples;
			if(haveSamples > samples) haveSamples = samples;
		}
	}

	if(haveSamples)				// something left to play?
	{
		if(cachedwav->channelCount == 1)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_mono_16le_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			else {
				interpolate_mono_8_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(cachedwav->channelCount == 2)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_stereo_i16le_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
			else {
				interpolate_stereo_i8_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
		} else {
			assert(false);
		}

		flpos += (float)haveSamples * speed;
	}

	if(haveSamples != samples)
	{
		unsigned long i;

		for(i=haveSamples;i<samples;i++)
			left[i] = right[i] = 0.0;

		_finished = true;
	}
*/
}


#if 0
class Synth_PLAY_WAV :public SynthModule {
protected:
	CachedWav *cachedwav;

	unsigned char *buffer;
	int channelCount;
	unsigned long bufferSize, position, bytesPerSample;

	// inputs:
	enum { PROP_FILENAME };

	// outputs:
	enum { LEFT, RIGHT, DONE };

public:
	void Initialize();
	void DeInitialize();
	void Calculate() { assert(false); }
	void CalculateBlock(unsigned long samples);
	string getParams() { return("_filename;left,right,done"); }
	static void *Creator() { return new Synth_PLAY_WAV; }
};

ModuleClient MC_Synth_PLAY_WAV(SynthModule::get_MS,"Synth_PLAY_WAV",Synth_PLAY_WAV::Creator);

void Synth_PLAY_WAV::CalculateBlock(unsigned long samples)
{
	unsigned long haveSamples = samples;
	unsigned long remainingSamples;

	remainingSamples = (bufferSize-position)/bytesPerSample;
	if(haveSamples > remainingSamples) haveSamples = remainingSamples;

	float *left = out[LEFT], *right = out[RIGHT], *done = out[DONE];
	unsigned long i;

	if(haveSamples)
	{
		if(channelCount == 1)
		{
			if(bytesPerSample == 2) {
				convert_mono_16le_float(haveSamples,&buffer[position],left);
			}
			else {
				convert_mono_8_float(haveSamples,&buffer[position],left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(channelCount == 2)
		{
			if(bytesPerSample == 2) {
				convert_stereo_i16le_2float(haveSamples,&buffer[position],
																	left,right);
			}
			else {
				convert_stereo_i8_2float(haveSamples,&buffer[position],
																	left,right);
			}
		} else {
			assert(false);
		}

		for(i=0;i<haveSamples;i++) done[i] = 0.0;

		position += bytesPerSample*channelCount*haveSamples;
	}

	for(i=haveSamples;i<samples;i++)
	{
		left[i] = right[i] = 0.0; done[i] = 1.0;		// ready, kill me ;)
	}
}

void Synth_PLAY_WAV::DeInitialize()
{
	cachedwav->decRef();
}

void Synth_PLAY_WAV::Initialize()
{
	cachedwav = CachedWav::load(Synthesizer->getCache(),
									getStringProperty(PROP_FILENAME));

	// may take some speed to access cachedwav every time
	bufferSize = cachedwav->bufferSize;	
	channelCount = cachedwav->channelCount;
	buffer = cachedwav->buffer;
	bytesPerSample = cachedwav->sampleWidth/8;

	haveCalculateBlock = true;
	position = 0;
}

class Synth_PLAY_PITCHED_WAV :public SynthModule {
protected:
	CachedWav *cachedwav;
	float flpos;

	// inputs:
	enum { FREQUENCY,RECFREQUENCY, PROP_FILENAME };

	// outputs:
	enum { LEFT, RIGHT, DONE };

public:
	void Initialize();
	void DeInitialize();
	void Calculate() { assert(false); }
	void CalculateBlock(unsigned long samples);
	string getParams() { return("frequency,recfrequency,_filename;left,right,done"); }
	static void *Creator() { return new Synth_PLAY_PITCHED_WAV; }
};

ModuleClient MC_Synth_PLAY_PITCHED_WAV(SynthModule::get_MS,"Synth_PLAY_PITCHED_WAV",Synth_PLAY_PITCHED_WAV::Creator);

void Synth_PLAY_PITCHED_WAV::CalculateBlock(unsigned long samples)
{
	float frequency = in[FREQUENCY][0], recfrequency = in[RECFREQUENCY][0];
	float allSamples = cachedwav->bufferSize*8 /
	                   cachedwav->sampleWidth/cachedwav->channelCount;
	float fHaveSamples = allSamples - flpos;
	float speed = cachedwav->samplingRate / (float)samplingRate *
				  frequency / recfrequency;

	fHaveSamples /= speed;
	fHaveSamples -= 2.0;		// one due to interpolation and another against
							// rounding errors

	unsigned long haveSamples;

	if(fHaveSamples < 0)
	{
		haveSamples = 0;
	}
	else
	{
		haveSamples = fHaveSamples;
		if(haveSamples > samples) haveSamples = samples;
	}

	float *left = out[LEFT], *right = out[RIGHT], *done = out[DONE];
	unsigned long i;

	if(haveSamples)
	{
		if(cachedwav->channelCount == 1)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_mono_16le_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			else {
				interpolate_mono_8_float(haveSamples,
							flpos,speed,cachedwav->buffer,left);
			}
			memcpy(right,left,sizeof(float)*haveSamples);
		}
		else if(cachedwav->channelCount == 2)
		{
			if(cachedwav->sampleWidth == 16) {
				interpolate_stereo_i16le_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
			else {
				interpolate_stereo_i8_2float(haveSamples,
							flpos,speed,cachedwav->buffer,left,right);
			}
		} else {
			assert(false);
		}

		for(i=0;i<haveSamples;i++) done[i] = 0.0;

		flpos += (float)haveSamples * speed;
	}

	for(i=haveSamples;i<samples;i++)
	{
		left[i] = right[i] = 0.0; done[i] = 1.0;		// ready, kill me ;)
	}
}

void Synth_PLAY_PITCHED_WAV::DeInitialize()
{
	cachedwav->decRef();
}

void Synth_PLAY_PITCHED_WAV::Initialize()
{
	cachedwav = CachedWav::load(Synthesizer->getCache(),
									getStringProperty(PROP_FILENAME));

	haveCalculateBlock = true;
	flpos = 0.0;
}
#endif

#else
#ifdef __GNUC__
#warning "No libaudiofile available, that means, you won't be able to play wavs"
#endif
#endif

