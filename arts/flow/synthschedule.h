	/*

	Copyright (C) 1998,1999 Stefan Westerfeld
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

#ifndef _SYNTHMODULE_H
#define _SYNTHMODULE_H

#include "artsflow.h"
#include "flowsystem.h"
#include <string>
#include <vector>
#include <list>

class SynthBuffer
{
public:		// yes, bad style, but needs to be fast
	float *data;
	unsigned long position,size;

	/* every time I write data to a buffer, needread is incremented
	   by the amount of data for every module connected to that buffer,
	   since that (target) module has to read that data before I can write.
	   I can only write at max. (size-needread) items to that buffer
	*/
	unsigned long needread;

	SynthBuffer(float initialvalue, unsigned long size);
	void setValue(float value);
	~SynthBuffer();
};


class StdScheduleNode;

class Port {
protected:
	std::string _name;
	void *_ptr;
	AttributeType _flags;
	StdScheduleNode *parent;

	bool _dynamicPort;

	// functionality to remove all connections automatically as soon as
	// the module gets destroyed
	std::list<Port *> autoDisconnect;

	/**
	 * call these from your (dis)connect implementation as soon as a the
	 * port gets (dis)connected to some other port (only one call per
	 * connection: destinationport->addAutoDisconnect(sourceport), not
	 * for the other direction)
	 */
	void addAutoDisconnect(Port *source);
	void removeAutoDisconnect(Port *source);

public:
	Port(std::string name, void *ptr, long flags, StdScheduleNode* parent);
	virtual ~Port();

	AttributeType flags();
	std::string name();
	void setPtr(void *ptr);

	inline bool dynamicPort()    { return _dynamicPort; }
	inline void setDynamicPort() { _dynamicPort = true; }

	virtual class AudioPort *audioPort();
	virtual class ASyncPort *asyncPort();

	virtual void disconnectAll();
	virtual void connect(Port *) = 0;
	virtual void disconnect(Port *) = 0;
};

class AudioPort : public Port {
protected:
	AudioPort *source;

public:
	StdScheduleNode *sourcemodule;
	SynthBuffer *buffer, *lbuffer;
	unsigned long destcount, position;

	AudioPort(std::string name, void *ptr, long flags,StdScheduleNode *parent);
	~AudioPort();

	virtual class AudioPort *audioPort();

	void setFloatValue(float f);
	void connect(Port *psource);
	void disconnect(Port *psource);

//------------- I/O ---------------------------------------------------------

	inline unsigned long haveIn()
	{
        if(sourcemodule)
			return(buffer->position - position);
		else
			return(buffer->size);// constant value; no read limitation          
	}

    inline unsigned long outRoom()
    {
        long room;
 
        room = buffer->size - buffer->needread;
        if(room < 0) return 0;
        return (unsigned long)room;
    }                                                                           

	inline unsigned long readSegment(unsigned long offset, unsigned long len)
	{
		unsigned long rbsize = buffer->size;
		unsigned long rbmask = buffer->size - 1;
		unsigned long rbpos = (position+offset) & rbmask;

		*((float **)_ptr) = &buffer->data[rbpos];

		if(len > (rbsize-rbpos)) len = rbsize - rbpos;
		return len;
	}

	inline unsigned long writeSegment(unsigned long offset, unsigned long len)
	{
		unsigned long rbsize = buffer->size;
		unsigned long rbmask = buffer->size - 1;
		unsigned long rbpos = (buffer->position+offset) & rbmask;

		*((float **)_ptr) = &buffer->data[rbpos];

		if(len > (rbsize-rbpos)) len = rbsize - rbpos;
		return len;
	}

	inline void read(unsigned long cycles)
	{
		position += cycles;
		buffer->needread -= cycles;
	}

	inline void write(unsigned long cycles)
	{
		buffer->position += cycles;
		buffer->needread += cycles * destcount;
	}
};

class MultiPort : public Port {
protected:
	std::list<AudioPort *> parts;
	typedef float *float_ptr;
	float **conns;
	long nextID;
	void initConns();

public:
	MultiPort(std::string name, void *ptr, long flags,StdScheduleNode *parent);
	~MultiPort();

	void connect(Port *port);
	void disconnect(Port *port);
};

class StdScheduleNode :public ScheduleNode
{
	bool running;

	Object_skel *_object;
	SynthModule_base *module;
	FlowSystem_impl *flowSystem;
	std::list<Port *> ports;
	AudioPort **inConn;
	AudioPort **outConn;
	unsigned long inConnCount, outConnCount;
	typedef AudioPort *AudioPort_ptr;

	void freeConn();
	void rebuildConn();
	void accessModule();

	public: /* TODO? */
	Port *findPort(std::string name);

	/** scheduling variables - directly accessed by other schedNodes **/

	long Busy,BusyHit,NeedCycles, CanPerform;

public:

	StdScheduleNode(Object_skel *object, FlowSystem_impl *flowSystem);
	virtual ~StdScheduleNode();
	void initStream(std::string name, void *ptr, long flags);
	void addDynamicPort(Port *port);
	void removeDynamicPort(Port *port);

	void start();
	void stop();
	void requireFlow();


	void connect(std::string port, ScheduleNode *dest, std::string destport);
	void disconnect(std::string port, ScheduleNode *dest, std::string destport);

	AttributeType queryFlags(const std::string& port);
	void setFloatValue(std::string port, float value);
	long request(long amount);
	unsigned long calc(unsigned long cycles);

	Object_skel *object();
	void *cast(const std::string &target);
};

class StdFlowSystem :public FlowSystem_impl
{
public:
	ScheduleNode *addObject(Object_skel *object);
	void removeObject(ScheduleNode *node);

	/* remote accessibility */
	void startObject(Object* node);
	void stopObject(Object* node);
	void connectObject(Object* sourceObject, const std::string& sourcePort,
						Object* destObject, const std::string& destPort);
	void disconnectObject(Object* sourceObject, const std::string& sourcePort,
						Object* destObject, const std::string& destPort);
	AttributeType queryFlags(Object* node, const std::string& port);

	FlowSystemReceiver_base *createReceiver(Object *object, const std::string &port,
										FlowSystemSender_base *sender);
};

#endif
