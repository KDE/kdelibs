    /*

    Copyright (C) 2000,2001 Stefan Westerfeld
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
	struct LoaderData {
		LoaderData() : init(false) { }
		bool init;
		Loader loader;
	};

	list<ExtensionLoader *> extensions;
	map<string, long> capabilities;
	map<string, LoaderData> loaders;
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

	TraderQuery query;
	query.supports("InterfaceName", name);
	vector<TraderOffer> *offers = query.query();

	vector<TraderOffer>::iterator oi;
	for(oi = offers->begin(); oi != offers->end(); oi++)
	{
		/*
		 * check whether we provide everything that this
		 * implementation requires to run
		 */
		bool requirementsOk = true;
		vector<string> *requires = oi->getProperty("Requires");

		vector<string>::iterator ri;
		for(ri = requires->begin(); ri != requires->end(); ri++)
		{
			if(d->capabilities[*ri] <= 0)
				requirementsOk = false;
		}

		vector<string> *language = oi->getProperty("Language");
		vector<string> *libs = oi->getProperty("Library");

		if(language->size() == 0 && libs->size() == 1)
		{
			arts_warning("ObjectManager: %s - assuming 'Language=C++' because library given",name.c_str());
			language->push_back("C++");
		}
		if(language->size() != 1)
		{
			arts_warning("ObjectManager: %s - Language missing", name.c_str());
			requirementsOk = false;
		}
		if(requirementsOk && language->front() == "C++" && libs->empty())
		{
			arts_warning("ObjectManager: %s - Library missing", name.c_str());
			requirementsOk = false;
		}

		/* loading C++ components */
		if(requirementsOk && language->front() == "C++")
		{
			string library = libs->front();

			ExtensionLoader *e = new ExtensionLoader(library);
			if(e->success())
			{
				d->extensions.push_back(e);
				for(i = factories.begin();i != factories.end(); i++)
				{
					Factory *f = *i;
					if(f->interfaceName() == name)
					{
						delete libs;
						delete offers;
						return f->createInstance();
					}
				}
			}
			else
			{
				arts_warning("MCOP ObjectManager:"
					" Could not load extension %s.", library.c_str());
				delete e;
			}
		}
		delete libs;

		/* other languages/binary formats */
		if(requirementsOk)
		{
			string lang = language->front();
			ObjectManagerPrivate::LoaderData &ld = d->loaders[lang];

			if(!ld.init)
			{
				TraderQuery query;
				query.supports("Interface", "Arts::Loader");
				query.supports("LoadLanguage", lang);

				vector<TraderOffer> *loffers = query.query();
				
				// TODO: error checking for SubClass
				if(loffers->size() > 0)
				{
					ld.loader = SubClass(loffers->front().interfaceName());
				}
				else
				{
					ld.loader = Arts::Loader::null();
				}

				delete loffers;
			}
			ld.init = true;

			if(!ld.loader.isNull())
			{
				/*
				 * ### change when breaking BC:
				 * it's bad that ObjectManager has to return _skel()s, but
				 * well, can't change it now
				 */

				Object obj = ld.loader.loadObject(*oi);
				delete offers;
				return obj._base()->_copy()->_skel();
			}
		}
	}

	arts_warning("MCOP ObjectManager: can't find implementation for %s.",
							name.c_str());

	delete offers;
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

void ObjectManager::shutdownExtensions()
{
	// give up references to the loaders
	d->loaders.clear();

	// shuts down all dynamically loaded extensions
	list<ExtensionLoader *>::iterator i;
	for(i=d->extensions.begin(); i != d->extensions.end(); i++)
		(*i)->shutdown();
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

void ObjectManager::provideCapability(const string& name)
{
	d->capabilities[name]++;
}

void ObjectManager::removeCapability(const string& name)
{
	d->capabilities[name]--;
}
