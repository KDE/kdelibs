    /*

    Copyright (C) 1998-2000 Stefan Westerfeld
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

#include "bus.h"
#include "flowsystem.h"
#include "debug.h"
#include <iostream>
#include <set>

#ifdef __SUNPRO_CC
/* SunPRO CC looses to link this when the_BusManager is static, because
   later a template implementation file references this symbol. */
#define the_BusManager __internal_aRts_the_BusManager__Bahh__
#else
static
#endif
  Arts::BusManager *the_BusManager = 0;

using namespace Arts;
using namespace std;

// shutdown bus manager on termination

namespace Arts {
	static class BusManagerShutdown :public StartupClass
	{
	public:
		void startup() { };
		void shutdown()
		{
			if(::the_BusManager)
			{
				delete ::the_BusManager;
				::the_BusManager = 0;
			}
		}
	}	The_BusManagerShutdown;
};

BusManager::BusManager()
{
	// this constructor isn't public (Singleton)
}

BusManager *BusManager::the()
{
	if(!::the_BusManager) ::the_BusManager = new BusManager;
	return(::the_BusManager);
}

BusManager::Bus *BusManager::findBus(const string& name)
{
	list<Bus *>::iterator bi;

	for(bi = _busList.begin(); bi != _busList.end(); bi++)
	{
		if((*bi)->name == name) return(*bi);
	}
	Bus *bus = new Bus;
	bus->left.start();
	bus->right.start();
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

void BusManager::addClient(const string& busname, BusClient *client)
{
	Bus *bus = findBus(busname);
	bus->clients.push_back(client);

	// attach the new client
	client->snode()->virtualize("left", bus->left._node(), "invalue");
	client->snode()->virtualize("right", bus->right._node(), "invalue");
}

void BusManager::removeClient(BusClient *client)
{
	list<Bus *>::iterator bi;
	for(bi = _busList.begin(); bi != _busList.end(); bi++)
	{
		Bus *bus = *bi;

		list<BusClient *>::iterator ci;
		for(ci = bus->clients.begin(); ci != bus->clients.end(); ci++)
		{
			if(*ci == client)
			{
				bus->clients.erase(ci);

				if(bus->clients.empty() && bus->servers.empty())
				{
					_busList.erase(bi);
					delete bus;
				}
				else
				{
					client->snode()->devirtualize("left",
							bus->left._node(), "invalue");
					client->snode()->devirtualize("right",
							bus->right._node(), "invalue");
				}
				return;
			}
		}
	}
}

void BusManager::addServer(const string& busname, BusClient *server)
{
	Bus *bus = findBus(busname);
	bus->servers.push_back(server);

	server->snode()->virtualize("left",bus->left._node(),"outvalue");
	server->snode()->virtualize("right",bus->right._node(),"outvalue");
}

void BusManager::removeServer(BusClient *server)
{
	list<Bus *>::iterator bi;
	
	for(bi = _busList.begin(); bi != _busList.end(); bi++)
	{
		Bus *bus = *bi;

		list<BusClient *>::iterator si;
		for(si = bus->servers.begin(); si != bus->servers.end(); si++)
		{
			if(*si == server)
			{
				bus->servers.erase(si);
				if(bus->clients.empty() && bus->servers.empty())
				{
					_busList.erase(bi);
					delete bus;
				}
				else
				{
					server->snode()->devirtualize("left",
									bus->left._node(), "outvalue");
					server->snode()->devirtualize("right",
									bus->right._node(),"outvalue");
				}
				return;
			}
		}
	}
}

namespace Arts {

class Synth_BUS_UPLINK_impl :public Synth_BUS_UPLINK_skel,
										public StdSynthModule, public BusClient
{
	BusManager *bm;
	bool running, active, relink;
	string _busname;
public:
	Synth_BUS_UPLINK_impl();

	string busname() { return _busname; }
	void busname(const string& newname);
	AutoSuspendState autoSuspend() { return asSuspend; }
	void streamInit();
	void streamEnd();
	ScheduleNode *snode() { return _node(); }

	void CallBack();

	void connect();
	void disconnect();
};

REGISTER_IMPLEMENTATION(Synth_BUS_UPLINK_impl);

};

Synth_BUS_UPLINK_impl::Synth_BUS_UPLINK_impl() :running(false)
{
	bm = BusManager::the();
}

void Synth_BUS_UPLINK_impl::streamInit()
{
	assert(!running);
	running = true;
	active = relink = false;

	connect();	// connect to the BusManager
}

void Synth_BUS_UPLINK_impl::busname(const string& newname)
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

void Synth_BUS_UPLINK_impl::connect()
{
	assert(active == false);
	if(_busname != "")
	{
		active = true;
		bm->addClient(_busname, this);
	}
}

void Synth_BUS_UPLINK_impl::disconnect()
{
	if(active == true)
	{
		bm->removeClient(this);
		active = false;
	}
}

void Synth_BUS_UPLINK_impl::CallBack()
{
	if(relink)
	{
		disconnect();
		connect();
		relink = false;
	}
}

void Synth_BUS_UPLINK_impl::streamEnd()
{
	disconnect();

	assert(running);
	running = false;
}

namespace Arts {

class Synth_BUS_DOWNLINK_impl :public Synth_BUS_DOWNLINK_skel,
										public StdSynthModule, public BusClient
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
	ScheduleNode *snode() { return _node(); }
};

REGISTER_IMPLEMENTATION(Synth_BUS_DOWNLINK_impl);

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
