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

#include "startupmanager.h"
#include "synthschedule.h"
#include "debug.h"
#include "asyncschedule.h"
#include "audiosubsys.h"
#include <algorithm>
#include <stdio.h>
#include <iostream>

using namespace std;

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
	//
}

Port::~Port()
{
	//
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
	while(!autoDisconnect.empty())
	{
		Port *other = *autoDisconnect.begin();

		// syntax is disconnect(source)
		if(_flags & streamIn)
			disconnect(other);		 // if we're incoming, other port is source
		else
			other->disconnect(this); // if we're outgoing, we're the source
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
	dport->connect(port);
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

			dport->disconnect(port);
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

	module = (SynthModule_base *)_object->_cast("SynthModule");
	if(!module)
	{
		cerr << "Only SynthModule derived classes should carry streams."
			<< endl;
	}
}

StdScheduleNode::StdScheduleNode(Object_skel *object, FlowSystem_impl *flowSystem) : ScheduleNode(object)
{
	_object = object;
	this->flowSystem = flowSystem;
	running = false;
	module = 0;
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
	if(flags & streamAsync)
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
	module->firstInitialize();
	module->initialize();
	module->start();
}

void StdScheduleNode::stop()
{
	assert(running);
	running = false;

	accessModule();
	module->deInitialize();
}

void StdScheduleNode::requireFlow()
{
	// cout << "rf" << module->_interfaceName() << endl;
	request(requestSize());
}

Port *StdScheduleNode::findPort(string name)
{
	list<Port *>::iterator i;
	for(i=ports.begin();i!=ports.end();i++)
	{
		Port *p = *i;
		if(p->name() == name) return p;
	}
	return 0;
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
			p1->connect(p2);
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->connect(p1);
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
			p1->disconnect(p2);
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->disconnect(p1);
		}
	}
}

AttributeType StdScheduleNode::queryFlags(const std::string& port)
{
	cout << "findPort(" << port << ")" << endl;
	cout << "have " << ports.size() << " ports" << endl;
	Port *p1 = findPort(port);
	cout << "done" << endl;

	if(p1)
	{
		cout << "result" << (long)p1->flags() << endl;
		return p1->flags();
	}
	cout << "failed" << endl;
	return (AttributeType)0;
}

void StdScheduleNode::setFloatValue(string port, float value)           
{
	AudioPort *p = findPort(port)->audioPort();

	if(p) {
		p->setFloatValue(value);
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

ScheduleNode *StdFlowSystem::addObject(Object_skel *object)
{
	return new StdScheduleNode(object,this);
}

void StdFlowSystem::removeObject(ScheduleNode *node)
{
	delete node;
}

/* remote accessibility */

void StdFlowSystem::startObject(Object* node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node->_node()->cast("StdScheduleNode");
	sn->start();
}

void StdFlowSystem::stopObject(Object* node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node->_node()->cast("StdScheduleNode");
	sn->stop();
}

void StdFlowSystem::connectObject(Object* sourceObject,const string& sourcePort,
					Object* destObject, const std::string& destPort)
{
	cout << "connect port " << sourcePort << " to " << destPort << endl;
	StdScheduleNode *sn =
		(StdScheduleNode *)sourceObject->_node()->cast("StdScheduleNode");
	assert(sn);

	Port *port = sn->findPort(sourcePort);
	assert(port);

	ASyncPort *ap = port->asyncPort();

	if(ap)
	{
		ASyncNetSend *netsend = new ASyncNetSend();
		ap->sendNet(netsend);

		FlowSystem_var remoteFs = destObject->_flowSystem();
		FlowSystemReceiver_var receiver;
		receiver = remoteFs->createReceiver(destObject, destPort, netsend);

		netsend->setReceiver(receiver);
		cout << "connected an asyncnetsend" << endl;		
	}
}

void StdFlowSystem::disconnectObject(Object* sourceObject,
		const string& sourcePort, Object* destObject, const string& destPort)
{
	assert(false);
}

AttributeType StdFlowSystem::queryFlags(Object* node, const std::string& port)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node->_node()->cast("StdScheduleNode");
	return sn->queryFlags(port);
}

FlowSystemReceiver_base *StdFlowSystem::createReceiver(Object *object,
							const string &port, FlowSystemSender_base *sender)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)object->_node()->cast("StdScheduleNode");

	Port *p = sn->findPort(port);
	assert(p);

	ASyncPort *ap = p->asyncPort();

	if(ap)
	{
		cout << "creating packet receiver" << endl;
		/*
		 * TODO: FIXME: this is to prevent the receiver from just disappearing
		 * which has the disadvantage that then datapackets which are still
		 * outstanding cause problems. However, like this, it will never get
		 * really disconnected on connection drop, which is also bad (but
		 * not as ugly as a crash)
		 */
		return (new ASyncNetReceive(ap, sender))->_copy();
	}
	return 0;
}

// hacked initialization of Dispatcher::the()->flowSystem ;)

static class SetFlowSystem : StartupClass {
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
