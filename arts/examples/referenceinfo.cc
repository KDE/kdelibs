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

#include "core.h"
#include <stdio.h>

using namespace Arts;

int main(int argc, char **argv)
{
	Dispatcher dispatcher;
	ObjectReference r;

	if(argc != 2) 
	{
		fprintf(stderr,"usage: %s <mcop reference>\n",argv[0]);
		exit(1);
	}

	if(!dispatcher.stringToObjectReference(r,argv[1]))
	{
		fprintf(stderr,
			"Can't read this object reference (invalid?)\n");
		exit(1);
	}
	printf("serverID: %s\n",r.serverID.c_str());
	printf("objectID: %ld\n\n",r.objectID);

	printf("URLs:\n");
	std::vector<std::string>::iterator ui;
	for(ui = r.urls.begin(); ui != r.urls.end();ui++)
		printf(" - %s\n",ui->c_str());

	return 0;
}
