    /*

    Copyright (C) 2000 Stefan Westerfeld
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

class Synth_ADD_impl :public Synth_ADD_skel, StdSynthModule
{
public:
	void calculateBlock(unsigned long cycles)
	{
		float *end = outvalue + cycles;

		while(outvalue != end) *outvalue++ = *invalue1++ + *invalue2++;
	}
};

REGISTER_IMPLEMENTATION(Synth_ADD_impl);
