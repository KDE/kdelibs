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

#ifndef OBJECT_H
#define OBJECT_H

#include "buffer.h"
#include "connection.h"
#include "flowsystem.h"

#include <map>

class ScheduleNode;
class Object_skel;

class Object {
protected:
	Object();
	virtual ~Object();

	/*
	 * internal management for streams
	 */
	ScheduleNode *_scheduleNode;
	map<string, void*> _streamMap;

	virtual Object_skel *_skel();

	long _refCnt;		// reference count
	bool _deleteOk;		// ensure that "delete" is not called manually
	static long _staticObjectCount;

public:
	/*
	 * generic capabilities, which allow find out what you can do with an
	 * object even if you don't know it's interface
	 */
	virtual long _lookupMethod(const class MethodDef &) = 0;
	virtual string _interfaceName() = 0;
	virtual class InterfaceDef* _queryInterface(const string& name) = 0;
	virtual class TypeDef* _queryType(const string& name) = 0;
	virtual string _toString() = 0;

	/*
	 * stuff for streaming (put in a seperate interface?)
	 */
	void *_lookupStream(string s);
	virtual void calculateBlock(unsigned long cycles);
	ScheduleNode *_node();

	/*
	 * reference counting
	 */
	virtual void _release() = 0;
	virtual void _copyRemote() = 0;
	virtual void _useRemote() = 0;
	virtual void _releaseRemote() = 0;

	inline Object *_copy() {
		assert(_refCnt > 0);
		_refCnt++;
		return this;
	}

	inline static long _objectCount() { return _staticObjectCount; }

	// static converter (from reference)
	static Object *_fromString(string objectref);
};

template<class T> class ReferenceHelper;

typedef ReferenceHelper<Object> Object_var;

/*
 * Dispatching
 */

class Buffer;
class MethodDef;

typedef void (*DispatchFunction)(void *object, Buffer *request, Buffer *result);

class Object_skel : virtual public Object {
private:
	struct MethodTableEntry;

	long _objectID;
	bool _methodTableInit;
	vector<struct MethodTableEntry> _methodTable;

protected:
	void _addMethod(DispatchFunction disp, void *object, const MethodDef& md);
	void _initStream(string name, void *ptr, long flags);

	Object_skel *_skel();

public:
	Object_skel();
	virtual ~Object_skel();

	void _dispatch(Buffer *request, Buffer *result,long methodID);
	long _lookupMethod(const MethodDef &);

	/*
	 * standard interface for every object skeleton
	 */
	static string _interfaceNameSkel();
	virtual void _buildMethodTable();
	virtual void *_cast(string interface) = 0;

	/*
	 * reference counting
	 */
	virtual void _release();
	virtual void _copyRemote();
	virtual void _useRemote();
	virtual void _releaseRemote();

	/*
	 * to inspect the (remote) object interface
	 */
	virtual string _interfaceName();
	InterfaceDef* _queryInterface(const string& name);
	TypeDef* _queryType(const string& name);
	virtual string _toString();
};

class Object_stub : virtual public Object {
protected:
	friend class Object;

	long _objectID,_lookupCacheRandom;
	Connection *_connection;

	Object_stub();
	Object_stub(Connection *connection, long objectID);
	virtual ~Object_stub();

	enum { _lookupMethodCacheSize = 337 };
	static long *_lookupMethodCache;

	long _lookupMethodFast(const char *method);
	long _lookupMethod(const MethodDef &);
public:

	/*
	 * to inspect the (remote) object interface
	 */
	string _interfaceName();
	InterfaceDef* _queryInterface(const string& name);
	TypeDef* _queryType(const string& name);
	string _toString();

	/*
	 * reference counting
	 */
	virtual void _release();
	virtual void _copyRemote();
	virtual void _useRemote();
	virtual void _releaseRemote();
};
#endif
