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
#include <algorithm>
#include <stdio.h>

using namespace Arts;
using namespace std;

#undef VPORT_DEBUG

/* virtual port connections */

/*

Port virtualization is used in the following case: suppose you have a module
M which has an input port and an output port like that:

input
  V
-----
  M
-----
  V
output

But suppose the module M is implement as combination of other modules, for
instance the effect of M is achieved by passing the signal first through an
A and then through an B module. Clients connecting M should "really" connect
A,B. For this virtualization is used.

There are two kinds:

- masquerading: which means (like in our case), the input of a module is really
  implemented with the input of another module. So M's input could be really
  implemented by A's input

  there is also output masquerading, which would be for instance that M's
  output is really implemented by B's output

- forwarding: if M in our example would choose to do nothing at all, it could
  simply forward its input to its output


The interface for the user:

MCOP will show the virtualize function to the user, which the user can use
to delegate the services of a port to another module onto another port.

- masquerading: in our case, for instance the user could call

    m._node()->virtualize("inputport",a._node(),"inputport");

  which would forward all input m gets on "inputport" to a's "inputport"

- forwarding: in the same way, the user could call

    m._node()->virtualize("inputport",m._node(),"outputport");

  which would make m forward its input directly to its output

The implementation:

Virtualization is implemented here, inside the flow system, using a fairly
complex forwarding strategy, which will have a graph which contains

- "user-made" connections (made with connect())
- "virtualize-made" connections, which can be either forwarding
  (input to output port) or masquerading (input to input or output to output)
 
Out of all these, the algorithm builds "real" connections, which are
then really performed inside the flow system. If you change the "user-made"
or "virtualize-made" connections, the "real" connections are recalculated.

The "real" connections are created by the expandHelper function. They are
called vcTransport here.

The strategy expandHelper uses is to go back to a port which is only output
port (non forwarded, non masqueraded), and then follow the graph recursively
over vcMasquerade and vcForward edges until it reaches a port which is only
input. Then it creates a real connection.

Some tweaks are there which allow that not on any change at the graph, all
real connections will be removed, but only these that could possibly be
affected by this change, and then not all real connections are created new,
but only those that could possibly be created by this virtual connection.

Every VPort contains a pointer to the "real" port, to let the flow system
know where the "real" connections where real data will flow must be made.

*/

VPortConnection::VPortConnection(VPort *source, VPort *dest, Style style)
		:source(source),dest(dest),style(style)
{
	if(style != vcTransport)
	{
		list<VPortConnection *>::iterator i;

		// remove transport connections ending at "source" (they will 
		// probably be forwarded/masqueraded elsewhere by this VPortConnection)
		i = source->incoming.begin();
		while(i != source->incoming.end())
		{
			if((*i)->style == vcTransport)
			{
				delete *i;
				i = source->incoming.begin();
			}
			else i++;
		}

		// remove transport connections starting at "dest" (they will 
		// probably be forwarded/masqueraded elsewhere by this VPortConnection)
		i = dest->outgoing.begin();
		while(i != dest->outgoing.end())
		{
			if((*i)->style == vcTransport)
			{
				delete *i;
				i = dest->outgoing.begin();
			}
			else i++;
		}
	}

	// add to the connection lists
	source->outgoing.push_back(this);
	dest->incoming.push_back(this);

	if(style == vcTransport)
	{
#ifdef VPORT_DEBUG
		arts_debug("emit a connection consumer = %s, producer = %s",
			dest->name(), source->name());
#endif
		dest->port->connect(source->port);
	}
	else
	{
		source->makeTransport(this);
	}
}

VPortConnection::~VPortConnection()
{
#ifdef VPORT_DEBUG
	cout << "~VPortConnection" << endl;
#endif

	if(style != vcTransport)
	{
		// remove transport connection which go through this connection
		source->removeTransport(this);
	}

	// remove this connection from the lists
	list<VPortConnection *>::iterator ci;

	ci = find(source->outgoing.begin(),source->outgoing.end(),this);
	assert(ci != source->outgoing.end());
	source->outgoing.erase(ci);

	ci = find(dest->incoming.begin(),dest->incoming.end(),this);
	assert(ci != dest->incoming.end());
	dest->incoming.erase(ci);

	if(style == vcTransport)
	{
#ifdef VPORT_DEBUG
		arts_debug("delete connection %s -> %s",dest->name(), source->name());
#endif
		dest->port->disconnect(source->port);
	}

	// reestablish all connections which started/ended here before
	if(style != vcTransport)
	{
		list<VPortConnection *>::iterator i;
		stack<VPortConnection *> todo;

		// reestablish transport connections which were ending at source...
		for(i = source->incoming.begin(); i != source->incoming.end(); i++)
			if((*i)->style != vcTransport) todo.push(*i);

		// ... and starting at dest
		for(i = dest->outgoing.begin(); i != dest->outgoing.end(); i++)
			if((*i)->style != vcTransport) todo.push(*i);

		// we need to do this with the stack as makeTransport can affect the
		// incoming/outgoing lists by adding new vcTransport connections
		while(!todo.empty())
		{
			todo.top()->source->makeTransport(todo.top());
			todo.pop();
		}
	}

#ifdef VPORT_DEBUG
	cout << "~VPortConnection done" << endl;
#endif
}

/*---------------------- virtual port implementation ----------------------*/

VPort::VPort(Port *port) :port(port)
{
#ifdef VPORT_DEBUG
	cout << "VPort: " << name() << endl;
#endif
}

VPort::~VPort()
{
#ifdef VPORT_DEBUG
	cout << "~VPort: " << name() << endl;
#endif
	while(!incoming.empty()) delete *incoming.begin();
	while(!outgoing.empty()) delete *outgoing.begin();
#ifdef VPORT_DEBUG
	cout << "~VPort done" << endl;
#endif
}

bool VPort::makeVirtualizeParams(VPort *forward, VPort*& source, VPort*& dest,
	VPortConnection::Style& style)
{
	source = dest = 0;
	// masquerading
	if((port->flags() & streamIn) && (forward->port->flags() & streamIn))
	{
		// input: data flow direction is from us to the "forward" port
		// XXX?
		source = this;
		dest = forward;
		style = VPortConnection::vcMasquerade;
	}
	else if((port->flags() & streamOut) && (forward->port->flags() & streamOut))
	{
		// output: data flow direction is from the "forward" port to us
		// XXX?
		source = forward;
		dest = this;
		style = VPortConnection::vcMasquerade;
	}
	// forwarding
	else if((port->flags() & streamIn) && (forward->port->flags() & streamOut))
	{
		source = this;
		dest = forward;
		style = VPortConnection::vcForward;
	}
	else if((port->flags() & streamOut) && (forward->port->flags() & streamIn))
	{
		source = forward;
		dest = this;
		style = VPortConnection::vcForward;
	}
	return source != 0;
}

/**
 * a->virtualize(b) means, that the functionality that port a should provide
 * (e.g. produce or consume data) is really provided by port b
 */
void VPort::virtualize(VPort *forward)
{
	VPort *source, *dest;
	VPortConnection::Style style;

	if(makeVirtualizeParams(forward,source,dest,style))
	{
#ifdef VPORT_DEBUG
		cout << "virtualize ... source (producer) is " << source->name() <<
				" dest (consumer) is " << dest->name() << endl;
#endif
		new VPortConnection(source,dest,style);
	}
}

void VPort::devirtualize(VPort *forward)
{
	VPort *source, *dest;
	VPortConnection::Style style;

	// XXX?
	if(makeVirtualizeParams(forward,source,dest,style))
	{
		list<VPortConnection *>::iterator i;
		for(i = source->outgoing.begin(); i != source->outgoing.end(); i++)
		{
			if((*i)->source == source && (*i)->dest == dest
			&& (*i)->style == style)
			{
				delete (*i);
				return;
			}
		}
	}
}

void VPort::setFloatValue(float value)
{
	if(outgoing.empty())
	{
		AudioPort *aport = port->audioPort();
		assert(aport);
		aport->setFloatValue(value);
	}
	else
	{
		list<VPortConnection *>::iterator i;
		for(i=outgoing.begin();i != outgoing.end(); i++)
		{
			VPortConnection *conn = *i;
			assert(conn->style == VPortConnection::vcMasquerade);

			conn->dest->setFloatValue(value);
		}
	}
}

void VPort::connect(VPort *dest)
{
	VPortConnection *conn;
	if(port->flags() & streamOut)
	{
		conn = new VPortConnection(this,dest,VPortConnection::vcConnect);
	}
	else
	{
		conn = new VPortConnection(dest,this,VPortConnection::vcConnect);
	}
}

void VPort::disconnect(VPort *dest)
{
	if(port->flags() & streamOut)
	{
		list<VPortConnection *>::iterator ci = outgoing.begin();
		while(ci != outgoing.end())
		{
			assert((*ci)->source == this);
			if((*ci)->dest == dest && (*ci)->style==VPortConnection::vcConnect)
			{
				delete (*ci);	// will remove itself from the outgoing list
				return;
			}
			ci++;
		}
	}
	else
	{
		if(dest->port->flags() & streamOut)
		{
			dest->disconnect(this);
			return;
		}
	}
}

void VPort::expandHelper(VPortConnection *conn, int state, VPort *current,
									VPort *source, VPort *dest, bool remove)
{
	list<VPortConnection *>::iterator ci;

#ifdef VPORT_DEBUG
	cout << "expandhelper state " << state << " name " << current->name() << endl;
#endif

	if(state == 1)				/* state 1: scan backward for output ports */
	{
		if(current->incoming.empty())
		{
			if(current->port->flags() & streamOut)
				expandHelper(conn,2,current,current,dest,remove);
		}
		for(ci = current->incoming.begin(); ci != current->incoming.end();ci++)
		{
			assert((*ci)->style != VPortConnection::vcTransport);
			expandHelper(conn,1,(*ci)->source,source,dest,remove);
		}
	}
	else if(state == 2)			/* state 2: output port expansion */
	{
		assert(current->port->flags() & streamOut);

		for(ci = current->outgoing.begin(); ci != current->outgoing.end();ci++)
		{
			/* xconn=0 ensures that only paths are counted which contain conn */
			VPortConnection *xconn = conn;
			if(*ci == conn) xconn = 0;

			if((*ci)->style == VPortConnection::vcMasquerade)
			{
				expandHelper(xconn,2,(*ci)->dest,source,dest,remove);
			}
			else if((*ci)->style == VPortConnection::vcConnect)
			{
				expandHelper(xconn,3,(*ci)->dest,source,(*ci)->dest,remove);
			}
		}
	}
	else if(state == 3)			/* state 3: input port expansion */
	{
		assert(current->port->flags() & streamIn);

		for(ci = current->outgoing.begin(); ci != current->outgoing.end();ci++)
		{
			/* xconn=0 ensures that only paths are counted which contain conn */
			VPortConnection *xconn = conn;
			if(*ci == conn) xconn = 0;	

			if((*ci)->style == VPortConnection::vcMasquerade)
			{
				// XXX ?
				expandHelper(xconn,3,(*ci)->dest,source,(*ci)->dest,remove);
			}
			else if((*ci)->style == VPortConnection::vcForward)
			{
				expandHelper(xconn,2,(*ci)->dest,source,dest,remove);
			}
		}

		if(current->outgoing.empty() && conn == 0)
		{
			if(remove)
			{
				// delete exactly one transport connection

				bool removeOk = false;
				ci = current->incoming.begin();
				while(ci != current->incoming.end() && !removeOk)
				{
					if((*ci)->source == source && (*ci)->dest == dest
						&& (*ci)->style == VPortConnection::vcTransport)
					{
						delete (*ci);
						removeOk = true;
					}
					else ci++;
				}
				assert(removeOk);
			}
			else
			{
				new VPortConnection(source,dest,VPortConnection::vcTransport);
			}
		}
	}
}

void VPort::makeTransport(VPortConnection *conn)
{
	expandHelper(conn,1,this,0,0,false);
}

void VPort::removeTransport(VPortConnection *conn)
{
	expandHelper(conn,1,this,0,0,true);
}

const char *VPort::name()
{
	if(_name == "")
	{
		_name = port->parent->object()->_interfaceName() + "." +
				port->name();
	}
	return _name.c_str();
}
