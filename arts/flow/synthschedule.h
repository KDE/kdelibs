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
	AttributeType _flags;
	string _name;
	void *_ptr;
	StdScheduleNode *parent;

public:
	Port(string name, void *ptr, long flags, StdScheduleNode* parent);
	virtual ~Port();

	AttributeType flags();
	string name();
	void setPtr(void *ptr);

	virtual class AudioPort *audioPort();

	virtual void connect(Port *) = 0;
	virtual void disconnect(Port *) = 0;
};

class AudioPort : public Port {
public:
	StdScheduleNode *sourcemodule;
	SynthBuffer *buffer, *lbuffer;
	unsigned long destcount, position;

	AudioPort(string name, void *ptr, long flags,StdScheduleNode *parent);
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
	list<AudioPort *> parts;
	typedef float *float_ptr;
	float **conns;
	long nextID;
	void initConns();

public:
	MultiPort(string name, void *ptr, long flags,StdScheduleNode *parent);
	~MultiPort();

	void connect(Port *port);
	void disconnect(Port *port);
};

class StdScheduleNode :public ScheduleNode
{
	Object_skel *object;
	SynthModule *module;
	FlowSystem *flowSystem;
	list<Port *> ports;
	AudioPort **inConn;
	AudioPort **outConn;
	unsigned long inConnCount, outConnCount;
	typedef AudioPort *AudioPort_ptr;

	void freeConn();
	void rebuildConn();
	void accessModule();

	Port *findPort(string name);

	/** scheduling variables - directly accessed by other schedNodes **/

	long Busy,BusyHit,NeedCycles, CanPerform;

public:

	StdScheduleNode(Object_skel *object, FlowSystem *flowSystem);
	virtual ~StdScheduleNode();
	void initStream(string name, void *ptr, long flags);
	void addDynamicPort(Port *port);
	void removeDynamicPort(Port *port);

	void start();
	void stop();
	void requireFlow();


	void connect(string port, ScheduleNode *dest, string destport);
	void disconnect(string port, ScheduleNode *dest, string destport);

	void setFloatValue(string port, float value);
	long request(long amount);
	unsigned long calc(unsigned long cycles);
};

class StdFlowSystem :public FlowSystem
{
public:
	ScheduleNode *addObject(Object_skel *object);
	void removeObject(ScheduleNode *node);
};

#endif
