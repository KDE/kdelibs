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

#ifndef OBJECT_H
#define OBJECT_H

#include "buffer.h"
#include "connection.h"
#include "notification.h"

#include <assert.h>
#include <map>
#include <list>

/*
 * BC - Status (2000-09-30): Object_base, Object_skel, Object_stub
 *
 * All of them have to be kept binary compatible carefully, due to interaction
 * with generated code. There are d ptrs in _skel and _stub, NOT TO BE USED
 * NORMALLY. Normally, do use _internalData instead, as this is much faster
 * than creating two d objects per MCOP implementation/stub. Handle with care.
 */


namespace Arts {
/* custom dispatching functions */

typedef void (*DispatchFunction)(void *object, Buffer *request, Buffer *result);
typedef void (*OnewayDispatchFunction)(void *object, Buffer *request);
typedef void (*DynamicDispatchFunction)(void *object, long methodID, Buffer *request, Buffer *result);

class ScheduleNode;
class Object_skel;
class Object_stub;
class FlowSystem;
class MethodDef;
class ObjectReference;
class WeakReferenceBase;
class Object;
class ObjectManager;
class DynamicSkeletonData;
class DynamicSkeletonBase;

class Object_base : public NotificationClient {
private:
	friend class DynamicRequest;
	friend class ObjectManager;
	bool _deleteOk;				// ensure that "delete" is not called manually

protected:
	/**
	 * ObjectInternalData contains private data structures for
	 *  - Object_base
	 *  - Object_stub
	 *  - Object_skel
	 *
	 * This is an optimization over adding each of them private data pointers,
	 * which would lead to some more bloat.
	 */
	class ObjectInternalData *_internalData;

	struct ObjectStreamInfo;

	Object_base();
	virtual ~Object_base();

	/*
	 * helpers for DynamicRequest, BC/BCI, when breaking binary compatibility,
	 * it may be better to get objectID defined here directly, and maybe do
	 * something about Connection as well
	 */
	long _getObjectID() const;
	Connection *_getConnection() const;

	/*
	 * internal management for streams
	 */
	ScheduleNode *_scheduleNode;
	std::list<ObjectStreamInfo *> _streamList;

	virtual Object_skel *_skel();
	virtual Object_stub *_stub();

	enum ObjectLocation { objectIsLocal, objectIsRemote };

	virtual ObjectLocation _location();

	std::string _internalObjectID;	// two objects are "_isEqual" when these match
	long _nextNotifyID;
	long _refCnt;				// reference count
	static long _staticObjectCount;

	void _destroy();			// use this instead of delete (takes care of
								// properly removing flow system node)
public:
	static unsigned long _IID;	// interface ID
	/**
	 * custom messaging: these can be used to send a custom data to other
	 * objects. Warning: these are *not* usable for local objects. You may
	 * only use these functions if you know that you are talking to a remote
	 * object. Use _allocCustomMessage to allocate a message. Put the data
	 * you want to send in the Buffer. After that, call _sendCustomMessage.
	 * Don't free the buffer - this will happen automatically.
	 */

	virtual Buffer *_allocCustomMessage(long handlerID);
	virtual void _sendCustomMessage(Buffer *data);

	/*
	 * generic capabilities, which allow find out what you can do with an
	 * object even if you don't know it's interface
	 */
	virtual long _lookupMethod(const Arts::MethodDef &) = 0;
	virtual std::string _interfaceName() = 0;
	virtual class InterfaceDef _queryInterface(const std::string& name) = 0;
	virtual class TypeDef _queryType(const std::string& name) = 0;
	virtual class EnumDef _queryEnum(const std::string& name) = 0;
	virtual std::string _toString() = 0;

	/*
	 * stuff for streaming (put in a seperate interface?)
	 */
	virtual void calculateBlock(unsigned long cycles);
	ScheduleNode *_node();
	virtual FlowSystem _flowSystem() = 0;

	/*
	 * reference counting
	 */
	virtual void _release() = 0;
	virtual void _copyRemote() = 0;
	virtual void _useRemote() = 0;
	virtual void _releaseRemote() = 0;

	// BC issue: added _cancelCopyRemote here to avoid virtual function
	void _cancelCopyRemote();

	void _addWeakReference(WeakReferenceBase *reference);
	void _removeWeakReference(WeakReferenceBase *reference);

	inline Object_base *_copy() {
		assert(_refCnt > 0);
		_refCnt++;
		return this;
	}

	// Default I/O info
	virtual std::vector<std::string> _defaultPortsIn() const;
	virtual std::vector<std::string> _defaultPortsOut() const;

	// cast operation
	virtual void *_cast(unsigned long iid);
	void *_cast(std::string interface);
	
	// Run-time type compatibility check
	virtual bool _isCompatibleWith(const std::string& interfacename) = 0;

	// Aggregation
	virtual std::string _addChild(Arts::Object child, const std::string& name) = 0;
	virtual bool _removeChild(const std::string& name) = 0;
	virtual Arts::Object _getChild(const std::string& name) = 0;
	virtual std::vector<std::string> * _queryChildren() = 0;

	/*
	 * when this is true, a fatal communication error has occured (of course
	 * only possible for remote objects) - maybe your returncode is invalid,
	 * maybe your last invocation didn't succeed...
	 */
	virtual bool _error();

	inline static long _objectCount() { return _staticObjectCount; }
	inline long _mkNotifyID() { return _nextNotifyID++; }

	// object creation
	static Object_base *_create(const std::string& subClass = "Object");

	// comparision
	bool _isEqual(Object_base *object) const;

	// static converter (from reference)
	static Object_base *_fromString(std::string objectref);
	static Object_base *_fromReference(class ObjectReference ref, bool needcopy);
};

/*
 * Dispatching
 */

class Buffer;
class MethodDef;


class Object_skel_private;
class AnyConstRef;
class AttributeDef;

class Object_skel : virtual public Object_base {
private:
	friend class Object_base;
	friend class DynamicSkeletonData;
	friend class DynamicSkeletonBase;

	Object_skel_private *_d_skel;// do not use until there is a very big problem

	long _objectID;

	// reference counting - remote object watching
	
	long _remoteSendCount;		// don't kill objects just sent to other server
	bool _remoteSendUpdated;	// timeout if they don't want the object
	std::list<class Connection *> _remoteUsers;	// who is using it?

protected:
	void _addMethod(DispatchFunction disp, void *object, const MethodDef& md);
	void _addMethod(OnewayDispatchFunction disp, void *object,
														const MethodDef& md);
	void _addMethod(DynamicDispatchFunction disp, void *object,
														const MethodDef& md);
	void _initStream(std::string name, void *ptr, long flags);

	/** stuff relative to attribute notifications **/
	bool _initAttribute(const Arts::AttributeDef& attribute);
	static bool _QueryInitStreamFunc(Object_skel *skel,const std::string& name);
	bool _generateSlots(const std::string& name, const std::string& interface);

	/** for DynamicSkeleton: **/
	const MethodDef& _dsGetMethodDef(long methodID);

protected:
	void _defaultNotify(const Notification& notification);
	void notify(const Notification& notification);
	void _emit_changed(const char *stream, const AnyConstRef& value);

	/**
	 * custom messaging: this is used to install a custom data handler that
	 * can be used to receive non-standard messages
	 */
	long _addCustomMessageHandler(OnewayDispatchFunction handler, void *object);

	Object_skel *_skel();
	ObjectLocation _location();

public:
	Object_skel();
	virtual ~Object_skel();

	// reference counting connection drop
	void _disconnectRemote(class Connection *connection);
	void _referenceClean();

	// synchronous & asynchronous dispatching
	void _dispatch(Buffer *request, Buffer *result,long methodID);
	void _dispatch(Buffer *request, long methodID);
	long _lookupMethod(const MethodDef &);

	/*
	 * standard interface for every object skeleton
	 */
	static std::string _interfaceNameSkel();
	virtual void _buildMethodTable();

	/*
	 * reference counting
	 */
	virtual void _release();
	virtual void _copyRemote();
	virtual void _useRemote();
	virtual void _releaseRemote();

	/*
	 * streaming
	 */
	FlowSystem _flowSystem();

	/*
	 * to inspect the (remote) object interface
	 */
	virtual std::string _interfaceName();
	InterfaceDef _queryInterface(const std::string& name);
	TypeDef _queryType(const std::string& name);
	EnumDef _queryEnum(const std::string& name);
	virtual std::string _toString();
	
	// Run-time type compatibility check
	bool _isCompatibleWith(const std::string& interfacename);

	// Aggregation
	std::string _addChild(Arts::Object child, const std::string& name);
	bool _removeChild(const std::string& name);
	Arts::Object _getChild(const std::string& name);
	std::vector<std::string> * _queryChildren();
};

class Object_stub_private;

class Object_stub : virtual public Object_base {
private:
	friend class Object_base;

	Object_stub_private *_d_stub;// do not use until there is a very big problem

protected:
	long _objectID,_lookupCacheRandom;
	Connection *_connection;

	Object_stub();
	Object_stub(Connection *connection, long objectID);
	virtual ~Object_stub();

	virtual Object_stub *_stub();
	ObjectLocation _location();

	enum { _lookupMethodCacheSize = 337 };
	static long *_lookupMethodCache;

	long _lookupMethodFast(const char *method);
	long _lookupMethod(const MethodDef &);

public:
	/*
	 * custom messaging
	 */

	Buffer *_allocCustomMessage(long handlerID);
	void _sendCustomMessage(Buffer *data);

	/*
	 * to inspect the (remote) object interface
	 */
	std::string _interfaceName();
	InterfaceDef _queryInterface(const std::string& name);
	TypeDef _queryType(const std::string& name);
	EnumDef _queryEnum(const std::string& name);
	std::string _toString();

	/*
	 * streaming
	 */
	FlowSystem _flowSystem();

	/*
	 * reference counting
	 */
	virtual void _release();
	virtual void _copyRemote();
	virtual void _useRemote();
	virtual void _releaseRemote();
	
	// Run-time type compatibility check
	bool _isCompatibleWith(const std::string& interfacename);

	// Aggregation
	std::string _addChild(Arts::Object child, const std::string& name);
	bool _removeChild(const std::string& name);
	Arts::Object _getChild(const std::string& name);
	std::vector<std::string> * _queryChildren();

	/*
	 * communication error? this is true when your connection to the remote
	 * object is lost (e.g. when the remote server crashed) - your return
	 * values are then undefined, so check this before relying too much
	 * on some invocation
	 */

	bool _error();

	/*
	 * global cleanup
	 */
	static void _cleanupMethodCache();
};

};
#endif
