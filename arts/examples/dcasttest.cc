    /*

    Copyright (C) 2000 Stefan Westerfeld stefan@space.twc.de

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

#include <stdio.h>
#include <soundserver.h>
#include <iostream>

using namespace std;
using namespace Arts;

/*
 * This is a little test program that illustrates remote dynamic casting.
 * Arts_SimpleSoundServer is a global object that is a SimpleSoundServer,
 * so the first dynamic cast should succeed, while it is no Synth_PLAY,
 * so the second dynamic cast should fail.
 */

int main()
{
	Dispatcher dispatcher;
	Object obj = Reference("global:Arts_SimpleSoundServer");

	if(obj.isNull())
	{
		cerr << "this test will only work if artsd is running" << endl;
		exit(1);
	}

	/*
	 * TODO: fix this. This fails currently as in the first line only an
	 * Object_stub will get created, no SimpleSoundServer_stub, and _cast
	 * which is used by DynamicCast wil only try to cast the _stub object,
	 * not do any further tricks.
	 */
	SimpleSoundServer server = DynamicCast(obj);
	if(server.isNull())
	{
		cerr << "remote dynamic casting is too restrictive" << endl;
		exit(1);
	}

	Synth_PLAY play = DynamicCast(obj);
	if(!play.isNull())
	{
		cerr << "remote dynamic casting doesn't do proper checks" << endl;
		exit(1);
	}

	cout << "remote dynamic casting seems to be fine" << endl;
	return 0;
}
