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

using namespace Arts;

namespace Arts {

class Synth_WAVE_SIN_impl : virtual public Synth_WAVE_SIN_skel,
							virtual public StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles)
	{
		for(unsigned long i=0; i<cycles; i++)
			outvalue[i] = sin(pos[i]*2*M_PI);
	}
};

REGISTER_IMPLEMENTATION(Synth_WAVE_SIN_impl);

};
