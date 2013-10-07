/***************************************************************************
 * wrapperinterface.h
 * This file is part of the KDE project
 * copyright (C)2008 by Sebastian Sauer  <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_WRAPPERINTERFACE_H
#define KROSS_WRAPPERINTERFACE_H

#include "krossconfig.h"

namespace Kross {

    /**
     * Wrapper-class used to provide handlers for custom types.
     *
     * Custom types are types other than QObject*, QWidget* or one
     * of the base types supported by QVariant. By using the
     * Kross::registerMetaTypeHandler() method such custom handlers
     * can be registered and used to either translate various
     * types to a by QVariant supported type or by providing on
     * the fly an own wrapper class that inherits from QObject
     * and does provide access to the functionality of the
     * wrapped custom type.
     *
     * Following sample demonstrates the usage by registering
     * a handler for the type "TestObject*". Once such a type
     * got returned by a C++ class, the handler got called. If
     * we return a QObject that implements the WrapperInterface,
     * what is not needed, then the wrappedObject() method will
     * be used to translate the wrapper back to the wrapped
     * object if a C++ function got called and the wrapper is
     * passed as argument.
     *
     * \code
     * // This is our wrapper class we are using to e.g. provide
     * // additional functionality on the fly or to provide access
     * // to a C++ type that does not inherit from QObject.
     * class MyWrapper : public QObject, public Kross::WrapperInterface {
     *     public:
     *         MyWrapper(QObject* obj) : QObject(obj) {}
     *         void* wrappedObject() const { return parent(); }
     * };
     * // This function will be called by Kross if a type named
     * // "TestObject*" got returned by a C++ method.
     * QVariant TestObjectHandler(void* ptr)
     * {
     *     TestObject* obj = static_cast<TestObject*>(ptr);
     *     MyWrapper* w = new MyWrapper(obj);
     *     QVariant r;
     *     r.setValue( (QObject*)w );
     *     return r;
     * }
     * // and somewhere else we finally register our function.
     * Kross::Manager::self().registerMetaTypeHandler("TestObject*", TestObjectHandler);
     * \endcode
     *
     * \since 4.2
     */
    class KROSSCORE_EXPORT WrapperInterface
    {
        public:

            /**
             * Destructor.
             */
            virtual ~WrapperInterface();

            /**
             * This method got called by Kross if the wrapper-instance
             * got passed to a C++ slot. It is recommed to return here
             * the wrapped instance, but you don't need to.
             */
            virtual void* wrappedObject() const = 0;

            //void wrapperConstructed() {}
            //void wrapperDestroyed() {}
    };

}

#endif
