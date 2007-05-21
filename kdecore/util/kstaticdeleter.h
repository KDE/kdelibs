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

#include <kdecore_export.h>

class KStaticDeleterBase;

namespace KStaticDeleterHelpers
{
    /**
     * Registers a static deleter.
     * @param d the static deleter to register
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    KDECORE_EXPORT void registerStaticDeleter(KStaticDeleterBase *d);

    /**
     * Unregisters a static deleter.
     * @param d the static deleter to unregister
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    KDECORE_EXPORT void unregisterStaticDeleter(KStaticDeleterBase *d);

    /**
     * Calls KStaticDeleterBase::destructObject() on all
     * registered static deleters and unregisters them all.
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    KDECORE_EXPORT void deleteStaticDeleters();
} // namespace KStaticDeleterHelpers

/**
 * @short Base class for KStaticDeleter
 *
 * Don't use this class directly; this class is used as a base class for
 * the KStaticDeleter template to allow polymorphism.
 *
 * @see KStaticDeleter
 * @see KStaticDeleterHelpers::registerStaticDeleter()
 * @see KStaticDeleterHelpers::unregisterStaticDeleter()
 * @see KStaticDeleterHelpers::deleteStaticDeleters()
 */
class KDECORE_EXPORT_DEPRECATED KStaticDeleterBase {
public:
    virtual ~KStaticDeleterBase();
    /**
     * Should destruct the resources managed by this KStaticDeleterBase.
     * Usually you also want to call it in your destructor.
     * @see KStaticDeleterHelpers::deleteStaticDeleters()
     */
    virtual void destructObject();
};

/**
 * @short Automatically deletes an object on termination
 *
 * Little helper class to clean up static objects that are held as a pointer.
 *
 * Static deleters are used to manage static resources. They can register
 * themselves with KStaticDeleterHelpers. KStaticDeleterHelpers will call destructObject() when
 * KStaticDeleterHelpers::deleteStaticDeleters() is called or when the process
 * finishes.
 *
 * When the library is unloaded, or the app is terminated, all static deleters
 * will be destroyed, which in turn destroys those static objects properly.
 * There are some rules that you should accept in the KStaticDeleter managed
 * class:
 * @li Don't rely on the global reference variable in the destructor of the
 * object, it will be '0' at destruction time.
 * @li Don't rely on other KStaticDeleter managed objects in the destructor
 * of the object, because they may be destroyed before your destructor get called.
 * This one can be tricky, because you might not know that you are actually using a
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
 * @warning Don't delete an object which is managed by %KStaticDeleter without
 * calling setObject() with a null pointer.
 *
 * @deprecated Use \ref K_GLOBAL_STATIC instead of %KStaticDeleter.
 */
template<class type> class KDE_DEPRECATED KStaticDeleter : public KStaticDeleterBase {
public:
    /**
     * Constructor. Initializes the KStaticDeleter. Note that the static
     * deleter ist not registered by the constructor.
     */
    KStaticDeleter() { deleteit = 0; globalReference = 0; array = false; }

    /**
     * Sets the object to delete and registers that object to
     * KStaticDeleterHelpers. If the given object is 0, the formerly registered
     * object is unregistered.
     * @param obj the object to delete
     * @param isArray tells the destructor to delete an array instead of an object
     * @deprecated See the other setObject variant.
     **/
    KDE_DEPRECATED type *setObject( type *obj, bool isArray = false) {
        deleteit = obj;
        globalReference = 0;
        array = isArray;
        if (obj)
            KStaticDeleterHelpers::registerStaticDeleter(this);
        else
            KStaticDeleterHelpers::unregisterStaticDeleter(this);
        return obj;
    }

    /**
     * Sets the object to delete and registers the object to be
     * deleted to KStaticDeleterHelpers. If the given object is 0, the previously
     * registered object is unregistered.
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
            KStaticDeleterHelpers::registerStaticDeleter(this);
        else
            KStaticDeleterHelpers::unregisterStaticDeleter(this);
        globalRef = obj;
        return obj;
    }

    /**
     * Destructs the registered object. This has the same effect as deleting
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
     * Destructor. Unregisters the static deleter and destroys the registered
     * object by calling destructObject().
     */
    virtual ~KStaticDeleter() {
        KStaticDeleterHelpers::unregisterStaticDeleter(this);
        destructObject();
    }
private:
    type *deleteit;
    type **globalReference;
    bool array;
};

#endif
