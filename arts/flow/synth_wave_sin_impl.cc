    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "artsflow.h"
#include "stdsynthmodule.h"
#include <math.h>

class Synth_WAVE_SIN_impl : virtual public Synth_WAVE_SIN_skel,
							virtual public StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles)
	{
		float *end = cycles + pos;
		while(pos != end) *outvalue++ = sin((*pos++)*2*M_PI);
	}
};

REGISTER_IMPLEMENTATION(Synth_WAVE_SIN_impl);
