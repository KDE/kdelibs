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

/*
 * BC - Status (2000-09-30): Reference, SubClass, Object, DynamicCast.
 *
 * Part of the public API. Must be kept binary compatible by NOT TOUCHING
 * AT ALL. Interaction with generated and written code.
 */


#ifndef MCOP_REFERENCE_H
#define MCOP_REFERENCE_H

#include "common.h"
#include <string>
#include <vector>

namespace Arts {

// Acts as a string or ObjectReference, but enables a different constructor
class Reference {
private:
	ObjectReference myref;
	std::string mys;
	bool strMode;
public:
	
	inline explicit Reference(const ObjectReference& ref) {
		myref=ref;
		strMode=false;
	}

	inline explicit Reference(const std::string& s) {
		mys=s;
		strMode=true;
	}
	
	inline explicit Reference(const char* c) {
		mys=c;
		strMode=true;
	}
	inline Reference& operator=(const std::string& s) {
		mys=s;
		strMode=true;
		return *this;
	}
	inline Reference& operator=(const char*c) {
		mys=c;
		strMode=true;
		return *this;
	}
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

	inline explicit SubClass(const std::string& s) : mys(s) {}
	inline explicit SubClass(const char* c) : mys(c) {}
	inline SubClass& operator=(const std::string& s) {mys=s; return *this;}
	inline SubClass& operator=(const char*c) {mys=c; return *this;}
//	inline operator const std::string() const {return mys;}
	inline const std::string& string() const {return mys;}
};


class ScheduleNode;
class FlowSystem;

// Object has _no_ virtual, and must not have. That way, all the
// wrappers can be passed as argument or return type, and there is also
// no virtual table bloat.
// Moreover, the virtual mechanism still works correctly thanks to the
// _pool->base redirection.
class Object {
protected:
	// Pool of common variables for a bunch a wrappers
	class Pool {
		friend class Object;
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

	inline Object(Object_base* (*cor)()) {
		_pool = new Pool(cor);
	}
	inline Object(Pool* p) : _pool(p) {
		_pool->Inc();
	}
	inline Object(Pool& p) : _pool(&p) {
		_pool->Inc();
	}
	inline Object(Object_base* b) {
		_pool = new Pool(b);
	}
public:
	typedef Object_base _base_class;

	// Dynamic cast constructor of inherited classes needs to access the _pool
	// of a generic object if casting successful. But it could not without this
	inline Pool* _get_pool() const {return _pool;}
	
	inline ~Object() {
		_pool->Dec();
	}
	
	// Those constructors are public, since we don't need an actual creator.
	// They enable generic object creation (like from a subclass defined at
	// run-time!)
	inline Object(const SubClass& s) {
		_pool = new Pool(Object_base::_create(s.string()));
	}
	inline Object(const Reference &r) {
		_pool = new Pool(r.isString()?(Object_base::_fromString(r.string())):(Object_base::_fromReference(r.reference(),true)));
	}
	inline Object(const Object& target) : _pool(target._pool) {
		_pool->Inc();
	}
	inline Object() { // creates a null object
		_pool = new Pool((Object_base*)0);
	}
	inline Object& operator=(const Object& target) {
		if (_pool == target._pool) return *this;
		_pool->Dec();
		_pool = target._pool;
		_pool->Inc();
		return *this;
	}
	// No problem for the creator, this class has protected constructors.
	// So creator should give back an actual implementation
	inline Object_base* _base() const {
		_pool->checkcreate();
		return _pool->base;
	}

	// null, error?
	inline bool isNull() const {
		_pool->checkcreate();
		return !(_pool->base);
	}
	inline bool error() const {
		_pool->checkcreate();
    	return _pool->base && _pool->base->_error();
	}
	
	// Comparision
	inline bool _isEqual(const Object& other) const {
		if(isNull() != other.isNull()) return false;

		// we can assume that things are created here, as we've
		// called isNull of both wrappers once
		if(!isNull())
			return _pool->base->_isEqual(other._pool->base);

		// both null references
		return true;
	}


// Object_base wrappers

	// Custom messaging - see Object_base for comments
	inline Buffer *_allocCustomMessage(long handlerID) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_allocCustomMessage(handlerID);
	}

	inline void _sendCustomMessage(Buffer *data) const {
		_pool->checkcreate();
		assert(_pool->base);
		_pool->base->_sendCustomMessage(data);
	}
	
	// generic capabilities, which allow find out what you can do with an
	// object even if you don't know it's interface
	inline long _lookupMethod(const MethodDef& methodDef) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_lookupMethod(methodDef);
	}
	inline std::string _interfaceName() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_interfaceName();
	}
	inline InterfaceDef _queryInterface(const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_queryInterface(name);
	}
	inline TypeDef _queryType(const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_queryType(name);
	}
	inline EnumDef _queryEnum(const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_queryEnum(name);
	}
	// Stringification
	inline std::string _toString() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_toString();
	}
	inline std::string toString() const {return _toString();}

	// stuff for streaming (put in a seperate interface?)
	inline void calculateBlock(unsigned long cycles) const {
		_pool->checkcreate();
		assert(_pool->base);
		_pool->base->calculateBlock(cycles);
	}
	// Node info
	inline ScheduleNode *_node() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_node();
	}
	
	// Ah! Flowsystem is not defined yet, so cannot be returned inline.
	FlowSystem _flowSystem() const;
	
	inline void _copyRemote() const {
		_pool->checkcreate();
		assert(_pool->base);
		_pool->base->_copyRemote();
	}
	inline void _useRemote() const {
		_pool->checkcreate();
		assert(_pool->base);
		_pool->base->_useRemote();
	}
	inline void _releaseRemote() const {
		_pool->checkcreate();
		assert(_pool->base);
		_pool->base->_releaseRemote();
	}
	
	// Default I/O info
	inline std::vector<std::string> _defaultPortsIn() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_defaultPortsIn();
	}
	inline std::vector<std::string> _defaultPortsOut() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_defaultPortsOut();
	}
	
	// aggregation
	inline std::string _addChild(Arts::Object child, const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_addChild(child, name);
	}
	inline bool _removeChild(const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_removeChild(name);
	}
	inline Arts::Object _getChild(const std::string& name) const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_getChild(name);
	}
	inline std::vector<std::string> * _queryChildren() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_queryChildren();
	}
	// Do we really need those in the Wrapper?
	// And would it really be sensible to make _cast wrappers?
/*	inline void _release() const {
		_pool->checkcreate();
		assert(_pool->base);
		return _pool->base->_release();
	}
	inline Object_base *_copy() const {
		_poo->checkcreate();
		assert(_pool->base);
		return _pool->base->_copy();
	}
*/

	// Object::null() returns a null object (and not just a reference to one)
	inline static Object null() {return Object((Object_base*)0);}
	inline static Object _from_base(Object_base* b) {return Object(b);}
};

// Enables a different constructor, that should do the cast
class DynamicCast {
private:
	Object obj;
public:
	inline explicit DynamicCast(const Object& o) : obj(o) {}
	inline const Object& object() const {return obj;}
};

};

#endif
