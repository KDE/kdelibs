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

#include <config.h>
#include "mcoputils.h"
#include "mcopconfig.h"
#include "objectmanager.h"
#include "dispatcher.h"
#include "extensionloader.h"
#include "debug.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace Arts;

class Arts::ObjectManagerPrivate {
public:
	list<ExtensionLoader *> extensions;
};

Object_skel *ObjectManager::create(string name)
{
	list<Factory *>::iterator i;

	/* first try: look through all factories we have */

	for(i = factories.begin();i != factories.end(); i++)
	{
		Factory *f = *i;
		if(f->interfaceName() == name ) return f->createInstance();
	}

	/* second try: look if there is a suitable extension we could load */

	// TODO: modularize language loading here
	//    => use trader instead of handmade access to the .mcopclass file

	string mcopclassName;
	string::iterator nameit = name.begin();
	while(nameit != name.end())
	{
		if(*nameit == ':')	// map the :: of namespaces to subdirectories
		{
			nameit++;
			if(nameit != name.end() && *nameit == ':')
			{
				nameit++;
				mcopclassName += '/';
			}
		}
		else				// copy all other stuff
		{
			mcopclassName += *nameit++;
		}
	}
	mcopclassName += ".mcopclass";

	MCOPConfig config(string(EXTENSION_DIR) + "/" + mcopclassName);
	string library = config.readEntry("Library");
	if(library != "")
	{
		ExtensionLoader *e = new ExtensionLoader(library);
		if(e->success())
		{
			d->extensions.push_back(e);
			for(i = factories.begin();i != factories.end(); i++)
			{
				Factory *f = *i;
//				cerr << "Found interfaceName: " << f->interfaceName() << endl;
				if(f->interfaceName() == name) return f->createInstance();
			}
		}
		else {
			arts_warning("MCOP ObjectManager: Could not load extension %s.",
							library.c_str());
			delete e;
		}
	}
	arts_warning("MCOP ObjectManager: can't find implementation for %s.",
							name.c_str());
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
	d = new ObjectManagerPrivate;
}

ObjectManager::~ObjectManager()
{
	// they should be unloaded before this object can be deleted
	assert(d->extensions.empty());
	delete d;
	assert(_instance);
	_instance = 0;
}

ObjectManager *ObjectManager::the()
{
	assert(_instance);
	return _instance;
}

void ObjectManager::removeExtensions()
{
	// unloads all dynamically loaded extensions
	list<ExtensionLoader *>::iterator i;
	for(i=d->extensions.begin(); i != d->extensions.end(); i++)
		delete *i;
	
	d->extensions.clear();
}

/*
 * global references
 */

bool ObjectManager::addGlobalReference(Object object, string name)
{
	bool result;

	result = Dispatcher::the()->globalComm().put(name,object.toString());
	if(result)
		referenceNames.push_back(name);

	return result;
}

string ObjectManager::getGlobalReference(string name)
{
	return Dispatcher::the()->globalComm().get(name);
}

void ObjectManager::removeGlobalReferences()
{
	list<string>::iterator i;

	for(i=referenceNames.begin(); i != referenceNames.end();i++)
		Dispatcher::the()->globalComm().erase(*i);
}
