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

namespace Arts {

class Synth_PLAY_impl :	virtual public Synth_PLAY_skel,
						virtual public ASProducer,
						virtual public StdSynthModule,
						virtual public IONotify,
						virtual public TimeNotify
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

	int audioReadFD;
	int audioWriteFD;
	bool audioOpen;

	typedef unsigned char uchar;

	unsigned char *outblock;
	unsigned long maxsamples;
	unsigned long channels;
	int format;
	int bits;

	bool retryOpen;
public:
	/*
	 * functions from the SynthModule interface (which is inherited by
	 * SynthPlay)
	 */
	void streamInit() {
		as = AudioSubSystem::the();

		maxsamples = 0;
		outblock = 0;
		retryOpen = false;
		audioOpen = false;
		inProgress = false;

		haveSubSys = as->attachProducer(this);
		if(!haveSubSys)
		{
			arts_info("Synth_PLAY: audio subsystem is already used");
			return;
		}

		audioOpen = as->open();
		if(!audioOpen)
		{
			if(Dispatcher::the()->flowSystem()->suspended())
			{
				arts_info("/dev/dsp currently unavailable (retrying)");
				Dispatcher::the()->ioManager()->addTimer(1000, this);
				retryOpen = true;
			}
			else
			{
				arts_info("Synth_PLAY: audio subsystem init failed");
				arts_info("ASError = %s",as->error());
			}
			audioReadFD = audioWriteFD = -1;
		}
		else
		{
			audioReadFD = as->selectReadFD();
			audioWriteFD = as->selectWriteFD();
		}

		channels = as->channels();
		format = as->format();
		bits = as->bits();
		arts_debug("audio format is %d Hz, %d bits, %d channels",
					as->samplingRate(), bits, channels);
	}

	void notifyTime() {
		assert(retryOpen);

		audioOpen = as->open();

		if(audioOpen)
		{
			audioReadFD = as->selectReadFD();
			audioWriteFD = as->selectWriteFD();

			streamStart();
			arts_info("/dev/dsp ok");
			Dispatcher::the()->ioManager()->removeTimer(this);
			retryOpen = false;
		}
	}

	void streamStart() {
		IOManager *iom = Dispatcher::the()->ioManager();

		if(audioReadFD >= 0)
			iom->watchFD(audioReadFD, IOType::read|IOType::except, this);

		if(audioWriteFD >= 0)
			iom->watchFD(audioWriteFD, IOType::write|IOType::except, this);
	}

	void streamEnd() {
		if(retryOpen)
			Dispatcher::the()->ioManager()->removeTimer(this);

		arts_debug("Synth_PLAY: closing audio fd");
		if(audioReadFD >= 0 || audioWriteFD >= 0)
		{
			IOManager *iom = Dispatcher::the()->ioManager();
			iom->remove(this,IOType::all);
			audioReadFD = audioWriteFD = -1;
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

		arts_assert(format == 8 || format == 16 || format == 17);
		if(channels == 1)
		{
			if(format == 8)
				convert_mono_float_8(samples,invalue_left,outblock);
			else if(format == 16)
				convert_mono_float_16le(samples,invalue_left,outblock);
			else if(format == 17)
				convert_mono_float_16be(samples,invalue_left,outblock);
		}
		else if(channels == 2)
		{
			if(format == 8)
				convert_stereo_2float_i8(samples,invalue_left,invalue_right,
													outblock);
			else if(format == 16)
				convert_stereo_2float_i16le(samples,invalue_left,invalue_right,
													outblock);
			else if(format == 17)
				convert_stereo_2float_i16be(samples,invalue_left,invalue_right,
													outblock);
		}
		else arts_warning("channels != 1 && channels != 2?");

		as->write(outblock,channels * (bits / 8) * samples);
	}

	/**
	 * notifyIO from the IONotify interface (IOManager)
	 */
	void notifyIO(int fd, int type)
	{
		arts_return_if_fail(as->running());
		assert(fd == audioReadFD || fd == audioWriteFD);

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

};
