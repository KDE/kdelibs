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

#include "virtualports.h"
#include "startupmanager.h"
#include "synthschedule.h"
#include "debug.h"
#include "asyncschedule.h"
#include "audiosubsys.h"
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stack>

using namespace std;
using namespace Arts;

// well, this was tuneable once...

static unsigned long requestSize()
{
	unsigned long reqSize = 0;
	if(!reqSize)
		reqSize = AudioSubSystem::the()->fragmentSize()/4;

	return reqSize;
}

// ----------- SynthBuffer -------------

SynthBuffer::SynthBuffer(float initialvalue, unsigned long size)
{
	this->size = size;
	data = new float[size];

	setValue(initialvalue);

	position = 0;
	needread = 0;
}

void SynthBuffer::setValue(float value)
{
	unsigned long i;
	for(i=0;i<size;i++) data[i] = value;
}

SynthBuffer::~SynthBuffer()
{
	delete[] data;
}

// ----------- Port -----------

Port::Port(string name, void *ptr, long flags, StdScheduleNode* parent)
	: _name(name), _ptr(ptr), _flags((AttributeType)flags),
	  parent(parent), _dynamicPort(false)
{
	_vport = new VPort(this);
}

Port::~Port()
{
	if(_vport)
		delete _vport;
}

AttributeType Port::flags()
{
	return _flags;
}

string Port::name()
{
	return _name;
}

ASyncPort *Port::asyncPort()
{
	return 0;
}

AudioPort *Port::audioPort()
{
	return 0;
}

void Port::addAutoDisconnect(Port *source)
{
	autoDisconnect.push_back(source);
	source->autoDisconnect.push_back(this);
}

void Port::removeAutoDisconnect(Port *source)
{
	list<Port *>::iterator adi;

	// remove our autodisconnection entry for source port
	adi = find(autoDisconnect.begin(),autoDisconnect.end(),source);
	assert(adi != autoDisconnect.end());
	autoDisconnect.erase(adi);

	// remove the source port autodisconnection entry to us
	adi=find(source->autoDisconnect.begin(),source->autoDisconnect.end(),this);
	assert(adi != source->autoDisconnect.end());
	source->autoDisconnect.erase(adi);
}

void Port::disconnectAll()
{
	if(_vport)
		delete _vport;
	_vport = 0;
	assert(autoDisconnect.empty());
	while(!autoDisconnect.empty())
	{
		Port *other = *autoDisconnect.begin();

		// syntax is disconnect(source)
		if(_flags & streamIn)
			// if we're incoming, other port is source
			vport()->disconnect(other->vport());
		else
			// if we're outgoing, we're the source
			other->vport()->disconnect(this->vport());
	}
}

void Port::setPtr(void *ptr)
{
	_ptr = ptr;
}

// ------- AudioPort ---------

AudioPort::AudioPort(string name, void *ptr, long flags,StdScheduleNode *parent)
		: Port(name,ptr,flags,parent)
{
	position = 0;
	destcount = 0;
	sourcemodule = 0;
	source = 0;
	lbuffer = buffer = new SynthBuffer(0.0, requestSize());
}

AudioPort::~AudioPort()
{
	delete lbuffer;
}

AudioPort *AudioPort::audioPort()
{
	return this;
}

void AudioPort::setFloatValue(float f)
{
	buffer->setValue(f);
}

void AudioPort::connect(Port *psource)
{
	source = psource->audioPort();
	assert(source);
	addAutoDisconnect(psource);

	buffer = source->buffer;
	position = buffer->position;
	source->destcount++;
	sourcemodule = source->parent;
}

void AudioPort::disconnect(Port *psource)
{
	assert(source);
	assert(source == psource->audioPort());
	removeAutoDisconnect(psource);

	assert(sourcemodule == source->parent);
	sourcemodule = 0;

	// skip the remaining stuff in the buffer
	read(buffer->position - position);
	source->destcount--;
	source = 0;

	position = lbuffer->position;
	buffer = lbuffer;
}

// --------- MultiPort ----------

MultiPort::MultiPort(string name, void *ptr, long flags,StdScheduleNode *parent)
	: Port(name,ptr,flags,parent)
{
	conns = 0;
	nextID = 0;
	initConns();
}

MultiPort::~MultiPort()
{
	if(conns)
	{
		delete[] conns;
		conns = 0;
	}
}

void MultiPort::initConns()
{
	if(conns != 0) delete[] conns;
	conns = new float_ptr[parts.size() + 1];
	conns[parts.size()] = (float *)0;

	*(float ***)_ptr = conns;

	long n = 0;
	list<AudioPort *>::iterator i;
	for(i = parts.begin();i != parts.end(); i++)
	{
		AudioPort *p = *i;
		p->setPtr((void *)&conns[n++]);
	}
}

void MultiPort::connect(Port *port)
{
	AudioPort *dport;
	char sid[20];
	sprintf(sid,"%ld",nextID++);

	addAutoDisconnect(port);

	dport = new AudioPort("_"+_name+string(sid),0,streamIn,parent);
	parts.push_back(dport);
	initConns();

	parent->addDynamicPort(dport);
	dport->vport()->connect(port->vport());
}

void MultiPort::disconnect(Port *sport)
{
	AudioPort *port = (AudioPort *)sport;
	removeAutoDisconnect(sport);

	list<AudioPort *>::iterator i;
	for(i = parts.begin(); i != parts.end(); i++)
	{
		AudioPort *dport = *i;

		if(dport->buffer == port->buffer)
		{
			parts.erase(i);
			initConns();

			dport->vport()->disconnect(port->vport());
			parent->removeDynamicPort(dport);

			delete dport;
			return;
		}
	}
}

// -------- StdScheduleNode ---------

void StdScheduleNode::freeConn()
{
	if(inConn)
	{
		delete inConn;
		inConn = 0;
	}
	if(outConn)
	{
		delete outConn;
		outConn = 0;
	}
	inConnCount = outConnCount = 0;
}

void StdScheduleNode::rebuildConn()
{
	list<Port *>::iterator i;

	freeConn();

	inConnCount = outConnCount = 0;
	inConn = new AudioPort_ptr[ports.size()];
	outConn = new AudioPort_ptr[ports.size()];

	for(i=ports.begin();i != ports.end();i++)
	{
		AudioPort *p = (*i)->audioPort();
		if(p)
		{
			if(p->flags() & streamIn) inConn[inConnCount++] = p;
			if(p->flags() & streamOut) outConn[outConnCount++] = p;
		}
	}
}

Object_skel *StdScheduleNode::object()
{
	return _object;
}

void *StdScheduleNode::cast(const string &target)
{
	if(target == "StdScheduleNode") return (StdScheduleNode *)this;
	return 0;
}


void StdScheduleNode::accessModule()
{
	if(module) return;

	module = (SynthModule_base *)_object->_cast(Arts::SynthModule_base::_IID);
	if(!module)
		arts_warning("Only SynthModule derived classes should carry streams.");
}

StdScheduleNode::StdScheduleNode(Object_skel *object, StdFlowSystem *flowSystem) : ScheduleNode(object)
{
	_object = object;
	this->flowSystem = flowSystem;
	running = false;
	suspended = false;
	module = 0;
	queryInitStreamFunc = 0;
	inConn = outConn = 0;
	inConnCount = outConnCount = 0;
	Busy = BusyHit = NeedCycles = CanPerform = 0;
}

StdScheduleNode::~StdScheduleNode()
{
	/* stop module if still running */
	if(running) stop();
	/* disconnect all ports */
	stack<Port *> disconnect_stack;

	/*
	 * we must be a bit careful here, as dynamic ports (which are created
	 * for connections by MultiPorts) will suddenly start disappearing, so
	 * we better make a copy of those ports that will stay, and disconnect
	 * them then
	 */
	list<Port *>::iterator i;
	for(i=ports.begin();i != ports.end();i++)
	{
		if(!(*i)->dynamicPort()) disconnect_stack.push(*i);
	}

	while(!disconnect_stack.empty())
	{
		disconnect_stack.top()->disconnectAll();
		disconnect_stack.pop();
	}
	/* free them */
	for(i=ports.begin();i != ports.end();i++)
		delete (*i);
	ports.clear();

	freeConn();
}

void StdScheduleNode::initStream(string name, void *ptr, long flags)
{
	if(flags == -1)
	{
		queryInitStreamFunc = (QueryInitStreamFunc)ptr;
	}
	else if(flags & streamAsync)
	{
		ports.push_back(new ASyncPort(name,ptr,flags,this));
	}
	else if(flags & streamMulti)
	{
		ports.push_back(new MultiPort(name,ptr,flags,this));
	}
	else
	{
		ports.push_back(new AudioPort(name,ptr,flags,this));
	}

	// TODO: maybe initialize a bit later
	rebuildConn();
}

void StdScheduleNode::addDynamicPort(Port *port)
{
	port->setDynamicPort();
	ports.push_back(port);
	rebuildConn();
}

void StdScheduleNode::removeDynamicPort(Port *port)
{
	list<Port *>::iterator i;
	for(i=ports.begin();i!=ports.end();i++)
	{
		Port *p = *i;
		if(p->name() == port->name())
		{
			ports.erase(i);
			rebuildConn();
			return;	
		}
	}
}

void StdScheduleNode::start()
{
	assert(!running);
	running = true;

	//cout << "start" << endl;
	accessModule();
	module->streamInit();
	module->streamStart();
}

void StdScheduleNode::stop()
{
	assert(running);
	running = false;

	accessModule();
	module->streamEnd();
}

void StdScheduleNode::requireFlow()
{
	// cout << "rf" << module->_interfaceName() << endl;
	//request(requestSize());
	flowSystem->schedule(requestSize());
}

bool StdScheduleNode::suspendable()
{
	accessModule();
	if(running) {
		return (module->autoSuspend() != asNoSuspend);
	}
	// if its not running, who cares?
	return true;
}

void StdScheduleNode::suspend()
{
	accessModule();
	if(running) {
		suspended = true;
		if(module->autoSuspend() == asSuspendStop) stop();
	}
}

void StdScheduleNode::restart()
{
	accessModule();
	if(running == false && suspended == true) {
		suspended = false;
		if(module->autoSuspend() == asSuspendStop) start();
	}
}

Port *StdScheduleNode::findPort(string name)
{
	list<Port *>::iterator i;
	for(i=ports.begin();i!=ports.end();i++)
	{
		Port *p = *i;
		if(p->name() == name) return p;
	}
	if(queryInitStreamFunc)
	{
		if(queryInitStreamFunc(_object,name))
		{
			for(i=ports.begin();i!=ports.end();i++)
			{
				Port *p = *i;
				if(p->name() == name) return p;
			}
		}
	}
	return 0;
}

void StdScheduleNode::virtualize(std::string port, ScheduleNode *implNode,
													std::string implPort)
{
	StdScheduleNode *impl=(StdScheduleNode *)implNode->cast("StdScheduleNode");
	if(impl)
	{
		Port *p1 = findPort(port);
		Port *p2 = impl->findPort(implPort);

		assert(p1);
		assert(p2);
		p1->vport()->virtualize(p2->vport());
	}
}

void StdScheduleNode::devirtualize(std::string port, ScheduleNode *implNode,
											std::string implPort)
{
	StdScheduleNode *impl=(StdScheduleNode *)implNode->cast("StdScheduleNode");
	if(impl)
	{
		Port *p1 = findPort(port);
		Port *p2 = impl->findPort(implPort);

		p1->vport()->devirtualize(p2->vport());
	}
}

void StdScheduleNode::connect(string port, ScheduleNode *dest, string destport)
{
	RemoteScheduleNode *rsn = dest->remoteScheduleNode();
	if(rsn)
	{
		// RemoteScheduleNodes know better how to connect remotely
		rsn->connect(destport,this,port);
		return;
	}

	Port *p1 = findPort(port);
	Port *p2 = ((StdScheduleNode *)dest)->findPort(destport);

	if(p1 && p2)
	{
		if((p1->flags() & streamIn) && (p2->flags() & streamOut))
		{
			p1->vport()->connect(p2->vport());
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->vport()->connect(p1->vport());
		}
	}
}

void StdScheduleNode::disconnect(string port,
								ScheduleNode *dest, string destport)
{
	RemoteScheduleNode *rsn = dest->remoteScheduleNode();
	if(rsn)
	{
		// RemoteScheduleNodes know better how to disconnect remotely
		rsn->disconnect(destport,this,port);
		return;
	}

	Port *p1 = findPort(port);
	Port *p2 = ((StdScheduleNode *)dest)->findPort(destport);

	if(p1 && p2)
	{
		if((p1->flags() & streamIn) && (p2->flags() & streamOut))
		{
			p1->vport()->disconnect(p2->vport());
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->vport()->disconnect(p1->vport());
		}
	}
}

AttributeType StdScheduleNode::queryFlags(const std::string& port)
{
	arts_debug("findPort(%s)", port.c_str());
	arts_debug("have %ld ports", ports.size());
	Port *p1 = findPort(port);
	arts_debug("done");

	if(p1)
	{
		arts_debug("result %d",(long)p1->flags());
		return p1->flags();
	}
	arts_debug("failed");
	return (AttributeType)0;
}

void StdScheduleNode::setFloatValue(string port, float value)           
{
	AudioPort *p = findPort(port)->audioPort();

	if(p) {
		p->vport()->setFloatValue(value);
	} else {
		assert(false);
	}
}

/* request a module to calculate a number of turns
   will return -1 if busy, count that has been done otherwise */

long StdScheduleNode::request(long amount)
{
	unsigned long in;
	int have_in,need_more,have_done,have_done_total = 0;

	if(!running)
	{
		arts_fatal("Calculating data on a module which was not started!\n"
					"Start modules by calling module.start() "
					"before connecting them to avoid this.");
		// not reached
	}

	if(Busy) { BusyHit++; return(-1); }

	Busy = 1;
	if(NeedCycles < amount)
	{
		NeedCycles = amount;
	}

	// cout << "re" << module->_interfaceName() << endl;
	//artsdebug("DSP %s: request of %d cycles\n",getClassName(),amount);
	do
	{
		/* assume we can satisfy the request */
		CanPerform = NeedCycles;

		/* then, check wether the input channels supply enough data to do so. */

		for(in=0;in<inConnCount;in++)
		{
			have_in = inConn[in]->haveIn();

			if(have_in < NeedCycles)
			{
				//artsdebug("connection %d indicates to have %d, "
                //       "thats not enough\n", in,have_in);
				/* if we can't calculate enough stuff due to a certain
				   ingoing connection, go to the associated module and
				   tell it that we need more data
				*/
				need_more = NeedCycles - have_in;
				//artsdebug("requesting %d\n", need_more);
				/* when there is no source (constant input value), then
					we don't need to request something, it's just that
					it can't supply more because the buffer isn't big enough
				*/
				if(inConn[in]->sourcemodule)
					inConn[in]->sourcemodule->request(need_more);

				have_in = inConn[in]->haveIn();
	
				//artsdebug("now connection %d indicates to have %d\n",in,have_in);
				if(CanPerform > have_in) CanPerform = have_in;
			}
		}
		have_done = calc(CanPerform);
		have_done_total += have_done;

		/*
		if(dsp->m_BusyHit != 0) artsdebug("got busyhit: %s\n",dsp->m_Name);
		*/

	} while(BusyHit && NeedCycles != CanPerform && have_done);
	/* makes only sense to repeat if
		 - there was a busyhit which indicates we are in a feedback loop
		 - we should have done more than we have done
         - actually something could be calculated
     */
	Busy = 0;
	return(have_done);
}

/* This routine would now actually let the plugin calculate some data
   that means generate sinus waves, mix audio signals etc.

   The number of cycles is guaranteed to work without input underrun
   by the flow system. But the routine still needs to check output
   stall situations (ring buffer full).
*/

unsigned long StdScheduleNode::calc(unsigned long cycles)
{
	unsigned long i,room;

	//cout << "ca" << module->_interfaceName() << endl;
	/* output sanity check:
	   when there is not enough room in one of the buffers, we
	   can't calculate that much cycles
	*/
	for(i=0;i<outConnCount;i++)
	{
		room = outConn[i]->outRoom();
		if(room < cycles)
		{
			cycles = room;
			/*
			artsdebug("- reduced calculation to %d due to lack of space\n",cycles);
			*/
		}
	}

	if(cycles == 0) return(0);

	//artsdebug("DSP %s: calculation of %d cycles\n",getClassName(),cycles);
	/* input sanity check:
		it's guaranteed that we have enough input, but do the check
		anyway... - you never know
	*/
	for(i=0;i<inConnCount;i++)
	{
		/* otherwise input is "overconsumed" */
		assert(inConn[i]->haveIn() >= cycles);

		/* check sanity of the needread setting:
			either data comes from fixed value (sourcemodule not assigned)
			then we can't expect needread to contain sensible setting,
			but otherwise: needread should have expected that we'll want
			to read the data and thus be more than the cycles! */

		assert((!inConn[i]->sourcemodule)
				|| (inConn[i]->buffer->needread >= cycles));
	}

	unsigned long j, donecycles = 0, cando = 0;
	
	while(donecycles != cycles)
	{	
		cando = cycles-donecycles;

		for(j=0;j<inConnCount;j++)
			cando = inConn[j]->readSegment(donecycles,cando);

		for(j=0;j<outConnCount;j++)
			cando = outConn[j]->writeSegment(donecycles,cando);

		module->calculateBlock(cando);
		donecycles += cando;
	}
	assert(donecycles == cycles);

	// actually update buffer by subtracting consumed input
	for(i=0;i<inConnCount;i++)
		inConn[i]->read(cycles);

	// and adding fresh output
	for(i=0;i<outConnCount;i++)
		outConn[i]->write(cycles);

	NeedCycles -= cycles;
	CanPerform -= cycles;
	return(cycles);
}

StdFlowSystem::StdFlowSystem()
{
	_suspended = false;
}

ScheduleNode *StdFlowSystem::addObject(Object_skel *object)
{
	// do not add new modules when being in suspended state
	restart();

	StdScheduleNode *node = new StdScheduleNode(object,this);
	nodes.push_back(node);
	return node;
}

void StdFlowSystem::removeObject(ScheduleNode *node)
{
	StdScheduleNode *xnode = (StdScheduleNode *)node->cast("StdScheduleNode");
	assert(xnode);
	nodes.remove(xnode);
	delete xnode;
}

bool StdFlowSystem::suspended()
{
	return _suspended;
}

bool StdFlowSystem::suspendable()
{
	list<StdScheduleNode *>::iterator i;
	for(i = nodes.begin();i != nodes.end();i++)
	{
		StdScheduleNode *node = *i;
		if(!node->suspendable()) return false;
	}
	return true;
}

void StdFlowSystem::suspend()
{
	if(!_suspended)
	{
		list<StdScheduleNode *>::iterator i;
		for(i = nodes.begin();i != nodes.end();i++)
		{
			StdScheduleNode *node = *i;
			node->suspend();
		}
		_suspended = true;
	}
}

void StdFlowSystem::restart()
{
	if(_suspended)
	{
		list<StdScheduleNode *>::iterator i;
		for(i = nodes.begin();i != nodes.end();i++)
		{
			StdScheduleNode *node = *i;
			node->restart();
		}
		_suspended = false;
	}
}

/* remote accessibility */

void StdFlowSystem::startObject(Object node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	sn->start();
}

void StdFlowSystem::stopObject(Object node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	sn->stop();
}

void StdFlowSystem::connectObject(Object sourceObject,const string& sourcePort,
					Object destObject, const std::string& destPort)
{
	arts_debug("connect port %s to %s", sourcePort.c_str(), destPort.c_str());
	StdScheduleNode *sn =
		(StdScheduleNode *)sourceObject._node()->cast("StdScheduleNode");
	assert(sn);

	Port *port = sn->findPort(sourcePort);
	assert(port);

	StdScheduleNode *destsn =
		(StdScheduleNode *)destObject._node()->cast("StdScheduleNode");
	if(destsn)
	{
		sn->connect(sourcePort,destsn,destPort);
		return;
	}
	
	ASyncPort *ap = port->asyncPort();

	if(ap)
	{
		FlowSystemSender sender;
		FlowSystemReceiver receiver;
		FlowSystem remoteFs;

		string dest = destObject.toString() + ":" + destPort;
		ASyncNetSend *netsend = new ASyncNetSend(ap, dest);

		sender = FlowSystemSender::_from_base(netsend); // don't release netsend
		remoteFs = destObject._flowSystem();
		receiver = remoteFs.createReceiver(destObject, destPort, sender);
		netsend->setReceiver(receiver);
		arts_debug("connected an asyncnetsend");
	}
}

void StdFlowSystem::disconnectObject(Object sourceObject,
  	const string& sourcePort, Object destObject, const std::string& destPort)
{
	arts_debug("disconnect port %s and %s",sourcePort.c_str(),destPort.c_str());
	StdScheduleNode *sn =
		(StdScheduleNode *)sourceObject._node()->cast("StdScheduleNode");
	assert(sn);

	Port *port = sn->findPort(sourcePort);
	assert(port);

	StdScheduleNode *destsn =
		(StdScheduleNode *)destObject._node()->cast("StdScheduleNode");
	if(destsn)
	{
		sn->disconnect(sourcePort,destsn,destPort);
		return;
	}

	ASyncPort *ap = port->asyncPort();
	if(ap)
	{
		string dest = destObject.toString() + ":" + destPort;
		ap->disconnectRemote(dest);
		arts_debug("disconnected an asyncnetsend");
	}
}

AttributeType StdFlowSystem::queryFlags(Object node, const std::string& port)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	assert(sn);
	return sn->queryFlags(port);
}

void StdFlowSystem::setFloatValue(Object node, const std::string& port,
							float value)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	assert(sn);
	sn->setFloatValue(port,value);
}

FlowSystemReceiver StdFlowSystem::createReceiver(Object object,
							const string &port, FlowSystemSender sender)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)object._node()->cast("StdScheduleNode");

	Port *p = sn->findPort(port);
	assert(p);

	ASyncPort *ap = p->asyncPort();

	if(ap)
	{
		arts_debug("creating packet receiver");
		return FlowSystemReceiver::_from_base(new ASyncNetReceive(ap, sender));
	}
	return FlowSystemReceiver::null();
}

void StdFlowSystem::schedule(unsigned long samples)
{
/** new style (dynamic size) scheduling **/

	unsigned long MCount = nodes.size();
	unsigned long *done = (unsigned long *)calloc(MCount,sizeof(unsigned long));
	unsigned long i;
	list<StdScheduleNode*>::iterator ni;
	long incomplete, died = 0;

	//printf("entering; samples = %d\n",samples);
	do {
		incomplete = 0;		/* assume we have calculated all samples for all
								consumers, and later increment if some are
								still missing */
		for(ni = nodes.begin(), i = 0; ni != nodes.end(); ni++, i++)
		{
			StdScheduleNode *node = *ni;

            unsigned int outConnections = 0,c;
            for(c=0;c<node->outConnCount && !outConnections;c++)
                outConnections += node->outConn[c]->destcount;                  

			if(outConnections == 0 && node->running)
			{
				//printf("consumber = %s,done = %d, samples = %d\n",SynthModules[i]->getClassName(),done[i],samples);
				/* a module whose input is not comsumed from other modules
					is a "push delivery" style module, such as speakers,
					or writing audio to log file, etc. and has to get
					external requests from the scheduling system */

				if(done[i] != samples)
					done[i] += node->request(samples-done[i]);
				assert(done[i] <= samples);
				if(done[i] != samples) incomplete++;
				//printf("*scheduler*\n");
				died ++;
				if(died > 10000)
				{
					free(done);
					arts_warning("scheduler confusion: circle?");
					return;
				}
			}
		}
	} while(incomplete);

	//printf("<=> done!!\n");
	free(done);
	//return true;
}

// hacked initialization of Dispatcher::the()->flowSystem ;)

namespace Arts {

static class SetFlowSystem : public StartupClass {
	FlowSystem_impl *fs;
public:
	void startup()
	{
		fs = new StdFlowSystem;
		Dispatcher::the()->setFlowSystem(fs);
	}
	void shutdown()
	{
		fs->_release();
	}
} sfs;

};
