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

#include "object.h"
#include "dispatcher.h"
#include "flowsystem.h"
#include "weakreference.h"
#include <stdio.h>
#include <iostream.h>

using namespace std;
using namespace Arts;

class Arts::ObjectInternalData {
public:
	list<WeakReferenceBase *> weakReferences;
};

struct Object_base::ObjectStreamInfo {
	string name;
	long flags;
	void *ptr;
};

struct Object_skel::MethodTableEntry {
	DispatchFunction dispatcher;
	OnewayDispatchFunction onewayDispatcher;
	void *object;
	MethodDef methodDef;
};

/*
 * Object: common for every object
 */

long Object_base::_staticObjectCount = 0;

Object_base::Object_base() :  _deleteOk(false), _scheduleNode(0), _nextNotifyID(1),
							_refCnt(1)
{
	_internalData = new Arts::ObjectInternalData();
	_staticObjectCount++;
}

void Object_base::_destroy()
{
	_deleteOk = true;

	if(_scheduleNode)
	{
		if(_scheduleNode->remoteScheduleNode())
		{
			delete _scheduleNode;
		}
		else
		{
			FlowSystem_impl *fs = Dispatcher::the()->flowSystem();
			assert(fs);

			fs->removeObject(_scheduleNode);
		}
	}
	delete this;
}

Object_base::~Object_base()
{
	if(!_deleteOk)
	{
		cerr << "error: reference counting violation - you may not" << endl;
		cerr << "       call delete manually - use _release() instead" << endl;
	}
	assert(_deleteOk);

	/* clear stream list */
	list<ObjectStreamInfo *>::iterator osii;
	for(osii = _streamList.begin(); osii != _streamList.end(); osii++)
		delete (*osii);

	/* inform weak references that we don't exist any longer */
	while(!_internalData->weakReferences.empty())
		_internalData->weakReferences.front()->release();

	delete _internalData;
	_staticObjectCount--;
}

ScheduleNode *Object_base::_node()
{
	if(!_scheduleNode)
	{
		switch(_location())
		{
			case objectIsLocal:
				{
					FlowSystem_impl *fs = Dispatcher::the()->flowSystem();
					assert(fs);
					_scheduleNode = fs->addObject(_skel());

					/* init streams */

					list<ObjectStreamInfo *>::iterator osii;
					for(osii = _streamList.begin(); osii != _streamList.end(); osii++)
					{
						_scheduleNode->initStream((*osii)->name,(*osii)->ptr,(*osii)->flags);
					}
				}
				break;

			case objectIsRemote:
					_scheduleNode=new RemoteScheduleNode(_stub());
				break;
		}

		assert(_scheduleNode);
	}
	return _scheduleNode;
}

void *Object_base::_cast(unsigned long iid)
{
	if(iid == Object_base::_IID) return (Object *)this;
	return 0;
}

void *Object_base::_cast(std::string interface)
{
	return _cast(MCOPUtils::makeIID(interface));
}

bool Object_base::_error()
{
	// no error as default ;)
	return false;
}

Object_skel *Object_base::_skel()
{
	assert(false);
	return 0;
}

Object_base::ObjectLocation Object_base::_location()
{
	assert(false);
	return objectIsLocal;
}

Object_stub *Object_base::_stub()
{
	assert(false);
	return 0;
}

Object_stub *Object_stub::_stub()
{
	return this;
}

Object_base::ObjectLocation Object_stub::_location()
{
	return objectIsRemote;
}

Object_skel *Object_skel::_skel()
{
	return this;
}

Object_base::ObjectLocation Object_skel::_location()
{
	return objectIsLocal;
}

void Object_skel::_initStream(string name, void *ptr, long flags)
{
	ObjectStreamInfo *osi = new ObjectStreamInfo;
	osi->name = name;
	osi->ptr = ptr;
	osi->flags = flags;
	_streamList.push_back(osi);
}

void Object_base::calculateBlock(unsigned long)
{
}

string Object_base::_interfaceName()
{
	assert(0); // derived classes *must* override this
	return "";
}

Buffer *Object_base::_allocCustomMessage(long /*handlerID*/)
{
	assert(0);
	return 0;
}

void Object_base::_sendCustomMessage(Buffer *buffer)
{
	assert(0);
	delete buffer;
}

// Default I/O: nothing at this level, use child virtuals
vector<std::string> Object_base::_defaultPortsIn() const
{
	vector<std::string> ret;
	return ret;
}
vector<std::string> Object_base::_defaultPortsOut() const
{
	vector<std::string> ret;
	return ret;
}

// Weak References

void Object_base::_addWeakReference(WeakReferenceBase *b)
{
	_internalData->weakReferences.push_back(b);
}

void Object_base::_removeWeakReference(WeakReferenceBase *b)
{
	_internalData->weakReferences.remove(b);
}

/*
 * Stuff for object skeletons
 */

Object_skel::Object_skel() :_remoteSendCount(0), _remoteSendUpdated(false)
{
	_objectID = Dispatcher::the()->addObject(this);
	_methodTableInit = false;

	char ioid[1024];
	sprintf(ioid,"SKEL:%p",this);
	_internalObjectID = ioid;
}

Object_skel::~Object_skel()
{
	Dispatcher::the()->removeObject(_objectID);
}

// flow system

FlowSystem Object_skel::_flowSystem()
{
	FlowSystem_base *fs = Dispatcher::the()->flowSystem();
	if(fs)
		return FlowSystem::_from_base(fs->_copy());
	else
		return FlowSystem::null();
}

// reference counting

void Object_skel::_release()
{
	assert(_refCnt > 0);
	_refCnt--;
	if((_refCnt+_remoteSendCount) == 0) _destroy();
}

void Object_skel::_copyRemote()
{
	// cout << "_copyRemote();" << endl;

	_remoteSendCount++;
	_remoteSendUpdated = true;
}

void Object_skel::_releaseRemote()
{
	//cout << "_releaseRemote();" << endl;

	Connection *conn = Dispatcher::the()->activeConnection();
	list<Connection *>::iterator i;
	bool found = false;

	for(i=_remoteUsers.begin(); !found && i != _remoteUsers.end(); i++)
	{
		found = (*i) == conn;
		if(found) _remoteUsers.erase(i);
	}
	assert(found);
	_release();
}

void Object_skel::_useRemote()
{
	//cout << "_useRemote();" << endl;

	Connection *conn = Dispatcher::the()->activeConnection();
	if(_remoteSendCount == 0)
	{
		cerr << "warning: _useRemote without prior _copyRemote() -"
					" this might fail sometimes" << endl;
	}
	else
	{
		_remoteSendCount--;
	}
	_refCnt++;
	_remoteUsers.push_back(conn);
}

void Object_skel::_disconnectRemote(Connection *conn)
{
	//cout << "_disconnectRemote();" << endl;

	int rcount = 0;
	list<Connection *>::iterator i;

	i=_remoteUsers.begin();
	while(i != _remoteUsers.end())
	{
		if((*i) == conn)
		{
			_remoteUsers.erase(i);
			i = _remoteUsers.begin();
			rcount++;
		}
		else i++;
	}

	while(rcount) {
		cerr << "client disconnected: dropped one object reference" << endl;
		rcount--;
		_release();
	}
	/* warning: object may not exist any longer here */
}

void Object_skel::_referenceClean()
{
	if(_remoteSendCount > 0)
	{
		if(_remoteSendUpdated)
		{
			// this ensures that every client gets at least five
			// seconds to connect
			_remoteSendUpdated = false;
		}
		else
		{
			cerr << "_referenceClean: found unused object"
			        " marked by _copyRemote => releasing" << endl;
			_remoteSendCount = 0;
			_refCnt++;
			_release();
		}
		/* warning: object may be gone here */
	}
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

bool Object_skel::_isCompatibleWith(const std::string& interfacename)
{
	if (interfacename=="Arts::Object") return true;
	return false;
}

InterfaceDef* Object_skel::_queryInterface(const string& name)
{
	return Dispatcher::the()->interfaceRepo().queryInterface(name);
}

TypeDef* Object_skel::_queryType(const string& name)
{
	return Dispatcher::the()->interfaceRepo().queryType(name);
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

void Object_skel::_addMethod(OnewayDispatchFunction disp, void *obj,
                                               const MethodDef& md)
{
	MethodTableEntry me;
	me.onewayDispatcher = disp;
	me.object = obj;
	me.methodDef = md;
	_methodTable.push_back(me);
}

long Object_skel::_addCustomMessageHandler(OnewayDispatchFunction handler,
																	void *obj)
{
	if(!_methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_methodTableInit = true;
	}
	MethodTableEntry me;
	me.onewayDispatcher = handler;
	me.object = obj;
	me.methodDef.name = "_userdefined_customdatahandler";
	_methodTable.push_back(me);
	return _methodTable.size()-1;
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

void Object_skel::_dispatch(Buffer *request,long methodID)
{
	if(!_methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_methodTableInit = true;
	}
	_methodTable[methodID].onewayDispatcher(_methodTable[methodID].object,
																	request);
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
static void _dispatch_Arts_Object_00(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	Arts::MethodDef methodDef(*request);
	result->writeLong(((Arts::Object_skel *)object)->_lookupMethod(methodDef));
}

// _interfaceName
static void _dispatch_Arts_Object_01(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	result->writeString(((Arts::Object_skel *)object)->_interfaceName());
}

// _queryInterface
static void _dispatch_Arts_Object_02(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	Arts::InterfaceDef *_returnCode = ((Arts::Object_skel *)object)->_queryInterface(name);
	_returnCode->writeType(*result);
	delete _returnCode;
}

// _queryType
static void _dispatch_Arts_Object_03(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	Arts::TypeDef *_returnCode = ((Arts::Object_skel *)object)->_queryType(name);
	_returnCode->writeType(*result);
	delete _returnCode;
}

// _toString
static void _dispatch_Arts_Object_04(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	result->writeString(((Arts::Object_skel *)object)->_toString());
}

// _isCompatibleWith
static void _dispatch_Arts_Object_05(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string interfacename;
	request->readString(interfacename);
	result->writeBool(((Arts::Object_skel *)object)->_isCompatibleWith(interfacename));
}

// _copyRemote
static void _dispatch_Arts_Object_06(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_copyRemote();
}

// _useRemote
static void _dispatch_Arts_Object_07(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_useRemote();
}

// _releaseRemote
static void _dispatch_Arts_Object_08(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_releaseRemote();
}

// _get__flowSystem
static void _dispatch_Arts_Object_09(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	Arts::FlowSystem returnCode = ((Arts::Object_skel *)object)->_flowSystem();
	writeObject(*result,returnCode._base());
}

void Arts::Object_skel::_buildMethodTable()
{
	Arts::Buffer m;
	m.fromString(
        "MethodTable:0e0000005f6c6f6f6b75704d6574686f6400050000006c6f6e6700"
        "020000000100000010000000417274733a3a4d6574686f64446566000a0000006d"
        "6574686f64446566000f0000005f696e746572666163654e616d65000700000073"
        "7472696e67000200000000000000100000005f7175657279496e74657266616365"
        "0013000000417274733a3a496e7465726661636544656600020000000100000007"
        "000000737472696e6700050000006e616d65000b0000005f717565727954797065"
        "000e000000417274733a3a54797065446566000200000001000000070000007374"
        "72696e6700050000006e616d65000a0000005f746f537472696e67000700000073"
        "7472696e67000200000000000000120000005f6973436f6d70617469626c655769"
        "74680008000000626f6f6c65616e00020000000100000007000000737472696e67"
        "000e000000696e746572666163656e616d65000c0000005f636f707952656d6f74"
        "650005000000766f69640002000000000000000b0000005f75736552656d6f7465"
        "0005000000766f69640002000000000000000f0000005f72656c6561736552656d"
        "6f74650005000000766f6964000200000000000000110000005f6765745f5f666c"
        "6f7753797374656d0011000000417274733a3a466c6f7753797374656d00020000"
        "0000000000",
		"MethodTable"
	);
	_addMethod(_dispatch_Arts_Object_00,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_01,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_02,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_03,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_04,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_05,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_06,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_07,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_08,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_09,this,Arts::MethodDef(m));
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
	_connection->_copy();
	_objectID = objectID;
	_lookupCacheRandom = rand();

	char ioid[1024];
	sprintf(ioid,"STUB:%ld:%p",_objectID,connection);
	_internalObjectID = ioid;
}

Object_stub::~Object_stub()
{
	/*
	 * invalidate method lookup cache entries of this object, as it might
	 * happen, that another Object_stub is created just at the same position
	 */
	if(_lookupMethodCache)
	{
		long c1 = (long)this,p;
		for(p=0;p<_lookupMethodCacheSize;p++)
		{
			long pos = 3*p;
			if(_lookupMethodCache[pos] == c1)
				_lookupMethodCache[pos] = 0;
		}
	}
	_connection->_release();
}

bool Object_stub::_error()
{
	/*
	 * servers are trustworthy - they don't do things wrong (e.g. send
	 * wrong buffers or things like that) - however, if the connection is
	 * lost, this indicates that something went terrible wrong (probably
	 * the remote server crashed, or maybe the network is dead), and you
	 * can't rely on results of invocations any longer
	 */
	return _connection->broken();
}

void Object_stub::_release()
{
	assert(_refCnt > 0);
	_refCnt--;
	if(_refCnt == 0)
	{
		_releaseRemote();
		_destroy();
	}
}

Object_base *Object_base::_create(const std::string& subClass)
{
	Object_skel *skel = ObjectManager::the()->create(subClass);
	assert(skel);
	return skel;
}

Object_base *Object_base::_fromString(string objectref)
{
	Object_base *result = 0;
	ObjectReference r;

	if(Dispatcher::the()->stringToObjectReference(r,objectref))
	{
		result = (Object_base *)Dispatcher::the()->connectObjectLocal(r,"Object");
		if(!result)
		{
			Connection *conn = Dispatcher::the()->connectObjectRemote(r);
			if(conn)
			{
				result = new Object_stub(conn,r.objectID);
				result->_useRemote();
			}
		}
	}
	return result;
}

Object_base *Object_base::_fromReference(ObjectReference r, bool needcopy)
{
	Object_base *result;
	result = (Object_base *)Dispatcher::the()->connectObjectLocal(r,"Object");
	if(!result)
	{
		Connection *conn = Dispatcher::the()->connectObjectRemote(r);
		if(conn)
		{
			result = new Object_stub(conn,r.objectID);
			if(needcopy) result->_copyRemote();
			result->_useRemote();
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

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return ""; // error
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

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return new InterfaceDef(); // error
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

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return new TypeDef(); // error
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

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return 0; // error
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

// other (normal) methods without fixed location

string Object_stub::_toString()
{
	long methodID = _lookupMethodFast("method:0a0000005f746f537472696e670007000000737472696e67000000000000000000");
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	// methodID = 7  =>  _toString
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return ""; // error
	string returnCode;
	result->readString(returnCode);
	delete result;
	return returnCode;
}

bool Arts::Object_stub::_isCompatibleWith(const std::string& interfacename)
{
	long methodID = _lookupMethodFast("method:120000005f6973436f6d70617469626c65576974680008000000626f6f6c65616e00020000000100000007000000737472696e67000e000000696e746572666163656e616d6500");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->writeString(interfacename);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return false; // error occured
	bool returnCode = result->readBool();
	delete result;
	return returnCode;
}

void Object_stub::_copyRemote()
{
	long methodID = _lookupMethodFast("method:0c0000005f636f707952656d6f74650005000000766f6964000000000000000000");
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	// methodID = 8  =>  _copyRemote
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

void Object_stub::_useRemote()
{
	long methodID = _lookupMethodFast("method:0b0000005f75736552656d6f74650005000000766f6964000000000000000000");
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	// methodID = 9  =>  _useRemote
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

void Object_stub::_releaseRemote()
{
	long methodID = _lookupMethodFast("method:0f0000005f72656c6561736552656d6f74650005000000766f6964000000000000000000");
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	// methodID = 10  =>  _releaseRemote
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

FlowSystem Object_stub::_flowSystem()
{
	long methodID = _lookupMethodFast("method:110000005f6765745f5f666c6f7753797374656d000b000000466c6f7753797374656d000200000000000000");
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return FlowSystem::null(); // error occured
	FlowSystem_base* returnCode;
	readObject(*result,returnCode);
	delete result;
	return FlowSystem::_from_base(returnCode);
}

/*
 * custom messaging
 */

Buffer *Object_stub::_allocCustomMessage(long handlerID)
{
	return Dispatcher::the()->createOnewayRequest(_objectID,handlerID);
}

void Object_stub::_sendCustomMessage(Buffer *buffer)
{
	buffer->patchLength();
	_connection->qSendBuffer(buffer);
}

unsigned long Object_base::_IID = MCOPUtils::makeIID("Object");
