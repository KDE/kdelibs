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

#include "dynamicrequest.h"
#include "core.h"
#include "debug.h"
#include <iostream.h>

using namespace Arts;
using namespace std;

class Arts::DynamicRequestPrivate {
public:
	Connection *connection;
	Buffer *buffer;
	MethodDef method;
	Object object;

	/**
	 * methodID is kept between the requests, so that we don't need to lookup
	 * a method again when it gets called twice - the value for "we need to
	 * lookup again" is -1, which gets set whenever the current request differs
	 * from the last
	 */
	long requestID, methodID, objectID;
	unsigned long paramCount;

	/**
	 * constructor
	 */
	DynamicRequestPrivate(const Object &obj)
		: buffer(0), object(obj), methodID(-1)
	{
	}
};

DynamicRequest::DynamicRequest(const Object& obj)
{
	d = new DynamicRequestPrivate(obj);
	d->connection = obj._base()->_getConnection();
	d->objectID = obj._base()->_getObjectID();
}

DynamicRequest::~DynamicRequest()
{
	delete d;
}

DynamicRequest& DynamicRequest::method(const string& method)
{
	assert(!d->buffer);

	// methodID will be set later
	d->buffer = Dispatcher::the()->createRequest(d->requestID, d->objectID, 0);

	if(d->method.name != method)
	{
		d->method.name = method;
		d->methodID = -1;
	}
	d->paramCount = 0;
	return *this;
}

DynamicRequest& DynamicRequest::param(const AnyConstRef& ref)
{
	if(d->paramCount == d->method.signature.size())
	{
		ParamDef pd;
		pd.type = ref.type();
		d->method.signature.push_back(pd);
	}
	else
	{
		if(d->method.signature[d->paramCount].type != ref.type())
		{
			d->method.signature[d->paramCount].type = ref.type();
			d->methodID = -1;
		}
	}
	d->paramCount++;
	ref.write(d->buffer);
	return *this;
}

bool DynamicRequest::invoke()
{
	AnyRef voidReference;
	return invoke(voidReference);
}

bool DynamicRequest::invoke(const AnyRef& returnCode)
{
	if(d->method.type != returnCode.type())
	{
		d->method.type = returnCode.type();
		d->methodID = -1;
	}
	if(d->method.signature.size() != d->paramCount)
		d->methodID = -1;

	/*
	 * need to lookup method? (if the requested method is exactly the
	 * same as the last, we need not, which signigicantly improves performance
	 */
	if(d->methodID == -1)
	{
		d->method.signature.resize(d->paramCount);
		d->methodID = d->object._lookupMethod(d->method);

		if(d->methodID == -1)
		{
			arts_warning("DynamicRequest: invalid method called");
			return false;
		}
	}


	d->buffer->patchLength();
	d->buffer->patchLong(16,d->methodID);
	d->connection->qSendBuffer(d->buffer);
	d->buffer = 0;

	Buffer *result =
		Dispatcher::the()->waitForResult(d->requestID,d->connection);

	if(result)
	{
		returnCode.read(result);
		delete result;
	}
	return result != 0;
}
