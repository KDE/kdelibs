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

#include "mcoputils.h"
#include <csignal>
#include <iostream>
#include <stdio.h>
#include "soundserver.h"
#include "audiosubsys.h"
#include "synthschedule.h"

using namespace std;

extern "C" void stopServer(int)
{
	Dispatcher::the()->terminate();
}

static void initSignals()
{
    signal(SIGHUP ,stopServer);
    signal(SIGQUIT,stopServer);
    signal(SIGINT ,stopServer);
    signal(SIGTERM,stopServer);                                                 
}

static void exitUsage(const char *progname)
{
	fprintf(stderr,"usage: %s [ options ]\n",progname);
	fprintf(stderr,"-r <samplingrate>   set samplingrate to use\n");
	fprintf(stderr,"-h                  display this help and exit\n");
	fprintf(stderr,"-n                  enable network transparency\n");
	fprintf(stderr,"-F <fragments>      number of fragments\n");
	fprintf(stderr,"-S <size>           fragment size in bytes\n");
	exit(1);	
}

static Dispatcher::StartServer	cfgServers		= Dispatcher::startUnixServer;
static int  					cfgSamplingRate	= 0;
static int  					cfgFragmentCount= 0;
static int  					cfgFragmentSize	= 0;

static void handleArgs(int argc, char **argv)
{
	int optch;
	while((optch = getopt(argc,argv,"r:nF:S:h")) > 0)
	{
		switch(optch)
		{
			case 'r': cfgSamplingRate = atoi(optarg);
				break;
			case 'n': cfgServers = static_cast<Dispatcher::StartServer>( cfgServers | Dispatcher::startTCPServer);
				break;
			case 'F': cfgFragmentCount = atoi(optarg);
				break;
			case 'S': cfgFragmentSize = atoi(optarg);
				break;
			case 'h':
			default: 
					exitUsage(argc?argv[0]:"artsd");
				break;
		}
	}
}

int main(int argc, char **argv)
{
	handleArgs(argc, argv);

	Dispatcher dispatcher(0,cfgServers);
	initSignals();

	/* apply configuration */
	if(cfgSamplingRate)  AudioSubSystem::the()->samplingRate(cfgSamplingRate);
	if(cfgFragmentCount) AudioSubSystem::the()->fragmentCount(cfgFragmentCount);
	if(cfgFragmentSize)  AudioSubSystem::the()->fragmentSize(cfgFragmentSize);

	/* start sound server implementation */
	SimpleSoundServer server;

	bool result = ObjectManager::the()
				->addGlobalReference(server,"Arts_SimpleSoundServer")
             &&   ObjectManager::the()
				->addGlobalReference(server,"Arts_PlayObjectFactory");
	if(!result)
	{
		cerr <<
"Error: Can't add object reference (perhaps it is already running?)." << endl <<
"       If you are sure it is not already running, remove the relevant files:"
              << endl << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SimpleSoundServer") << endl <<
"       "<< MCOPUtils::createFilePath("Arts_PlayObjectFactory") << endl << endl;
		return 1;
	}

	dispatcher.run();
	return 0;
}
