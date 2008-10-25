/***************************************************************************
 * object.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_OBJECT_H
#define KROSS_OBJECT_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QSharedData>

#include "krossconfig.h"
#include "errorinterface.h"

#include <ksharedptr.h>

namespace Kross {

    /**
     * The class Object does provide us scripting objects like
     * class instances to the C++ world.
     *
     * This class implementates reference counting for shared
     * objects. So, no need to take care of freeing objects.
     *
     * Sample that does use the \a Object functionality to
     * pass a Javascript classinstance to C++ code which then
     * calls a method the classinstance provides.
     * \code
     * class MyObject : public QObject
     * {
     *     public Q_SLOTS:
     *         QVariant myFunction(Kross::Object::Ptr obj) {
     *             QVariantList args;
     *             return obj->callMethod("myMethod", args);
     *         }
     * };
     * \endcode
     * \code
     * function MyClass(result) {
     *     this.myMethod = function() {
     *         return result;
     *     }
     * }
     * var myclass = new MyClass("my string");
     * var r = MyObject.myFunction(myclass);
     * print(r); // prints "my string"
     * \endcode
     *
     * \since 4.1
     */
    class KROSSCORE_EXPORT Object : public QSharedData, public ErrorInterface
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Object> Ptr;

            /**
             * Default constructor.
             */
            explicit Object();

            /**
             * Copy constructor.
             */
            Object(const Object &other);

            /**
             * Destructor.
             */
            virtual ~Object();

            /**
             * Pass a call to the object and evaluated it.
             *
             * \param name Each call has a name that says what
             * should be called.
             * \param args The optional list of arguments
             * passed to the call.
             * \return The call-result as QVariant.
             */
            virtual QVariant callMethod(const QString& name,
                    const QVariantList& args = QVariantList());

            /**
             * Return a list of supported callable objects.
             *
             * \return List of supported calls.
             */
            virtual QStringList methodNames();

            /**
             * \internal used virtual hook to easier the job to keep
             * binary compatibility.
             */
            virtual void virtual_hook(int id, void* ptr);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

Q_DECLARE_METATYPE(Kross::Object::Ptr)

#endif

