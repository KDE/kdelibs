    /*

    Copyright (C) 1999-2001 Stefan Westerfeld
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
#include "debug.h"
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
	arts_debug("play '%s'!",filename.c_str());

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
	int samplingRate = bsp.samplingRate();
	int channels = bsp.channels();
	int bits = bsp.bits();

	arts_debug("incoming stream, parameters: rate=%d, %d bit, %d channels",
		samplingRate, bits, channels);

	if((samplingRate < 500 || samplingRate > 2000000)
	|| (channels != 1 && channels != 2) || (bits != 8 && bits != 16))
	{
		arts_warning("invalid stream parameters: rate=%d, %d bit, %d channels",
						samplingRate, bits, channels);
		terminate();
		return;
	}

	convert.samplingRate(samplingRate);
	convert.channels(channels);
	convert.bits(bits);

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
 *    ( This place where other objects for playing wave files and such will
 *      be connected, to get their output mixed with the other clients ).
 *     _____________________
 *    |     soundcardBus    |     (a Synth_BUS_DOWNLINK for "out_soundcard")
 *     ~~~~~~~~~~~~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |      outstack       |      (here the user can plugin various effects)
 *     ~~~~~~~~~~~~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |     outVolume       |      (global output volume for the soundserver)
 *     ~~~~~~~~~~~~~~~~~~~~~
 *        |            |
 *     ___V____________V____
 *    |      playSound      |      (output to soundcard)
 *     ~~~~~~~~~~~~~~~~~~~~~
 */
SimpleSoundServer_impl::SimpleSoundServer_impl() : autoSuspendTime(60), bufferMultiplier(1)
{
	soundcardBus.busname("out_soundcard");
	connect(soundcardBus,_outstack);
	connect(_outstack,_outVolume);
	connect(_outVolume,playSound);

	if(AudioSubSystem::the()->fullDuplex())
	{
		recordBus.busname("in_soundcard");
		connect(recordSound,recordBus);

		recordBus.start();
		recordSound.start();
	}

	soundcardBus.start();
	_outVolume.start();
	playSound.start();

	asCount = 0; // AutoSuspend
	Dispatcher::the()->ioManager()->addTimer(200,this);

	// load Arts::MidiManager when installed
	TraderQuery query;
	query.supports("InterfaceName","Arts::MidiManager");

	vector<TraderOffer> *offers = query.query();
	if(offers->size())
		_addChild(Arts::SubClass("Arts::MidiManager"),
					"Extension_Arts::MidiManager");
	delete offers;
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
					* (AudioSubSystem::the()->bits() / 8);

	return 1000.0 * hardwareBuffer / playSpeed;
}

float SimpleSoundServer_impl::minStreamBufferTime()
{
	/*
	 * It is sane to assume that client side stream buffers must be >= server
	 * side hardware buffers (or it can come to dropouts during hardware
	 * buffer refill). The buffer size can be increased using the multiplier.
	 */
	return bufferMultiplier * serverBufferTime();
}

void SimpleSoundServer_impl::attach(ByteSoundProducer bsp)
{
	arts_return_if_fail(!bsp.isNull());

	jobs.push_back(new PlayStreamJob(bsp));
}

void SimpleSoundServer_impl::detach(ByteSoundProducer bsp)
{
	arts_return_if_fail(!bsp.isNull());
	arts_debug("detach incoming stream");

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
	// don't use SubClass(name) as this will abort if the object is not found
	return Object::_from_base(ObjectManager::the()->create(name));
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
			arts_debug("job finished");
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
		if(asCount > autoSuspendTime*5)
		{
			Dispatcher::the()->flowSystem()->suspend();
			arts_info("sound server suspended");
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
			extension.insert(extension.begin(), (char)tolower(*i));
	}

	/*
	 * query trader for PlayObjects which support this
	 */
	if(extensionok)
	{
		arts_debug("search playobject, extension = %s",extension.c_str());

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
		arts_debug("creating %s to play file", objectType.c_str());

		PlayObject result = SubClass(objectType);
		if(result.loadMedia(filename))
		{
			// TODO: check for existence of left & right streams
			Synth_BUS_UPLINK uplink;
			uplink.busname("out_soundcard");
			connect(result,"left",uplink,"left");
			connect(result,"right",uplink,"right");
			uplink.start();
			result._node()->start();
			result._addChild(uplink,"uplink");
			return result;
		}
		else arts_warning("couldn't load file %s", filename.c_str());
	}
	else arts_warning("file format extension %s unsupported",extension.c_str());

	return PlayObject::null();
}

#ifndef __SUNPRO_CC
/* SunPRO CC has problems finding the SimpleSoundServer_impl constructor
   implementation from a template instantiation file, if this is here,
   although I verified that the requested and provided symbols do indeed match,
   and the latter is global.  Wonderfully this problem goes away, if we don't
   register the implementation here, but in another file.  I bet this is
   because of the static var that is created by the macro.  */
REGISTER_IMPLEMENTATION(SimpleSoundServer_impl);
#endif
