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

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

using namespace Arts;

static void printInterface(InterfaceDef id)
{
	vector<MethodDef>::iterator mi;
	printf("interface %s {\n",id.name.c_str());
	for(mi = id.methods.begin();mi != id.methods.end(); mi++)
	{
		MethodDef& md = *mi;
		printf("  %s %s(",md.type.c_str(),md.name.c_str());

		bool first = true;
		vector<ParamDef>::iterator pi;
		for(pi = md.signature.begin(); pi != md.signature.end(); pi++)
		{
			ParamDef& pd = *pi;
			if(!first) printf(", ");
			printf("%s %s",pd.type.c_str(),pd.name.c_str());
			first = false;
		}
		printf(");\n");
	}
	printf("}\n");
}

/*
 * This demo shows that you can find out what interface an object has and
 * what types it needs to work without knowing anything but the object
 * reference.
 *
 * The reason for that is that every object offers the _interfaceName
 * _queryInterface and _queryType methods, which you can use to find out
 * anything you need to know to talk to that object.
 *
 * Just pass this programm an MCOP object reference, and it will print out
 * the corresponding interface.
 *
 * (TODO: one could make it print out all inherited interfaces and the
 * types that are used in the interface, too. Ports, etc could be shown
 * as well).
 */

/**
 * Interface repository demo - reasonable testcase:
 *
 *  - make sure artsd is running
 *  - irdemo global:Arts_SimpleSoundServer
 */
int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr,"usage: %s <mcop reference>\n",argv[0]);
		exit(1);
	}

	Dispatcher dispatcher;
	Object o = Reference(argv[1]);
	if(o.isNull())
	{
		fprintf(stderr,"can't connect to the object\n");
		exit(1);
	}

	string iname = o._interfaceName();
	InterfaceDef idef = o._queryInterface(iname);
	printInterface(idef);

	return 0;
}
