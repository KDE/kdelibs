#include "artsflow.h"
#include "fft.h"
#include "stdsynthmodule.h"
#include <math.h>
#include <iostream>

using namespace std;

class StereoFFTScope_impl : public StereoFFTScope_skel, public StdSynthModule {
protected:
	enum { SAMPLES = 4096 };
	vector<float> _scope;
	float window[SAMPLES];
	float inbuffer[SAMPLES];
	long inbufferpos;
public:
	void do_fft()
	{
		float out_real[SAMPLES],out_img[SAMPLES];
		fft_float(SAMPLES,0,inbuffer,0,out_real,out_img);

		_scope.clear();
    	int i = 3;
    	int j = 0;
    	for(;;) {
        	float xrange = 0.0;
        	while(j != i)
        	{
            	xrange += (fabs(out_img[j]) + fabs(out_real[j]))/(float)SAMPLES;
            	j++;
        	}
			_scope.push_back(xrange);

        	if(i == SAMPLES/2) return;

            i += i/2;
            if(i > SAMPLES/2) i = SAMPLES/2;
    	}
	}
	void initialize()
	{
		unsigned long i;
		for(i=0;i<SAMPLES;i++)
		{
			float x = (float)i/(float)SAMPLES; 
			window[i] = sin(x*DDC_PI)*sin(x*DDC_PI);
			inbuffer[i] = 0;
		}
		do_fft();	// initialize so that we never return an empty scope
	}
	void start()
	{
		inbufferpos = 0;
	}
	vector<float> *scope()
	{
		return new vector<float>(_scope);
	}
	/*
	  in audio stream inleft, inright;
	  out audio stream outleft, outright;
	*/
	void calculateBlock(unsigned long samples)
	{
		unsigned long i;
		for(i=0;i<samples;i++)
		{
			inbuffer[inbufferpos] =
				(inleft[i] + inright[i])*window[inbufferpos];
			if(++inbufferpos == SAMPLES)
			{
				do_fft();
				inbufferpos = 0;
			}
			/*
			   monitoring only tasks can't be done with that StereoEffect
			   interface nicely - copy input to output until there is
			   something better
			 */
			outleft[i] = inleft[i];
			outright[i] = inright[i];
		}
	}
};

REGISTER_IMPLEMENTATION(StereoFFTScope_impl);
