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
#include <math.h>
#include <stdio.h>

using namespace Arts;

namespace Arts {

class Synth_FREQUENCY_impl :public Synth_FREQUENCY_skel, public StdSynthModule
{
	float fpos;
public:
	void streamInit() {
		fpos = 0;
	}
	void calculateBlock(unsigned long cycles) {
		float fsample = samplingRateFloat;
		float finc = *frequency/fsample;

		while(cycles)
		{
			if(cycles >= 8 && ((finc*8+fpos) < 0.9))
			{
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				fpos += finc; *pos++ = fpos;
				cycles -= 8;
			}
			else
			{
				fpos += finc;
				fpos -= floor(fpos);
				*pos++ = fpos;
				cycles--;
			}
		}
	}
};

REGISTER_IMPLEMENTATION(Synth_FREQUENCY_impl);

};
