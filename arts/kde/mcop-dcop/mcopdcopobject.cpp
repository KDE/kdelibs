/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "mcopdcopobject.h"

class MCOPDCOPObjectPrivate
{
public:
	QCStringList dynamicFunctions;
};

MCOPDCOPObject::MCOPDCOPObject(QCString name) : DCOPObject(name)
{
    d = new MCOPDCOPObjectPrivate();
}

MCOPDCOPObject::~MCOPDCOPObject()
{
    delete d;
}

QCStringList MCOPDCOPObject::functions()
{
	QCStringList returnList = DCOPObject::functions();
	
	QCStringList::iterator it;
	for(it = d->dynamicFunctions.begin(); it != d->dynamicFunctions.end(); ++it)
		returnList.push_back(*it);
	
	return returnList;
}

void MCOPDCOPObject::addDynamicFunction(QCString value)
{
	d->dynamicFunctions.push_back(value);
}
