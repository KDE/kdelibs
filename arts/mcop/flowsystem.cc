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

#include "flowsystem.h"

//-------------- ScheduleNode -----------------

ScheduleNode::ScheduleNode(Object *object) : _nodeObject(object)
{
	//
}

ScheduleNode::~ScheduleNode()
{
	//
}

Object *ScheduleNode::nodeObject()
{
	return _nodeObject;
}

RemoteScheduleNode *ScheduleNode::remoteScheduleNode()
{
	return 0;
}

void *ScheduleNode::cast(const std::string &target)
{
	return 0;
}


//----------- RemoteScheduleNode -------------

RemoteScheduleNode::RemoteScheduleNode(Object_stub *stub) : ScheduleNode(stub)
{
	//
}

RemoteScheduleNode *RemoteScheduleNode::remoteScheduleNode()
{
	return this;
}

void RemoteScheduleNode::initStream(std::string name, void *ptr, long flags)
{
	// not for stub objects
	assert(false);
}

void RemoteScheduleNode::requireFlow()
{
	// stub objects can't require flow on this server (they don't live here)
	assert(false);
}

// interface to modify the node from outside
	
void RemoteScheduleNode::start()
{
	FlowSystem_var fs = nodeObject()->_flowSystem();
	fs->startObject(nodeObject());
}

void RemoteScheduleNode::stop()
{
	FlowSystem_var fs = nodeObject()->_flowSystem();
	fs->stopObject(nodeObject());
}

void RemoteScheduleNode::connect(std::string port, ScheduleNode *remoteNode,
			                        std::string remotePort)
{
	FlowSystem_var fs = nodeObject()->_flowSystem();
	AttributeType flags = fs->queryFlags(nodeObject(),port);

	// connectObject must be called as connectObject([sourcePort], [destPort]);

	if(flags & streamOut)	// if our port is an output port, this order
	{
		fs->connectObject(nodeObject(),port,remoteNode->nodeObject(),remotePort);
	}
	else if(flags & streamIn)
	{
		FlowSystem_var remoteFs = remoteNode->nodeObject()->_flowSystem();
		remoteFs->connectObject(remoteNode->nodeObject(),
										remotePort,nodeObject(),port);
	} else {
		assert(false);
	}
}

void RemoteScheduleNode::disconnect(std::string port, ScheduleNode *remoteNode,
			                            std::string remotePort)
{
	FlowSystem_var fs = nodeObject()->_flowSystem();
	AttributeType flags = fs->queryFlags(nodeObject(),port);

	// connectObject must be called as connectObject([sourcePort], [destPort]);

	if(flags & streamOut)	// if our port is an output port, this order
	{
		fs->disconnectObject(nodeObject(),port,remoteNode->nodeObject(),remotePort);
	}
	else if(flags & streamIn)
	{
		FlowSystem_var remoteFs = remoteNode->nodeObject()->_flowSystem();
		remoteFs->disconnectObject(remoteNode->nodeObject(),
										remotePort,nodeObject(),port);
	} else {
		assert(false);
	}
}

// constant values
void RemoteScheduleNode::setFloatValue(std::string port, float value)
{
	// currently unsupported; TODO
	assert(false);
}
