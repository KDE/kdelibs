#ifndef _KSTATIC_DELETER_H_
#define _KSTATIC_DELETER_H_

#include <kglobal.h>

class KStaticDeleterBase {
public:
    virtual void destructObject() = 0;
};

/**
 * little helper class to clean up static objects that are
 * held as pointer.
 *
 * A typical use is
 * static KStaticDeleter<MyClass> sd;
 *
 * MyClass::self() {
 *   if (!_self) { _self = sd.setObject(new MyClass()); }
 * }
 */
template<class type> class KStaticDeleter : public KStaticDeleterBase {
public:
    KStaticDeleter() { deleteit = 0; }
    /**
     * sets the object to delete and registers the object to be
     * deleted to KGlobal. if the given object is 0, the former
     * registration is unregistred
     * @param isArray tells the destructor to delete an array instead of an object
     **/
    type *setObject( type *obj, bool isArray = false) {
        deleteit = obj;
	array = isArray;
	if (obj)
            KGlobal::registerStaticDeleter(this);
	else
	    KGlobal::unregisterStaticDeleter(this);
        return obj;
    }
    virtual void destructObject() {
	if (array)
	   delete [] deleteit;
	else
	   delete deleteit;
    	deleteit = 0;
    }
    virtual ~KStaticDeleter() {
    	KGlobal::unregisterStaticDeleter(this);
	destructObject();
    }
private:
    type *deleteit;
    bool array;
};

#endif
