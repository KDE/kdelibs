    /*

    Copyright (C) 1999-2000 Stefan Westerfeld
                            stefan@space.twc.de
                       2001 Matthias Kretz
                            kretz@kde.org

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

#ifndef SIMPLESOUNDSERVER_IMPL_H
#define SIMPLESOUNDSERVER_IMPL_H

#include "soundserver.h"
#include "artsflow.h"
#include <list>

namespace Arts {

class SoundServerJob
{
public:
	long ID;
	virtual ~SoundServerJob();

	virtual void detach(const Object& object);
	virtual void terminate() = 0;
	virtual bool done() = 0;
};

class PlayWavJob : public SoundServerJob
{
protected:
	Synth_PLAY_WAV wav;
	Synth_AMAN_PLAY out;
	bool terminated;

public:
	PlayWavJob(const std::string& filename);

	void terminate();
	bool done();
};

class PlayStreamJob : public SoundServerJob
{
protected:
	ByteSoundProducer sender;
	ByteStreamToAudio convert;
	Synth_AMAN_PLAY out;

public:
	PlayStreamJob(ByteSoundProducer bsp);

	void detach(const Object& object);
	void terminate();
	bool done();
};

class RecordStreamJob : public SoundServerJob
{
protected:
	ByteSoundReceiver receiver;
	AudioToByteStream convert;
	Synth_AMAN_RECORD in;

public:
	RecordStreamJob(ByteSoundReceiver bsr);

	void detach(const Object& object);
	void terminate();
	bool done();
};

class SimpleSoundServer_impl : virtual public SimpleSoundServer_skel,
										public TimeNotify
{
protected:
	Synth_PLAY playSound;
	Synth_RECORD recordSound;
	Synth_BUS_DOWNLINK soundcardBus;
	Synth_BUS_UPLINK recordBus;
	std::list<SoundServerJob *> jobs;
	StereoEffectStack _outstack;
	StereoVolumeControl _outVolume;
	long asCount;
	long autoSuspendTime;
	long bufferMultiplier;

public:
	SimpleSoundServer_impl();
	~SimpleSoundServer_impl();

	void notifyTime();

	// streaming audio
	float minStreamBufferTime();
	float serverBufferTime();
	void attach(ByteSoundProducer bsp);
	void detach(ByteSoundProducer bsp);
	void attachRecorder(ByteSoundReceiver bsr);
	void detachRecorder(ByteSoundReceiver bsr);

	// simple soundserver interface
	long play(const std::string& s);

	// kmedia2
	PlayObject createPlayObject(const std::string& filename);
	StereoEffectStack outstack();
	Object createObject(const std::string& name);
};

};

#endif /* SIMPLESOUNDSERVER_IMPL_H */
