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

#ifndef MCOP_DYNAMICREQUEST_H
#define MCOP_DYNAMICREQUEST_H

#include "buffer.h"
#include "anyref.h"
#include <vector>
#include <string>

/*
 * BC - Status (2000-09-30): DynamicRequest
 *
 * Has to be kept binary compatible (use d ptr).
 */

namespace Arts {

class Object;
class Type;
class DynamicRequestPrivate;

/**
 * The DynamicRequest class can be used to invoke requests on objects, without
 * using IDL generated code to do so (i.e. you can talk to objects without
 * having to know their interfaces at compile time)
 *
 * Suppose you have the interface
 *
 * interface SimpleSoundServer {
 *   [...]
 *   long play(string file);	// plays a file and returns an id
 *   [...]
 * };
 *
 * And server is of type SimpleSoundServer, you can write in your C++ code:
 *
 *   long id;
 *   if(DynamicRequest(server).method("play").param("/tmp/bong.wav").invoke(id))
 *   {
 *     cout << "playing file now, id is " << id << endl;
 *   }
 *   else
 *   {
 *     cout << "something went wrong with the dynamic request" << endl;
 *   }
 *
 * You can of course also add parameters and other information one-by-one:
 *
 *   DynamicRequest request(server);
 *   request.method("play");
 *   request.param("/tmp/bong.wav");
 *
 *   long id;
 *   if(request.invoke(id)) cout << "success" << endl;
 */

class DynamicRequest {
public:
	/**
	 * creates a dynamic request which will be sent to a specific object
	 */
	DynamicRequest(const Object& object);

	/**
	 * deletes the dynamic request
	 */
	~DynamicRequest();

	/**
	 * says that the following method will be a oneway method, for example
	 *
	 * DynamicRequest(someobject).oneway().method("stop").invoke();
	 */
	DynamicRequest& oneway();

	/**
	 * sets the method to invoke
	 */
	DynamicRequest& method(const std::string& method);

	/**
	 * adds a parameter to the call
	 */
	DynamicRequest& param(const AnyConstRef& value);

	/**
	 * executes the request, call this if you don't expect a return type
	 * 
	 * @returns true if the request could be performed
	 */
	bool invoke();

	/**
	 * executes the request: this version accepts an AnyRef& as result type
	 *
	 * @returns true if the request could be performed
	 */
	bool invoke(const AnyRef& result);

	/*
	 * TODO: Some types can't yet be used in dynamic requests, these are
	 * enum, sequence<enum>, type, sequence<type>, object, sequence<object>
	 */
private:
	DynamicRequestPrivate *d;
};

};

#endif /* MCOP_DYNAMICREQUEST_H */
