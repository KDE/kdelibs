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

#ifndef IFACEREPO_H
#define IFACEREPO_H

#include "core.h"

namespace Arts {

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
		InterfaceEntry(Buffer& stream, long moduleID) :InterfaceDef(stream),
			moduleID(moduleID) { };
	};

	std::list<TypeEntry *> types;
	std::list<InterfaceEntry *> interfaces;

	long nextModuleID;

	InterfaceDef queryInterfaceLocal(const std::string& name);
public:

	InterfaceRepo_impl();

	long insertModule(const ModuleDef& newModule);
	void removeModule(long moduleID);
	InterfaceDef queryInterface(const std::string& name);
	TypeDef queryType(const std::string& name);
};
};
#endif /* IFACEREPO_H */
