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

#ifndef STARTUPMANAGER_H
#define STARTUPMANAGER_H

#include <list>

/*
 * BC - Status (2000-09-30): StartupClass, StartupManager.
 *
 * The class needs to be kept BC, as the startup system bases on inheritance
 * and virtual functions. Do not change.
 *
 * The manager is a collection class without nonstatic data, so no BC issues
 * here.
 */

namespace Arts {

class StartupClass {
public:
	StartupClass();
	virtual void startup() = 0;
	virtual void shutdown();
};

class StartupManager {
	static std::list<StartupClass *> *startupClasses;
	static class ExtensionLoader *activeExtensionLoader;
	static bool running;
public:
	static void add(StartupClass *sc);
	static void startup();
	static void shutdown();

	/**
	 * since extensions will register startup classes in the same global way
	 * all other classes do (with StartupManager::add(<some startup class>)),
	 * the StartupManager supports forwarding all add requests to the
	 * active extension, which will allow synchronizing the lt_dlopen
	 * lt_dlclose with the calls of startup()/shutdown() of all StartupClasses
	 * present in the extension
	 */
	static void setExtensionLoader(class ExtensionLoader *extension);

	/**
	 * to free all resources when the application terminates (internal use only)
	 */
	static void internalFreeAll();
};

};
#endif /* STARTUPMANAGER_H */
