    /*

    Copyright (C) 2000 Stefan Westerfeld
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

#include "soundserver.h"
#include "stdsynthmodule.h"
#include "artsversion.h"
#include "debug.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

using namespace std;
using namespace Arts;

int cfgSamplingRate = 44100;
int cfgBits = 16;
int cfgChannels = 2;

class Receiver :	public ByteSoundReceiver_skel,
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
	Receiver(FILE *input,float minStreamBufferTime) : pfile(input), waiting(false)
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

	~Receiver()
	{
		if(waiting) Dispatcher::the()->ioManager()->remove(this,IOType::write);
	}

	long samplingRate() { return cfgSamplingRate; }
	long channels()     { return cfgChannels; }
	long bits()         { return cfgBits; }
	bool finished()     { return (pfile == 0); }

	enum { packetCapacity = 4096 };
	void process_indata(DataPacket<mcopbyte> *packet)
	{
		if(!waiting)
		{
			int size = write( pfd, packet->contents, packet->size );
			if( size != packet->size ) {
				arts_debug( "Not written enough" );
				//Dispatcher::the()->ioManager()->watchFD(pfd,IOType::write,this);
				//waiting = true;
			}
			packet->processed();
			return;
		}

		wqueue.push(packet);
	}

	void notifyIO(int,int)
	{
#if 0
		assert(waiting);

		DataPacket<mcopbyte> *packet = wqueue.front();
		packet->size = read(pfd, packet->contents, packetCapacity);
		assert(packet->size >= 0);
		if(packet->size == 0) {
			return;
		}
		packet->send();

		wqueue.pop();

		if(wqueue.empty())
		{
			Dispatcher::the()->ioManager()->remove(this,IOType::write);
			waiting = false;
		}
#endif
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
				printf("artsdump %s\n",ARTS_VERSION);
				exit(0);
				break;
			case 'h':
			default: 
					exitUsage(argc?argv[0]:"artsdump");
				break;
		}
	}

	FILE *outfile = stdout;

	if (optind < argc)
    {
		string filename = argv[optind];
		if(filename != "-")
		{
			outfile = fopen(filename.c_str(),"w");
			if(!outfile)
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

	ByteSoundReceiver receiver = ByteSoundReceiver::_from_base(new Receiver(outfile,server.minStreamBufferTime()));
	server.attachRecorder(receiver);
	receiver.start();
	dispatcher.run();
	receiver.stop();
	server.detachRecorder(receiver);
}
