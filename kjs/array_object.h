// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _ARRAY_OBJECT_H_
#define _ARRAY_OBJECT_H_

#include "internal.h"
#include "function_object.h"

namespace KJS {

  class ArrayInstanceImp : public ObjectImp {
  public:
    ArrayInstanceImp(const Object &proto);

    virtual void put(ExecState *exec, const UString &propertyName, const Value &value, int attr = None);
    virtual void putDirect(ExecState *exec, const UString &propertyName, const Value &value, int attr = None);
    /**
     * A shallow hasProperty() variant that doesn't look at the prototype's
     * properties.
     */
    virtual bool hasOwnProperty(ExecState *exec, const UString &propertyName);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  };

 class ArrayPrototypeImp : public ArrayInstanceImp {
  public:
    ArrayPrototypeImp(ExecState *exec,
                      ObjectPrototypeImp *objProto);
    Value get(ExecState *exec, const UString &p) const;
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class ArrayProtoFuncImp : public InternalFunctionImp {
  public:
    ArrayProtoFuncImp(ExecState *exec, int i, int len);

    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    enum { ToString, ToLocaleString, Concat, Join, Pop, Push,
	   Reverse, Shift, Slice, Sort, Splice, UnShift };
  private:
    int id;
  };

  class ArrayObjectImp : public InternalFunctionImp {
  public:
    ArrayObjectImp(ExecState *exec,
                   FunctionPrototypeImp *funcProto,
                   ArrayPrototypeImp *arrayProto);

    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

  };

} // namespace

#endif
