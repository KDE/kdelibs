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

    */

#ifndef FLOWSYSTEM_H
#define FLOWSYSTEM_H

#include "object.h"

class Object_skel;

class ScheduleNode
{
public:
	virtual ~ScheduleNode() {};

	// internal interface against Object_skel
	
	virtual void initStream(string name, void *ptr, long flags) = 0;

	// interface against node implementation
	
	virtual void requireFlow() = 0;

	// interface to modify the node from outside
	
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void connect(string port, ScheduleNode *remoteNode,
			                                  string remotePort) = 0;
	virtual void disconnect(string port, ScheduleNode *remoteNode,
			                                  string remotePort) = 0;

	// constant values
	virtual void setFloatValue(string port, float value) = 0;
};

class FlowSystem
{
public:
	virtual ScheduleNode *addObject(Object_skel *object) = 0;
	virtual void removeObject(ScheduleNode *node) = 0;
};

#endif
