/*
 
        Copyright (C) 2000-2001 Jeff Tranter
                                tranter@kde.org

								Stefan Westerfeld
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
 

------------------------------------------------------------------------

    artsshell - command line access to aRts functions

This is a utility (similar to what was in aRts 0.3) to provide command
line access to aRts functions.

usage:

artsshell [options] <command> [command-options]

Options:

-q   - suppress all output.
-h   - display command usage.
-v   - show version.

Commands:

help, ?

Show brief summary of commands.

version

Display aRts version.

suspend

This suspends the sound server if it is not in use. It can be useful
when running programs that are not artsd-enabled, although in most
cases you should try to use artsdsp first.

Returns zero status if successful or if the sound server was already
suspended, 1 if unable to connect to the sound server (e.g. it is not
running), and 2 if unable to suspend the sound server (e.g. it is
being used by an application). The sound server will attempt to start
again if it receives any requests.

status

Display status information about the server, i.e. is it running, is it
suspended, real-time status, etc.

terminate

Terminate the sound server.

autosuspend <seconds>

Sets the autosuspend time for the specified number of seconds. A value
of zero disables the autosuspend feature.

networkbuffers <n>

When running artsd over a network connection a large buffer size is
desirable to avoid dropouts. This command allows increasing the buffer
size by a factor of <n> from the default.

stereoeffect insert [top|bottom] <module name>
stereoefect remove <id>
stereoeffect list

Inserts or removes a stereo effect into the stereo effect stack. The
list option lists all stereo effects. When inserting, returns an
identifier that can be used for removing it. It can be installed at
the top or the bottom (the default).

e.g. stereoeffect insert bottom Arts::Synth_FREEVERB
     stereoeffect remove 1
     stereoeffect list

midi (future)

When the MIDI manager functionality is all implemented this would be a
good place to implement the commands. Possible options include:

midi list
midi route <from> <to>

execute (future)

Being able to load a module into the sound server from the command
line would be useful. The relevant code could be taken from
artbuilder. Suggested command format:

execute <module.arts>

*/

#include <unistd.h>
#include <iostream>
#include "soundserver.h"
#include "dispatcher.h"
#include "artsversion.h"
#include <stdio.h>

bool quiet = false;
const char *filename = 0;

using namespace std;

// Display command usage and exit
void usage()
{
	cerr <<
"usage: artsshell [options] <command> [command-options]\n\
\n\
Options:\n\
  -q                  - suppress all output\n\
  -h                  - display command usage\n\
  -v                  - show version\n\
  -f <filename>       - execute commands from <filename>\n\
\n\
Commands:\n\
  suspend             - suspend sound server\n\
  status              - display sound server status information\n\
  terminate           - terminate sound server (might confuse/kill apps using it)\n\
  autosuspend <secs>  - set autosuspend time\n\
  networkbuffers <n>  - increase network buffers by a factor of <n>\n\
  volume [<volume>]   - display/set the volume of the soundserver\n\
  stereoeffect insert [top|bottom] <name>  - insert stereo effect\n\
  stereoefect remove <id>  - remove stereo effect\n\
  stereoeffect list   - list available effects\
" << endl;
	exit(0);
}


// Parse command line options
void parseOptions(int argc, char **argv)
{
	int optch;

	if (argc == 0)
		usage();

	while((optch = getopt(argc, argv, "qhvf:")) > 0)
	{
		switch(optch)
		{
		  case 'q': quiet = true;
			  break;
		  case 'v':
		  	  printf("artsshell %s\n", ARTS_VERSION);
			  exit(0);
			  break;
		  case 'h':
			  usage();
			  break;
		  case 'f':
		  	  filename = optarg;
			  break;
		  default:
			  usage();
			  break;
		}
	}
}


// Suspend sound server, if possible
int suspend(Arts::SoundServer server)
{
	switch (server.secondsUntilSuspend())
	{
	  case 0:
		  if (!quiet)
			  cerr << "sound server was already suspended"  << endl;
		  return 0;
		  break;
		  
	  case -1:
		  if (!quiet)
			  cerr << "sound server is busy"  << endl;
		  return 2;
		  break;

	  default:
		  if (server.suspend() == true)
		  {
			  if (!quiet)
				  cerr << "sound server suspended"  << endl;
			  return 0;
		  } else {
			  if (!quiet)
				  cerr << "unable to suspend sound server" << endl;
			  return 2;
		  }
	}
	return 0;
}


// Display server status information
void status(Arts::SoundServerV2 server)
{
	Arts::RealtimeStatus rtStatus = server.realtimeStatus();
	long seconds = server.secondsUntilSuspend();

	cout << "server status: ";
	switch (seconds)
	{
	  case -1:
		  cout << "busy" << endl;
		  break;
	  case 0:
		  cout << "suspended" << endl;
		  break;
	  case -2:
		  cout << "running, autosuspend disabled" << endl;
		  break;
	  default:
		  cout << "running, will suspend in " << seconds << " s" << endl;
	}

	cout << "real-time status: ";
	switch (rtStatus)
	{
	  case Arts::rtRealtime:
		  cout << "real-time" << endl;
		  break;
	  case Arts::rtNoSupport:
		  cout << "no real-time support" << endl;
		  break;
	  case Arts::rtNoWrapper:
		  cout << "not started through real-time wrapper" << endl;
		  break;
	  case Arts::rtNoRealtime:
		  cout << "not real-time" << endl;
		  break;
	  default:
		  cout << "unknown" << endl;
		  break;
	}

	cout << "server buffer time: " << server.serverBufferTime() << " ms" << endl;
	cout << "buffer size multiplier: " << server.bufferSizeMultiplier() << endl;
	cout << "minimum stream buffer time: " << server.minStreamBufferTime() << " ms" << endl;
	cout << "auto suspend time: " << server.autoSuspendSeconds() << " s" << endl;
	cout << "audio method: " << server.audioMethod() << endl;
	cout << "sampling rate: " << server.samplingRate() << endl;
	cout << "channels: " << server.channels() << endl;
	cout << "sample size: " << server.bits() << " bits" << endl;

	if (server.fullDuplex())
		cout << "duplex: full" << endl;
	else
		cout << "duplex: half" << endl;
	cout << "device: " << server.audioDevice() << endl;
	cout << "fragments: " << server.fragments() << endl;
	cout << "fragment size: " << server.fragmentSize() << endl;
}

// terminate the sound server
void terminate(Arts::SoundServer server)
{
	if(server.terminate())
	{
		cout << "sound server terminated" << endl;
		return;
	}
	else
	{
		cout << "there were problems terminating the sound server" << endl;
		return;
	}
}


// set autosuspend time
void autosuspend(Arts::SoundServerV2 server, int secs)
{
	server.autoSuspendSeconds(secs);
}

// set network buffers size
void networkBuffers(Arts::SoundServerV2 server, int n)
{
	if (n > 0)
		server.bufferSizeMultiplier(n);
}

// set the output volume
void setVolume(Arts::SoundServerV2 server, float volume)
{
	server.outVolume().scaleFactor(volume);
}

// return the output volume
float getVolume(Arts::SoundServerV2 server)
{
	return server.outVolume().scaleFactor();
}

// stereoeffect command
void stereoEffect(Arts::SoundServerV2 server, int argc, char **argv)
{
	// stereoeffect list
	if (!strcmp(argv[0], "list"))
	{
		Arts::TraderQuery query;
		query.supports("Interface", "Arts::StereoEffect");
		vector<Arts::TraderOffer> *offers = query.query();
		vector<Arts::TraderOffer>::iterator i;
		for (i = offers->begin(); i != offers->end(); i++)
			cout << i->interfaceName() << endl;
		delete offers;
		return;
	}

	// stereoeffect insert [top|bottom] <module name>
	if (!strcmp(argv[0], "insert"))
	{
		if (argc < 2 || argc > 3)
		{
			cerr << "invalid arguments" << endl;
			return;
		}

		bool bottom = true;
		if (argc == 3)
		{
			if (!strcmp(argv[1], "bottom"))
				bottom = true;
			else if (!strcmp(argv[1], "top"))
				bottom = false;
			else
			{
				cerr << "invalid arguments" << endl;
				return;
			}
		}

		char *name;
		if (argc == 2)
			name = argv[1];
		else
			name = argv[2];

		// first check if the interface exists using the Trader
		Arts::TraderQuery query;
		query.supports("Interface", name);
		vector<Arts::TraderOffer> *offers = query.query();
		if (offers->empty())
		{
			cerr << "no such interface: " << name << endl;
			delete offers;
			return;
		}
		delete offers;

		Arts::Object obj = (server.createObject(name));
		if (obj.isNull())
		{
			cerr << "unable to create: " << name << endl;
			return;
		}
		Arts::StereoEffect effect = Arts::DynamicCast(obj);
		if (effect.isNull())
		{
			cerr << "unable to load effect: " << name << endl;
			return;
		}
		effect.start();
		Arts::StereoEffectStack effectstack = server.outstack();
		long id;
		if (bottom)
			id = effectstack.insertBottom(effect, name);
		else
			id = effectstack.insertTop(effect, name);
		cout << id << endl;
		return;
	}

	// stereoeffect remove <id>
	if (!strcmp(argv[0], "remove"))
	{
		if (argc != 2)
		{
			cerr << "invalid arguments" << endl;
			return;
		}
		Arts::StereoEffectStack effectstack = server.outstack();
		long id = atoi(argv[1]);
		effectstack.remove(id);
		return;
	}

	cerr << "invalid arguments" << endl;
}

void help()
{
	cout << "Commands:\n\
  suspend                  - suspend sound server\n\
  status                   - display sound server status information\n\
  terminate                - terminate sound server\n\
  autosuspend <secs>       - set autosuspend time\n\
  networkbuffers <n>       - increase network buffers by a factor of <n>\n\
  volume [<volume>]        - display/set the volume of the soundserver\n\
  stereoeffect insert [top|bottom] <name>  - insert stereo effect\n\
  stereoefect remove <id>  - remove stereo effect\n\
  stereoeffect list        - list available effects\n\
  version                  - show sound server version\n\
  help, ?                  - show commands\n";
}

void version()
{
	cout << "aRts version " << ARTS_VERSION << endl;
}

int executeCommand(Arts::SoundServerV2 server, int argc, char **argv)
{
	if (!strcmp(argv[0], "help") || !strcmp(argv[0], "?")) {
		help();
		return 0;
	}

	if (!strcmp(argv[0], "version")) {
		version();
		return 0;
	}

	if (!strcmp(argv[0], "suspend")) {
		suspend(server);
		return 0;
	}

	if (!strcmp(argv[0], "status")) {
		status(server);
		return 0;
	}

	if(!strcmp(argv[0], "terminate")) {
		terminate(server);
		return 0;
	}

	if(!strcmp(argv[0], "volume") && (argc == 2)) {
		setVolume(server,atof(argv[1]));
		return 0;
	}
	if(!strcmp(argv[0], "volume") && (argc == 1)) {
		cout << getVolume(server) << endl;;
		return 0;
	}

	if(!strcmp(argv[0], "autosuspend") && (argc == 2)) {
		int secs = atoi(argv[1]);
		autosuspend(server, secs);
		return 0;
	}

	if(!strcmp(argv[0], "networkbuffers") && (argc == 2)) {
		int n = atoi(argv[1]);
		networkBuffers(server, n);
		return 0;
	}

	if(!strcmp(argv[0], "stereoeffect") && (argc >= 2)) {
		stereoEffect(server, argc-1, &argv[1]);
		return 0;
	}

	return -1;
}

int execute(Arts::SoundServerV2 server, const char *filename)
{
	char command[1024];
	FILE *input = stdin;
	bool prompt;

	if(filename)
	{
		input = fopen(filename,"r");
		if(!input)
		{
			printf("can't open file '%s'\n", filename);
			return 1;
		}
	}

	prompt = isatty(fileno(input));

	if(prompt)
	{
		printf("> ");
		fflush(stdout);
	}

	while(fgets(command, 1024, input) != 0)
	{
		char **argv = 0;
		int argc = 0;
		while(char *arg = strtok(argc?0:command, " \t\n"))
		{
			argv = (char **)realloc(argv, sizeof(char *)*(argc+1));
			argv[argc++] = arg;
		}
		if (argv != 0)
		{
			if (executeCommand(server, argc, argv) == -1)
				cerr << "Invalid command, type 'help' for a list of commands." << endl;
			free(argv);
		}

		if(prompt)
		{
			printf("> ");
			fflush(stdout);
		}
	}

	if(prompt)
		printf("\n");
	
	if(input != stdin)
		fclose(input);

	return 0;
}

int main(int argc, char *argv[])
{
	Arts::Dispatcher dispatcher;
	Arts::SoundServerV2 server(Arts::Reference("global:Arts_SoundServer"));

	parseOptions(argc, argv);

	if (server.isNull())
	{
		if (!quiet)
			cerr << "unable to connect to sound server" << endl;
		exit(1);
	}

	if (argc == optind)
		return execute (server, filename);

	if (executeCommand (server, argc-optind, &argv[optind]) == -1)
		usage();

	return 0;
}
