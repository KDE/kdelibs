/*
 
        Copyright (C) 2000 Jeff Tranter
                           tranter@kde.org
 
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

This is the start of a utility (similar to what was in aRts 0.3)
to provide command line access to aRts functions. Currently the
only function provided is to suspend the sound server.

usage:

artsshell [options] <command> [command-options]

Options:

-q   - suppress all output.
-h   - display command usage.

Commands:

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
#include <iostream.h>
#include "soundserver.h"
#include "dispatcher.h"

bool quiet = false;


// Display command usage and exit
void usage()
{
	cerr <<
"usage: artsshell [options] <command> [command-options]\n\
\n\
Options:\n\
  -q   - suppress all output\n\
  -h   - display command usage\n\
\n\
Commands:\n\
  suspend     - suspend sound server\n\
  status      - display sound server status information\n\
  terminate   - terminate sound server (might confuse/kill apps using it)\
" << endl;
	exit(0);
}


// Parse command line options
void parseOptions(int argc, char **argv)
{
	int optch;

	if (argc == 0)
		usage();

	while((optch = getopt(argc, argv, "qh")) > 0)
	{
		switch(optch)
		{
		  case 'q': quiet = true;
			  break;
		  case 'h':
			  usage();
			  break;
		  default:
			  usage();
			  break;
		}
	}                                         

	// should be at least one more argument
	if (optind >= argc)
	{
		usage();
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
		  exit(0);
		  break;
		  
	  case -1:
		  if (!quiet)
			  cerr << "sound server is busy"  << endl;
		  exit(2);
		  break;

	  default:
		  if (server.suspend() == true)
		  {
			  if (!quiet)
				  cerr << "sound server suspended"  << endl;
			  exit(0);
		  } else {
			  if (!quiet)
				  cerr << "unable to suspend sound server" << endl;
			  exit (2);
		  }
	}
	return 0;
}


// Display server status information
void status(Arts::SoundServer server)
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

	cout << "minimum stream buffer time: " << server.minStreamBufferTime() << " ms" << endl;
	cout << "server buffer time: " << server.serverBufferTime() << " ms" << endl;
}           

// terminate the sound server
void terminate(Arts::SoundServer server)
{
	if(server.terminate())
	{
		cout << "sound server terminated" << endl;
		exit(0);
	}
	else
	{
		cout << "there were problems terminating the sound server" << endl;
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	Arts::Dispatcher dispatcher;
	Arts::SoundServer server(Arts::Reference("global:Arts_SoundServer"));

	parseOptions(argc, argv);

	if (server.isNull())
	{
		if (!quiet)
			cerr << "unable to connect to sound server" << endl;
		exit(1);
	}

	if (!strcmp(argv[optind], "suspend")) {
		suspend(server);
		return 0;
	}

	if (!strcmp(argv[optind], "status")) {
		status(server);
		return 0;
	}

	if(!strcmp(argv[optind], "terminate")) {
		terminate(server);
		return 0;
	}
	
	usage();
	return 0;
}
