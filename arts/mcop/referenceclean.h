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

/*
 * BC - Status (2000-09-30): ReferenceClean
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. Can be changed arbitarily even non BC.
 */

#ifndef REFERENCECLEAN_H
#define REFERENCECLEAN_H

#include "iomanager.h"
#include "dispatcher.h"

namespace Arts {

class ReferenceClean : public TimeNotify {
	Pool<Object_skel>& objectPool;
protected:
	void clean();

public:
	ReferenceClean(Pool<Object_skel>& objectPool);

	/**
	 * this routine forces cleaning of all tagged remote objects
	 *
	 * it will be called on dispatcher shutdown, since after this there
	 * is no remote interaction anyway, it is be used to prevent memory
	 * leaks
	 */
	void forceClean();

	void notifyTime();
	virtual ~ReferenceClean();
};

};
#endif
