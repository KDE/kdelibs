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

#include "artsflow.h"
#include "fft.h"
#include "stdsynthmodule.h"
#include <math.h>
#include <iostream>

using namespace std;
using namespace Arts;

namespace Arts {

class StereoFFTScope_impl : public StereoFFTScope_skel, public StdSynthModule {
protected:
	static const unsigned long SAMPLES = 4096;
	vector<float> _scope;
	/*
	 * some gcc versions expose ugly behaviour with virtual inheritance:
	 * putting window[4096] & inbuffer[4096] here bloats the vtable then,
	 * and tells you to recompile with -fhuge-objects ... so allocate them
	 * dynamically
	 */
	float *window;
	float *inbuffer;
	unsigned long inbufferpos;
public:
	void do_fft()
	{
		float out_real[SAMPLES],out_img[SAMPLES];
		fft_float(SAMPLES,0,inbuffer,0,out_real,out_img);

		_scope.clear();
    	unsigned int i = 3;
    	unsigned int j = 0;
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
	void streamInit()
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
	void streamStart()
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

	/* prevent vtable overflows (see above) */
	StereoFFTScope_impl() {
		window = new float[SAMPLES];
		inbuffer = new float[SAMPLES];
	}
	~StereoFFTScope_impl() {
		delete [] window;
		delete [] inbuffer;
	}
};

const unsigned long StereoFFTScope_impl::SAMPLES;

REGISTER_IMPLEMENTATION(StereoFFTScope_impl);

};
