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
 * BC - Status (2000-10-05): InterfaceRepo_impl
 *
 * This is an implementation class, and not kept binary compatible. It's
 * only here since the REGISTER_IMPLEMENTATION bootstrapping may not yet
 * be done before the first piece of startup code (IDLFileReg) needs the
 * Interface repository.
 */

#ifndef IFACEREPO_H
#define IFACEREPO_H

#include "core.h"
#include <map>

namespace Arts {

class InterfaceRepo_impl : virtual public InterfaceRepoV2_skel {
	class Entry {
	public:
		long moduleID;
		Entry(long moduleID) : moduleID(moduleID)
		{
		}
	};
	class EnumEntry : public EnumDef, public Entry {
	public:
		EnumEntry(Buffer& stream, long moduleID)
				:EnumDef(stream), Entry(moduleID)
		{
		}
	};
	class TypeEntry : public TypeDef, public Entry {
	public:
		TypeEntry(Buffer& stream, long moduleID)
				:TypeDef(stream), Entry(moduleID)
		{
		}
	};
	class InterfaceEntry : public InterfaceDef, public Entry {
	public:
		InterfaceEntry(Buffer& stream, long moduleID)
				:InterfaceDef(stream), Entry(moduleID)
		{
		};
	};

	std::list<EnumEntry *> enums;
	std::list<TypeEntry *> types;
	std::list<InterfaceEntry *> interfaces;
	std::map<std::string, TypeIdentification> tiMap;
	std::list<long> unloadModuleList;

	long nextModuleID;

	InterfaceDef queryInterfaceLocal(const std::string& name);
public:

	InterfaceRepo_impl();
	~InterfaceRepo_impl();

	long insertModule(const ModuleDef& newModule);
	void removeModule(long moduleID);
	InterfaceDef queryInterface(const std::string& name);
	TypeDef queryType(const std::string& name);
	EnumDef queryEnum(const std::string& name);

	TypeIdentification identifyType(const std::string& name);

	std::vector<std::string> *queryChildren(const std::string& name);
	std::vector<std::string> *queryInterfaces();
	std::vector<std::string> *queryTypes();
	std::vector<std::string> *queryEnums();
};
};
#endif /* IFACEREPO_H */
