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
#include "artsserver_impl.h"
#include "convert.h"
#include "objectmanager.h"
#include "audiosubsys.h"
#include "dispatcher.h"
#include "iomanager.h"
#include <stdio.h>

class Synth_PLAY_impl :	virtual public Synth_PLAY_skel,
						virtual public ASProducer,
						virtual public IONotify
{
protected:
	AudioSubSystem *as;
	bool haveSubSys;
	int audiofd;

	typedef unsigned char uchar;

	unsigned char *outblock;
	unsigned long maxsamples;
	unsigned long channels;

public:
	/*
	 * functions from the SynthModule interface (which is inherited by
	 * SynthPlay)
	 */
	void firstInitialize() {
		//cout << "Synth_PLAY: firstInitialize() called." << endl;
	}

	void initialize() {
		//cout << "Synth_PLAY: initialize() called." << endl;
		channels = 2;
		maxsamples = 0;
		outblock = 0;

		as = AudioSubSystem::the();

		haveSubSys = as->attachProducer(this);
		if(!haveSubSys)
		{
			printf("SynthGenericPlay: audio subsystem is already used\n");
			return;
		}

					/* fragments, fsize, samplingrate, channels, fullduplex */
		audiofd = as->open(7,1024, 44100,channels, false);
		if(audiofd < 0)
		{
			printf("SynthGenericPlay: audio subsystem init failed\n");
			printf("ASError = %s\n",as->error());
			return;
		}
	}

	void start() {
		//cout << "Synth_PLAY: start() called." << endl;
		if(audiofd >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			iom->watchFD(audiofd,IOType::write|IOType::except,this);
		}
	}

	void deInitialize() {
		cout << "Synth_PLAY: deInitialize() called." << endl;

		artsdebug("SynthGenericPlay: closing audio fd\n");
		if(audiofd >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			iom->remove(this,IOType::all);
			audiofd = 0;
		}
		AudioSubSystem::the()->detachProducer();

		if(outblock)
		{
			delete[] outblock;
			outblock = 0;
		}
	}

	void calculateBlock(unsigned long samples)
	{
		// no audio subsystem, no play
		if(!as->running() || !haveSubSys) return;

		if(samples > maxsamples)
		{
			maxsamples = samples;

			if(outblock) delete[] outblock;
			outblock = new uchar[maxsamples * 4]; // 2 channels, 16 bit
		}

		assert(channels);

		if(channels == 1)
			convert_mono_float_16le(samples,invalue_left,outblock);

		if(channels == 2)
			convert_stereo_2float_i16le(samples,invalue_left,invalue_right,
													outblock);

		as->write(outblock,channels * 2 * samples);
	}

	/**
	 * notifyIO from the IONotify interface (IOManager)
	 */
	void notifyIO(int fd, int type)
	{
		if(!as->running())
		{
			printf("SynthGenericPlay: got notifyIO while audio subsystem"
				 	"is down\n");
			return;
		}
		assert(fd == audiofd);

		switch(type)
		{
			case IOType::read: type = AudioSubSystem::ioRead;
					break;
			case IOType::write: type = AudioSubSystem::ioWrite;
					break;
			default: assert(false);
		}
		as->handleIO(type);
	}

	/**
	 * needmore from the ASProducer interface (AudioSubSystem)
	 */
	void needMore()
	{
		_node()->requireFlow();
	}
	
};

REGISTER_IMPLEMENTATION(Synth_PLAY_impl);
