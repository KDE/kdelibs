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
#include <stdarg.h>

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

bool findFile(const vector<string> *path, const string& filename)
{
	vector<string>::const_iterator pi;
	for(pi = path->begin(); pi != path->end(); pi++)
	{
		string pfilename = *pi + "/" + filename;

		if(access(pfilename.c_str(),F_OK) == 0)
			return true;
	}
	return false;
}

void collectInterfaces(InterfaceRepoV2 ir, const string& interfaceName, map<string, int>& i)
{
	InterfaceDef idef = ir.queryInterface(interfaceName);

	if(idef.name != "")
	{
		if(i[idef.name] == 1) return;
		i[idef.name]++;
	}
	vector<string>::const_iterator ii;
	for(ii = idef.inheritedInterfaces.begin(); ii != idef.inheritedInterfaces.end(); ii++)
		collectInterfaces(ir, *ii, i);
	collectInterfaces(ir, "Arts::Object", i);
}

bool wroteHeader;
string interfaceName;

void check(bool cond, const char *fmt, ...)
{
	if(cond)
		return;

	if(!wroteHeader)
	{
		wroteHeader = true;
		printf("Trader inconsistency in \'%s\':\n", interfaceName.c_str());

	}
	printf(" * ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	printf("\n");
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

		wroteHeader = false;
		interfaceName = offer.interfaceName();


		if(haveProperty(offer,"Type"))
		{
			// check type file consistency
			check(haveProperty(offer,"TypeFile"),
				"trader entries with a Type entry MUST have a TypeFile entry");

			check(!haveProperty(offer,"Language"),
				"trader entries with a Type entry MUST NOT have a Language entry");
		}
		else if(haveProperty(offer,"Language") || haveProperty(offer,"Library"))
		{
			// check class file consistency
			InterfaceDef idef = ir.queryInterface(offer.interfaceName());
			if(idef.name == "")
			{
				check(false, "interface type not found");
			}
			else
			{
				// verify correctness of the Interface= line
				map<string,int> ifaces;
				collectInterfaces(ir, offer.interfaceName(), ifaces);

				vector<string>* plist = offer.getProperty("Interface");
				vector<string>::iterator pi;
				for(pi = plist->begin(); pi != plist->end(); pi++)
				{
					ifaces[*pi]+=2;
				}
				delete plist;

				map<string,int>::iterator ii;

				for(ii = ifaces.begin(); ii != ifaces.end(); ii++)
				{
					switch(ii->second)
					{
						case 0:
							check(false, "INTERNAL verification error");
							break;

						case 1:
							check(false, "missing interface %s in Interface entry",
															ii->first.c_str());
							break;

						case 2:
							check(false, "given unimplemented(?) interface %s in Interface entry",
															ii->first.c_str());
							break;

						case 3:
							/* the way things should be */
							break;

						default:
							check(false, "given interface %s in Interface entry more than once?",
											ii->first.c_str());
							break;
					}
				}
			}

			if(haveProperty(offer,"Library"))
			{
				check(getSingleProperty(offer,"Language") == "C++",
					"trader entries with a Library entry SHOULD have a Language=C++ entry");

				string library = getSingleProperty(offer,"Library");
				check(findFile(MCOPUtils::extensionPath(), library),
					"Library entry MUST be loadable via extension path");

			}

			check(haveProperty(offer, "Interface"),
				"entries with Language/Library MUST have an Interface entry");
		}
		else
		{
			check(false,"entry MUST have either Language or Type entry");
		}
	}
	delete allObjects;
	return 0;
}
