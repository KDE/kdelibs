    /*

    Copyright (C) 1999 Stefan Westerfeld stefan@space.twc.de
                       Nicolas Brodu, brodu@kde.org

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

#include "hello.h"
#include <iostream>

using namespace std;
using namespace Arts;

int main()
{
	Dispatcher dispatcher;
	
	// Just use plain C++!
	Hello server;
	server.hello("server");

	// creation from MCOP reference
	Hello client( Reference( server.toString() ) );
	if (!client.isNull()) client.hello("local test");
	else cout << "fatal: a valid reference can't be decoded" << endl;

	// creation from wrong reference
	Hello xclient( Reference("fdshjkhdsf") );

	// check result => should be null
	if (xclient.isNull()) cout << "passed invalid reference test" << endl;
	else cout << "fatal: an invalid reference can be decoded" << endl;
	
	// using a constructor	
	Hello hint(3);
	cout << hint.myValue() << endl;


	Hello castsrc;
	HelloBase casthint = castsrc;
	casthint.hellobase("cast test passed");
	if(castsrc.isNull()) cout << "cast problem with auto creation" << endl;
	castsrc.hello("second cast test passed");

	// Can copy (or here copy constructor) the objects
	Hello hcopy = hint;
	hcopy.add(5);
	hcopy.printTotal();

	// dynamic creation is OK
	Hello* dyn = new Hello;
	cout << dyn->concat("I am"," a dynamic hello") << endl;
	dyn->hellobase("dynamic hellobase hello");
	delete dyn;

//	dispatcher.run();
	return 0;
	
}
