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

#include "stdsynthmodule.h"
#include "debug.h"
#include "artsflow.h"

class BusClient
{
public:
	virtual void configureBus(const std::vector<Arts::Synth_MULTI_ADD>&
				channels) = 0;
	virtual Arts::SynthModule module() = 0;
};


class BusManager
{
protected:
	struct Bus
	{
		std::string name;
		std::list<BusClient *> clients;
		std::list<BusClient *> servers;
		std::vector<Arts::Synth_MULTI_ADD> channels;
	};
	std::list<Bus *> _busList;

	BusManager();

public:
	Bus* findBus(std::string name);
	void reBuild(Bus *bus);
	void erase(BusClient *busclient);
	void addClient(std::string busname, BusClient *client);
	void removeClient(BusClient *client);
	void addServer(std::string busname, BusClient *server);
	void removeServer(BusClient *server);

	std::vector<std::string> *busList();

	static BusManager *the();
};
