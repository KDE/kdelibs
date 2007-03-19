/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_FRONTENDOBJECT_H
#define SOLID_FRONTENDOBJECT_H

#include <QObject>

#include <solid/solid_export.h>

namespace Solid
{
    class ManagerBase;
    class FrontendObjectPrivate;

    /**
     * Base classes for all the frontend objects in the Solid namespace.
     *
     * Provides the needed logic to keep a reference on a backend object,
     * and be notified when it got deleted.
     */
    class SOLID_EXPORT FrontendObject : public QObject
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(FrontendObject)

    public:
        /**
         * Constructs a FrontendObject
         *
         * @param parent the parent of this object
         */
        explicit FrontendObject( QObject *parent = 0 );

        /**
         * Destroys a FrontendObject
         */
        virtual ~FrontendObject();

        /**
         * Indicates if this frontend object is valid.
         * A frontend object is considered valid if it holds a backend object.
         *
         * @return true if this frontend object holds a backend object, false otherwise
         */
        bool isValid() const;

    protected:
        /**
         * @internal
         */
        explicit FrontendObject(FrontendObjectPrivate &dd, QObject *parent = 0);

        FrontendObjectPrivate *d_ptr;

        /**
         * Retrieves the backend object currently used.
         *
         * @return the backend object
         */
        QObject *backendObject() const;

        /**
         * Changes the backend object used.
         *
         * All the connections between the old backend object and the frontend object
         * are properly removed. The new backend object is then connected to the
         * slotDestroyed() slot.
         *
         * @param backendObject the new backend object
         */
        void setBackendObject( QObject *backendObject );

    protected Q_SLOTS:
        /**
         * Notifies when the backend object disappears.
         *
         * @param object the backend object destroyed
         */
        virtual void slotDestroyed( QObject *object );

    private:
        friend class ManagerBase;
    };
}

#endif
