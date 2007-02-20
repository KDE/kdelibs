/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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
#include <kglobal.h>

namespace Solid
{
    class FrontendObject;

    class SOLID_EXPORT ManagerBase : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Returns a text describing the error that occurred while loading
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
        ManagerBase( const QString &description, const char *serviceName, const char *backendClassName );

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
        virtual void setManagerBackend( QObject *backend );

        /**
         * Load a backend from a plugin and set the error message if everything went wrong.
         * @see errorText()
         *
         * @param description the description of the manager (used in error messages)
         * @param serviceName the name of the service type we're looking for
         * @param backendClassName the fully qualified name of the type of the backend
         * @return the loaded backend
         */
        QObject *loadBackend( const QString &description, const char *serviceName, const char *backendClassName );

    private:
        class Private;
        Private * const d;
    };

    /**
     * @internal
     */
    template<typename T>
    class SingletonHelper { public: T instance; };

#define SOLID_SINGLETON( Type )                                   \
public:                                                           \
    static Type &self();                                          \
    static Type &selfForceBackend( QObject *backend );            \
private:                                                          \
    friend class Solid::SingletonHelper< Type >;

#define SOLID_SINGLETON_IMPLEMENTATION( Type, Name )              \
    K_GLOBAL_STATIC(Solid::SingletonHelper< Type >, global##Name) \
                                                                  \
    Type &Type::self()                                            \
    {                                                             \
        Solid::SingletonHelper< Type > *singleton = global##Name; \
                                                                  \
        return singleton->instance;                               \
    }                                                             \
                                                                  \
    Type &Type::selfForceBackend( QObject *backend )              \
    {                                                             \
        Solid::SingletonHelper< Type > *singleton = global##Name; \
                                                                  \
        singleton->instance.setManagerBackend( backend );         \
                                                                  \
        return singleton->instance;                               \
    }
}

#endif
