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

#include "ifacerepo_impl.h"
#include "debug.h"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace Arts;

InterfaceRepo_impl::InterfaceRepo_impl()
{
	nextModuleID = 1;
	tiMap["void"] = tiVoid;
	tiMap["byte"] = tiByte;
	tiMap["string"] = tiString;
	tiMap["boolean"] = tiBoolean;
	tiMap["float"] = tiFloat;
	tiMap["long"] = tiLong;
	tiMap["object"] = tiInterface;
}

InterfaceRepo_impl::~InterfaceRepo_impl()
{
	while(!unloadModuleList.empty())
	{
		removeModule(unloadModuleList.front());
		unloadModuleList.pop_front();
	}
}

long InterfaceRepo_impl::insertModule(const ModuleDef& newModule)
{
	long moduleID = nextModuleID++;

	/* insert interfaces */
	vector<InterfaceDef>::const_iterator ii;
	for(ii=newModule.interfaces.begin();
		ii != newModule.interfaces.end();ii++)
	{
		Buffer b;
		ii->writeType(b);
		InterfaceEntry *ie = new InterfaceEntry(b,moduleID);
		interfaces.push_back(ie);

		tiMap[ie->name] = tiInterface;
	}

	/* insert types */
	vector<TypeDef>::const_iterator ti;
	for(ti=newModule.types.begin();
		ti != newModule.types.end();ti++)
	{
		Buffer b;
		ti->writeType(b);
		TypeEntry *entry = new TypeEntry(b,moduleID);
		types.push_back(entry);

		tiMap[entry->name] = tiType;
	}

	/* insert enums */
	vector<EnumDef>::const_iterator ei;
	for(ei=newModule.enums.begin();
		ei != newModule.enums.end();ei++)
	{
		Buffer b;
		ei->writeType(b);
		EnumEntry *entry = new EnumEntry(b,moduleID);
		enums.push_back(entry);

		tiMap[entry->name] = tiEnum;
	}

	return moduleID;
}

void InterfaceRepo_impl::removeModule(long moduleID)
{
	/* erase interfaces */
	list<InterfaceEntry *>::iterator ii;
	ii = interfaces.begin();
	while(ii != interfaces.end())
	{
		if((*ii)->moduleID == moduleID)
		{
			delete (*ii);
			interfaces.erase(ii);
			ii = interfaces.begin();
		}
		else ii++;
	}

	/* erase types */
	list<TypeEntry *>::iterator ti;
	ti = types.begin();
	while(ti != types.end())
	{
		if((*ti)->moduleID == moduleID)
		{
			delete (*ti);
			types.erase(ti);
			ti = types.begin();
		}
		else ti++;
	}

	/* erase enums */
	list<EnumEntry *>::iterator ei;
	ei = enums.begin();
	while(ei != enums.end())
	{
		if((*ei)->moduleID == moduleID)
		{
			delete (*ei);
			enums.erase(ei);
			ei = enums.begin();
		}
		else ei++;
	}
}

InterfaceDef InterfaceRepo_impl::queryInterfaceLocal(const string& name)
{
	list<InterfaceEntry *>::iterator ii;

	for(ii = interfaces.begin();ii != interfaces.end();ii++)
	{
		if((*ii)->name == name)
			return **ii;
	}
	return InterfaceDef();
}

InterfaceDef InterfaceRepo_impl::queryInterface(const string& name)
{
	InterfaceDef def = queryInterfaceLocal(name);

	if(def.name == "")
	{
		TraderQuery q;
		q.supports("Type",name);
		vector<TraderOffer> *offers = q.query();
		vector<TraderOffer>::iterator i;
		for(i = offers->begin(); i != offers->end();i++)
		{
			TraderOffer& offer = *i;

			if(def.name == "")
			{
				vector<string> *types = offer.getProperty("TypeFile");
				if(types->size() == 1)
				{
					const vector<string> *path = MCOPUtils::traderPath();

					vector<string>::const_iterator pi = path->begin();
					while(pi != path->end() && def.name == "")
					{
						string filename = *pi++ + "/" + types->front();

						FILE *extfile = fopen(filename.c_str(),"r");
						if(extfile)
						{
							arts_debug("InterfaceRepo: loading %s",
								filename.c_str());

							Buffer b;
							int c;
							while((c = fgetc(extfile)) >= 0) b.writeByte(c);
							fclose(extfile);

							long id = insertModule(ModuleDef(b));
							def = queryInterfaceLocal(name);
							unloadModuleList.push_back(id);
						}
					}
				}
				delete types;
			}
		}
		delete offers;
	}

	if(def.name == "")
	{
		arts_warning("InterfaceRepo: no information about the interface %s "
					 "is known", name.c_str());
	}
		 
	return def;
}

TypeDef InterfaceRepo_impl::queryType(const string& name)
{
	list<TypeEntry *>::iterator ti;

	for(ti = types.begin();ti != types.end();ti++)
	{
		if((*ti)->name == name)
			return **ti;
	}

	arts_warning("InterfaceRepo: no information about the type %s is known.",
		name.c_str());
	return TypeDef();
}

EnumDef InterfaceRepo_impl::queryEnum(const string& name)
{
	list<EnumEntry *>::iterator ei;

	for(ei = enums.begin();ei != enums.end();ei++)
	{
		if((*ei)->name == name)
			return **ei;
	}

	arts_warning("InterfaceRepo: no information about the enum %s is known.",
		name.c_str());
	return EnumDef();
}

vector<string> *InterfaceRepo_impl::queryChildren(const std::string& name)
{
	vector<string> *result = new vector<string>;
	list<InterfaceEntry *>::iterator ii;

	for(ii = interfaces.begin();ii != interfaces.end();ii++)
	{
		bool found = false;
		vector<string>::iterator j;

		for(j = (*ii)->inheritedInterfaces.begin();
		       j != (*ii)->inheritedInterfaces.end() && !found; j++)
		{
			if(*j == name)
			{
				result->push_back((*ii)->name);
				found = true;
			}
		}
		if((*ii)->inheritedInterfaces.empty() && ((name == "Arts::Object") || (name == "object")) && ((*ii)->name != "Arts::Object"))
			result->push_back((*ii)->name);
	}

	return result;
}

vector<string> *InterfaceRepo_impl::queryInterfaces()
{
	vector<string> *result = new vector<string>;
	list<InterfaceEntry *>::iterator ii;

	for(ii = interfaces.begin();ii != interfaces.end();ii++)
		result->push_back((*ii)->name);

	return result;
}

vector<string> *InterfaceRepo_impl::queryTypes()
{
	vector<string> *result = new vector<string>;
	list<TypeEntry *>::iterator ti;

	for(ti = types.begin();ti != types.end();ti++)
		result->push_back((*ti)->name);

	return result;
}

vector<string> *InterfaceRepo_impl::queryEnums()
{
	vector<string> *result = new vector<string>;
	list<EnumEntry *>::iterator ei;

	for(ei = enums.begin();ei != enums.end();ei++)
		result->push_back((*ei)->name);

	return result;
}

TypeIdentification InterfaceRepo_impl::identifyType(const string& name)
{
	return tiMap[name];
}
