    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "artsflow.h"
#include "debug.h"
#include "convert.h"
#include "objectmanager.h"
#include "audiosubsys.h"
#include "dispatcher.h"
#include "iomanager.h"
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace Arts;

class Synth_PLAY_impl :	virtual public Synth_PLAY_skel,
						virtual public ASProducer,
						virtual public StdSynthModule,
						virtual public IONotify
{
protected:
	AudioSubSystem *as;
	bool haveSubSys;
	/*
	 * these are to prevent the following situation
	 * 1) audio subsystem needs more data
	 * 2) calculation is started
	 * 3) somehow, some module makes a synchronous invocation to the outside
	 *    world and waits for the result
	 * 4) since the audio subsystem still needs data, and since we are in an
	 *    idle state now, another calculation will be started, which will of
	 *    course fail due to reentrancy
	 * 5) repeat 4) until result is there => lots of wasted CPU cycles (when
	 *    running with realtime priority: system freeze)
	 */
	bool inProgress;		// we are just doing some calculations
	bool restartIOHandling;	// I/O handlers removed upon reaching 4: restart

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
	void streamInit() {
		as = AudioSubSystem::the();

		//cout << "Synth_PLAY: streamInit() called." << endl;
		channels = as->channels();
		maxsamples = 0;
		outblock = 0;
		inProgress = false;

		haveSubSys = as->attachProducer(this);
		if(!haveSubSys)
		{
			printf("Synth_PLAY: audio subsystem is already used\n");
			return;
		}

		audiofd = as->open();
		if(audiofd < 0)
		{
			printf("Synth_PLAY: audio subsystem init failed\n");
			printf("ASError = %s\n",as->error());
			return;
		}
	}

	void streamStart() {
		//cout << "Synth_PLAY: streamStart() called." << endl;
		if(audiofd >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			int types = IOType::write|IOType::except;

			if(as->fullDuplex()) types |= IOType::read;
			iom->watchFD(audiofd,types,this);
		}
	}

	void streamEnd() {
		cout << "Synth_PLAY: streamEnd() called." << endl;

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

	AutoSuspendState autoSuspend()
	{
		return asSuspendStop;
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
			printf("Synth_PLAY: got notifyIO while audio subsystem"
				 	"is down\n");
			return;
		}
		assert(fd == audiofd);

		if(inProgress)
		{
			if(!restartIOHandling)
			{
				// prevent lots of retries - we just can't do calculations
				// now, so we need to wait until the situation has resolved
				Dispatcher::the()->ioManager()->remove(this,IOType::all);
				restartIOHandling = true;
			}
			return;
		}

		// convert iomanager notification types to audiosubsys notification
		int asType = 0;

		if(type & IOType::read)		asType |= AudioSubSystem::ioRead;
		if(type & IOType::write)	asType |= AudioSubSystem::ioWrite;
		assert(asType != 0);

		restartIOHandling = false;
		inProgress = true;
		as->handleIO(asType);
		inProgress = false;
		if(restartIOHandling) streamStart();
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
