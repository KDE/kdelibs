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

#include <QString>
#include <QMap>
#include <QVariant>
#include <QSharedData>

#include "krossconfig.h"
#include "errorinterface.h"

namespace Kross {

    /**
     * The common Object class all other object-classes are
     * inheritated from.
     *
     * The Object class is used as base class to provide
     * common functionality. It's similar to what we know
     * in Python as PyObject or in Qt as QObject.
     *
     * Inherited from e.g. \a Value, \a Module and \a Class .
     *
     * This class implementates reference counting for shared
     * objects. So, no need to take care of freeing objects.
     */
    class KROSSCORE_EXPORT Object : public QSharedData, public ErrorInterface
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<Object> Ptr;

            /**
             * Constructor.
             */
            explicit Object();

            /**
             * Destructor.
             */
            virtual ~Object();

            /**
             * Pass a call to the object and evaluated it recursive
             * down the object-hierachy. Objects like \a Class are able
             * to handle call's by just implementing this function.
             * If the call is done the \a called() method will be
             * executed recursive from bottom up the call hierachy.
             *
             * \param name Each call has a name that says what
             *        should be called. In the case of a \a Class
             *        the name is the functionname.
             * \param arguments The list of arguments passed to
             *        the call.
             * \return The call-result as QVariant
             */
            virtual QVariant call(const QString& name,
                                  const QVariantList& args = QVariantList());

            /**
             * Return a list of supported callable objects.
             *
             * \return List of supported calls.
             */
            virtual QStringList getCalls();
    };
}

Q_DECLARE_METATYPE(Kross::Object::Ptr)

#endif

