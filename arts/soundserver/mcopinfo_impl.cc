    /*

    Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

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

#define protected public
#include "object.h"
#undef protected

#include "mcopinfo_impl.h"
#include "dispatcher.h"
#include "debug.h"

using namespace Arts;

long MCOPInfo_impl::objectCount()
{
	return Object_base::_objectCount();
}

Object MCOPInfo_impl::objectForNumber(long which)
{
	int counter = 0;
	
	if(which >= objectCount())
		return Object::null();
	
	list<Object_skel *> objList = Dispatcher::the()->activeObjectPool().enumerate();
	for(list<Object_skel *>::iterator it = objList.begin(); it != objList.end(); it++)
	{
		if(counter == (int) which)
		{
			Object_base *base = *it;
			base->_copy();
			
			return Object::_from_base(base);
		}
		
		counter++;
	}		
}

#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(MCOPInfo_impl);
#endif
