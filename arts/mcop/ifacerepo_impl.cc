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
}

long InterfaceRepo_impl::insertModule(const ModuleDef& newModule)
{
	long moduleID = nextModuleID++;

	vector<InterfaceDef>::const_iterator ii;
	for(ii=newModule.interfaces.begin();
		ii != newModule.interfaces.end();ii++)
	{
		Buffer b;
		ii->writeType(b);
		interfaces.push_back(new InterfaceEntry(b,moduleID));
	}

	vector<TypeDef>::const_iterator ti;
	for(ti=newModule.types.begin();
		ti != newModule.types.end();ti++)
	{
		Buffer b;
		ti->writeType(b);
		types.push_back(new TypeEntry(b,moduleID));
	}

	return moduleID;
}

void InterfaceRepo_impl::removeModule(long moduleID)
{
	list<TypeEntry *>::iterator ti;

	/* erase types */
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
}

InterfaceDef InterfaceRepo_impl::queryInterfaceLocal(const string& name)
{
	list<InterfaceEntry *>::iterator ii;

	for(ii = interfaces.begin();ii != interfaces.end();ii++)
	{
		if((*ii)->name == name)
		{
			Buffer b;
			(*ii)->writeType(b);
			return InterfaceDef(b);
		}
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
					string filename = string(EXTENSION_DIR)
					                + "/" + types->front();

					arts_debug("InterfaceRepo: loading %s", filename.c_str());

					FILE *extfile = fopen(filename.c_str(),"r");
					Buffer b;
					int c;
					while((c = fgetc(extfile)) >= 0) b.writeByte(c);
					fclose(extfile);

					insertModule(ModuleDef(b));
					def = queryInterfaceLocal(name);
					//removeModule(id);
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
		{
			Buffer b;
			(*ti)->writeType(b);
			return TypeDef(b);
		}
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
		{
			Buffer b;
			(*ei)->writeType(b);
			return EnumDef(b);
		}
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
