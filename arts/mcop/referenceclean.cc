    /*

    Copyright (C) 2000 Stefan Westerfeld
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

#include "referenceclean.h"

using namespace std;

ReferenceClean::ReferenceClean(Pool<Object_skel>& objectPool)
	:objectPool(objectPool)
{
	Dispatcher::the()->ioManager()->addTimer(5000, this);
}

void ReferenceClean::notifyTime()
{
	list<Object_skel *> items = objectPool.enumerate();
	list<Object_skel *>::iterator i;

	for(i=items.begin();i != items.end();i++) (*i)->_referenceClean();
}

ReferenceClean::~ReferenceClean()
{
	Dispatcher::the()->ioManager()->removeTimer(this);
}
