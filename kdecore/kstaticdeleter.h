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
 * When the library is unloaded, or the app terminated, all static deleters
 * are destroyed, which in turn destroys those static objects properly.
 *
 * A typical use is
 * static KStaticDeleter<MyClass> sd;
 *
 * MyClass::self() {
 *   if (!_self) { sd.setObject(_self, new MyClass()); }
 * }
 */
template<class type> class KStaticDeleter : public KStaticDeleterBase {
public:
    KStaticDeleter() { deleteit = 0; globalReference = 0; }
    /**
     * sets the object to delete and registers the object to be
     * deleted to KGlobal. if the given object is 0, the former
     * registration is unregistred
     * @param isArray tells the destructor to delete an array instead of an object
     * @deprecated. See the other setObject variant.
     **/
    type *setObject( type *obj, bool isArray = false) {
        deleteit = obj;
        globalReference = 0;
	array = isArray;
	if (obj)
            KGlobal::registerStaticDeleter(this);
	else
	    KGlobal::unregisterStaticDeleter(this);
        return obj;
    }
    /**
     * sets the object to delete and registers the object to be
     * deleted to KGlobal. if the given object is 0, the former
     * registration is unregistred
     * @param globalRef the static pointer where this object is stored
     * This pointer will be reset to 0 after deletion of the object.
     * @param isArray tells the destructor to delete an array instead of an object
     **/
    type *setObject( type* & globalRef, type *obj, bool isArray = false) {
        globalReference = &globalRef;
        deleteit = obj;
	array = isArray;
	if (obj)
            KGlobal::registerStaticDeleter(this);
	else
	    KGlobal::unregisterStaticDeleter(this);
        globalRef = obj;
	return obj;
    }
    virtual void destructObject() {
	if (array)
	   delete [] deleteit;
	else
	   delete deleteit;
    	deleteit = 0;
        if (globalReference)
           *globalReference = 0;
    }
    virtual ~KStaticDeleter() {
    	KGlobal::unregisterStaticDeleter(this);
	destructObject();
    }
private:
    type *deleteit;
    type **globalReference;
    bool array;
};

#endif
