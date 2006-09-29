/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_MANAGERBASE_H
#define SOLID_MANAGERBASE_H

#include <QObject>

#include <kdelibs_export.h>

#include <kstaticdeleter.h>

namespace Solid
{
    class FrontendObject;

    class SOLID_EXPORT ManagerBase : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Returns a text describing the error that occured while loading
         * the backend.
         *
         * @return the error description, or QString() if the backend loaded successfully
         */
        const QString &errorText() const;

        /**
         * Retrieves a reference to the loaded backend.
         *
         * @return a pointer to the backend, or 0 if no backend is loaded
         */
        QObject *managerBackend() const;

    protected:
        /**
         * Constructs a new manager, loading the backend from a plugin.
         *
         * @param description the description of the manager (used in error messages)
         * @param serviceName the name of the service type we're looking for
         * @param backendClassName the fully qualified name of the type of the backend
         */
        ManagerBase( QString description, const char *serviceName, const char *backendClassName );

        /**
         * Constructs a new manager from an already loaded backend.
         *
         * @param backend the backend object to use
         */
        ManagerBase( QObject *backend );

        /**
         * Destroys a ManagerBase object.
         */
        virtual ~ManagerBase();

        /**
         * Changes the backend used.
         *
         * All the connections between the old backend object and the frontend object
         * are properly removed.
         *
         * @param backend the new backend
         */
        void setManagerBackend( QObject *backend );

        /**
         * Load a backend from a plugin and set the error message if everything went wrong.
         * @see errorText()
         *
         * @param description the description of the manager (used in error messages)
         * @param serviceName the name of the service type we're looking for
         * @param backendClassName the fully qualified name of the type of the backend
         * @return the loaded backend
         */
        QObject *loadBackend( QString description, const char *serviceName, const char *backendClassName );

    private:
        class Private;
        Private *d;
    };

#define SOLID_SINGLETON( Type )                              \
public:                                                      \
    static Type &self();                                     \
    static Type &selfForceBackend( QObject *backend );       \
private:                                                     \
    static Type *s_self;                                     \
    friend void ::KStaticDeleter< Type >::destructObject();

#define SOLID_SINGLETON_IMPLEMENTATION( Type )               \
    static ::KStaticDeleter< Type > sd;                      \
                                                             \
    Type *Type::s_self = 0;                                  \
                                                             \
    Type &Type::self()                                       \
    {                                                        \
        if( !s_self )                                        \
        {                                                    \
            s_self = new Type();                             \
            sd.setObject( s_self, s_self );                  \
        }                                                    \
                                                             \
        return *s_self;                                      \
    }                                                        \
                                                             \
    Type &Type::selfForceBackend( QObject *backend )         \
    {                                                        \
        if( !s_self )                                        \
        {                                                    \
            s_self = new Type( backend );                    \
            sd.setObject( s_self, s_self );                  \
        }                                                    \
                                                             \
        return *s_self;                                      \
    }
}

#endif
