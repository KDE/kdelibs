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
#include "startupmanager.h"
#include "extensionloader.h"
#include <iostream>
#include <assert.h>

using namespace std;
using namespace Arts;

/*
 * StartupClass implementation
 */

StartupClass::StartupClass()
{
	StartupManager::add(this);
}

void StartupClass::shutdown()
{
	// shutdown is optional, startup is mandatory
}

/*
 * StartupManager implementation
 */

list<StartupClass *> *StartupManager::startupClasses = 0;
ExtensionLoader *StartupManager::activeExtensionLoader = 0;

void StartupManager::add(StartupClass *sc)
{
	if(activeExtensionLoader)
	{
		activeExtensionLoader->addStartupClass(sc);
	}
	else
	{
		if(!startupClasses) startupClasses = new list<StartupClass *>;

		startupClasses->push_back(sc);
	}
}

void StartupManager::setExtensionLoader(ExtensionLoader *extension)
{
	/*
	 * this is not reentrant: you can't load two extensions at the same time,
	 * and it is impossible that an extension loads an extension while being
	 * loaded (nothing forbids you to load another extension from an extension
	 * in a StartupClass or any time later - just don't do it while being
	 * lt_dlopen()d
	 */
	if(activeExtensionLoader)
		assert(extension == 0);
	else
		assert(extension != 0);

	activeExtensionLoader = extension;
}

void StartupManager::startup()
{
	if(startupClasses)
	{
		list<StartupClass *>::iterator i;

		for(i = startupClasses->begin(); i != startupClasses->end(); i++)
			(*i)->startup();
	}
}

void StartupManager::shutdown()
{
	if(startupClasses)
	{
		list<StartupClass *>::iterator i;

		for(i = startupClasses->begin(); i != startupClasses->end(); i++)
			(*i)->shutdown();
	}
}

void StartupManager::internalFreeAll()
{
	if(startupClasses)
	{
		startupClasses->erase(startupClasses->begin(),startupClasses->end());

		delete startupClasses;
		startupClasses = 0;
	}
}

namespace Arts {

static class StartupManagerFree {
public:
	~StartupManagerFree() { StartupManager::internalFreeAll(); }
} The_StartupManagerFree;

};
