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

#include "startupmanager.h"
#include "extensionloader.h"
#include <iostream>

using namespace std;

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

		startupClasses->erase(startupClasses->begin(),startupClasses->end());

		delete startupClasses;
		startupClasses = 0;
	}
}
