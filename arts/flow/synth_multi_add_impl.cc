#include "artsflow.h"
#include "stdsynthmodule.h"

class Synth_MULTI_ADD_impl :public Synth_MULTI_ADD_skel, StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles);
};

REGISTER_IMPLEMENTATION(Synth_MULTI_ADD_impl);

void Synth_MULTI_ADD_impl::calculateBlock(unsigned long cycles)
{
	float *outend = outvalue + cycles;
	float *outp, *inp;

	if(!invalue[0])
	{
		// no invalue? zero out the outvalue
		for(outp = outvalue; outp != outend; outp++) *outp = 0.0;
	}
	else
	{
		// copy first signal
		for(outp = outvalue, inp = invalue[0]; outp != outend; outp++, inp++)
			*outp = *inp;

		// add the others
		for(int sig=1;(inp = invalue[sig]) != 0;sig++)
		{
			for(outp = outvalue; outp != outend;outp++,inp++)
				*outp += *inp;
		}
	}
}
