	/*

	Copyright (C) 1998-1999 Stefan Westerfeld
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

    */

#include "bus.h"
#include "flowsystem.h"
#include <set>

using namespace Arts;
using namespace std;
static BusManager *the_BusManager = 0;

BusManager::BusManager()
{
	// this cosntructor isn't public (Singleton)
}

BusManager *BusManager::the()
{
	if(!the_BusManager) the_BusManager = new BusManager;
	return(the_BusManager);
}

BusManager::Bus *BusManager::findBus(string name)
{
	list<Bus *>::iterator bi;

	for(bi = _busList.begin(); bi != _busList.end(); bi++)
	{
		if((*bi)->name == name) return(*bi);
	}
	Bus *bus = new Bus;
	bus->name = name;
	_busList.push_back(bus);
	return(bus);
}

vector<string> *BusManager::busList()
{
	set<string> names;
	set<string>::iterator si;
	
	list<Bus *>::iterator bi;
	for(bi = _busList.begin(); bi != _busList.end(); bi++)
		names.insert((*bi)->name);

	vector<string> *bl = new vector<string>;
	for(si=names.begin();si != names.end();si++) bl->push_back(*si);
	return bl; 
}

void BusManager::reBuild(Bus *bus)
{
	list<BusClient *>::iterator client,server;
	long channels = 2;

	artsdebug("rebuilding bus %s\n",bus->name.c_str());
	artsdebug(" - %ld channels\n",channels);
	artsdebug(" - %d clients\n",bus->clients.size());
	artsdebug(" - %d servers\n",bus->servers.size());

	// clean room approach: delete old channels, and make a new
	//                      Synth_MULTI_ADD for summing up the data on the bus

	Arts::Synth_MULTI_ADD left, right;
	bus->channels.clear();
	bus->channels.push_back(left);
	bus->channels.push_back(right);

	for(client = bus->clients.begin(); client != bus->clients.end(); client++)
	{
		Arts::SynthModule m = (*client)->module();
		m._node()->virtualize("left", left._node(), "invalue");
		m._node()->virtualize("right", right._node(), "invalue");
	}

	left.start();
	right.start();

	for(server = bus->servers.begin(); server != bus->servers.end(); server++)
	{
		(*server)->configureBus(bus->channels);
	}
}

void BusManager::erase(BusClient *busclient)
{
	list<Bus *>::iterator bi;
	
	for(bi = _busList.begin(); bi != _busList.end(); bi++)
	{
		int found = 0;
		Bus *bus = *bi;

		list<BusClient *>::iterator client = bus->clients.begin();
		while(client != bus->clients.end())
		{
			if((*client) == busclient)
			{
				bus->clients.erase(client);
				client = bus->clients.begin();
				found++;
			}
			else client++;
		}

		list<BusClient *>::iterator server = bus->servers.begin();
		while(server != bus->servers.end())
		{
			if((*server) == busclient)
			{
				bus->clients.erase(server);
				server = bus->servers.begin();
				found++;
			}
			else server++;
		}

		// found something?
		if(found != 0)
		{
			if(bus->clients.empty() && bus->servers.empty())
			{
				// obsolete bus, remove
				cout << "removing obsolete bus " << bus->name << endl;
				_busList.erase(bi);
				delete bus;
			}
			else
			{
				reBuild(bus);
			}
			return;
		}
	}
}

void BusManager::addClient(string busname, BusClient *client)
{
	Bus *bus = findBus(busname);
	bus->clients.push_back(client);
	reBuild(bus);
}

void BusManager::removeClient(BusClient *client)
{
	erase(client);
}

void BusManager::addServer(string busname, BusClient *server)
{
	Bus *bus = findBus(busname);
	bus->servers.push_back(server);
	reBuild(bus);
}

void BusManager::removeServer(BusClient *server)
{
	erase(server);
}

class Synth_BUS_UPLINK_impl :public Synth_BUS_UPLINK_skel,
										StdSynthModule, BusClient
{
	BusManager *bm;
	bool running, active, relink;
	string _busname;
public:
	Synth_BUS_UPLINK_impl();

	void configureBus(const vector<Synth_MULTI_ADD>& channels);
	string busname() { return _busname; }
	void busname(const string& newname);
	AutoSuspendState autoSuspend() { return asSuspend; }
	void streamInit();
	void streamEnd();
	SynthModule module();

	void CallBack();

	void connect();
	void disconnect();
};

Synth_BUS_UPLINK_impl::Synth_BUS_UPLINK_impl() :running(false)
{
	bm = BusManager::the();
}

void Synth_BUS_UPLINK_impl::streamInit()
{
	cout << "+ streamInit()" << endl;

	assert(!running);
	running = true;
	active = relink = false;

	connect();	// connect to the BusManager
	cout << "- streamInit()" << endl;
}

void Synth_BUS_UPLINK_impl::busname(const string& newname)
{
	cout << "+ busname(" << newname << ")" << endl;
	_busname = newname;

	/* TODO */
	// to be sure that reconnection happens when outside the scheduling cycle
	if(running)
	{
		relink = true;
		CallBack();
	}
	cout << "- busname(" << newname << ")" << endl;
}

void Synth_BUS_UPLINK_impl::connect()
{
	cout << "+ connect()" << endl;
	assert(active == false);
	if(_busname != "")
	{
		active = true;
		bm->addClient(_busname, this);
	}
	cout << "- connect()" << endl;
}

void Synth_BUS_UPLINK_impl::disconnect()
{
	cout << "+ disconnect()" << endl;
	if(active == true)
	{
		bm->removeClient(this);
		active = false;
	}
	cout << "- disconnect()" << endl;
}

void Synth_BUS_UPLINK_impl::CallBack()
{
	cout << "+ CallBack()" << endl;
	if(relink)
	{
		disconnect();
		connect();
		relink = false;
	}
	cout << "- CallBack()" << endl;
}

void Synth_BUS_UPLINK_impl::streamEnd()
{
	cout << "+ disconnect()" << endl;
	disconnect();

	assert(running);
	running = false;
	cout << "- disconnect()" << endl;
}

SynthModule Synth_BUS_UPLINK_impl::module()
{
	return SynthModule::_from_base(_copy());
}

void Synth_BUS_UPLINK_impl::configureBus(const vector<Synth_MULTI_ADD>&)
{
	// only in downlinks
}

REGISTER_IMPLEMENTATION(Synth_BUS_UPLINK_impl);

class Synth_BUS_DOWNLINK_impl :public Synth_BUS_DOWNLINK_skel,
										StdSynthModule, BusClient
{
	bool running, active, relink;
	BusManager *bm;

	string _busname;

	void connect();
	void disconnect();

public:
	string busname() { return _busname; }
	void busname(const string& newname);

	Synth_BUS_DOWNLINK_impl();
	AutoSuspendState autoSuspend() { return asSuspend; }
	void streamInit();
	void streamEnd();
	void CallBack();
	void configureBus(const vector<Synth_MULTI_ADD>& channels);
	SynthModule module();
};

Synth_BUS_DOWNLINK_impl::Synth_BUS_DOWNLINK_impl() :running(false)
{
	bm = BusManager::the();
}

void Synth_BUS_DOWNLINK_impl::streamInit()
{
	assert(!running);
	running = true;

	active = relink = false;
	connect();
}

void Synth_BUS_DOWNLINK_impl::streamEnd()
{
	assert(running);
	running = false;

	disconnect();
}

void Synth_BUS_DOWNLINK_impl::connect()
{
	assert(active == false);
	if(_busname != "")
	{
		active = true;
		bm->addServer(_busname, this);
	}
}

void Synth_BUS_DOWNLINK_impl::disconnect()
{
	if(active == true)
	{
		bm->removeServer(this);
		active = false;
	}
}

void Synth_BUS_DOWNLINK_impl::CallBack()
{
	if(relink)
	{
		disconnect();
		connect();
		relink = false;
	}
}

void Synth_BUS_DOWNLINK_impl::busname(const string& newname)
{
	_busname = newname;

	/* TODO */
	// to be sure that reconnection happens when outside the scheduling cycle
	if(running)
	{
		relink = true;
		CallBack();
	}
}

SynthModule Synth_BUS_DOWNLINK_impl::module()
{
	return SynthModule::_from_base(_copy());
}

void Synth_BUS_DOWNLINK_impl::configureBus(const vector<Synth_MULTI_ADD>& channels)
{
	_node()->virtualize("left",channels[0]._node(),"outvalue");
	_node()->virtualize("right",channels[1]._node(),"outvalue");
}

REGISTER_IMPLEMENTATION(Synth_BUS_DOWNLINK_impl);
