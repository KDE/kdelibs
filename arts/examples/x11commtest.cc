    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de
    Modified by Nicolas Brodu, brodu@kde.org

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

#include <iostream>
#include "core.h"

using namespace std;
using namespace Arts;

/*
 * Test program for X11GlobalComm (communication over X11 root window
 * properties), and since X11GlobalComm is an dynamically extension,
 * extension loading is tested as well.
 */
int main(int argc, char **argv)
{
	Dispatcher dispatcher;
	GlobalComm gcomm( SubClass("Arts::X11GlobalComm") );
	
	if (gcomm.isNull()) {
		cerr << "Cannot create a X11GlobalComm object" << endl;
		return 2;
	}

	if(argc == 4)
	{
		if(string(argv[1]) == "put")
		{
			gcomm.put(argv[2],argv[3]);
			return 0;
		}
	}
	if(argc == 3)
	{
		if(string(argv[1]) == "get")
		{
			cout << gcomm.get(argv[2]) << endl;
			return 0;
		}
		if(string(argv[1]) == "erase")
		{
			gcomm.erase(argv[2]);
			return 0;
		}
	}

	cerr << "This is a test for the X11GlobalComm class. Use" << endl << endl
	     << "    " << argv[0] << " put <variable> <value>" << endl
	     << "    " << argv[0] << " get <variable>" << endl
	     << "    " << argv[0] << " erase <variable>" << endl << endl
	     << "to test the communication via X11 RootWindow properties." << endl;
	return 1;
}
