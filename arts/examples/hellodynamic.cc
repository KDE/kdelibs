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

#include "hello.h"
#include "dynamicrequest.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace Arts;

/*
 * Hellodynamic illustrates the usage of the DynamicRequest class to perform
 * dynamic requests. There are two things you can try
 *
 *   hellodynamic
 *
 * which will use a local implementation of a hello object, and
 *
 *   hellodynamic <mcop reference>
 *
 * which will use a remote implementation of a hello object.
 */
int main(int argc, char **argv)
{
	Dispatcher dispatcher;
	Hello h;

	// Check if we should connect to a remote server:
	if(argc == 2)
	{
		h = Reference(argv[1]);
		if(h.isNull())
		{
			fprintf(stderr,"can't connect to the object\n");
			exit(1);
		}
	}

	// find out logname
	const char *who = getenv("LOGNAME");
	if(!who) who = "stefan";

	printf("calling h.hello(\"%s\")\n", who);

	// the dynamic equivalent to h.hello(who)
	DynamicRequest hellorequest(h);

	if(!hellorequest.method("hello").param(who).invoke())
		cout << "dynamic invocation to h.hello(...) failed" << endl;

		/*
		   Note: you could reuse hellorequest here, and indeed, if you
		   call the _same_ method over and over again, you'll get quite
		   some speed gain as the signature is only looked up once.
		 */

	// set a value (no error checking here, I am lazy ;)
	DynamicRequest(h).method("_set_myValue").param(42).invoke();

	// get a value via "normal" interface
	printf("h.myValue() is %ld (after setting it to 42)\n", h.myValue());

	// call h.concat("MCOP ","rules.") dynamically
	string s;
	DynamicRequest(h).method("concat").param("MCOP ").param("rules.").invoke(s);
	cout << "Conclusion of this test: " << s << endl;

	return 0;
}
