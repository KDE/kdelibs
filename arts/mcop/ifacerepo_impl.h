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

#ifndef IFACEREPO_H
#define IFACEREPO_H

#include "core.h"

class InterfaceRepo_impl : virtual public InterfaceRepo_skel {
	class TypeEntry : public TypeDef {
	public:
		long moduleID;
		TypeEntry(Buffer& stream, long moduleID) :TypeDef(stream)
		{
			this->moduleID = moduleID;
		}
	};
	class InterfaceEntry : public InterfaceDef {
	public:
		long moduleID;
		InterfaceEntry(Buffer& stream, long moduleID) :InterfaceDef(stream)
		{
			this->moduleID = moduleID;
		}
	};

	list<TypeEntry *> types;
	list<InterfaceEntry *> interfaces;

	long nextModuleID;
public:

	InterfaceRepo_impl();

	long insertModule(const ModuleDef& newModule);
	void removeModule(long moduleID);
	InterfaceDef* queryInterface(const string& name);
	TypeDef* queryType(const string& name);
};

#endif /* IFACEREPO_H */
