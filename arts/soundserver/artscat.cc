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
#include "artsversion.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

using namespace std;
using namespace Arts;

int cfgSamplingRate = 44100;
int cfgBits = 16;
int cfgChannels = 2;

class Sender :	public ByteSoundProducer_skel,
				public StdSynthModule,
				public IONotify
{
protected:
	FILE *pfile;
	int pfd;
	int packets;
	bool waiting;
	queue< DataPacket<mcopbyte>* > wqueue;
public:
	Sender(FILE *input,float minStreamBufferTime) : pfile(input), waiting(false)
	{
		pfd = fileno(pfile);

		int rc = fcntl(pfd, F_SETFL, O_NONBLOCK);
		assert(rc != -1);

		/*
		 * calculate stream buffer paramters
		 */
		float streamBufferTime;
		packets = 7;
		do {
			packets++;
			streamBufferTime = (float)(packets * packetCapacity * 1000)
			                 / (float)(samplingRate() * channels() * 2);
		} while(streamBufferTime < minStreamBufferTime);
	}

	~Sender()
	{
		if(waiting) Dispatcher::the()->ioManager()->remove(this,IOType::read);
	}

	long samplingRate() { return cfgSamplingRate; }
	long channels()     { return cfgChannels; }
	long bits()         { return cfgBits; }
	bool finished()     { return (pfile == 0); }
	string title()		{ return "artscat"; }

	void streamStart()
	{
		/*
		 * start streaming
		 */
		outdata.setPull(packets, packetCapacity);
	}

	enum { packetCapacity = 4096 };
	void handle_eof()
	{
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

		/*
		 * terminate dispatcher to end the program
		 */
		Dispatcher::the()->terminate();
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

static void exitUsage(const char *progname)
{
	fprintf(stderr,"usage: %s [ options ] [ <filename> ]\n",progname);
	fprintf(stderr,"-r <samplingrate>   set samplingrate to use\n");
	fprintf(stderr,"-b <bits>           set number of bits (8 or 16)\n");
	fprintf(stderr,"-c <channels>       set number of channels (1 or 2)\n");
	fprintf(stderr,"-v                  show version\n");
	fprintf(stderr,"-h                  display this help and exit\n");
	exit(1);	
}

int main(int argc, char **argv)
{
	int optch;
	while((optch = getopt(argc,argv,"r:b:c:hv")) > 0)
	{
		switch(optch)
		{
			case 'r': cfgSamplingRate = atoi(optarg);
				break;
			case 'b': cfgBits = atoi(optarg);
				break;
			case 'c': cfgChannels = atoi(optarg);
				break;
			case 'v': 
				printf("artscat %s\n",ARTS_VERSION);
				exit(0);
				break;
			case 'h':
			default: 
					exitUsage(argc?argv[0]:"artscat");
				break;
		}
	}

	FILE *infile = stdin;

	if (optind < argc)
    {
		string filename = argv[optind];
		if(filename != "-")
		{
			infile = fopen(filename.c_str(),"r");
			if(!infile)
			{
				cerr << "Can't open file '" << argv[optind] << "'." << endl;
				exit(1);
			}
		}
	}                                                                    
	Dispatcher dispatcher;
	SimpleSoundServer server(Reference("global:Arts_SimpleSoundServer"));

	if(server.isNull())
	{
		cerr << "Can't connect to sound server" << endl;
		return 1;
	}

	ByteSoundProducer sender = ByteSoundProducer::_from_base(new Sender(infile,server.minStreamBufferTime()));
	server.attach(sender);
	sender.start();
	dispatcher.run();
	server.detach(sender);
}
