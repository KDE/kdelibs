/*
    Copyright (C) 2000 Nicolas Brodu
    nicolas.brodu@free.fr

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

#include "connect.h"
#include "flowsystem.h"
#include <assert.h>
#include "debug.h"

// NDEBUG is the symbol name that can remove the assertions
#ifdef NDEBUG
#define mywarning(a,b)
#else
#include <iostream.h>
#define mywarning(a,b) if (!(a)) arts_warning("MCOP connection warning: %s",b)
#endif

using namespace std;

namespace Arts {

// Connect function overloaded for objects with default port
void connect(const Object& src, const std::string& output, const Object& dest, const std::string& input)
{
	ScheduleNode *node = src._node();
	assert(node);
	node->connect(output, dest._node(), input);
}

void connect(const Object& src, const string& output, const Object& dest)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsIn = dest._defaultPortsIn();
	mywarning(portsIn.size() == 1, "default input not found in void connect(const Object& src, const string& output, const Object& dest);");
	node->connect(output, dest._node(), portsIn[0]);
}

void connect(const Object& src, const Object& dest, const string& input)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsOut = src._defaultPortsOut();
	mywarning(portsOut.size() == 1, "default output not found in void connect(const Object& src, const Object& dest, const string& input);");
	node->connect(portsOut[0], dest._node(), input);
}

void connect(const Object& src, const Object& dest)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsOut = src._defaultPortsOut();
	vector<std::string> portsIn = dest._defaultPortsIn();
	mywarning(portsOut.size() == portsIn.size(), "number of defaults for src and dest don't match in void connect(const Object& src, const Object& dest);");
	mywarning(!portsOut.empty(), "no defaults for src and dest in void connect(const Object& src, const Object& dest);");
	vector<std::string>::iterator ot = portsOut.begin();
	vector<std::string>::iterator it = portsIn.begin();
	for (; ot != portsOut.end(); it++, ot++) {
		node->connect(*ot, dest._node(), *it);
	}
}

void disconnect(const Object& src, const std::string& output, const Object& dest, const std::string& input)
{
	ScheduleNode *node = src._node();
	assert(node);
	node->disconnect(output, dest._node(), input);
}

void disconnect(const Object& src, const string& output, const Object& dest)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsIn = dest._defaultPortsIn();
	mywarning(portsIn.size() == 1, "default input not found in void disconnect(const Object& src, const string& output, const Object& dest);");
	node->disconnect(output, dest._node(), portsIn[0]);
}

void disconnect(const Object& src, const Object& dest, const string& input)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsOut = src._defaultPortsOut();
	mywarning(portsOut.size() == 1, "default output not found in void disconnect(const Object& src, const Object& dest, const string& input);");
	node->disconnect(portsOut[0], dest._node(), input);
}

void disconnect(const Object& src, const Object& dest)
{
	ScheduleNode *node = src._node();
	assert(node);
	vector<std::string> portsOut = src._defaultPortsOut();
	vector<std::string> portsIn = dest._defaultPortsIn();
	mywarning(portsOut.size() == portsIn.size(), "number of defaults for src and dest don't match in void disconnect(const Object& src, const Object& dest);");
	mywarning(!portsOut.empty(), "no defaults for src and dest in void disconnect(const Object& src, const Object& dest);");
	vector<std::string>::iterator ot = portsOut.begin();
	vector<std::string>::iterator it = portsIn.begin();
	for (; ot != portsOut.end(); it++, ot++) {
		node->disconnect(*ot, dest._node(), *it);
	}
}

// setValue function overloaded for objects with default port
void setValue(const Object& c, const std::string& port, const float fvalue)
{
	ScheduleNode *node = c._node();
	assert(node);
	node->setFloatValue(port, fvalue);
}

void setValue(const Object& c, const float fvalue)
{
	ScheduleNode *node = c._node();
	assert(node);
	vector<std::string> portsIn = c._defaultPortsIn();
	mywarning(!portsIn.empty(), "c has no default ports in void setValue(const Object& c, const float fvalue);");
	vector<std::string>::iterator it = portsIn.begin();
	for (; it != portsIn.end(); it++) {
		node->setFloatValue(*it, fvalue);
	}
}

/* String values???
void setValue(Object& c, const string& port, const string& svalue)
{
}

void setValue(Object& c, const string& svalue)
{
}
*/

};
