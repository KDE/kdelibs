#include "config.h"

#ifdef HAVE_LIBAUDIOFILE

#include "soundserver.h"
#include "cachedwav.h"
#include "stdsynthmodule.h"
#include "convert.h"
#include <iostream>

using namespace Arts;

class WavPlayObject_impl :public WavPlayObject_skel, public StdSynthModule {
protected:
	CachedWav *wav;
	float flpos;
	poState _state;

public:
	/*
	 * construction, destruction
	 */
	WavPlayObject_impl() :wav(0), flpos(0.0), _state(posPaused)
	{
		cout << "WavPlayObject_impl" << endl;
	}

	virtual ~WavPlayObject_impl()
	{
		cout << "~WavPlayObject_impl" << endl;
		if(wav) wav->decRef();
	}

	/*
	 * KMedia2 private interface
	 */
	
	bool loadMedia(const string &filename) {
		cout << "loadMedia" << endl;
		wav = CachedWav::load(Cache::the(), filename);
		return (wav != 0);
	}

	/*
	 * KMedia2 interface
	 */
	string description() {
	}

	void description(const string &) {
	}

	poTime currentTime() {
	}

	void currentTime(const class poTime &) {
	}

    poTime overallTime() {
	}

	poCapabilities capabilities() {
  		return capPause;  /* no seek supported */
	}

	string mediaName() {
	}

	poState state() {
		return _state;
	}

	void play() {
		cout << "play" << endl;
		_state = posPlaying;
	}

	void seek(const class poTime &) {
		cout << "seek" << endl;
	}

	void pause() {
		cout << "pause" << endl;
		_state = posPaused;
	}

	/*
	 * SynthModule interface
	 * - where is stop? initialize?
	 */

	void streamInit() {
		cout << "streamInit" << endl;
	}

	void streamStart() {
		cout << "streamStart" << endl;
	}

	void calculateBlock(unsigned long samples) {
		unsigned long haveSamples = 0;

		if(wav && _state == posPlaying)
		{
			float speed = wav->samplingRate / samplingRateFloat;

			haveSamples = uni_convert_stereo_2float(samples, wav->buffer,
		   		wav->bufferSize,wav->channelCount,wav->sampleWidth,
		   		left,right,speed,flpos);

			flpos += (float)haveSamples * speed;
		}

		if(haveSamples != samples)
		{
			unsigned long i;

			for(i=haveSamples;i<samples;i++)
				left[i] = right[i] = 0.0;

			if(_state == posPlaying) {
				_state = posFinished;
				
				// should we really reset to start here?
				flpos = 0.0;
			}
		}
	}

	void streamEnd() {
		cout << "streamEnd" << endl;
	}
};

REGISTER_IMPLEMENTATION(WavPlayObject_impl);
#endif
