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
#include "flowsystem.h"
#include <stdio.h>
#include <iostream>

using namespace std;

AttachedProducer::AttachedProducer(ByteSoundProducer *sender,
										ByteStreamToAudio *receiver)
{
	this->sender = sender->_copy();
	this->receiver = receiver->_copy();
}

bool AttachedProducer::finished()
{
	// TODO: might cut the last few milliseconds in some cases
	return (sender->finished() || sender->_error());
}

SimpleSoundServer_impl::SimpleSoundServer_impl()
{
	play_obj = ObjectManager::the()->create("Synth_PLAY");
	assert(play_obj);

	add_left = ObjectManager::the()->create("Synth_MULTI_ADD");
	assert(add_left);

	add_right = ObjectManager::the()->create("Synth_MULTI_ADD");
	assert(add_right);

	Object_skel *obj = ObjectManager::the()->create("StereoEffectStack");
	assert(obj);

	_outstack = (StereoEffectStack *)obj->_cast("StereoEffectStack");
	assert(_outstack);

	/*
	play_obj->_node()->connect("invalue_left",add_left->_node(),"outvalue");
	play_obj->_node()->connect("invalue_right",add_right->_node(),"outvalue");
	*/

	_outstack->setInputs(add_left,"outvalue",add_right,"outvalue");
	_outstack->setOutputs(play_obj,"invalue_left",play_obj,"invalue_right");

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

long SimpleSoundServer_impl::attach(ByteSoundProducer *bsp)
{
	printf("Attach ByteSoundProducer!\n");

	Object_skel *convertObj = ObjectManager::the()->create("ByteStreamToAudio");
	assert(convertObj);

	ByteStreamToAudio_var convert =
		(ByteStreamToAudio *)convertObj->_cast("ByteStreamToAudio");
	assert(convert);

//	convert->samplingRate(bsp->samplingRate());
//	convert->channels(bsp->channels());
//	convert->bits(bsp->bits());

	convert->_node()->connect("indata",bsp->_node(),"outdata");
	add_left->_node()->connect("invalue",convert->_node(),"left");
	add_right->_node()->connect("invalue",convert->_node(),"right");

	convert->_node()->start();

	activeProducers.push_back(new AttachedProducer(bsp,convert));
	return 1;
}

StereoEffectStack *SimpleSoundServer_impl::outstack()
{
	return _outstack->_copy();
}

Object *SimpleSoundServer_impl::createObject(const string& name)
{
	return ObjectManager::the()->create(name);
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

	list<AttachedProducer *>::iterator p;

	p = activeProducers.begin();
	while(p != activeProducers.end())
	{
		AttachedProducer *prod = (*p);
		if(prod->finished())
		{
			cout << "stream finished" << endl;
			delete prod;

			activeProducers.erase(p);
			p = activeProducers.begin();
		}
		else p++;
	}
}

PlayObject *SimpleSoundServer_impl::createPlayObject(const string& filename)
{
/*
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
*/
	string extension="";
	if(filename.size()>4)
	{
		extension = filename.substr(filename.size()-4);
		for(int i=1;i<4;i++) extension[i] = toupper(extension[i]);
	}

	ReferenceHelper<Object_skel> obj;
	cout << "extension = " << extension << endl;
	if(extension == ".WAV")
	{
		cout << "Creating WavPlayObject" << endl;
		obj = ObjectManager::the()->create("WavPlayObject");
	}
	else if(extension == ".MP3")
	{
		cout << "Creating MP3PlayObject" << endl;
		obj = ObjectManager::the()->create("MP3PlayObject");
	}
	else if(extension == ".MPG")
	{
		cout << "Creating MP3PlayObject" << endl;
		obj = ObjectManager::the()->create("MP3PlayObject");
	}

	if(obj)
	{
		PlayObject *result = (PlayObject *)obj->_cast("PlayObject");
		if(result)
		{
			if(result->loadMedia(filename))
			{
				// TODO: check for existence of left & right streams
				add_left->_node()->connect("invalue",result->_node(),"left");
				add_right->_node()->connect("invalue",result->_node(),"right");
				result->_node()->start();
				return result->_copy();
			}
			else
			{
				cout << "loadmedia failed." << endl;
			}
		}
		else
		{
			cout << "cast failed." << endl;
		}
	}
	else
	{
		cout << "object not available" << endl;
	}

	return 0;
}
