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

#include "artsflow.h"
#include "execrequest.h"

class ExecutionManager_impl :virtual public ExecutionManager_skel
{
protected:
	long nextID;
	ArtsServer *localServer;

	void lockScheduler();
	void unlockScheduler();

public:
	ExecutionManager_impl();

	StructureDesc* expandStructureDesc(const StructureDesc& structuredesc);
	long createStructure(const StructureDesc& structuredesc);
	bool isExecuting(long id);
	bool freeStructure(long id);
	void forceTermination();
};

REGISTER_IMPLEMENTATION(ExecutionManager_impl);

ExecutionManager_impl::ExecutionManager_impl()
{
	nextID = 1;

	/*
	 * FIXME: which servers to access? should also be handled differently
	 */
	Object_skel *localServer_object =
		ObjectManager::the()->create("ArtsServer");
	assert(localServer_object);

	localServer = (ArtsServer *) localServer_object->_cast("ArtsServer");
	assert(localServer);
}

StructureDesc* ExecutionManager_impl::expandStructureDesc(
									const StructureDesc& structuredesc)
{
	assert(0);
	return 0;
}

long ExecutionManager_impl::createStructure(const StructureDesc& structuredesc)
{
    long currentID = nextID++;

	/*
	 * FIXME: do structure expansion if required
	 */
    bool needExpansion = structuredesc.containsStructures;
	assert(!needExpansion);

	/*
	 * FIXME: insert server selection algorithm here
	 */
	ExecRequest *er = new ExecRequest(localServer,currentID,structuredesc.Name);

	/* normally, do a loop over all servers */

	// first step: create modules
	unsigned long m;
	for(m=0;m<structuredesc.Modules.size();m++)
		er->createModule(structuredesc.Modules[m]);

    // While the modifications to the flow system are made, no scheduling
	// should be done!
	lockScheduler();

	// second step: establish local connections between the modules
	er->localConnectModules();

	/*
	 * FIXME: insert remote connection setup here
	 */

	// session data restauration here

	// finalize the modules (does the last initialization before start)
	er->finalizeModules();

	// finally start the modules
	er->startModules();

	unlockScheduler();

	return currentID;
}

bool ExecutionManager_impl::isExecuting(long id)
{
}

bool ExecutionManager_impl::freeStructure(long id)
{
}

void ExecutionManager_impl::forceTermination()
{
}

void ExecutionManager_impl::lockScheduler()
{
	// TODO
}

void ExecutionManager_impl::unlockScheduler()
{
	// TODO
}
