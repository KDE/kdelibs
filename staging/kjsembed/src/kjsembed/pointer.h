/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef POINTER_H
#define POINTER_H

#include <algorithm>
#include <typeinfo>
#include <QtCore/QVariant>

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
//         qDebug("new pointer %s %0x", typeid(ValueType).name(), value);
    }
    ~Pointer( )
    {
        
    }
    void cleanup()
    {
//         qDebug("delete pointer %s %0x", typeid(ValueType).name(), ptr );
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
struct Value : public PointerBase
{
public:
    Value( ValueType val) : value(val)
    {
            //qDebug("new value %s", typeid(ValueType).name());
    }
    ~Value( )
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
    NullPtr( ) : ptr(0)
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
        return &ptr;
    }

    void* ptr;

};

template<typename ValueType> 
ValueType *pointer_cast( PointerBase *pointer )
{
//    qDebug("pointers %s %s", typeid(ValueType).name(), pointer->type().name() );
   if( typeid(ValueType) != pointer->type() )
       return 0L;
  Pointer<ValueType> *upcast = static_cast< Pointer<ValueType> *>(pointer);
  return upcast->ptr;
}

#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;

