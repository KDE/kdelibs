#include "config.h"

#ifdef HAVE_LIBAUDIOFILE

#include "soundserver.h"
#include "cachedwav.h"
#include "stdsynthmodule.h"
#include "convert.h"
#include "debug.h"
#include <iostream>
#include <math.h>

using namespace std;
using namespace Arts;

class WavPlayObject_impl :public WavPlayObject_skel, public StdSynthModule {
protected:
	CachedWav *wav;
	double flpos;
	poState _state;

	int sampleCount()
	{
		if(!wav) return 0;
		return wav->bufferSize / wav->channelCount / (wav->sampleWidth/8);
	}
public:
	/*
	 * construction, destruction
	 */
	WavPlayObject_impl() :wav(0), flpos(0.0), _state(posIdle)
	{
		arts_debug("WavPlayObject_impl");
	}

	virtual ~WavPlayObject_impl()
	{
		arts_debug("~WavPlayObject_impl");
		if(wav) wav->decRef();
	}

	/*
	 * KMedia2 private interface
	 */
	
	bool loadMedia(const string &filename) {
		arts_debug("Wav: loadMedia %s", filename.c_str());
		wav = CachedWav::load(Cache::the(), filename);
		return (wav != 0);
	}

	/*
	 * KMedia2 interface
	 */
	string description() {
		return "no description (see wavplayobject_impl.cc)";
	}

	poTime currentTime() {
		if(!wav) return poTime(0,0,0,"samples");

		float timesec = flpos / (float)wav->samplingRate;
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime(timesec, timems, (int)flpos, "samples");
	}

    poTime overallTime() {
		float timesec = (float)sampleCount() / (float)wav->samplingRate;
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime(timesec, timems, (int)flpos, "samples");
	}

	poCapabilities capabilities() {
  		return static_cast<poCapabilities>(capPause+capSeek);
	}

	string mediaName() {
		return "";
	}

	poState state() {
		return _state;
	}

	void play() {
		arts_debug("play");
		_state = posPlaying;
	}

	void halt() {
		arts_debug("stop");
		_state = posIdle;
		flpos = 0.0;
	}

	void seek(const class poTime &newTime) {
		if(!wav) return;

		float fnewsamples = -1;
		if(newTime.seconds != -1 && newTime.ms != -1)
		{
			float flnewtime = (float)newTime.seconds+((float)newTime.ms/1000.0);
			fnewsamples = flnewtime * (float)wav->samplingRate;
		}
		else if(newTime.custom >= 0 && newTime.customUnit == "samples")
		{
			fnewsamples = newTime.custom;
		}

		if(fnewsamples > (float)sampleCount())
			fnewsamples = (float)sampleCount();

		if(fnewsamples >= 0) flpos = fnewsamples;
	}

	void pause() {
		arts_debug("pause");
		_state = posPaused;
	}

	/*
	 * SynthModule interface
	 * - where is stop? initialize?
	 */

	void streamInit() {
		arts_debug("streamInit");
	}

	void streamStart() {
		arts_debug("streamStart");
	}

	void calculateBlock(unsigned long samples) {
		unsigned long haveSamples = 0;

		if(wav && _state == posPlaying)
		{
			double speed = wav->samplingRate / samplingRateFloat;

			haveSamples = uni_convert_stereo_2float(samples, wav->buffer,
		   		wav->bufferSize,wav->channelCount,wav->sampleWidth,
		   		left,right,speed,flpos);

			flpos += (double)haveSamples * speed;
		}

		if(haveSamples != samples)
		{
			unsigned long i;

			for(i=haveSamples;i<samples;i++)
				left[i] = right[i] = 0.0;

			if(_state == posPlaying) {
				_state = posIdle;
				flpos = 0.0;
			}
		}
	}

	void streamEnd() {
		arts_debug("streamEnd");
	}
};

REGISTER_IMPLEMENTATION(WavPlayObject_impl);
#endif
