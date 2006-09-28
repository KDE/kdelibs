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

    class KDE_EXPORT ManagerBase : public QObject
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
        ManagerBase( QString description, const char *serviceName, const char *backendClassName );
        ManagerBase( QObject *backend );
        virtual ~ManagerBase();

        void setManagerBackend( QObject *backend );
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
