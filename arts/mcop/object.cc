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

#include "object.h"
#include "dispatcher.h"
#include <stdio.h>

struct Object_skel::MethodTableEntry {
	DispatchFunction dispatcher;
	void *object;
	MethodDef methodDef;
};

/*
 * Object: common for every object
 */

Object::Object()
{
	_scheduleNode = 0;
}

Object::~Object()
{
	if(_scheduleNode)
	{
		FlowSystem *fs = Dispatcher::the()->flowSystem();
		assert(fs);

		fs->removeObject(_scheduleNode);
	}
}

ScheduleNode *Object::_node()
{
	if(!_scheduleNode)
	{
		FlowSystem *fs = Dispatcher::the()->flowSystem();
		assert(fs);

		_scheduleNode = fs->addObject(_skel());
		assert(_scheduleNode);
	}
	return _scheduleNode;
}

Object_skel *Object::_skel()
{
	assert(false);
	return 0;
}

Object_skel *Object_skel::_skel()
{
	return this;
}

void Object_skel::_initStream(string name, void *ptr, long flags)
{
	_streamMap[name] = ptr;
	_node()->initStream(name,ptr,flags);
}

void *Object::_lookupStream(string name)
{
	return _streamMap[name];
}

void Object::calculateBlock(unsigned long)
{
}

string Object::_interfaceName()
{
	assert(0); // derived classes *must* override this
	return "";
}


/*
 * Stuff for object skeletons
 */

Object_skel::Object_skel()
{
	_objectID = Dispatcher::the()->addObject(this);
	_methodTableInit = false;
}

Object_skel::~Object_skel()
{
	Dispatcher::the()->removeObject(_objectID);
}

string Object_skel::_toString()
{
	return Dispatcher::the()->objectToString(_objectID);
}

string Object_skel::_interfaceName()
{
	assert(0); // derived classes *must* override this
	return "";
}

string Object_skel::_interfaceNameSkel()
{
	assert(0); // derived classes *must* override this
	return "";
}


InterfaceDef* Object_skel::_queryInterface(const string& name)
{
	return Dispatcher::the()->interfaceRepo()->queryInterface(name);
}

TypeDef* Object_skel::_queryType(const string& name)
{
	return Dispatcher::the()->interfaceRepo()->queryType(name);
}

void Object_skel::_addMethod(DispatchFunction disp, void *obj,
                                               const MethodDef& md)
{
	MethodTableEntry me;
	me.dispatcher = disp;
	me.object = obj;
	me.methodDef = md;
	_methodTable.push_back(me);
}

void Object_skel::_dispatch(Buffer *request, Buffer *result,long methodID)
{
	if(!_methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_methodTableInit = true;
	}
	_methodTable[methodID].dispatcher(_methodTable[methodID].object,
														request,result);
}

long Object_skel::_lookupMethod(const MethodDef& md)
{
	long mcount = 0;

	assert(_methodTableInit);

	vector<MethodTableEntry>::iterator i;
	for(i=_methodTable.begin(); i != _methodTable.end(); i++)
	{
		MethodDef& mdm = i->methodDef;
		if(mdm.name == md.name && mdm.type == md.type)
		{
		/*	TODO: compare signature
			vector<ParamDef *>::const_iterator j,k;
			j = md.signature.begin();
			k = mdm.signature.begin();
			while(j != md.signature.end() && (*j) == (*k))
			{
				j++;
				k++;
			}

			if(j == md.signature.end() && k == mdm.signature.end())
			*/
				return mcount;
		}
		mcount++;
	}
	return -1;
}

// _lookupMethod
static void _dispatch_Object_00(void *object, Buffer *request, Buffer *result)
{
	MethodDef methodDef(*request);
	result->writeLong(((Object_skel *)object)->_lookupMethod(methodDef));
}

// _interfaceName
static void _dispatch_Object_01(void *object, Buffer *, Buffer *result)
{
	result->writeString(((Object_skel *)object)->_interfaceName());
}

// _queryInterface
static void _dispatch_Object_02(void *object, Buffer *request, Buffer *result)
{
	string name;
	request->readString(name);
	InterfaceDef *_returnCode = ((Object_skel *)object)->_queryInterface(name);
	_returnCode->writeType(*result);
	delete _returnCode;
}

// _queryType
static void _dispatch_Object_03(void *object, Buffer *request, Buffer *result)
{
	string name;
	request->readString(name);
	TypeDef *_returnCode = ((Object_skel *)object)->_queryType(name);
	_returnCode->writeType(*result);
	delete _returnCode;
}

void Object_skel::_buildMethodTable()
{
	Buffer m;
	m.fromString("MethodTable:0e0000005f6c6f6f6b75704d6574686f6400050000006c6f6e670000000000010000000a0000004d6574686f64446566000a0000006d6574686f64446566000f0000005f696e746572666163654e616d650007000000737472696e67000000000000000000100000005f7175657279496e74657266616365000d000000496e7465726661636544656600000000000100000007000000737472696e6700050000006e616d65000b0000005f71756572795479706500080000005479706544656600000000000100000007000000737472696e6700050000006e616d6500","MethodTable");
	_addMethod(_dispatch_Object_00,this,MethodDef(m));
	_addMethod(_dispatch_Object_01,this,MethodDef(m));
	_addMethod(_dispatch_Object_02,this,MethodDef(m));
	_addMethod(_dispatch_Object_03,this,MethodDef(m));
	/*
	Buffer m;
	m.fromString("MethodTable:0e0000005f6c6f6f6b75704d6574686f6400050000006c6f6e670000000000010000000a0000004d6574686f64446566000a0000006d6574686f6444656600","MethodTable");
	_addMethod(_dispatch_Object_skel_00,this,MethodDef(m));
	*/
}

/*
 * Stuff for object stubs
 */

Object_stub::Object_stub()
{
	assert(0); // never use this constructor, as it doesn't assign an object ID
}

Object_stub::Object_stub(Connection *connection, long objectID)
{
	_connection = connection;
	_objectID = objectID;
	_lookupCacheRandom = rand();
}

Object_stub::~Object_stub()
{
	/*
	 * invalidate method lookup cache entries of this object, as it might
	 * happen, that another Object_stub is created just at the same position
	 */
	long c1 = (long)this,p;
	for(p=0;p<_lookupMethodCacheSize;p++)
	{
		long pos = 3*p;
		if(_lookupMethodCache[pos] == c1)
			_lookupMethodCache[pos] = 0;
	}
}

Object *Object::_fromString(string objectref)
{
	Object *result = 0;
	ObjectReference r;

	if(Dispatcher::the()->stringToObjectReference(r,objectref))
	{
		result = (Object *)Dispatcher::the()->connectObjectLocal(r,"Object");
		if(!result)
		{
			Connection *conn = Dispatcher::the()->connectObjectRemote(r);
			if(conn)
				result = new Object_stub(conn,r.objectID);
		}
	}
	return result;
}
string Object_stub::_interfaceName()
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,1);
	// methodID = 1  =>  _interfaceName (always)
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID);
	string returnCode;
	result->readString(returnCode);
	delete result;
	return returnCode;
}

InterfaceDef* Object_stub::_queryInterface(const string& name)
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,2);
	// methodID = 2  =>  _queryInterface (always)
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID);
	InterfaceDef *_returnCode = new InterfaceDef(*result);
	delete result;
	return _returnCode;
}

TypeDef* Object_stub::_queryType(const string& name)
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,3);
	// methodID = 3  =>  _queryType (always)
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID);
	TypeDef *_returnCode = new TypeDef(*result);
	delete result;
	return _returnCode;
}

long Object_stub::_lookupMethod(const MethodDef& methodDef)
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,0);
	// methodID = 0  =>  _lookupMethod (always)
	methodDef.writeType(*request);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID);
	long returnCode = result->readLong();
	delete result;
	return returnCode;
}

long *Object_stub::_lookupMethodCache = 0;

long Object_stub::_lookupMethodFast(const char *method)
{
	long c1 = (long)this;
	long c2 = (long)method;
	long pos = 3*((c1 ^ c2 ^ _lookupCacheRandom)%_lookupMethodCacheSize);

	if(!_lookupMethodCache)
	  _lookupMethodCache=(long *)calloc(_lookupMethodCacheSize,3*sizeof(long));
	
	if(_lookupMethodCache[pos] == c1 && _lookupMethodCache[pos+1] == c2)
		return _lookupMethodCache[pos+2];
	
	Buffer _methodBuffer;
	_methodBuffer.fromString(method,"method");
	long methodID = _lookupMethod(MethodDef(_methodBuffer));
	//cout << "methodID = " << methodID << endl;

	_lookupMethodCache[pos] = c1;
	_lookupMethodCache[pos+1] = c2;
	_lookupMethodCache[pos+2] = methodID;
	return methodID;
}
