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

#include "execrequest.h"

ExecRequest::ExecRequest(ArtsServer *Server, long StructureID,
										const string& structureName)
{
	this->_Server = Server;		// FIXME: duplicate?
	mid = _Server->createModules();
	_ServerID = 42; // FIXME: Server->ID();
	_StructureID = StructureID;
	_structureName = structureName;
}

const char *ExecRequest::structureName()
{
	return _structureName.c_str();
}

ArtsServer *ExecRequest::Server()
{
	return _Server;
}

long ExecRequest::ServerID()
{
	return _ServerID;
}

long ExecRequest::StructureID()
{
	return _StructureID;
}

long ExecRequest::MID()
{
	return mid;
}

/*
const Arts::ModuleDescSeq& ExecRequest::Modules()
{
	return _Modules;
}
*/

// FIXME: eventually implement releaseModules which releases the modules
// sequence as soon as it is no longer needed in the creation process

bool ExecRequest::createModule(ModuleDesc *desc)
{
	_Modules.push_back(desc);	// FIXME: duplicate?

	return(_Server->createModule(mid, *desc));
}

bool ExecRequest::localConnectModules()
{
	return(_Server->localConnectModules(mid));
}

bool ExecRequest::remoteConnectModules(ExecRequest *)
{
/*	FIXME: missing remoteConnectModules
	return(_Server->remoteConnectModules(mid,remote->mid,remote->Modules(),remote->_Server));
*/
	assert(0);
	return false;
}

bool ExecRequest::finalizeModules()
{
	return(_Server->finalizeModules(mid));
}

void ExecRequest::startModules()
{
	_Server->startModules(mid);
}

void ExecRequest::deleteModules()
{
	_Server->deleteModules(mid);
}
