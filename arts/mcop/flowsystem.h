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

#ifndef FLOWSYSTEM_H
#define FLOWSYSTEM_H

#include "object.h"
#include "core.h"

class Object_skel;
class Object_stub;
class RemoteScheduleNode;

class ScheduleNode
{
private:
	Object *_nodeObject;
public:
	ScheduleNode(Object *object);
	virtual ~ScheduleNode();

	Object *nodeObject();

	// check if this is a remote schedule node

	virtual RemoteScheduleNode *remoteScheduleNode();

	// other casts
	
	virtual void *cast(const std::string& target);

	// internal interface against Object_skel
	
	virtual void initStream(std::string name, void *ptr, long flags) = 0;

	// interface against node implementation
	
	virtual void requireFlow() = 0;

	// interface to modify the node from outside
	
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void connect(std::string port, ScheduleNode *remoteNode,
			                        std::string remotePort) = 0;
	virtual void disconnect(std::string port, ScheduleNode *remoteNode,
			                            std::string remotePort) = 0;

	// constant values
	virtual void setFloatValue(std::string port, float value) = 0;
};

class RemoteScheduleNode : public ScheduleNode
{
public:
	RemoteScheduleNode(Object_stub *stub);

	RemoteScheduleNode *remoteScheduleNode();

	// internal interface against Object_skel
	
	void initStream(std::string name, void *ptr, long flags);

	// interface against node implementation
	
	void requireFlow();

	// interface to modify the node from outside
	
	void start();
	void stop();
	void connect(std::string port, ScheduleNode *remoteNode,
			                        std::string remotePort);
	void disconnect(std::string port, ScheduleNode *remoteNode,
			                            std::string remotePort);

	// constant values
	void setFloatValue(std::string port, float value);
};

class FlowSystem_impl :virtual public FlowSystem_skel
{
public:
	virtual ScheduleNode *addObject(Object_skel *object) = 0;
	virtual void removeObject(ScheduleNode *node) = 0;
};

#endif
