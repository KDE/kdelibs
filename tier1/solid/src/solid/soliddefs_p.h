/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_SOLIDDEFS_P_H
#define SOLID_SOLIDDEFS_P_H

#include <QtCore/QObject>

#define return_SOLID_CALL(Type, Object, Default, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=0) \
    { \
         return t->Method; \
    } \
    else \
    { \
         return Default; \
    }



#define SOLID_CALL(Type, Object, Method) \
    Type t = qobject_cast<Type>(Object); \
    if (t!=0) \
    { \
         t->Method; \
    }

#endif
