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

#ifndef _EXECREQUEST_H
#define _EXECREQUEST_H

#include "artsflow.h"

class ExecRequest {
	ArtsServer *_Server;
	std::vector<ModuleDesc *> _Modules;
	long _ServerID,_StructureID,mid;
	std::string _structureName;

public:
	ExecRequest(ArtsServer *Server, long StructureID, const std::string&
																structureName);

	//const Arts::ModuleDescSeq& Modules();		// FIXME: remove me?
	long ServerID();
	long StructureID();
	long MID();		// FIXME: remove me
	const char *structureName();

	bool createModule(ModuleDesc *desc);
	bool localConnectModules();
	bool remoteConnectModules(ExecRequest *remote);
	bool finalizeModules();
   	void startModules();
   	void deleteModules();

	ArtsServer *Server();
};
#endif
