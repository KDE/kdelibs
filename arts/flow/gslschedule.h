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

#ifndef _SYNTHMODULE_H
#define _SYNTHMODULE_H

#include "artsflow.h"
#include "flowsystem.h"
#include <gsl/gsldefs.h>
#include <string>
#include <list>

/*
 * BC - Status (2000-09-30): SynthBuffer, Port, AudioPort, MultiPort,
 *   StdFlowSystem, StdScheduleNode
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of libartsflow's
 * StdFlowSystem, and subject to change with the needs of it.
 *
 * If you want to access flowsystem functionality, do so over the core.idl
 * specified flowsystem interface, (object)->_node() or stuff in mcop/connect.h.
 */

namespace Arts {

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
	friend class VPort;				// TODO: debugging, remove me when it works?
	friend class VPortConnection;	// TODO: debugging, remove me when it works?
protected:
	std::string _name;
public:
	void *_ptr;
public:				// FIXME: GSL
	AttributeType _flags;
	StdScheduleNode *parent;

	// each port has a virtual port, which allows port redirection
	class VPort *_vport;

	// dynamic ports are created to implement multiports
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

	inline VPort* vport()		 { assert(_vport); return _vport; }
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
public:		// FIXME: GSL
	AudioPort *source;

public:
	StdScheduleNode *sourcemodule;
	SynthBuffer *buffer, *lbuffer;
	unsigned long destcount, position;
	unsigned long gslEngineChannel;

	/* GSL */
	bool gslIsConstant;
	float gslConstantValue;

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

class StdFlowSystem;

class StdScheduleNode :public ScheduleNode
{
	friend class StdFlowSystem;
	bool running;
	bool suspended;

	Object_skel *_object;
	SynthModule_base *module;
	StdFlowSystem *flowSystem;
	std::list<Port *> ports;
	AudioPort **inConn;
	AudioPort **outConn;
	unsigned long inConnCount, outConnCount;
	typedef AudioPort *AudioPort_ptr;
	QueryInitStreamFunc queryInitStreamFunc;

	void freeConn();
	void rebuildConn();
	void accessModule();

	static void gslProcess(GslModule *module, guint n_values);
	public: /* TODO? */
	Port *findPort(std::string name);

	/** scheduling variables - directly accessed by other schedNodes **/

	long Busy,BusyHit,NeedCycles, CanPerform;

public:

	GslModule *gslModule;

	StdScheduleNode(Object_skel *object, StdFlowSystem *flowSystem);
	virtual ~StdScheduleNode();
	void initStream(std::string name, void *ptr, long flags);
	void addDynamicPort(Port *port);
	void removeDynamicPort(Port *port);

	void start();
	void stop();
	void requireFlow();
	void virtualize(std::string port, ScheduleNode *implNode,
											std::string implPort);
	void devirtualize(std::string port, ScheduleNode *implNode,
											std::string implPort);

	// AutoSuspend stuff
	bool suspendable();
	void suspend();
	void restart();

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
protected:
	std::list<StdScheduleNode *> nodes;
	bool _suspended;
public:
	StdFlowSystem();

	ScheduleNode *addObject(Object_skel *object);
	void removeObject(ScheduleNode *node);

	/* AutoSuspend */
	bool suspendable();
	bool suspended();
	void suspend();
	void restart();

	/* remote accessibility */
	void startObject(Object node);
	void stopObject(Object node);
	void connectObject(Object sourceObject, const std::string& sourcePort,
						Object destObject, const std::string& destPort);
	void disconnectObject(Object sourceObject, const std::string& sourcePort,
						Object destObject, const std::string& destPort);
	AttributeType queryFlags(Object node, const std::string& port);
	void setFloatValue(Object node, const std::string& port, float value);

	FlowSystemReceiver createReceiver(Object object, const std::string &port,
										FlowSystemSender sender);

	/* interface to StdScheduleNode */
	void schedule(unsigned long samples);
};

};

#endif
