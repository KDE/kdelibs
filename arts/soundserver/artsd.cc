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

#include "simplesoundserver_impl.h"
#include "mcoputils.h"
#include <signal.h>

void stopServer(int)
{
	Dispatcher::the()->terminate();
}

void initSignals()
{
    signal(SIGHUP ,stopServer);
    signal(SIGQUIT,stopServer);
    signal(SIGINT ,stopServer);
    signal(SIGTERM,stopServer);                                                 
}

int main()
{
	initSignals();

	Dispatcher dispatcher;
	SimpleSoundServer_var server = new SimpleSoundServer_impl;

	bool result = ObjectManager::the()
				->addGlobalReference(server,"Arts_SimpleSoundServer")
             &&   ObjectManager::the()
				->addGlobalReference(server,"Arts_PlayObjectFactory");
	if(!result)
	{
		cerr <<
"Error: Can't add object reference (perhaps it is already running?)." << endl <<
"       If you are sure it is not already running, remove the relevant file:"
              << endl << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SimpleSoundServer") << endl << endl;
		return 1;
	}

	dispatcher.run();
	return 0;
}
