/*
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
*/

#ifndef POINTER_H
#define POINTER_H

#include <algorithm>
#include <typeinfo>
#include <QVariant>

struct PointerBase
{
public:
    virtual ~PointerBase() {;}
    virtual void cleanup() = 0;
    virtual const std::type_info &type() const = 0;
    virtual void *voidStar() = 0;
};
Q_DECLARE_METATYPE(PointerBase*)

template<typename ValueType>
struct Pointer : public PointerBase
{
public:
    Pointer( ValueType *value) : ptr(value) 
    {
        //qDebug("new pointer %s %0x", typeid(ValueType).name(), value);
    }
    ~Pointer( ) 
    {
        //qDebug("delete pointer");
    }
    void cleanup()
    {
        delete ptr;
        ptr=0L;
    }
    const std::type_info &type() const
    {
        return typeid(ValueType);
    }

    void *voidStar()
    {
        return (void*)ptr;
    }

    ValueType *ptr;
};

template<typename ValueType>
struct Scalar : public PointerBase
{
public:
    Scalar( ValueType val) : value(val) 
    {
            //qDebug("new value %s", typeid(ValueType).name());
    }
    ~Scalar( ) 
    {
            //qDebug("delete value");
    }

    void cleanup()
    {

    }

    const std::type_info &type() const
    {
        return typeid(ValueType);
    }

    void *voidStar()
    {
        return (void*)&value;
    }

    ValueType value;
};

struct NullPtr : public PointerBase
{
    NullPtr( ) 
    {
        ;
    }
    ~NullPtr( ) 
    {
        ;
    }
    void cleanup()
    {
        ;
    }

    const std::type_info &type() const
    {
        return typeid(NullPtr);
    }

    void *voidStar()
    {
        return (void*)0;
    }

};

#endif
