#ifndef OBJECTWRAPPER_H
#define OBJECTWRAPPER_H

class FlowSystem;

class Object : public SmartWrapper {
private:
	static Object_base* _Creator();
	Object_base *_cache;
	inline Object_base *_method_call() {
		if(!_cache) {
			_pool->checkcreate();
			if(_pool->base) {
				_cache=(Object_base *)_pool->base->_cast(Object_base::_IID);
				assert(_cache);
			}
		}
		return _cache;
	}

public:
	inline Object() : SmartWrapper(_Creator), _cache(0) {}
	inline Object(const SubClass& s) :
		SmartWrapper(Object_base::_create(s.string())), _cache(0) {}
	inline Object(const Reference &r) :
		SmartWrapper(r.isString()?(Object_base::_fromString(r.string())):(Object_base::_fromReference(r.reference(),true))), _cache(0) {}
	inline Object(Object_base* b) : SmartWrapper(b), _cache(0) {}
	inline Object(const Object& target) : SmartWrapper(target._pool), _cache(target._cache) {}
	inline Object(SmartWrapper::Pool& p) : SmartWrapper(p), _cache(0) {}
	inline Object& operator=(const Object& target) {
		if (_pool == target._pool) return *this;
		_pool->Dec();
		_pool = target._pool;
		_cache = target._cache;
		_pool->Inc();
		return *this;
	}
	// TODO: remove the operator
	inline operator Object_base*() {return _method_call();}
	inline Object_base* _base() {return _method_call();}

	//inline FlowSystem _flowSystem() {return _method_call()->_flowSystem();}
	FlowSystem _flowSystem();
	inline long _lookupMethod(const MethodDef& methodDef) {return _method_call()->_lookupMethod(methodDef);}
	inline std::string _interfaceName() {return _method_call()->_interfaceName();}
	inline InterfaceDef* _queryInterface(const std::string& name) {return _method_call()->_queryInterface(name);}
	inline TypeDef* _queryType(const std::string& name) {return _method_call()->_queryType(name);}
	inline std::string _toString() {return _method_call()->_toString();}
	inline void _copyRemote() {return _method_call()->_copyRemote();}
	inline void _useRemote() {return _method_call()->_useRemote();}
	inline void _releaseRemote() {return _method_call()->_releaseRemote();}
};

#endif /* OBJECTWRAPPER_H */
