    /*

    Copyright (C) 2000-2001 Stefan Westerfeld
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
#include <signal.h>
#include <iostream>
#include <stdio.h>
#include "soundserver.h"
#include "audiosubsys.h"
#include "audioio.h"
#include "synthschedule.h"
#include "tcpserver.h"
#include "cpuusage.h"
#include "debug.h"
#include "artsversion.h"

using namespace std;
using namespace Arts;

extern "C" void stopServer(int)
{
	Dispatcher::the()->terminate();
}

static void initSignals()
{
    signal(SIGHUP ,stopServer);
    signal(SIGINT ,stopServer);
    signal(SIGTERM,stopServer);
}

static void exitUsage(const char *progname)
{
	fprintf(stderr,"usage: %s [ options ]\n",progname);
	fprintf(stderr,"\n");
	fprintf(stderr,"server/network options:\n");
	fprintf(stderr,"-n                  enable network transparency\n");
	fprintf(stderr,"-p <port>           set TCP port to use (implies -n)\n");
	fprintf(stderr,"-u                  public, no authentication (dangerous!)\n");
	fprintf(stderr,"-N                  use larger network buffers\n");
	fprintf(stderr,"-w <n>              increase network buffers by factor of <n>\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"audio options:\n");
	fprintf(stderr,"-a <audioiomethod>  select audio i/o method (oss, alsa, ...)\n");
	fprintf(stderr,"-r <samplingrate>   set samplingrate to use\n");
	fprintf(stderr,"-b <bits>           set number of bits (8 or 16)\n");
	fprintf(stderr,"-d                  enable full duplex operation\n");
	fprintf(stderr,"-D <devicename>     audio device (usually /dev/dsp)\n");
	fprintf(stderr,"-F <fragments>      number of fragments\n");
	fprintf(stderr,"-S <size>           fragment size in bytes\n");
	fprintf(stderr,"-s <seconds>        auto-suspend time in seconds\n");
	fprintf(stderr,"-f                  force starting artsd (if no soundcard is there, uses the null output device)\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"misc options:\n");
	fprintf(stderr,"-h                  display this help and exit\n");
	fprintf(stderr,"-A                  list possible audio i/o methods (for -a)\n");
	fprintf(stderr,"-v                  show version\n");
	fprintf(stderr,"-l <level>          information level\n");
	fprintf(stderr,"  3: quiet, 2: warnings, 1: info, 0: debug\n");
	fprintf(stderr,"-m <appName>        application to display messages\n");
	exit(1);	
}

static void exitListAudioIO()
{
	fprintf(stderr,"possible choices for the audio i/o method:\n");
	fprintf(stderr,"\n");

	for(int i = 0; i < AudioIO::queryAudioIOCount(); i++)
	{
		fprintf(stderr, "  %-10s%s\n",
			AudioIO::queryAudioIOParamStr(i, AudioIO::name),
			AudioIO::queryAudioIOParamStr(i, AudioIO::fullName));
	}
	fprintf(stderr,"\n");
	exit(0);	
}

static Dispatcher::StartServer	cfgServers		= Dispatcher::startUnixServer;
static int  					cfgSamplingRate	= 0;
static int  					cfgBits			= 0;
static int  					cfgFragmentCount= 0;
static int  					cfgFragmentSize	= 0;
static int  					cfgPort			= 0;
static int  					cfgDebugLevel	= 2;
static const char			   *cfgDebugApp		= 0;
static bool  					cfgFullDuplex	= 0;
static bool  					cfgForceStart	= 0;
static const char			   *cfgDeviceName   = 0;
#if defined(__osf__)
// tru64 does not have sound devices so avoid the default auto-detect
static const char              *cfgAudioIO      = "null";
#else
static const char              *cfgAudioIO      = 0;
#endif
static int                      cfgAutoSuspend  = 0;
static int                      cfgBuffers      = 0;

static bool						cmdListAudioIO  = false;

static void handleArgs(int argc, char **argv)
{
	int optch;
	while((optch = getopt(argc,argv,"r:p:nuF:S:hD:dl:a:Ab:s:m:vNw:f")) > 0)
	{
		switch(optch)
		{
			case 'p': cfgPort = atoi(optarg); // setting a port => network transparency
			case 'n': cfgServers = static_cast<Dispatcher::StartServer>( cfgServers | Dispatcher::startTCPServer);
				break;
			case 'a': cfgAudioIO = optarg;
				break;
			case 'r': cfgSamplingRate = atoi(optarg);
				break;
			case 'b': cfgBits = atoi(optarg);
				break;
			case 's': cfgAutoSuspend = atoi(optarg);
				break;
			case 'F': cfgFragmentCount = atoi(optarg);
				break;
			case 'S': cfgFragmentSize = atoi(optarg);
				break;
			case 'D': cfgDeviceName = optarg;
				break;
			case 'd': cfgFullDuplex = true;
				break;
			case 'l': cfgDebugLevel = atoi(optarg);
				break;
			case 'm': cfgDebugApp = optarg;
				break;
			case 'u': cfgServers = static_cast<Dispatcher::StartServer>( cfgServers | Dispatcher::noAuthentication);
				break;
			case 'A': cmdListAudioIO = true;
				break;
			case 'v': printf("artsd %s\n",ARTS_VERSION);
					  exit(0);
				break;
			case 'N': cfgBuffers = 5;
				break;
			case 'w': cfgBuffers = atoi(optarg);
				break;
			case 'f': cfgForceStart = true;
				break;
			case 'h':
			default:
					exitUsage(argc?argv[0]:"artsd");
				break;
		}
	}
}

static bool publishReferences(SoundServerV2 server,
							  AudioManager audioManager, bool silent)
{
	ObjectManager *om = ObjectManager::the();
	bool result;

	result=om->addGlobalReference(server,"Arts_SoundServerV2")
	    && om->addGlobalReference(server,"Arts_SoundServer")
	    && om->addGlobalReference(server,"Arts_SimpleSoundServer")
        && om->addGlobalReference(server,"Arts_PlayObjectFactory")
        && om->addGlobalReference(audioManager,"Arts_AudioManager");
	
	if(!result && !silent)
	{
		cerr <<
"Error: Can't add object reference (probably artsd is already running)."
              << endl <<
"       If you are sure it is not already running, remove the relevant files:"
              << endl << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SoundServerV2") << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SoundServer") << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SimpleSoundServer") << endl <<
"       "<< MCOPUtils::createFilePath("Arts_PlayObjectFactory") << endl <<
"       "<< MCOPUtils::createFilePath("Arts_AudioManager") << endl << endl;
	}
	return result;
}

static int cleanReference(const string& reference)
{
	Object test;
	test = Reference("global:"+reference);
	if(test.isNull())
	{
		Dispatcher::the()->globalComm().erase(reference);
		return 1;
	}
	else
		return 0;
}

static void cleanUnusedReferences()
{
	int i = 0;

	cerr << "There are already artsd objects registered, "
			"looking if they are active..." << endl;

	sleep(1); // maybe an artsd process has just started (give it some time)

	i += cleanReference("Arts_SoundServerV2");
	i += cleanReference("Arts_SoundServer");
	i += cleanReference("Arts_SimpleSoundServer");
	i += cleanReference("Arts_PlayObjectFactory");
	i += cleanReference("Arts_AudioManager");

	if(i)
		cerr << "... cleaned " <<i<< " unused mcop global references." << endl;
	cerr << endl;
}

int main(int argc, char **argv)
{
	handleArgs(argc, argv);

	Debug::init("[artsd]", static_cast<Debug::Level>(cfgDebugLevel));

	if (cfgDebugApp)
		Debug::messageApp(cfgDebugApp);

	if(cfgPort)			 TCPServer::setPort(cfgPort);

	CPUUsage	cpuUsage;
	Dispatcher	dispatcher(0,cfgServers);

	initSignals();

	/* execute commands, if any */
	if(cmdListAudioIO)	 exitListAudioIO();

	/* apply configuration */
	if(cfgAudioIO)       AudioSubSystem::the()->audioIO(cfgAudioIO);
	if(cfgSamplingRate)  AudioSubSystem::the()->samplingRate(cfgSamplingRate);
	if(cfgFragmentCount) AudioSubSystem::the()->fragmentCount(cfgFragmentCount);
	if(cfgFragmentSize)  AudioSubSystem::the()->fragmentSize(cfgFragmentSize);
	if(cfgFullDuplex)	 AudioSubSystem::the()->fullDuplex(cfgFullDuplex);
	if(cfgDeviceName)	 AudioSubSystem::the()->deviceName(cfgDeviceName);
	if(cfgBits)			 AudioSubSystem::the()->format(cfgBits);

	if(cfgForceStart && !AudioSubSystem::the()->check())
	{
		/* hack to get this message to the user in any case */
		Debug::init("[artsd]", static_cast<Debug::Level>(1));
		arts_info(
			"Error while initializing the sound driver:\n"
			"%s\n\n"
			"The sound server will continue, using the null output device.",
			AudioSubSystem::the()->error());
		Debug::init("[artsd]", static_cast<Debug::Level>(cfgDebugLevel));
		AudioSubSystem::the()->audioIO("null");
	}

	if(!AudioSubSystem::the()->check())
	{
		string msg = "Error while initializing the sound driver:\n";
		msg += AudioSubSystem::the()->error();
		arts_fatal(msg.c_str());
		exit(1);
	}

	/* start sound server implementation */
	SoundServerV2 server;
	AudioManager audioManager;

	if (cfgAutoSuspend)
		server.autoSuspendSeconds(cfgAutoSuspend);

	if (cfgBuffers)
		server.bufferSizeMultiplier(cfgBuffers);

	/* make global MCOP references available */
	if(!publishReferences(server,audioManager,true))
	{
		cleanUnusedReferences();
		if(!publishReferences(server,audioManager,false)) return 1;
	}

	/* warn if there was a problem with artswrapper */
	char *wrapper = getenv("STARTED_THROUGH_ARTSWRAPPER");
	if (wrapper && !strcmp(wrapper, "2"))
		arts_warning(
			"Can't set real-time scheduling priority.\n"
			"You need to run artswrapper as root or\n"
			"setuid root. This means that you will\n"
			"likely not be able to produce acceptable\n"
			"sound (i.e. without clicks and breaks).");

	if (wrapper && !strcmp(wrapper, "3"))
		arts_warning(
			"This system has no support for real-time\n"
			"scheduling priority. This means that you\n"
			"will likely not be able to produce acceptable\n"
			"sound (i.e. without clicks and breaks).");

	dispatcher.run();
	return 0;
}

#ifdef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
#include "simplesoundserver_impl.h"
REGISTER_IMPLEMENTATION(SimpleSoundServer_impl);
#include "soundserver_impl.h"
REGISTER_IMPLEMENTATION(SoundServer_impl);
#include "soundserverv2_impl.h"
REGISTER_IMPLEMENTATION(SoundServerV2_impl);
#endif
