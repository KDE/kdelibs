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

#include "artsflow.h"
#include "debug.h"
#include "synthschedule.h"
#include <map>

class SynthPortWrapper {
public:
	PortDesc port;
	class SynthExecNode *node;

	SynthPortWrapper(const PortDesc& port, SynthExecNode *node)
	{
		this->node = node;
		this->port = port;
	}
	void connect(SynthPortWrapper *src);
};

class SynthExecNode {
protected:
	string _name;

public:
	SynthModule *module;
	list<SynthPortWrapper *> ports;

	long ID, mID;
	SynthExecNode(SynthModule *module)
	{
		this->module = module;
	}

	void setName(string s)
	{
		_name = s;
	}

	string getName()
	{
		return _name;
	}

	void createPort(PortDesc *pd)
	{
		if(pd->hasValue && pd->Type.ConnType == conn_stream)
		{
			/*
			cout << "p" << pd->Type.ConnType << " "<< conn_stream << endl;
			cout << "sfv" << pd->Name << " "<<pd->FloatValue << endl;
			*/
			module->_node()->setFloatValue(pd->Name, pd->FloatValue);
		}
		ports.push_back(new SynthPortWrapper(*pd,this));
	}
};

void SynthPortWrapper::connect(SynthPortWrapper *src)
{
	node->module->_node()->
		connect(port.Name,src->node->module->_node(),src->port.Name);
}

class ArtsServer_impl :virtual public ArtsServer_skel
{
protected:
	long nextMID;
	vector<SynthExecNode *> execNodes;

public:
	ArtsServer_impl();

	long createModules();
	bool createModule(long mid, const ModuleDesc& desc);
	bool localConnectModules(long mid);
	bool finalizeModules(long mid);
	void startModules(long mid);
	void deleteModules(long mid);

	/** internal interface */
	void run();
};

REGISTER_IMPLEMENTATION(ArtsServer_impl);

ArtsServer_impl *asReference = 0;

void artsServerRun()
{
	asReference->run();
}

ArtsServer_impl::ArtsServer_impl()
{
	nextMID = 1;
	asReference = this;
}

long ArtsServer_impl::createModules()
{
	return nextMID++;
}

bool ArtsServer_impl::createModule(long mid, const ModuleDesc& desc)
{
	Object_skel *object = ObjectManager::the()->create(desc.Name);
	assert(object);

	SynthModule *NewModule = (SynthModule *)object->_cast("SynthModule");
	assert(NewModule);

	SynthExecNode *schedNode = new SynthExecNode(NewModule);
	schedNode->setName(desc.Name);

	schedNode->ID = desc.ID;
	schedNode->mID = mid;

	execNodes.push_back(schedNode);

	/*
	unsigned long inportcount = 0;
	unsigned long outportcount = 0;
	*/
	const vector<PortDesc *>& ports = desc.Ports;

	setartsdebug(true);
	unsigned long p;
	for(p=0;p<ports.size();p++)
	{
		PortDesc *pd = ports[p];
		cout << "createport " << pd->Name << endl;
		schedNode->createPort(pd);
	}
/*
		PortDesc *pd = ports[p];
		PortType pt = pd->Type;

		if(pt.Direction == input)
		{
			assert(pt.ConnType != event); // no event channels yet

			SynthPort *synthport =
				new SynthPort(schedNode, inportcount++, pd->ID);
			schedNode->inPorts.push_back(synthport);

			if(pt.ConnType == stream)
			{
				assert(pt.DataType == audio_data);
	
				if(pt.isMultiPort)
				{
					artsdebug("lookup multiport %s\n",pd->Name.c_str());
					float ***streamptr =
						(float ***)NewModule->_lookupStream(pd->Name);
					assert(streamptr);

				}
				else
				{
					artsdebug("looking up port %s\n",pd->Name.c_str());
					float **streamptr =
						(float **)NewModule->_lookupStream(pd->Name);
					assert(streamptr);

					SynthConnection *conn =
						new SynthConnection(schedNode,pd->ID,streamptr);

					schedNode->assignInConn(conn);
					synthport->setConnection(conn);

					if(pd->isConnected)
					{
						assert(pd->ConnectedIDs.size() == 1);

						long wantedID = pd->ConnectedIDs[0];
						conn->needConnect = true;
						conn->wantedID = wantedID;
					}
					else
					{
						float initialvalue = 0.0;

						if(pd->hasValue)
							initialvalue = pd->FloatValue;

						conn->allocBuffer(initialvalue,RBSize);
					}
				}
			}
			if(pt.ConnType == property)
			{
				assert(pt.DataType == string_data);

				if(pd->isConnected)
				{
					// if its connected, find the connection partner
					assert(pd->ConnectedIDs.size() == 1);

					long wantedID = pd->ConnectedIDs[0];
					synthport->setNeedConnect(true, wantedID);
				}
				else
				{
					// if its not connected simply look up value and create
					// a property that will hold the value

					// TODO
				}
			}
		}
		if(pt.Direction == output)
		{
			artsdebug("creating output port id %ld\n",pd->ID);

			SynthPort *synthport =
				new SynthPort(schedNode, outportcount++, pd->ID);
			schedNode->outPorts.push_back(synthport);

			if(pt.ConnType == stream)
			{
				float **streamptr = (float **)NewModule->_lookupStream(pd->Name);
				assert(streamptr);

				SynthConnection *conn =
					new SynthConnection(schedNode,pd->ID,streamptr);
				conn->allocBuffer(0.0,RBSize);
				schedNode->assignOutConn(conn);
				synthport->setConnection(conn);
			}
			if(pt.ConnType == property)
			{
				// TODO
			}
		}
	}
*/
	return true;
}

bool ArtsServer_impl::localConnectModules(long mid)
{
	artsdebug("Execute II localConnectModules\n");

	map<long,SynthPortWrapper *> portMap;

	unsigned long i;
	for(i=0;i<execNodes.size();i++)
	{
		SynthExecNode *NewModule = execNodes[i];

		if(NewModule->mID == mid)
		{
			list<SynthPortWrapper *>::iterator i;
			for(i=NewModule->ports.begin();i!=NewModule->ports.end();i++)
			{
				SynthPortWrapper *p = *i;
				portMap[p->port.ID] = p;
			}
		}
	}
	for(i=0;i<execNodes.size();i++)
	{
		SynthExecNode *NewModule = execNodes[i];

		if(NewModule->mID == mid)
		{
			list<SynthPortWrapper *>::iterator i;
			for(i=NewModule->ports.begin();i!=NewModule->ports.end();i++)
			{
				SynthPortWrapper *p = *i;
				if(p->port.Type.Direction == input && p->port.isConnected)
				{
					for(long l=0;l<p->port.ConnectedIDs.size();l++)
					{
						cout << p->port.ConnectedIDs[l] << endl;
						SynthPortWrapper *src=portMap[p->port.ConnectedIDs[l]];
						if(src) p->connect(src);
					}
				}
				/*
				const list<SynthConnection *>& conns = p->connections();
				list<SynthConnection *>::const_iterator ci;

				for(ci = conns.begin(); ci != conns.end();ci++)
				{
					SynthConnection *conn = *ci;

					if(conn && conn->needConnect)
					{
						SynthConnection *source = 0;

						SynthPort *sourceport = portMap[conn->wantedID];
						if(sourceport)
						{
							if(sourceport->connections().size() == 1)
								source = *sourceport->connections().begin();
						}

						if(!source)
						{
							artsdebug("NewModule %s missing source (remote?)\n",
							NewModule->getName().c_str());
						}
						else
						{
							conn->connectToSource(source);
							conn->needConnect = false;
						}
					}
				}
				*/
			}
		}
	}
	return true;
}

bool ArtsServer_impl::finalizeModules(long mid)
{
/*
	artsdebug("Execute III <Prepare & Initialization> in finalizeModules\n");

	unsigned long i;
	for(i=0;i<execNodes.size();i++)
	{
		SynthExecNode *node = execNodes[i];

		if(node->mID == mid)
		{
			node->initialize();
	
			// EXPERIMENTAL: prepareExecution after Initialize
			node->prepareExecution();
		}
	}
*/
	return true;
}

void ArtsServer_impl::startModules(long mid)
{
	artsdebug("Execute IV <IO Initialization> in startModules\n");

	unsigned long i;
	for(i=0;i<execNodes.size();i++)
	{
		SynthExecNode *node = execNodes[i];
		if(node->mID == mid) node->module->_node()->start();
	}
}


void ArtsServer_impl::deleteModules(long int)
{
}

void ArtsServer_impl::run()
{
#if 0
	// check scheduler locks?
	// FIXME: where should scheduler locks be? ExecutionManager? ArtsServer?

	unsigned long cycles = 128; /* _RTConfig.Cycles; */
	unsigned long *done =
		(unsigned long *)calloc(execNodes.size(),sizeof(unsigned long));
	long incomplete, died = 0;

	// printf("entering; cycles = %d\n",cycles);
	do {
		incomplete = 0;		/* assume we have calculated all cycles for all
								consumers, and later increment if some are
								still missing */
		vector<SynthExecNode *>::iterator i;
		int k = 0;

		for(i=execNodes.begin();i != execNodes.end();i++,k++)
		{
			SynthExecNode *current = *i;
			if(current->clients == 0 && current->enabled)
			{
				// printf("consumer = %s,done = %d, cycles = %d\n",
				//    SynthModules[i]->getClassName(),done[i],cycles);

				/* a module whose input is not comsumed from other modules
					is a "push delivery" style module, such as speakers,
					or writing audio to log file, etc. and has to get
					external requests from the scheduling system */

				if(done[k] != cycles)
					done[k] += current->request(cycles-done[k]);
				assert(done[k] <= cycles);
				if(done[k] != cycles) incomplete++;
				//printf("*scheduler*\n");
				died ++;
				if(died > 10000)
				{
					free(done);
					return;
					// return false;
				}
			}
		}
	} while(incomplete);

	//printf("<=> done!!\n");
	free(done);
	// return true;
#endif
}
