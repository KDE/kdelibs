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

#include "referenceclean.h"
#include "time.h"

using namespace std;
using namespace Arts;

ReferenceClean::ReferenceClean(Pool<Object_skel>& objectPool)
	:objectPool(objectPool)
{
	Dispatcher::the()->ioManager()->addTimer(5000, this);
}

void ReferenceClean::notifyTime()
{
	/*
	 * This last_notify and now check is because IOManager accumulates
	 * notifyTime() calls, so it may happen that we don't get one for ten
	 * seconds, and then two. However, this breaks the "second-chance"
	 * algorithm referenceClean is using, which depends on the fact that
	 * there is some significant time delay between two calls. So we'll
	 * verify it by hand.
	 */

	static time_t last_notify = 0;
	time_t now;

	time(&now);
	if(now-last_notify > 4)
	{
		clean();
		last_notify = now;
	}
}

void ReferenceClean::forceClean()
{
	/*
	 * as we're giving a second chance on reference clean, we need to
	 * clean twice to really really clean up 
	 */
	clean();
	clean();
}

void ReferenceClean::clean()
{
 	/*
	 * we manually iterate through the pool here, to get around the problem
	 * that if an object disappears, ten other objects might cease to exist
	 * as well (as they are dependant), which would break a list which we
	 * could obtain via enumerate() and would probably lead to a crash
	 */
	unsigned long l;

	for(l=0; l<objectPool.max(); l++)
	{
		Object_skel *skel = objectPool[l]; 
		if(skel) skel->_referenceClean();
	}
}

ReferenceClean::~ReferenceClean()
{
	Dispatcher::the()->ioManager()->removeTimer(this);
}
