/*
	Copyright (C) 2000 Nicolas Brodu, nicolas.brodu@free.fr
	                   Stefan Westerfeld, stefan@space.twc.de

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

#ifndef MCOP_COMPONENT_REFERENCE_H
#define MCOP_COMPONENT_REFERENCE_H

#include "common.h"
#include <string>
#include <vector>

// Acts as a string or ObjectReference, but enables a different constructor
class Reference {
private:
	ObjectReference myref;
	std::string mys;
	bool strMode;
public:
	
	inline Reference(const ObjectReference& ref) {
		myref=ref;
		strMode=false;
	}

	inline Reference(const std::string& s) {
		mys=s;
		strMode=true;
	}
	
	inline Reference(const char* c) {
		mys=c;
		strMode=true;
	}
	inline Reference& operator=(const std::string& s) {mys=s; return *this;}
	inline Reference& operator=(const char*c) {mys=c; return *this;}
	inline bool isString() const {return strMode;}
//	inline operator const std::string() const {return mys;}
	inline const std::string& string() const {return mys;}
//	inline operator const ObjectReference() const {return myref;}
	inline const ObjectReference& reference() const {return myref;}
};

// Acts as a string, but enables a different constructor
class SubClass {
private:
	std::string mys;
public:

	inline SubClass(const std::string& s) : mys(s) {}
	inline SubClass(const char* c) : mys(c) {}
	inline SubClass& operator=(const std::string& s) {mys=s; return *this;}
	inline SubClass& operator=(const char*c) {mys=c; return *this;}
//	inline operator const std::string() const {return mys;}
	inline const std::string& string() const {return mys;}
};

class ScheduleNode;

// SmartWrapper has _no_ virtual, and must not have. That way, all the
// wrappers can be passed as argument or return type, and there is also
// no virtual table bloat.
// Moreover, the virtual mechanism still works correctly thanks to the
// _pool->base redirection.
// No check is done here, only what's necessary in the child classes
// To inforce this, the constructors are protected
class SmartWrapper {
protected:
	
	// Pool of common variables for a bunch a wrappers
	class Pool {
		Object_base* (*creator)();
		bool created;
		int count;
	public:
		Object_base* base;
		inline Pool(Object_base* b)
			: creator(0), created(true), count(1), base(b) {}
		inline Pool(Object_base* (*cor)())
			: creator(cor), created(false), count(1), base(0) {}
		inline void Inc() {count++;}
		inline void Dec() {
			if (--count==0) {
				if(base) base->_release();
				delete this;
			}
		}
		inline void checkcreate() {
			if (!created) {base = creator(); created=true;}
		}
	} *_pool;

	inline SmartWrapper(Object_base* (*cor)()) {
		_pool = new Pool(cor);
	}
	inline SmartWrapper(Object_base* b) {
		_pool = new Pool(b);
	}
	inline SmartWrapper(Pool* p) : _pool(p) {
		_pool->Inc();
	}
public:
	// null, error?
	inline bool isNull() const {
		_pool->checkcreate();
		return !(_pool->base);
	}
	inline bool error() const {
		_pool->checkcreate();
    	return _pool->base && _pool->base->_error();
	}
	
	// Default I/O info
	inline vector<std::string> defaultPortsIn() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_defaultPortsIn();
	}
	inline vector<std::string> defaultPortsOut() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_defaultPortsOut();
	}
	// Node info
	inline ScheduleNode *node() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_node();
	}
	
	inline std::string toString() const {return _pool->base->_toString();}
};

#endif
