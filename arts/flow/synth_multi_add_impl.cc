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
#include "stdsynthmodule.h"

using namespace Arts;

namespace Arts {

class Synth_MULTI_ADD_impl :public Synth_MULTI_ADD_skel, public StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles);
	AutoSuspendState autoSuspend() { return asSuspend; }
};

REGISTER_IMPLEMENTATION(Synth_MULTI_ADD_impl);

};

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
