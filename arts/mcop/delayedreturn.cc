    /*

    Copyright (C) 2001 Stefan Westerfeld
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

#include <assert.h>
#include "anyref.h"
#include "connection.h"
#include "delayedreturn.h"

using namespace Arts;
using namespace std;

DelayedReturn::DelayedReturn() : buffer(0), connection(0)
{
	//
}

void DelayedReturn::initialize(Connection *connection, Buffer *buffer)
{
	this->connection = connection;
	this->buffer = buffer;
	connection->_copy();
}

DelayedReturn::~DelayedReturn()
{
	connection->_release();
	connection = 0;
	buffer = 0;
}

void DelayedReturn::doReturn(const AnyConstRef& value)
{
	assert(connection);
	assert(buffer);

	value.write(buffer);
	buffer->patchLength();
	connection->qSendBuffer(buffer);

	delete this;
}

void DelayedReturn::doReturn()
{
	assert(connection);
	assert(buffer);

	buffer->patchLength();
	connection->qSendBuffer(buffer);

	delete this;
}
