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
#include "debug.h"

using namespace Arts;

//-------------- ScheduleNode -----------------

ScheduleNode::ScheduleNode(Object_base *object) : _nodeObject(object)
{
	//
}

ScheduleNode::~ScheduleNode()
{
	//
}

Object ScheduleNode::nodeObject()
{
	assert(_nodeObject);
	return Object::_from_base(_nodeObject->_copy());
}

RemoteScheduleNode *ScheduleNode::remoteScheduleNode()
{
	return 0;
}

void *ScheduleNode::cast(const std::string&)
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

void RemoteScheduleNode::initStream(std::string, void *, long)
{
	// not for stub objects
	assert(false);
}

void RemoteScheduleNode::requireFlow()
{
	// stub objects can't require flow on this server (they don't live here)
	assert(false);
}

void RemoteScheduleNode::virtualize(std::string, ScheduleNode *, std::string)
{
	// (de)virtualization of ports is only implemented locally
	assert(false);
}

void RemoteScheduleNode::devirtualize(std::string, ScheduleNode *, std::string)
{
	// (de)virtualization of ports is only implemented locally
	assert(false);
}

// interface to modify the node from outside
	
void RemoteScheduleNode::start()
{
	FlowSystem remoteFs = nodeObject()._flowSystem();
	arts_return_if_fail(!remoteFs.isNull());

	remoteFs.startObject(nodeObject());
}

void RemoteScheduleNode::stop()
{
	FlowSystem remoteFs = nodeObject()._flowSystem();
	arts_return_if_fail(!remoteFs.isNull());

	remoteFs.stopObject(nodeObject());
}

void RemoteScheduleNode::connect(std::string port, ScheduleNode *remoteNode,
			                        std::string remotePort)
{
	arts_return_if_fail(remoteNode != 0);

	FlowSystem fs = nodeObject()._flowSystem();
	arts_return_if_fail(!fs.isNull());

	AttributeType flags = fs.queryFlags(nodeObject(),port);
	arts_return_if_fail(flags != 0);

	// connectObject must be called as connectObject([sourcePort], [destPort]);

	if(flags & streamOut)	// if our port is an output port, this order
	{
		fs.connectObject(nodeObject(),port,remoteNode->nodeObject(),remotePort);
	}
	else if(flags & streamIn)
	{
		FlowSystem remoteFs = remoteNode->nodeObject()._flowSystem();
		arts_return_if_fail(!remoteFs.isNull());

		remoteFs.connectObject(remoteNode->nodeObject(),
										remotePort,nodeObject(),port);
	} else {
		assert(false);
	}
}

void RemoteScheduleNode::disconnect(std::string port, ScheduleNode *remoteNode,
			                            std::string remotePort)
{
	arts_return_if_fail(remoteNode != 0);

	FlowSystem fs = nodeObject()._flowSystem();
	arts_return_if_fail(!fs.isNull());

	AttributeType flags = fs.queryFlags(nodeObject(),port);
	arts_return_if_fail(flags != 0);

	// connectObject must be called as connectObject([sourcePort], [destPort]);

	if(flags & streamOut)	// if our port is an output port, this order
	{
		fs.disconnectObject(nodeObject(),port,remoteNode->nodeObject(),remotePort);
	}
	else if(flags & streamIn)
	{
		FlowSystem remoteFs = remoteNode->nodeObject()._flowSystem();
		arts_return_if_fail(!remoteFs.isNull());

		remoteFs.disconnectObject(remoteNode->nodeObject(),
										remotePort,nodeObject(),port);
	} else {
		assert(false);
	}
}

// constant values
void RemoteScheduleNode::setFloatValue(std::string port, float value)
{
	FlowSystem fs = nodeObject()._flowSystem();
	arts_return_if_fail(!fs.isNull());

	fs.setFloatValue(nodeObject(), port, value);
}
