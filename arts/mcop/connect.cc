/*
	Copyright (C) 2000 Nicolas Brodu
	nicolas.brodu@free.fr

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

*/
#include "component.h"
#include "connect.h"
#include "flowsystem.h"
//#include <cassert>

//namespace MCOP {

// Connect function overloaded for components with default port
void connect(Component* src, const std::string& output, Component* dest, const std::string& input)
{
	assert(src); assert(dest);
	src->node()->connect(output, dest->node(), input);
}

void connect(Component* src, const string& output, Component* dest)
{
	assert(src); assert(dest);
	vector<std::string> portsIn = dest->defaultPortsIn();
	assert(portsIn.size() == 1);
	src->node()->connect(output, dest->node(), portsIn[0]);
}

void connect(Component* src, Component* dest, const string& input)
{
	assert(src); assert(dest);
	vector<std::string> portsOut = src->defaultPortsOut();
	assert(portsOut.size() == 1);
	src->node()->connect(portsOut[0], dest->node(), input);
}

void connect(Component* src, Component* dest)
{
	assert(src); assert(dest);
	vector<std::string> portsOut = src->defaultPortsOut();
	vector<std::string> portsIn = dest->defaultPortsIn();
	assert(portsOut.size() == portsIn.size());
	assert(!portsOut.empty());
	vector<std::string>::iterator ot = portsOut.begin();
	vector<std::string>::iterator it = portsIn.begin();
	for (; ot != portsOut.end(); it++, ot++) {
		src->node()->connect(*ot, dest->node(), *it);
	}
}


// setValue function overloaded for components with default port
void setValue(Component* c, const std::string& port, const float fvalue)
{
	assert(c);
	c->node()->setFloatValue(port, fvalue);
}

void setValue(Component* c, const float fvalue)
{
	assert(c);
	vector<std::string> portsIn = c->defaultPortsIn();
	assert(!portsIn.empty());
	vector<std::string>::iterator it = portsIn.begin();
	for (; it != portsIn.end(); it++) {
		c->node()->setFloatValue(*it, fvalue);
	}
}

/* String values???
void setValue(Component* c, const string& port, const string& svalue)
{
}

void setValue(Component* c, const string& svalue)
{
}
*/

//}
