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

    */

#include "core.h"
#include "hello_impl.h"

#include <stdio.h>
#include <vector>
#include <string>

int main()
{
	Dispatcher dispatcher;
	Hello_impl server;

	string reference = server._toString();
	printf("%s\n",reference.c_str());

	Hello *h = Hello::_fromString(reference);
	if(h)
		h->hello("local test");
	else
		printf("Local access to the Hello_impl object failed?\n");

	dispatcher.run();
	return 0;
}
