#include "artsflow.h"
#include "stdsynthmodule.h"

class Synth_MUL_impl :public Synth_MUL_skel, StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles)
	{
		float *end = outvalue + cycles;

		while(outvalue != end) *outvalue++ = *invalue1++ * *invalue2++;
	}
};

REGISTER_IMPLEMENTATION(Synth_MUL_impl);
