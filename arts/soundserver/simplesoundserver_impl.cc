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

AttachedProducer::AttachedProducer(ByteSoundProducer_base *sender,
										ByteStreamToAudio_base *receiver)
{
	_sender = sender->_copy();
	_receiver = receiver->_copy();
}

ByteSoundProducer_base *AttachedProducer::sender()
{
	return _sender;
}

ByteStreamToAudio_base *AttachedProducer::receiver()
{
	return _receiver;
}

/*
 *    ( This place where we will put the objects for playing wave files and
 *      such, they are then connected to addLeft and addRight, to get their
 *      output mixed with the other clients ).
 *     _________   __________
 *    | addLeft | | addRight |
 *     ~~~~~~~~~   ~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |      outStack       |      (here the user can plugin various effects)
 *     ~~~~~~~~~~~~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |      playSound      |      (output to soundcard)
 *     ~~~~~~~~~~~~~~~~~~~~~
 */
SimpleSoundServer_impl::SimpleSoundServer_impl()
{
	playSound = Synth_PLAY_base::_create();
	addLeft = Synth_MULTI_ADD_base::_create();
	addRight = Synth_MULTI_ADD_base::_create();

	_outstack = StereoEffectStack_base::_create();
	_outstack->setInputs(addLeft,"outvalue",addRight,"outvalue");
	_outstack->setOutputs(playSound,"invalue_left",playSound,"invalue_right");

	addLeft->_node()->start();
	addRight->_node()->start();
	playSound->_node()->start();

	asCount = 0; // AutoSuspend
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

	Synth_PLAY_WAV_base *playwav = Synth_PLAY_WAV_base::_create();
	playwav->filename(filename);

	addLeft->_node()->connect("invalue",playwav->_node(),"left");
	addRight->_node()->connect("invalue",playwav->_node(),"right");

	playwav->_node()->start();

	activeWavs.push_back(playwav);
	return 1;
}

void SimpleSoundServer_impl::attach(ByteSoundProducer_base *bsp)
{
	printf("Attach ByteSoundProducer!\n");

	ByteStreamToAudio_var convert = ByteStreamToAudio_base::_create();

//	convert->samplingRate(bsp->samplingRate());
//	convert->channels(bsp->channels());
//	convert->bits(bsp->bits());

	convert->_node()->connect("indata",bsp->_node(),"outdata");
	addLeft->_node()->connect("invalue",convert->_node(),"left");
	addRight->_node()->connect("invalue",convert->_node(),"right");

	convert->_node()->start();

	activeProducers.push_back(new AttachedProducer(bsp,convert));
}

void SimpleSoundServer_impl::detach(ByteSoundProducer_base *bsp)
{
	printf("Detach ByteSoundProducer!\n");
	list<AttachedProducer *>::iterator p;

	for(p = activeProducers.begin();p != activeProducers.end();p++)
	{
		AttachedProducer *prod = (*p);
		if(bsp->_isEqual(prod->sender()))
		{
			/* 
			 * Hint: the order of the next lines is not unimportant:
			 *
			 * delete prod involves _release()ing remote objects,
			 * and while this is happening, other producers could
			 * attach/detach and we could end up doing something wrong
             */
			activeProducers.erase(p);

			activeConverters.push_back(prod->receiver()->_copy());
			delete prod;

			return;
		}
	}
	assert(false);		// you shouldn't detach things you never attached!
}

StereoEffectStack_base *SimpleSoundServer_impl::outstack()
{
	return _outstack->_copy();
}

Object *SimpleSoundServer_impl::createObject(const string& name)
{
	return Object::_create(name);
}

void SimpleSoundServer_impl::notifyTime()
{
	static long lock = 0;
	assert(!lock);		// paranoid reentrancy check (remove me later)
	lock++;
	/*
	 * Three times the same game: look if a certain object is still
	 * active - if yes, keep, if no, remove
	 */

	/* look for WAVs which may have terminated by now */
	list<Synth_PLAY_WAV_base *>::iterator i;

	i = activeWavs.begin();
	while(i != activeWavs.end())
	{
		Synth_PLAY_WAV_base *playwav = (*i);
		if(playwav->finished())
		{
			activeWavs.erase(i);

			cout << "finished" << endl;
			playwav->_release();

			i = activeWavs.begin();
		}
		else i++;
	}

	/* look for producers which servers have died */
	list<AttachedProducer *>::iterator p;

	p = activeProducers.begin();
	while(p != activeProducers.end())
	{
		AttachedProducer *prod = (*p);
		if(prod->sender()->_error())
		{
			activeProducers.erase(p);

			cout << "stream closed (client died)" << endl;
			activeConverters.push_back(prod->receiver()->_copy());
			delete prod;

			p = activeProducers.begin();
		}
		else p++;
	}

	/* look for converters which are no longer running */
	list<ByteStreamToAudio_base *>::iterator ci;

	ci = activeConverters.begin();
	while(ci != activeConverters.end())
	{
		ByteStreamToAudio_base *conv = (*ci);
		if(!conv->running())
		{
			activeConverters.erase(ci);

			cout << "converter (for stream) finished" << endl;
			conv->_release();

			ci = activeConverters.begin();
		}
		else ci++;
	}
/*
 * AutoSuspend
 */
	if(Dispatcher::the()->flowSystem()->suspendable() &&
	  !Dispatcher::the()->flowSystem()->suspended())
	{
		asCount++;
		if(asCount > 300)
		{
			Dispatcher::the()->flowSystem()->suspend();
			cout << "[artsd] suspend" << endl;
		}
	}
	else
		asCount = 0;
	lock--;
}

PlayObject_base *SimpleSoundServer_impl::createPlayObject(const string& filename)
{
	string extension="", objectType = "";
	if(filename.size()>4)
	{
		extension = filename.substr(filename.size()-4);
		for(int i=1;i<4;i++) extension[i] = toupper(extension[i]);

		cout << "extension = " << extension << endl;
	}

	if(extension == ".WAV")			objectType = "WavPlayObject";
	/* TODO: write a service which can find out which object decodes what
	else if(extension == ".MP3") 	objectType = "MP3PlayObject";
	else if(extension == ".MPG")	objectType = "MP3PlayObject";
	*/

	if(objectType != "")
	{
		cout << "Creating " << objectType << " to play file." << endl;
		PlayObject_var result = PlayObject_base::_create(objectType);
		if(result->loadMedia(filename))
		{
			// TODO: check for existence of left & right streams
			addLeft->_node()->connect("invalue",result->_node(),"left");
			addRight->_node()->connect("invalue",result->_node(),"right");
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
		cout << "can't play this" << endl;
	}

	return 0;
}

REGISTER_IMPLEMENTATION(SimpleSoundServer_impl);
