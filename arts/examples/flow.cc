    /*

    Copyright (C) 1999 Stefan Westerfeld, stefan@space.twc.de
                       Nicolas Brodu, nicolas.brodu@free.fr

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
#include "connect.h"

using namespace Arts;

int main()
{
	Dispatcher dispatcher;

	// object creation
	Synth_FREQUENCY freq;
	Synth_WAVE_SIN  sin;
	Synth_PLAY      play;

	// object initialization
	setValue(freq, 440.0);

	// object connection
	connect(freq, sin);
	connect(sin, play, "invalue_left");
	connect(sin, play, "invalue_right");

	// start all objects (maybe we should group objects like with QWidget
	// parents and such?)
	freq.start();
	sin.start();
	play.start();

	// go
	dispatcher.run();
}
