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
#include "audiosubsys.h"
#include "connect.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace Arts;

void SoundServerJob::detach(const Object&)
{
	// nothing by default
}

SoundServerJob::~SoundServerJob()
{
	// virtual destructor, since we've got virtual functions
}

PlayWavJob::PlayWavJob(const string& filename) :terminated(false)
{
	printf("Play '%s'!\n",filename.c_str());

	connect(wav,out);
	wav.filename(filename);
	wav.start();
	out.start();
}

void PlayWavJob::terminate()
{
	terminated = true;
}

bool PlayWavJob::done()
{
	return terminated || wav.finished();
}

PlayStreamJob::PlayStreamJob(ByteSoundProducer bsp) : sender(bsp)
{
	printf("Attach ByteSoundProducer!\n");

	convert.samplingRate(bsp.samplingRate());
	convert.channels(bsp.channels());
	convert.bits(bsp.bits());

	connect(sender,"outdata",convert,"indata");
	connect(convert,out);

	convert.start();
	out.start();
}

void PlayStreamJob::detach(const Object& object)
{
	if(object._isEqual(sender))
		terminate();
}

void PlayStreamJob::terminate()
{
	sender = ByteSoundProducer::null();
}

bool PlayStreamJob::done()
{
	// when the sender is not alive any longer, assign a null object
	if(!sender.isNull() && sender.error())
		sender = ByteSoundProducer::null();

	return sender.isNull() && !convert.running();
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
 *    |      outstack       |      (here the user can plugin various effects)
 *     ~~~~~~~~~~~~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |      playSound      |      (output to soundcard)
 *     ~~~~~~~~~~~~~~~~~~~~~
 */
SimpleSoundServer_impl::SimpleSoundServer_impl()
{
	soundcardBus.busname("out_soundcard");
	connect(soundcardBus,"left",addLeft);
	connect(soundcardBus,"right",addRight);
	connect(addLeft,_outstack,"inleft");
	connect(addRight,_outstack,"inright");
	connect(_outstack,playSound);

	if(AudioSubSystem::the()->fullDuplex())
	{
		recordBus.busname("in_soundcard");
		connect(recordSound,recordBus);

		recordBus.start();
		recordSound.start();
	}

	soundcardBus.start();
	addLeft.start();
	addRight.start();
	playSound.start();

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
	jobs.push_back(new PlayWavJob(filename));
	return 1;
}

float SimpleSoundServer_impl::serverBufferTime()
{
	float hardwareBuffer = AudioSubSystem::the()->fragmentSize()
						 * AudioSubSystem::the()->fragmentCount();

	float playSpeed = AudioSubSystem::the()->channels()
	                * AudioSubSystem::the()->samplingRate()
					* 2; /* TODO: check bits */
	
	return 1000.0 * hardwareBuffer / playSpeed;
}

float SimpleSoundServer_impl::minStreamBufferTime()
{
	/*
	 * it is sane to assume that client side stream buffers must be >= server
	 * side hardware buffers (or it can come to dropouts during hardware
	 * buffer refill)
	 */
	return serverBufferTime();
}

void SimpleSoundServer_impl::attach(ByteSoundProducer bsp)
{
	jobs.push_back(new PlayStreamJob(bsp));
}

void SimpleSoundServer_impl::detach(ByteSoundProducer bsp)
{
	printf("Detach ByteSoundProducer!\n");
	list<SoundServerJob *>::iterator j;

	for(j = jobs.begin();j != jobs.end();j++)
		(*j)->detach(bsp);
}

StereoEffectStack SimpleSoundServer_impl::outstack()
{
	return _outstack;
}

Object SimpleSoundServer_impl::createObject(const string& name)
{
	return Object(SubClass(name));
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

	/* look for jobs which may have terminated by now */
	list<SoundServerJob *>::iterator i;

	i = jobs.begin();
	while(i != jobs.end())
	{
		SoundServerJob *job = *i;

		if(job->done())
		{
			delete job;
			jobs.erase(i);
			cout << "job finished" << endl;
			i = jobs.begin();
		}
		else i++;
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

PlayObject SimpleSoundServer_impl::createPlayObject(const string& filename)
{
	string objectType = "";

	/*
	 * figure out extension (as lowercased letters)
	 */
	string extension = "";
	bool extensionok = false;
	string::const_reverse_iterator i;
	for(i = filename.rbegin(); i != filename.rend() && !extensionok; i++)
	{
		if(*i == '.')
			extensionok = true;
		else
			extension = (char)tolower(*i) + extension;
	}

	/*
	 * query trader for PlayObjects which support this
	 */
	if(extensionok)
	{
		cout << "extension = " << extension << endl;

		TraderQuery query;
		query.supports("Interface","Arts::PlayObject");
		query.supports("Extension",extension);

		vector<TraderOffer> *offers = query.query();
		if(!offers->empty())
			objectType = offers->front().interfaceName();	// first offer

		delete offers;
	}

	/*
	 * create a PlayObject and connect it
	 */
	if(objectType != "")
	{
		cout << "Creating " << objectType << " to play file." << endl;
		PlayObject result = SubClass(objectType);
		if(result.loadMedia(filename))
		{
			// TODO: check for existence of left & right streams
			connect(result,"left",addLeft);
			connect(result,"right",addRight);
			result._node()->start();
			return result;
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

	return PlayObject::null();
}

REGISTER_IMPLEMENTATION(SimpleSoundServer_impl);
