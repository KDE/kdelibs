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
#include "notification.h"

#include <cassert>
#include <map>
#include <list>

/* custom dispatching functions */

typedef void (*DispatchFunction)(void *object, Buffer *request, Buffer *result);
typedef void (*OnewayDispatchFunction)(void *object, Buffer *request);

class ScheduleNode;
class Object_skel;
class Object_stub;
class FlowSystem;

class Object : public NotificationClient {
private:
	bool _deleteOk;				// ensure that "delete" is not called manually

protected:
	struct ObjectStreamInfo;

	Object();
	virtual ~Object();

	/*
	 * internal management for streams
	 */
	ScheduleNode *_scheduleNode;
	list<ObjectStreamInfo *> _streamList;

	virtual Object_skel *_skel();
	virtual Object_stub *_stub();

	enum ObjectLocation { objectIsLocal, objectIsRemote };

	virtual ObjectLocation _location();

	long _nextNotifyID;
	long _refCnt;				// reference count
	static long _staticObjectCount;

	void _destroy();			// use this instead of delete (takes care of
								// properly removing flow system node)
public:
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
	virtual long _lookupMethod(const class MethodDef &) = 0;
	virtual std::string _interfaceName() = 0;
	virtual class InterfaceDef* _queryInterface(const std::string& name) = 0;
	virtual class TypeDef* _queryType(const std::string& name) = 0;
	virtual std::string _toString() = 0;

	/*
	 * stuff for streaming (put in a seperate interface?)
	 */
	virtual void calculateBlock(unsigned long cycles);
	ScheduleNode *_node();
	virtual FlowSystem * _flowSystem() = 0;

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

	/*
	 * when this is true, a fatal communication error has occured (of course
	 * only possible for remote objects) - maybe your returncode is invalid,
	 * maybe your last invocation didn't succeed...
	 */
	virtual bool _error();

	inline static long _objectCount() { return _staticObjectCount; }
	inline long _mkNotifyID() { return _nextNotifyID++; }

	// static converter (from reference)
	static Object *_fromString(std::string objectref);
	static Object *_fromReference(class ObjectReference ref, bool needcopy);
};

template<class T> class ReferenceHelper;

typedef ReferenceHelper<Object> Object_var;

/*
 * Dispatching
 */

class Buffer;
class MethodDef;


class Object_skel : virtual public Object {
private:
	struct MethodTableEntry;

	long _objectID;
	bool _methodTableInit;
	std::vector<struct MethodTableEntry> _methodTable;

	// reference counting - remote object watching
	
	long _remoteSendCount;		// don't kill objects just sent to other server
	bool _remoteSendUpdated;	// timeout if they don't want the object
	std::list<class Connection *> _remoteUsers;	// who is using it?

protected:
	void _addMethod(DispatchFunction disp, void *object, const MethodDef& md);
	void _addMethod(OnewayDispatchFunction disp, void *object,
														const MethodDef& md);
	void _initStream(std::string name, void *ptr, long flags);

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
	virtual void *_cast(std::string interface) = 0;

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
	FlowSystem * _flowSystem();

	/*
	 * to inspect the (remote) object interface
	 */
	virtual std::string _interfaceName();
	InterfaceDef* _queryInterface(const std::string& name);
	TypeDef* _queryType(const std::string& name);
	virtual std::string _toString();
};

class Object_stub : virtual public Object {
protected:
	friend class Object;

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
	InterfaceDef* _queryInterface(const std::string& name);
	TypeDef* _queryType(const std::string& name);
	std::string _toString();

	/*
	 * streaming
	 */
	FlowSystem * _flowSystem();

	/*
	 * reference counting
	 */
	virtual void _release();
	virtual void _copyRemote();
	virtual void _useRemote();
	virtual void _releaseRemote();

	/*
	 * communication error? this is true when your connection to the remote
	 * object is lost (e.g. when the remote server crashed) - your return
	 * values are then undefined, so check this before relying too much
	 * on some invocation
	 */

	bool _error();
};
#endif
