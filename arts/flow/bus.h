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

#ifndef ARTS_BUS_H
#define ARTS_BUS_H

#include "stdsynthmodule.h"
#include "artsflow.h"

/*
 * BC - Status (2000-09-30): BusClient, BusManager
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of libartsflow's
 * Synth_BUS_* and Synth_AMAN_* and AudioManager interfaces. Use these for
 * public use instead, and don't touch this here.
 */


namespace Arts {

class BusClient
{
public:
	virtual Arts::ScheduleNode *snode() = 0;
};


class BusManager
{
protected:
	struct Bus
	{
		std::string name;
		std::list<BusClient *> clients;
		std::list<BusClient *> servers;
		Arts::Synth_MULTI_ADD left, right;
	};
	std::list<Bus *> _busList;

	BusManager();

public:
	Bus* findBus(const std::string& name);

	void addClient(const std::string& busname, BusClient *client);
	void removeClient(BusClient *client);
	void addServer(const std::string& busname, BusClient *server);
	void removeServer(BusClient *server);

	std::vector<std::string> *busList();

	static BusManager *the();
};
};

#endif /* ARTS_BUS_H */
