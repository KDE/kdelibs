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
#include "debug.h"
#include "flowsystem.h"
#include <stdio.h>

/*
 * After getting a comment back why that API looks so broken...: the API
 * used in testflow and testdynflow is not the API you'll use when programming
 * C++ with MCOP, the C++ API looks like that:
 */

/* possible enhancements after Nicolas Brodu's mail */

/* that code should go into MCOP or something similar */

#define creator(class)                                                  \
class * class ## _create(const string& subclass = __STRING(class))      \
{                                                                       \
	Object_skel *skel = ObjectManager::the()->create(subclass);         \
	assert(skel);                                                       \
	class *result = (class *)skel->_cast(__STRING(class));              \
	assert(result);                                                     \
	return result;                                                      \
}

creator(Synth_FREQUENCY);
creator(Synth_WAVE_SIN);
creator(Synth_PLAY);
creator(ExecutionManager);

void connect(Object *from, string fromPort, Object *to, string toPort)
{
	from->_node()->connect(fromPort, to->_node(), toPort);
}

void start(Object *tostart)
{
	tostart->_node()->start();
}

/* enhancements end */
int main()
{
	Dispatcher dispatcher;

	// object creation
	Synth_FREQUENCY_var freq = Synth_FREQUENCY_create();
	Synth_WAVE_SIN_var sin = Synth_WAVE_SIN_create();
	Synth_PLAY_var play = Synth_PLAY_create();

	// object initialization
	freq->_node()->setFloatValue("frequency",440.0);

	// object connection
	connect(freq,"pos",sin,"pos");
	connect(sin,"outvalue",play,"invalue_left");
	connect(sin,"outvalue",play,"invalue_right");

	// start all objects (maybe we should group objects like with QWidget
	// parents and such?)
	start(freq);
	start(sin);
	start(play);

	// go
	dispatcher.run();
}
