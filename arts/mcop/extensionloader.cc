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
#include "extensionloader.h"
#include "startupmanager.h"
#include "mcoputils.h"
#include "debug.h"
#include <unistd.h>
#include <assert.h>

using namespace std;
using namespace Arts;

ExtensionLoader::ExtensionLoader(const string& filename) :handle(0)
{
	string dlfilename;

	assert(filename.size());
	if(filename[0] == '/')
		dlfilename = filename;
	else
	{
		const vector<string> *path = MCOPUtils::extensionPath();

		vector<string>::const_iterator pi;
		for(pi = path->begin(); pi != path->end(); pi++)
		{
			dlfilename = *pi + "/" + filename;

			if(access(dlfilename.c_str(),F_OK) == 0)
				break;
		}
	}


	/* this will catch all startup classes here */
	StartupManager::setExtensionLoader(this);

    lt_dlinit();
	handle = lt_dlopen(dlfilename.c_str());

	StartupManager::setExtensionLoader(0);

	if(handle)
	{
		/* now process all startup classes of the loaded extension */
		list<StartupClass *>::iterator i;

		for(i = startupClasses.begin(); i != startupClasses.end(); i++)
			(*i)->startup();
		needShutdown = true;
	}
	else
	{
		arts_warning("loading extension from '%s' failed: %s",
						dlfilename.c_str(), lt_dlerror());
	}
}

bool ExtensionLoader::success()
{
	return (handle != 0);
}

void ExtensionLoader::shutdown()
{
	if(handle && needShutdown)
	{
		/* shutdown the loaded extension properly */
		list<StartupClass *>::iterator i;

		for(i = startupClasses.begin(); i != startupClasses.end(); i++)
			(*i)->shutdown();
		needShutdown = false;
	}
}

ExtensionLoader::~ExtensionLoader()
{
	if(handle)
	{
		shutdown();
		lt_dlclose(handle);

		/*
		 * lt_dlexit & lt_dlinit have a counter, so there is no harm in
		 * calling one lt_dlinit in the constructor and one lt_dlexit here
		 */
		lt_dlexit();
	}
}

void ExtensionLoader::addStartupClass(class StartupClass *sc)
{
	startupClasses.push_back(sc);
}
