    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "core.h"
#include "debug.h"
#include <stdio.h>

using namespace Arts;
using namespace std;

bool haveProperty(TraderOffer& offer, string property)
{
	vector<string>* plist = offer.getProperty(property);
	bool result = !plist->empty();
	delete plist;

	return result;
}

string getSingleProperty(TraderOffer& offer, string property)
{
	string result="";

	vector<string>* plist = offer.getProperty(property);
	if(!plist->empty())
		result = plist->front();
	delete plist;

	return result;
}

int main()
{
	Dispatcher dispatcher;
	InterfaceRepoV2 ir = DynamicCast(dispatcher.interfaceRepo());

	/* prevent the screen from being filled with aRts warnings */
	Debug::init("", Debug::lFatal);

	TraderQuery everything;		/* a query without any restriction */
	vector<TraderOffer> *allObjects = everything.query();
	vector<TraderOffer>::iterator i;

	for(i = allObjects->begin(); i != allObjects->end(); i++)
	{
		TraderOffer& offer = *i;
		if(haveProperty(offer,"Type"))
		{
			// check type file consistency
		}
		else
		{
			// check class file consistency
			InterfaceDef idef = ir.queryInterface(offer.interfaceName());
			if(idef.name == "")
			{
				printf("warning: %s interface type not found\n",
								offer.interfaceName().c_str());
									
			}
		}
	}
	delete allObjects;
	return 0;
}
