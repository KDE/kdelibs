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

#include "audioio.h"
#include <map>
#include <list>
#include <assert.h>

using namespace Arts;
using namespace std;

class Arts::AudioIOPrivate {
public:
	map<AudioIO::AudioParam, int> paramMap;
	map<AudioIO::AudioParam, string> paramStrMap;
};

AudioIO::AudioIO() :d(new AudioIOPrivate)
{
}

AudioIO::~AudioIO()
{
	delete d;
}

int& AudioIO::param(AudioParam param)
{
	/*
	 * returns d->paramMap[param], initializes new elements with -1 (for those
	 * parameters not handled explicitely by the derived AudioIO* class).
	 */

	map<AudioIO::AudioParam, int>::iterator pi = d->paramMap.find(param);
	if(pi == d->paramMap.end())
	{
		int& result = d->paramMap[param];
		result = -1;
		return result;
	}
	else return pi->second;
}

string& AudioIO::paramStr(AudioParam param)
{
	return d->paramStrMap[param];
}

void AudioIO::setParamStr(AudioParam p, const char *value)
{
	paramStr(p) = value;
}

void AudioIO::setParam(AudioParam p, int& value)
{
	param(p) = value;
}

int AudioIO::getParam(AudioParam p)
{
	return param(p);
}

const char *AudioIO::getParamStr(AudioParam p)
{
	return paramStr(p).c_str();
}

/* ---- factories ---- */

static list<AudioIOFactory *> *audioIOFactories = 0;

AudioIO *AudioIO::createAudioIO(const char *name)
{
	if(audioIOFactories)
	{
		list<AudioIOFactory *>::iterator i;
		for(i = audioIOFactories->begin(); i != audioIOFactories->end(); i++)
		{
			AudioIOFactory *factory = *i;

			if(strcmp(factory->name(),name) == 0)
				return factory->createAudioIO();
		}
	}
	return 0;
}

int AudioIO::queryAudioIOCount()
{
	return audioIOFactories->size();
}

int AudioIO::queryAudioIOParam(int /*audioIO*/, AudioParam /*p*/)
{
	return 0;
}

const char *AudioIO::queryAudioIOParamStr(int audioIO, AudioParam p)
{
	list<AudioIOFactory *>::iterator i = audioIOFactories->begin();

	while(audioIO && i != audioIOFactories->end()) { i++; audioIO--; }
	if(i == audioIOFactories->end()) return 0;
	
	switch(p)
	{
		case name:	
			return (*i)->name();
		case fullName:
			return (*i)->fullName();
		default:
			return 0;
	}
}

void AudioIO::addFactory(AudioIOFactory *factory)
{
	if(!audioIOFactories)
		audioIOFactories = new list<AudioIOFactory *>;
	
	audioIOFactories->push_back(factory);
}

void AudioIO::removeFactory(AudioIOFactory *factory)
{
	assert(audioIOFactories);

	audioIOFactories->remove(factory);
	if(audioIOFactories->empty())
	{
		delete audioIOFactories;
		audioIOFactories = 0;
	}
}

void AudioIOFactory::startup()
{
	AudioIO::addFactory(this);
}

void AudioIOFactory::shutdown()
{
	AudioIO::removeFactory(this);
}
