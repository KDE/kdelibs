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
#include "namedstore.h"
#include "debug.h"
#include "anyref.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace Arts;

namespace Arts {
	class AttributeSlotBind;
};

class Arts::ObjectInternalData {
public:
	struct MethodTableEntry {
		union {
			DispatchFunction dispatcher;
			OnewayDispatchFunction onewayDispatcher;
			DynamicDispatchFunction dynamicDispatcher;
		} dispFunc;
		enum { dfNormal, dfOneway, dfDynamic } dispatchStyle;
		void *object;
		MethodDef methodDef;
	};

	list<WeakReferenceBase *> weakReferences;
	NamedStore<Arts::Object> children;

	// for _skel classes only:
	bool methodTableInit;
	std::vector<MethodTableEntry> methodTable;
	list<AttributeSlotBind *> attributeSlots;
};

struct Object_base::ObjectStreamInfo {
	string name;
	long flags;
	void *ptr;
};

/*****************************************************************************/
/* the following methods/classes are necessary for attribute notifications   */
/*****************************************************************************/

namespace Arts {
	class AttributeDataPacket : public GenericDataPacket {
	public:
		Buffer b;
		AttributeDataPacket(GenericDataChannel *channel)
			        : GenericDataPacket(channel)
		{
			size = 0;
			b.writeLong(0);
		}                                         
		void add(const AnyConstRef& r)
		{
			r.write(&b);
			b.patchLong(0,++size);
		}

    	void ensureCapacity(int)
		{
        }
		void read(Buffer& stream)
		{
			vector<mcopbyte> all;
			size = stream.readLong();
			b.patchLong(0,size);
			stream.read(all,stream.remaining());
			b.write(all);
		}
		void write(Buffer& stream)
		{
			vector<mcopbyte> all;
			b.rewind();
			b.read(all,b.remaining());
			stream.write(all);
		}
	};
	class AttributeSlotBind : public GenericAsyncStream {
	public:
    	GenericDataPacket *createPacket(int)
    	{
			return allocPacket();
		}
		AttributeDataPacket *allocPacket()
		{
			return new AttributeDataPacket(channel);
    	}
 
    	void freePacket(GenericDataPacket *packet)
    	{
			delete packet;
    	}
 
    	GenericAsyncStream *createNewStream()
    	{
			return new AttributeSlotBind();
    	}

		virtual ~AttributeSlotBind()
		{
		}

		string method;
		bool output;
	};
};

bool Object_skel::_initAttribute(const AttributeDef& attribute)
{
	long flags = attribute.flags;

	if(flags & attributeAttribute)
	{
		flags |= attributeStream | streamAsync;
		flags &= ~attributeAttribute;
	}
	else
	{
		arts_warning("attribute init on stream %s", attribute.name.c_str());
		return false;
	}

	list<AttributeSlotBind *>::iterator i;
	for(i = _internalData->attributeSlots.begin();
		i != _internalData->attributeSlots.end(); i++)
	{
		AttributeSlotBind *b = *i;
		if(b->method == "_set_"+attribute.name
		|| b->method == attribute.name + "_changed")
		{
			arts_warning("double attribute init %s",b->method.c_str());
			return false;
		}
	}
	if(flags & streamIn)
	{
		AttributeSlotBind *b = new AttributeSlotBind();
		b->output = false;
		b->method = "_set_"+attribute.name;
		_internalData->attributeSlots.push_back(b);

		_scheduleNode->initStream(attribute.name, b, flags & (~streamOut));
	}
	if(flags & streamOut)
	{
		string changed = attribute.name + "_changed";
		AttributeSlotBind *b = new AttributeSlotBind();
		b->output = true;
		b->method = changed;
		_internalData->attributeSlots.push_back(b);

		_scheduleNode->initStream(changed, b, flags & (~streamIn));
	}
	return true;
}

void Object_skel::_defaultNotify(const Notification& notification)
{
	list<AttributeSlotBind *>::iterator i;
	list<AttributeSlotBind *>& slots = _internalData->attributeSlots;

	for(i = slots.begin(); i != slots.end(); i++)
	{
		if((*i)->notifyID() == notification.ID)
		{
			GenericDataPacket *dp = (GenericDataPacket *)notification.data;
			Buffer params;

			dp->write(params);

			if(!_internalData->methodTableInit)
			{
				// take care that the object base methods are at the beginning
				Object_skel::_buildMethodTable();
				_buildMethodTable();
				_internalData->methodTableInit = true;
			}
		
			typedef ObjectInternalData::MethodTableEntry MTE;
			vector<MTE>::iterator mti;

			for(mti = _internalData->methodTable.begin();
				mti != _internalData->methodTable.end(); mti++)
			{
				if(mti->methodDef.name == (*i)->method)
				{
					Buffer result;

					long count = params.readLong();
					while(params.remaining())
					{
						if(mti->dispatchStyle == MTE::dfNormal)
						{
							mti->dispFunc.dispatcher(mti->object, &params, &result);
						}
						else if(mti->dispatchStyle == MTE::dfDynamic)
						{
							long methodID;
							methodID = mti - _internalData->methodTable.begin();
							mti->dispFunc.dynamicDispatcher(mti->object, methodID,
												   &params, &result);
						}
						else
						{
							arts_assert(0);
						}
						count--;
					}
					arts_assert(count == 0);
				}
			}
			dp->processed();
		}
	}
}

void Object_skel::notify(const Notification& notification)
{
	_copy();
	_defaultNotify(notification);
	_release();
}

/* for simplicity and efficiency, enums are emitted as normal "long" values,
 * so that finding out/using the enum type via value.type() is not be possible
 */
void Object_skel::_emit_changed(const char *attrib, const AnyConstRef& value)
{
	list<AttributeSlotBind *>::iterator i;
	list<AttributeSlotBind *>& slots = _internalData->attributeSlots;

	for(i = slots.begin(); i != slots.end(); i++)
	{
		if((*i)->method == attrib) 
		{
			AttributeDataPacket *adp =
				(AttributeDataPacket *)(*i)->createPacket(1);
			adp->add(value);
			adp->send();
			return;
		}
	}
}


bool Object_skel::_generateSlots(const std::string& name,
										const std::string& interface)
{
	InterfaceDef d = _queryInterface(interface);
	vector<string>::iterator ii;
	for(ii = d.inheritedInterfaces.begin();
					ii != d.inheritedInterfaces.end(); ii++)
	{
		if(_generateSlots(name, *ii)) return true;
	}

	vector<AttributeDef>::iterator ai;
	for(ai = d.attributes.begin(); ai != d.attributes.end(); ai++)
	{
		if(ai->flags & attributeAttribute)
		{
			if((ai->flags & streamIn && ai->name == name)
			|| (ai->flags & streamOut && ai->name+"_changed" == name))
			{
				_initAttribute(*ai);
				return true;
			}
		}
	}
	return false;
}

bool Object_skel::_QueryInitStreamFunc(Object_skel *skel, const
															std::string& name)
{
	/*
	 * this function
	 *
	 * checks if there is a stream which should be added called <name>,
	 * and returns true if it in fact added a stream, so that requesting
	 * function needs to retry
	 */
	bool result = skel->_generateSlots(name, skel->_interfaceName());
	if(!result)
	{
		arts_warning("used stream %s on object %s, which doesn't seem to exist",
					name.c_str(), skel->_interfaceName().c_str());
	}
	return result;
}

/*****************************************************************************/


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
		arts_fatal("reference counting violation - you may not call delete "
		           "manually - use _release() instead");
	}
	assert(_deleteOk);

	/* remove attribute slots */
	list<AttributeSlotBind *>::iterator ai;
	for(ai = _internalData->attributeSlots.begin();
		ai != _internalData->attributeSlots.end(); ai++)
	{
		delete (*ai);
	}
	
	/* clear stream list */
	list<ObjectStreamInfo *>::iterator osii;
	for(osii = _streamList.begin(); osii != _streamList.end(); osii++)
		delete (*osii);

	/* inform weak references that we don't exist any longer */
	while(!_internalData->weakReferences.empty())
		_internalData->weakReferences.front()->release();

	/* inform notification manager that we don't exist any longer */
	NotificationManager::the()->removeClient(this);

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
					_scheduleNode->initStream("QueryInitStreamFunc",
								(void *)Object_skel::_QueryInitStreamFunc, -1);
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

long Object_base::_getObjectID() const
{
	// BCI: add const to the other methods when breaking binary compatibility
	Object_base *xthis = const_cast<Object_base*>(this);
	switch(xthis->_location())
	{
		case objectIsLocal:
			return xthis->_skel()->_objectID;
		case objectIsRemote:
			return xthis->_stub()->_objectID;
	}
	assert(false);
	return 0;
}

Connection *Object_base::_getConnection() const
{
	// BCI: add const to the other methods when breaking binary compatibility
	Object_base *xthis = const_cast<Object_base*>(this);
	switch(xthis->_location())
	{
		case objectIsLocal:
			return Dispatcher::the()->loopbackConnection();
		case objectIsRemote:
			return xthis->_stub()->_connection;
	}
	assert(false);
	return 0;
}

bool Object_base::_isEqual(Object_base *object) const
{
	return (_internalObjectID == object->_internalObjectID);
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
	_internalData->methodTableInit = false;

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
	arts_return_if_fail(_refCnt > 0);

	_refCnt--;
	if(_refCnt == 0) _destroy();
}

void Object_skel::_copyRemote()
{
	// cout << "_copyRemote();" << endl;

	_copy();
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
		arts_warning("_useRemote without prior _copyRemote() - this might fail sometimes");
		_copyRemote();
	}

	_remoteSendCount--;
	_remoteUsers.push_back(conn);
}

/*
 * This is needed when we've received an object from wire which we now
 * hold locally. Of course it has been _copyRemote()d (rightly so), but
 * we will not use it remotely, so we need to cancel the _copyRemote().
 *
 * Added to _base due to BC.
 */
void Object_base::_cancelCopyRemote()
{
	assert(_location() == objectIsLocal);

	if(_skel()->_remoteSendCount == 0)
	{
		arts_warning("_cancelCopyRemote without prior _copyRemote() - this might fail sometimes");
	}
	else
	{
		_skel()->_remoteSendCount--;
		_release();
	}
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
		arts_debug("client disconnected: dropped one object reference");
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
			int rcount = _remoteSendCount;

			arts_debug("_referenceClean: found unused object marked by "
			           "_copyRemote => releasing");

			while(rcount--)
			{
				_remoteSendCount--;
				_release();
			}
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
	// derived classes *must* override this, but we return a sane value here
	// anyway, because DynamicSkeleton depends on this
	return "Arts::Object";
}

bool Object_skel::_isCompatibleWith(const std::string& interfacename)
{
	if (interfacename=="Arts::Object") return true;
	return false;
}

InterfaceDef Object_skel::_queryInterface(const string& name)
{
	return Dispatcher::the()->interfaceRepo().queryInterface(name);
}

TypeDef Object_skel::_queryType(const string& name)
{
	return Dispatcher::the()->interfaceRepo().queryType(name);
}

EnumDef Object_skel::_queryEnum(const string& name)
{
	return Dispatcher::the()->interfaceRepo().queryEnum(name);
}

// Aggregation
std::string Object_skel::_addChild(Arts::Object child, const std::string& name) 
{
	return _internalData->children.put(name,child);
}

bool Object_skel::_removeChild(const std::string& name)
{
	return _internalData->children.remove(name);
}

Arts::Object Object_skel::_getChild(const std::string& name)
{
	Arts::Object result;
	if(_internalData->children.get(name,result)) 
		return result;
	else
		return Arts::Object::null();
}

std::vector<std::string> *Object_skel::_queryChildren()
{
	return _internalData->children.contents();
}

void Object_skel::_addMethod(DispatchFunction disp, void *obj,
                                               const MethodDef& md)
{
	Arts::ObjectInternalData::MethodTableEntry me;
	me.dispFunc.dispatcher = disp;
	me.dispatchStyle = ObjectInternalData::MethodTableEntry::dfNormal;
	me.object = obj;
	me.methodDef = md;
	_internalData->methodTable.push_back(me);
}

void Object_skel::_addMethod(OnewayDispatchFunction disp, void *obj,
                                               const MethodDef& md)
{
	Arts::ObjectInternalData::MethodTableEntry me;
	me.dispFunc.onewayDispatcher = disp;
	me.dispatchStyle = ObjectInternalData::MethodTableEntry::dfOneway;
	me.object = obj;
	me.methodDef = md;
	_internalData->methodTable.push_back(me);
}

void Object_skel::_addMethod(DynamicDispatchFunction disp, void *obj,
                                               const MethodDef& md)
{
	Arts::ObjectInternalData::MethodTableEntry me;
	me.dispFunc.dynamicDispatcher = disp;
	me.dispatchStyle = ObjectInternalData::MethodTableEntry::dfDynamic;
	me.object = obj;
	me.methodDef = md;
	_internalData->methodTable.push_back(me);
}


long Object_skel::_addCustomMessageHandler(OnewayDispatchFunction handler,
																	void *obj)
{
	if(!_internalData->methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_internalData->methodTableInit = true;
	}
	Arts::ObjectInternalData::MethodTableEntry me;
	me.dispFunc.onewayDispatcher = handler;
	me.dispatchStyle = ObjectInternalData::MethodTableEntry::dfOneway;
	me.object = obj;
	me.methodDef.name = "_userdefined_customdatahandler";
	_internalData->methodTable.push_back(me);
	return _internalData->methodTable.size()-1;
}

void Object_skel::_dispatch(Buffer *request, Buffer *result,long methodID)
{
	if(!_internalData->methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_internalData->methodTableInit = true;
	}

	const ObjectInternalData::MethodTableEntry& me
				= _internalData->methodTable[methodID];
	
	if(me.dispatchStyle == ObjectInternalData::MethodTableEntry::dfNormal)
		me.dispFunc.dispatcher(me.object, request, result);
	else if(me.dispatchStyle == ObjectInternalData::MethodTableEntry::dfDynamic)
		me.dispFunc.dynamicDispatcher(me.object, methodID, request, result);
	else
	{
		arts_assert(0);
	}
}

void Object_skel::_dispatch(Buffer *request,long methodID)
{
	if(!_internalData->methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_internalData->methodTableInit = true;
	}
	const ObjectInternalData::MethodTableEntry& me
				= _internalData->methodTable[methodID];

	if(me.dispatchStyle == ObjectInternalData::MethodTableEntry::dfOneway)
		me.dispFunc.onewayDispatcher(me.object, request);
	else if(me.dispatchStyle == ObjectInternalData::MethodTableEntry::dfDynamic)
		me.dispFunc.dynamicDispatcher(me.object, methodID, request, 0);
	else
	{
		arts_assert(0);
	}
}

long Object_skel::_lookupMethod(const MethodDef& md)
{
	long mcount = 0;

	if(!_internalData->methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_internalData->methodTableInit = true;
	}

	vector<Arts::ObjectInternalData::MethodTableEntry>::iterator i;
	for(i=_internalData->methodTable.begin(); i != _internalData->methodTable.end(); i++)
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
	arts_warning("_lookupMethod %s %s failed this might be caused by "
	             "incompatible IDL files and is likely to result in crashes",
				 md.type.c_str(),md.name.c_str());
	return -1;
}

const MethodDef& Object_skel::_dsGetMethodDef(long methodID)
{
	if(!_internalData->methodTableInit)
	{
		// take care that the object base methods are at the beginning
		Object_skel::_buildMethodTable();
		_buildMethodTable();
		_internalData->methodTableInit = true;
	}
	return _internalData->methodTable[methodID].methodDef;
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
	Arts::InterfaceDef _returnCode = ((Arts::Object_skel *)object)->_queryInterface(name);
	_returnCode.writeType(*result);
}

// _queryType
static void _dispatch_Arts_Object_03(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	Arts::TypeDef _returnCode = ((Arts::Object_skel *)object)->_queryType(name);
	_returnCode.writeType(*result);
}

// _queryEnum
static void _dispatch_Arts_Object_04(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	Arts::EnumDef _returnCode = ((Arts::Object_skel *)object)->_queryEnum(name);
	_returnCode.writeType(*result);
}

// _toString
static void _dispatch_Arts_Object_05(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	result->writeString(((Arts::Object_skel *)object)->_toString());
}

// _isCompatibleWith
static void _dispatch_Arts_Object_06(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string interfacename;
	request->readString(interfacename);
	result->writeBool(((Arts::Object_skel *)object)->_isCompatibleWith(interfacename));
}

// _copyRemote
static void _dispatch_Arts_Object_07(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_copyRemote();
}

// _useRemote
static void _dispatch_Arts_Object_08(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_useRemote();
}

// _releaseRemote
static void _dispatch_Arts_Object_09(void *object, Arts::Buffer *, Arts::Buffer *)
{
	((Arts::Object_skel *)object)->_releaseRemote();
}

// _addChild
static void _dispatch_Arts_Object_10(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	Arts::Object_base* _temp_child;
	readObject(*request,_temp_child);
	Arts::Object child = Arts::Object::_from_base(_temp_child);
	std::string name;
	request->readString(name);
	result->writeString(((Arts::Object_skel *)object)->_addChild(child,name));
}

// _removeChild
static void _dispatch_Arts_Object_11(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	result->writeBool(((Arts::Object_skel *)object)->_removeChild(name));
}

// _getChild
static void _dispatch_Arts_Object_12(void *object, Arts::Buffer *request, Arts::Buffer *result)
{
	std::string name;
	request->readString(name);
	Arts::Object returnCode = ((Arts::Object_skel *)object)->_getChild(name);
	writeObject(*result,returnCode._base());
}

// _queryChildren
static void _dispatch_Arts_Object_13(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	std::vector<std::string> *_returnCode = ((Arts::Object_skel *)object)->_queryChildren();
	result->writeStringSeq(*_returnCode);
	delete _returnCode;
}

// _get__flowSystem
static void _dispatch_Arts_Object_14(void *object, Arts::Buffer *, Arts::Buffer *result)
{
	Arts::FlowSystem returnCode = ((Arts::Object_skel *)object)->_flowSystem();
	writeObject(*result,returnCode._base());
}

void Arts::Object_skel::_buildMethodTable()
{
	Arts::Buffer m;
	m.fromString(
        "MethodTable:0000000e5f6c6f6f6b75704d6574686f6400000000056c6f6e6700"
        "000000020000000100000010417274733a3a4d6574686f64446566000000000a6d"
        "6574686f644465660000000000000000000000000f5f696e746572666163654e61"
        "6d650000000007737472696e6700000000020000000000000000000000105f7175"
        "657279496e746572666163650000000013417274733a3a496e7465726661636544"
        "656600000000020000000100000007737472696e6700000000056e616d65000000"
        "0000000000000000000b5f717565727954797065000000000e417274733a3a5479"
        "706544656600000000020000000100000007737472696e6700000000056e616d65"
        "0000000000000000000000000b5f7175657279456e756d000000000e417274733a"
        "3a456e756d44656600000000020000000100000007737472696e6700000000056e"
        "616d650000000000000000000000000a5f746f537472696e670000000007737472"
        "696e6700000000020000000000000000000000125f6973436f6d70617469626c65"
        "576974680000000008626f6f6c65616e0000000002000000010000000773747269"
        "6e67000000000e696e746572666163656e616d650000000000000000000000000c"
        "5f636f707952656d6f74650000000005766f696400000000020000000000000000"
        "0000000b5f75736552656d6f74650000000005766f696400000000020000000000"
        "0000000000000f5f72656c6561736552656d6f74650000000005766f6964000000"
        "000200000000000000000000000a5f6164644368696c640000000007737472696e"
        "67000000000200000002000000076f626a65637400000000066368696c64000000"
        "000000000007737472696e6700000000056e616d65000000000000000000000000"
        "0d5f72656d6f76654368696c640000000008626f6f6c65616e0000000002000000"
        "0100000007737472696e6700000000056e616d650000000000000000000000000a"
        "5f6765744368696c6400000000076f626a65637400000000020000000100000007"
        "737472696e6700000000056e616d650000000000000000000000000f5f71756572"
        "794368696c6472656e00000000082a737472696e67000000000200000000000000"
        "00000000115f6765745f5f666c6f7753797374656d0000000011417274733a3a46"
        "6c6f7753797374656d00000000020000000000000000",
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
	_addMethod(_dispatch_Arts_Object_10,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_11,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_12,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_13,this,Arts::MethodDef(m));
	_addMethod(_dispatch_Arts_Object_14,this,Arts::MethodDef(m));
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
	arts_return_if_fail(_refCnt > 0);

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
	if(result)
	{
		if(!needcopy)
			result->_cancelCopyRemote();
	}
	else
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

InterfaceDef Object_stub::_queryInterface(const string& name)
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,2);
	// methodID = 2  =>  _queryInterface (always)
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return InterfaceDef(); // error
	InterfaceDef _returnCode(*result);
	delete result;
	return _returnCode;
}

TypeDef Object_stub::_queryType(const string& name)
{
	long requestID;
	Buffer *request, *result;
	request = Dispatcher::the()->createRequest(requestID,_objectID,3);
	// methodID = 3  =>  _queryType (always)
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return TypeDef(); // error
	TypeDef _returnCode(*result);
	delete result;
	return _returnCode;
}

EnumDef Object_stub::_queryEnum(const string& name)
{
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,4);
	// methodID = 4  =>  _queryEnum (always)
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return EnumDef(); // error occured
	EnumDef _returnCode(*result);
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
	unsigned long c1 = (unsigned long)this;
	unsigned long c2 = (unsigned long)method;
	unsigned long pos = 3*((c1^c2^_lookupCacheRandom)%_lookupMethodCacheSize);

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

std::string Arts::Object_stub::_toString()
{
	long methodID = _lookupMethodFast("method:0000000a5f746f537472696e670000000007737472696e6700000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return""; // error occured
	std::string returnCode;
	result->readString(returnCode);
	delete result;
	return returnCode;
}

bool Arts::Object_stub::_isCompatibleWith(const std::string& interfacename)
{
	long methodID = _lookupMethodFast("method:000000125f6973436f6d70617469626c65576974680000000008626f6f6c65616e00000000020000000100000007737472696e67000000000e696e746572666163656e616d65000000000000000000");
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

void Arts::Object_stub::_copyRemote()
{
	long methodID = _lookupMethodFast("method:0000000c5f636f707952656d6f74650000000005766f696400000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

void Arts::Object_stub::_useRemote()
{
	long methodID = _lookupMethodFast("method:0000000b5f75736552656d6f74650000000005766f696400000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

void Arts::Object_stub::_releaseRemote()
{
	long methodID = _lookupMethodFast("method:0000000f5f72656c6561736552656d6f74650000000005766f696400000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(result) delete result;
}

std::string Arts::Object_stub::_addChild(Arts::Object child, const std::string& name)
{
	long methodID = _lookupMethodFast("method:0000000a5f6164644368696c640000000007737472696e67000000000200000002000000076f626a65637400000000066368696c64000000000000000007737472696e6700000000056e616d65000000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	writeObject(*request,child._base());
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return""; // error occured
	std::string returnCode;
	result->readString(returnCode);
	delete result;
	return returnCode;
}

bool Arts::Object_stub::_removeChild(const std::string& name)
{
	long methodID = _lookupMethodFast("method:0000000d5f72656d6f76654368696c640000000008626f6f6c65616e00000000020000000100000007737472696e6700000000056e616d65000000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if(!result) return false; // error occured
	bool returnCode = result->readBool();
	delete result;
	return returnCode;
}

Arts::Object Arts::Object_stub::_getChild(const std::string& name)
{
	long methodID = _lookupMethodFast("method:0000000a5f6765744368696c6400000000076f626a65637400000000020000000100000007737472696e6700000000056e616d65000000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->writeString(name);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if (!result) return Arts::Object::null();
	Arts::Object_base* returnCode;
	readObject(*result,returnCode);
	delete result;
	return Arts::Object::_from_base(returnCode);
}

std::vector<std::string> * Arts::Object_stub::_queryChildren()
{
	long methodID = _lookupMethodFast("method:0000000f5f71756572794368696c6472656e00000000082a737472696e6700000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	std::vector<std::string> *_returnCode = new std::vector<std::string>;
	if(!result) return _returnCode; // error occured
	result->readStringSeq(*_returnCode);
	delete result;
	return _returnCode;
}

Arts::FlowSystem Arts::Object_stub::_flowSystem()
{
	long methodID = _lookupMethodFast("method:000000115f6765745f5f666c6f7753797374656d0000000011417274733a3a466c6f7753797374656d00000000020000000000000000");
	long requestID;
	Arts::Buffer *request, *result;
	request = Arts::Dispatcher::the()->createRequest(requestID,_objectID,methodID);
	request->patchLength();
	_connection->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID,_connection);
	if (!result) return Arts::FlowSystem::null();
	Arts::FlowSystem_base* returnCode;
	readObject(*result,returnCode);
	delete result;
	return Arts::FlowSystem::_from_base(returnCode);
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

/*
 * global cleanup
 */

void Object_stub::_cleanupMethodCache()
{
	if(_lookupMethodCache)
	{
		free(_lookupMethodCache);
		_lookupMethodCache = 0;
	}
}

namespace Arts {
	static class Object_stub_Shutdown : public StartupClass {
	public:
		void startup() { }
		void shutdown() { Object_stub::_cleanupMethodCache(); }
	} The_Object_stub_Shutdown;
};
