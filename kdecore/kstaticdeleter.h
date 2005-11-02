/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
 *               2001 KDE Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _KSTATIC_DELETER_H_
#define _KSTATIC_DELETER_H_

#include <kglobal.h>

/**
 * @short Base class for KStaticDeleter
 *
 * Don't use this class directly; this class is used as a base class for
 * the KStaticDeleter template to allow polymprphism.
 *
 * @see KStaticDeleter
 * @see KGlobal::registerStaticDeleter()
 * @see KGlobal::unregisterStaticDeleter()
 * @see KGlobal::deleteStaticDeleters()
 */
class KDECORE_EXPORT KStaticDeleterBase {
public:
    virtual ~KStaticDeleterBase() { }
    /**
     * Should destruct the resources managed by this KStaticDeleterBase.
     * Usually you also want to call it in your destructor.
     * @see KGlobal::deleteStaticDeleters()
     */
    virtual void destructObject();
};

/**
 * @short Automatically deletes an object on termination
 *
 * Little helper class to clean up static objects that are held as pointer.
 *
 * Static deleters are used to manage static resources. They can register
 * themselves with KGlobal. KGlobal will call destructObject() when
 * KGlobal::deleteStaticDeleters() is called or when it the process
 * finishes.
 *
 * When the library is unloaded, or the app terminated, all static deleters
 * are destroyed, which in turn destroys those static objects properly.
 * There are some rules which you should accept in the KStaticDeleter managed
 * class:
 * @li Don't rely on the global reference variable in the destructor of the
 * object, it will be '0' at destruction time.
 * @li Don't rely on other KStaticDeleter managed objects in the destructor
 * of the object, because they may be destroyed before your destructor get called.
 * This one can be tricky, because you might not know that you actually use a
 * KStaticDeleter managed class. So try to keep your destructor simple.
 *
 * A typical use is
 * \code
 * static KStaticDeleter<MyClass> sd;
 *
 * MyClass &MyClass::self() {
 *   if (!_self) { sd.setObject(_self, new MyClass()); }
 *   return *_self;
 * }
 * \endcode
 *
 * @warning Don't delete an object which is managed by KStaticDeleter without
 * calling setObject() with a null pointer.
 */
template<class type> class KStaticDeleter : public KStaticDeleterBase {
public:
    /**
     * Constructor. Initializes the KStaticDeleter. Note that the static
     * deleter ist not registered by the constructor.
     */
    KStaticDeleter() { deleteit = 0; globalReference = 0; array = false; }

    /**
     * Sets the object to delete and registers the object to be
     * deleted to KGlobal. If the given object is 0, the former
     * registration is unregistered.
     * @param obj the object to delete
     * @param isArray tells the destructor to delete an array instead of an object
     * @deprecated See the other setObject variant.
     **/
    KDE_DEPRECATED type *setObject( type *obj, bool isArray = false) {
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
     * Sets the object to delete and registers the object to be
     * deleted to KGlobal. If the given object is 0, the former
     * registration is unregistered.
     * @param globalRef the static pointer where this object is stored.
     * This pointer will be reset to 0 after deletion of the object.
     * @param obj the object to delete
     * @param isArray tells the destructor to delete an array instead of an object
     * @return the object to delete, @p obj
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

    /**
     * Destructs the object. This has the same effect as deleting
     * the KStaticDeleter.
     */
    virtual void destructObject() {
        if (globalReference)
           *globalReference = 0;
	if (array)
	   delete [] deleteit;
	else
	   delete deleteit;
    	deleteit = 0;
    }

    /**
     * Destructor. Unregisters the static deleter and destroys the
     * object by calling destructObject().
     */
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
