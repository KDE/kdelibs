    /*

    Copyright (C) 2000 Stefan Westerfeld
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

#include "soundserver.h"
#include "stdsynthmodule.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule,
				public IONotify
{
protected:
	FILE *pfile;
	int pfd;
	bool waiting;
	queue< DataPacket<mcopbyte>* > wqueue;
public:
	Sender(FILE *input) : pfile(input), waiting(false)
	{
		pfd = fileno(pfile);

		int rc = fcntl(pfd, F_SETFL, O_NONBLOCK);
		assert(rc != -1);
	}

	~Sender()
	{
		if(waiting) Dispatcher::the()->ioManager()->remove(this,IOType::read);
		Dispatcher::the()->terminate();
	}

	long samplingRate() { return 44100; }
	long channels()     { return 2; }
	long bits()         { return 16; }
	bool finished()     { return (pfile == 0); }

	void start()
	{
		/*
		 * start streaming
		 */
		outdata.setPull(16, packetCapacity);
	}

	enum { packetCapacity = 4096 };
	void handle_eof()
	{
		cout << "EOF" << endl;
		/*
		 * cleanup
		 */
		outdata.endPull();
		pclose(pfile);
		pfile = 0;
		pfd = 0;

		/*
		 * remove all packets from the wqueue
		 */
		while(!wqueue.empty())
		{
			DataPacket<mcopbyte> *packet = wqueue.front();
			packet->size = 0;
			packet->send();
			wqueue.pop();
		}
		
		/*
		 * go out of the waiting mode
		 */
		if(waiting)
		{
			Dispatcher::the()->ioManager()->remove(this,IOType::read);
			waiting = false;
		}
	}
	void request_outdata(DataPacket<mcopbyte> *packet)
	{
		if(!waiting)
		{
			packet->size = read(pfd, packet->contents, packetCapacity);
			if(packet->size > 0)
			{
				// got something - done here
				packet->send();
				return;
			}
			Dispatcher::the()->ioManager()->watchFD(pfd,IOType::read,this);
			waiting = true;
		}

		wqueue.push(packet);
	}

	void notifyIO(int,int)
	{
		assert(waiting);

		DataPacket<mcopbyte> *packet = wqueue.front();
		packet->size = read(pfd, packet->contents, packetCapacity);
		assert(packet->size >= 0);
		if(packet->size == 0) {
			handle_eof();
			return;
		}
		packet->send();

		wqueue.pop();

		if(wqueue.empty())
		{
			Dispatcher::the()->ioManager()->remove(this,IOType::read);
			waiting = false;
		}
	}
};

int main(int argc, char **argv)
{
	if(argc != 1)
	{
		cerr << "usage: " << argv[0] << endl;
		return 1;
	}

	Dispatcher dispatcher;
	SimpleSoundServer_var server;

	server = SimpleSoundServer::_fromString("global:Arts_SimpleSoundServer");
	if(!server)
	{
		cerr << "Can't connect to sound server" << endl;
		return 1;
	}

	Sender *sender = new Sender(stdin);
	server->attach(sender);
	sender->start();
	// we don't hold a reference to the sender at all any more - the
	// soundserver should do so, as long as he wants
	sender->_release();		// TODO: race condition? shouldn't be.
	dispatcher.run();
}
