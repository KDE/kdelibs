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

#include "mcoputils.h"
#include "objectmanager.h"
#include "dispatcher.h"
#include <stdio.h>
#include <unistd.h>

Object_skel *ObjectManager::create(string name)
{
	list<Factory *>::iterator i;

	for(i = factories.begin();i != factories.end(); i++)
	{
		Factory *f = *i;
		if(f->interfaceName() == name) return f->createInstance();
	}
	cerr << "MCOP ObjectManager: warning: can't find implementation for " << name << endl;
	return 0;
}

void ObjectManager::registerFactory(Factory *factory)
{
	factories.push_back(factory);
}

void ObjectManager::removeFactory(Factory *factory)
{
	list<Factory *>::iterator i;

	i = factories.begin();
	while(i != factories.end())
	{
		if(*i == factory) {
			factories.erase(i);
			i = factories.begin();
		}
		else i++;
	}
}

ObjectManager *ObjectManager::_instance = 0;

ObjectManager::ObjectManager()
{
	assert(!_instance);
	_instance = this;
}

ObjectManager::~ObjectManager()
{
	list<string>::iterator i;

	for(i=referenceFiles.begin(); i != referenceFiles.end();i++)
		unlink((*i).c_str());

	assert(_instance);
	_instance = 0;
}

ObjectManager *ObjectManager::the()
{
	assert(_instance);
	return _instance;
}

/*
 * global references
 */

bool ObjectManager::addGlobalReference(Object_skel *object, string name)
{
	string filename = MCOPUtils::createFilePath(name);
	FILE *infile = fopen(filename.c_str(),"r");
	if(infile)
	{
		fclose(infile);
		return false;
	}

	FILE *outfile = fopen(filename.c_str(),"w");
	if(outfile)
	{
		string reference = object->_toString();
		fprintf(outfile,"%s\n",reference.c_str());
		fclose(outfile);

		referenceFiles.push_back(filename);

		return true;
	}
	else
	{
		return false;
	}
}

string ObjectManager::getGlobalReference(string name)
{
	string filename = MCOPUtils::createFilePath(name);
	FILE *infile = fopen(filename.c_str(),"r");
	if(infile)
	{
		char objref[4096];
       if(fgets(objref,4096,infile))
        {
			objref[4095] = 0;
            objref[strlen(objref)-1] = 0;
			return string(objref);
        }                                                                       
	}
	return "";
}
