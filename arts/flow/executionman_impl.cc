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

#include "artsflow.h"
#include "execrequest.h"

using namespace Arts;

namespace Arts {

class ExecutionManager_impl :virtual public ExecutionManager_skel
{
protected:
	long nextID;
	ArtsServer localServer;

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

};

REGISTER_IMPLEMENTATION(ExecutionManager_impl);

ExecutionManager_impl::ExecutionManager_impl()
{
	nextID = 1;

	/*
	 * FIXME: which servers to access? should also be handled differently
	 * than simply using a new localServer always
	 */
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
  // TODO
  return false;
}

bool ExecutionManager_impl::freeStructure(long id)
{
  // TODO
  return false;
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
