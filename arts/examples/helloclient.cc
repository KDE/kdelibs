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

#include <stdio.h>
#include <stdlib.h>

using namespace Arts;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr,"usage: %s <mcop reference>\n",argv[0]);
		exit(1);
	}

	Dispatcher dispatcher;

	Hello h = Reference(argv[1]);
	if(h.isNull())
	{
		fprintf(stderr,"can't connect to the object\n");
		exit(1);
	}

	const char *who = getenv("LOGNAME");
	if(!who) who = "stefan";

	printf("calling h.hello(\"%s\")\n", who);
	h.hello(who);
/*
	printf("h->sum(2,4) is %ld\n",
		h->sum2(2,4));

	printf("h->sum(2,4,6) is %ld\n",
		h->sum3(2,4,6));
*/
	h.myValue(6);
	printf("h.myValue(6), h.myValue()  is %ld\n",
		h.myValue());
	
	printf("h.concat(\"MCOP \",\"is great!\") is \"%s\"\n",
		h.concat("MCOP ","is great!").c_str());

	/*
	int i,j = 0;
	for(i=0;i<100000;i++)
		j += h->sum2(2,4);
	printf("%d\n",j);
	*/
	return 0;
}
