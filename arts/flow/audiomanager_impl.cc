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
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include "bus.h"

using namespace Arts;
using namespace std;

namespace Arts {

class AudioManagerClient_impl;

class AudioManagerAssignable {
public:
	virtual long ID() = 0;
	virtual void destination(const string& newDestination) = 0;
};

#ifdef __SUNPRO_CC
/* Bloody SunPRO CC has problems with instantiation of the below two
   templates, if the _clients and assignable member of AudioManager_impl
   are declared to be directly the type list<xxx *>. So instead be move
   a typedef in between, which makes it magically work.
   We could also use an explicit instantiation, but this is not allowed
   on all C++ compilers in this scope.  Note also, that we don't need
   to replace _all_ occurences of list<xxx*> below, only the two in the
   member declaration.  What a mess.  */
typedef list<AudioManagerClient_impl *> L_AMC;
typedef list<AudioManagerAssignable *> L_AMA;
#else
/* with normal compilers there is no need for this typedef.  */
#define L_AMC list<AudioManagerClient_impl *>
#define L_AMA list<AudioManagerAssignable *>
#endif

class AudioManager_impl : virtual public AudioManager_skel
{
protected:
	L_AMC _clients;
	L_AMA assignable;
	long _changes, nextID;
	static AudioManager_impl *instance;
public:
	AudioManager_impl()
	{
		assert(!instance);
		instance = this;
		nextID = 1;
	}
	~AudioManager_impl()
	{
		assert(instance);
		instance = 0;
	}
	vector<string> *destinations() { return BusManager::the()->busList(); }
	long changes() { return _changes; }

	vector<AudioManagerInfo> *clients();
	void setDestination(long ID, const string& newDestination);
	AudioManagerClient_impl *findClient(long ID);

	// non MCOP interface
	static AudioManager_impl *the() {
		assert(instance);
		return instance;
	}
	long addClient(AudioManagerClient_impl *client) {
		_clients.push_back(client);
		_changes++;
		return nextID++;
	}
	void removeClient(AudioManagerClient_impl *client) {
		_changes++;
		_clients.remove(client);
	}
	void addAssignable(AudioManagerAssignable *a);
	void removeAssignable(AudioManagerAssignable *a) {
		assignable.remove(a);
	}
};

AudioManager_impl *AudioManager_impl::instance = 0;

class AudioManagerClient_impl :virtual public AudioManagerClient_skel
{
protected:
	string _title, _autoRestoreID;
	long _ID;
	AudioManagerDirection _direction;
	string _destination;

public:
	AudioManagerClient_impl() {
		_ID = AudioManager_impl::the()->addClient(this);
	}
	~AudioManagerClient_impl() {
		AudioManager_impl::the()->removeClient(this);
	}

	void title(const string& newvalue) { _title = newvalue; }
	string title() { return _title; }

	void autoRestoreID(const string& newvalue) { _autoRestoreID = newvalue; }
	string autoRestoreID() { return _autoRestoreID; }

	long ID() { return _ID; }

    AudioManagerDirection direction() { return _direction; }
	void direction(AudioManagerDirection newvalue) { _direction = newvalue; }

	void constructor(AudioManagerDirection cdirection, const string& ctitle,
					 const string& cautoRestoreID)
	{
		direction(cdirection);
		title(ctitle);
		autoRestoreID(cautoRestoreID);
	}

	/* non MCOP interface */
	string destination() { return _destination; }
	void destination(const string& newvalue) { _destination = newvalue; }
};

// this suffers a tiny bit from copypasting due to simpler
// inheritance hierarchy
class Synth_AMAN_PLAY_impl :virtual public Synth_AMAN_PLAY_skel,
							virtual public AudioManagerAssignable,
							virtual public StdSynthModule
{
protected:
	Synth_BUS_UPLINK uplink;
	AudioManagerClient client;
	bool externalClient;
public:
	/* forward requests for title/autoRestoreID to client */
	void title(const string& newvalue) { client.title(newvalue); }
	string title() { return client.title(); }

	void autoRestoreID(const string& newvalue) {client.autoRestoreID(newvalue);}
	string autoRestoreID() { return client.autoRestoreID(); }

	/* assign an already well-known client */
	void constructor(AudioManagerClient client) {
		this->client = client;
		externalClient = true;
	}

	Synth_AMAN_PLAY_impl() : externalClient(false) {
		_node()->virtualize("left",uplink._node(),"left");
		_node()->virtualize("right",uplink._node(),"right");
	}
	void streamInit() {
		if(externalClient)
			assert(client.direction() == amPlay);
		else
			client.direction(amPlay);

		AudioManager_impl::the()->addAssignable(this);
		uplink.start();
	}
	void streamEnd() {
		AudioManager_impl::the()->removeAssignable(this);
		uplink.stop();
	}

	// Assignable:
	void destination(const string& destination) { uplink.busname(destination); }
	long ID() { return client.ID(); }
};

// this suffers a tiny bit from copypasting due to simpler
// inheritance hierarchy
class Synth_AMAN_RECORD_impl :virtual public Synth_AMAN_RECORD_skel,
							virtual public AudioManagerAssignable,
							virtual public StdSynthModule
{
protected:
	Synth_BUS_DOWNLINK downlink;
	AudioManagerClient client;
	bool externalClient;
public:
	/* forward requests for title/autoRestoreID to client */
	void title(const string& newvalue) { client.title(newvalue); }
	string title() { return client.title(); }

	void autoRestoreID(const string& newvalue) {client.autoRestoreID(newvalue);}
	string autoRestoreID() { return client.autoRestoreID(); }

	/* assign an already well-known client */
	void constructor(AudioManagerClient client) {
		this->client = client;
		externalClient = true;
	}

	Synth_AMAN_RECORD_impl() : externalClient(false) {
		_node()->virtualize("left",downlink._node(),"left");
		_node()->virtualize("right",downlink._node(),"right");
	}

	void streamInit() {
		if(externalClient)
			assert(client.direction() == amRecord);
		else
			client.direction(amRecord);

		AudioManager_impl::the()->addAssignable(this);
		downlink.start();
	}
	void streamEnd() {
		AudioManager_impl::the()->removeAssignable(this);
		downlink.stop();
	}

	// Assignable:
	void destination(const string& destination){downlink.busname(destination);}
	long ID() { return client.ID(); }
};

vector<AudioManagerInfo> *AudioManager_impl::clients()
{
	vector<AudioManagerInfo> *result = new vector<AudioManagerInfo>;
	list<AudioManagerClient_impl *>::iterator i;

	for(i = _clients.begin(); i != _clients.end(); i++)
	{
		AudioManagerClient_impl *client = *i;

		AudioManagerInfo info;
		info.ID = client->ID();
		info.direction = client->direction();
		info.title = client->title();
		info.autoRestoreID = client->autoRestoreID();
		info.destination = client->destination();

		result->push_back(info);
	}
	return result;
}


AudioManagerClient_impl *AudioManager_impl::findClient(long ID)
{
	list<AudioManagerClient_impl *>::iterator i;

	for(i = _clients.begin(); i != _clients.end(); i++)
	{
		AudioManagerClient_impl *client = *i;

		if(client->ID() == ID)
			return client;
	}
	return 0;
}

void AudioManager_impl::setDestination(long ID, const string& newDestination)
{
	AudioManagerClient_impl *client = findClient(ID);
	if(client)
	{
		client->destination(newDestination);

		list<AudioManagerAssignable *>::iterator i;
		for(i = assignable.begin(); i != assignable.end(); i++)
		{
			AudioManagerAssignable *a = *i;
			if(a->ID() == ID) a->destination(newDestination);
		}
		_changes++;
	}
}

void AudioManager_impl::addAssignable(AudioManagerAssignable *a)
{
	// ensure that this client is assigned properly
	AudioManagerClient_impl *client = findClient(a->ID());
	assert(client);

	if(client->destination() == "")
	{
		// TODO: more flexible default assignment policies - load autoRestoreID
		// related stuff here (or somewhere else)

		switch(client->direction())
		{
			case amPlay:	client->destination("out_soundcard");
				break;
			case amRecord:	client->destination("in_soundcard");
				break;
		}
	}

	assignable.push_back(a);
	a->destination(client->destination());
}

REGISTER_IMPLEMENTATION(AudioManagerClient_impl);
REGISTER_IMPLEMENTATION(AudioManager_impl);
REGISTER_IMPLEMENTATION(Synth_AMAN_PLAY_impl);
REGISTER_IMPLEMENTATION(Synth_AMAN_RECORD_impl);

};
