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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef _KSTATIC_DELETER_H_
#define _KSTATIC_DELETER_H_

#include <kglobal.h>

class KStaticDeleterBase {
public:
    virtual ~KStaticDeleterBase() { }
    virtual void destructObject();
};

/**
 * Little helper class to clean up static objects that are
 * held as pointer.
 * When the library is unloaded, or the app terminated, all static deleters
 * are destroyed, which in turn destroys those static objects properly.
 *
 * A typical use is
 * <pre>
 * static KStaticDeleter<MyClass> sd;
 *
 * MyClass::self() {
 *   if (!_self) { sd.setObject(_self, new MyClass()); }
 * }
 * </pre>
 */
template<class type> class KStaticDeleter : public KStaticDeleterBase {
public:
    KStaticDeleter() { deleteit = 0; globalReference = 0; array = false; }
    /**
     * sets the object to delete and registers the object to be
     * deleted to KGlobal. if the given object is 0, the former
     * registration is unregistered
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
     * registration is unregistered
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
        if (globalReference)
           *globalReference = 0;
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
    type **globalReference;
    bool array;
};

#endif
