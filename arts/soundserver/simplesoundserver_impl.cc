    /*

    Copyright (C) 1999-2000 Stefan Westerfeld
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

#include "simplesoundserver_impl.h"
#include "artsflow.h"
#include <stdio.h>
#include <iostream>

using namespace std;

SimpleSoundServer_impl::SimpleSoundServer_impl()
{
	play_obj = ObjectManager::the()->create("Synth_PLAY");
	assert(play_obj);

	add_left = ObjectManager::the()->create("Synth_MULTI_ADD");
	assert(add_left);

	add_right = ObjectManager::the()->create("Synth_MULTI_ADD");
	assert(add_right);

	play_obj->_node()->connect("invalue_left",add_left->_node(),"outvalue");
	play_obj->_node()->connect("invalue_right",add_right->_node(),"outvalue");

	add_left->_node()->start();
	add_right->_node()->start();
	play_obj->_node()->start();

	Dispatcher::the()->ioManager()->addTimer(200,this);
}

SimpleSoundServer_impl::~SimpleSoundServer_impl()
{
	/*
	 * we don't need to care about the flow system nodes we started, since
	 * we have put them into Object_var's, which means that they will be
	 * freed automatically here
	 */
	Dispatcher::the()->ioManager()->removeTimer(this);
}

long SimpleSoundServer_impl::play(const string& filename)
{
	printf("Play '%s'!\n",filename.c_str());

	Object_skel *playwavobj = ObjectManager::the()->create("Synth_PLAY_WAV");
	assert(playwavobj);

	Synth_PLAY_WAV *playwav =
		(Synth_PLAY_WAV *)playwavobj->_cast("Synth_PLAY_WAV");
	assert(playwav);

	playwav->filename(filename);

	add_left->_node()->connect("invalue",playwav->_node(),"left");
	add_right->_node()->connect("invalue",playwav->_node(),"right");

	playwav->_node()->start();

	activeWavs.push_back(playwav);
	return 1;
}

void SimpleSoundServer_impl::notifyTime()
{
	list<Synth_PLAY_WAV *>::iterator i;

	i = activeWavs.begin();
	while(i != activeWavs.end())
	{
		Synth_PLAY_WAV *playwav = (*i);
		if(playwav->finished())
		{
			cout << "finished" << endl;
			playwav->_release();
			activeWavs.erase(i);
			i = activeWavs.begin();
		}
		else i++;
	}
}

PlayObject *SimpleSoundServer_impl::createPlayObject(const string& filename)
{
	ReferenceHelper<Object_skel>
		obj = ObjectManager::the()->create("WavPlayObject");
	if(obj)
	{
		WavPlayObject *result = (WavPlayObject *)obj->_cast("WavPlayObject");
		if(result)
		{
			if(result->loadMedia(filename))
			{
				add_left->_node()->connect("invalue",result->_node(),"left");
				add_right->_node()->connect("invalue",result->_node(),"right");
				result->_node()->start();
				return result->_copy();
			}
		}
	}
	return 0;
}
