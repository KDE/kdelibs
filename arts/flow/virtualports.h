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


#ifndef VIRTUALPORTS_H
#define VIRTUALPORTS_H

#include "synthschedule.h"
#include <list>

/*
 * BC - Status (2000-09-30): VPortConnection, VPort.
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of libartsflow's
 * StdFlowSystem, and subject to change with the needs of it.
 */

namespace Arts {

class VPortConnection {
private:
	friend class VPort;
	VPort *source;
	VPort *dest;

public:
	enum Style { vcForward, vcMasquerade, vcConnect, vcTransport } style;
	VPortConnection(VPort *source, VPort *dest, Style style);
	~VPortConnection();
};

class VPort {
private:
	friend class VPortConnection;
protected:
	Port *port;
	std::string _name;
	std::list<VPortConnection *> incoming, outgoing;

	void makeTransport(VPortConnection *conn);
	void removeTransport(VPortConnection *conn);
	void expandHelper(VPortConnection *conn, int state, VPort *current,
								VPort *source, VPort *dest, bool remove);

	bool makeVirtualizeParams(VPort *forward, VPort *& source, VPort *& dest,
									VPortConnection::Style &style);

	const char *name();
public:
	VPort(Port *p);
	~VPort();

	void setFloatValue(float value);

	void connect(VPort *vport);
	void disconnect(VPort *port);

	/**
	 * a->virtualize(b) means that the functionality that port a should provide
	 * (produce or consume data) is really provided by port b
	 */
	void virtualize(VPort *port);
	void devirtualize(VPort *port);
};
};

#endif /* VIRTUALPORTS_H */
