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
	playSound = Synth_PLAY::_create();
	addLeft = Synth_MULTI_ADD::_create();
	addRight = Synth_MULTI_ADD::_create();

	_outstack = StereoEffectStack::_create();
	_outstack->setInputs(addLeft,"outvalue",addRight,"outvalue");
	_outstack->setOutputs(playSound,"invalue_left",playSound,"invalue_right");

	addLeft->_node()->start();
	addRight->_node()->start();
	playSound->_node()->start();

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

	Synth_PLAY_WAV *playwav = Synth_PLAY_WAV::_create();
	playwav->filename(filename);

	addLeft->_node()->connect("invalue",playwav->_node(),"left");
	addRight->_node()->connect("invalue",playwav->_node(),"right");

	playwav->_node()->start();

	activeWavs.push_back(playwav);
	return 1;
}

long SimpleSoundServer_impl::attach(ByteSoundProducer *bsp)
{
	printf("Attach ByteSoundProducer!\n");

	ByteStreamToAudio_var convert = ByteStreamToAudio::_create();

//	convert->samplingRate(bsp->samplingRate());
//	convert->channels(bsp->channels());
//	convert->bits(bsp->bits());

	convert->_node()->connect("indata",bsp->_node(),"outdata");
	addLeft->_node()->connect("invalue",convert->_node(),"left");
	addRight->_node()->connect("invalue",convert->_node(),"right");

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
	return Object::_create(name);
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
		PlayObject_var result = PlayObject::_create(objectType);
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
