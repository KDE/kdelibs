    /*

    Copyright (C) 1999 Stefan Westerfeld
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

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace Arts;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		cerr << "usage: " << argv[0] << " <filename>" << endl;
		return 1;
	}

	Dispatcher dispatcher;
	SimpleSoundServer server;
	server = Arts::Reference("global:Arts_SimpleSoundServer");

	if(server.isNull())
	{
		cerr << "Can't connect to sound server" << endl;
		return 1;
	}
	return server.play(argv[1]) != 0;
}
