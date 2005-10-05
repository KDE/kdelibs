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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "object_object.h"
#include "function_object.h"
#include "lookup.h"
#include <stdio.h>
#include <assert.h>

using namespace KJS;

// ------------------------------ ObjectPrototypeImp --------------------------------

ObjectPrototypeImp::ObjectPrototypeImp(ExecState *exec,
                                       FunctionPrototypeImp *funcProto)
  : ObjectImp() // [[Prototype]] is Null()
{
  Value protect(this);
  putDirect(toStringPropertyName, new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::ToString,
							 0, toStringPropertyName), DontEnum);
  putDirect(toLocaleStringPropertyName, new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::ToLocaleString,
							       0, toLocaleStringPropertyName), DontEnum);
  putDirect(valueOfPropertyName, new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::ValueOf,
							0, valueOfPropertyName), DontEnum);
  putDirect("hasOwnProperty", new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::HasOwnProperty,
						     1,"hasOwnProperty"),DontEnum);
  putDirect("isPrototypeOf", new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::IsPrototypeOf,
						    1,"isPrototypeOf"),DontEnum);
  putDirect("propertyIsEnumerable", new ObjectProtoFuncImp(exec,funcProto,ObjectProtoFuncImp::PropertyIsEnumerable,
							   1,"propertyIsEnumerable"),DontEnum);

#ifndef KJS_PURE_ECMA // standard compliance location is the Global object
  // see http://www.devguru.com/Technologies/ecmascript/quickref/object.html
  put(exec, "eval",
      Object(new GlobalFuncImp(exec, funcProto,GlobalFuncImp::Eval, 1, "eval")),
      DontEnum);
#endif
}

// ------------------------------ ObjectProtoFuncImp --------------------------------

ObjectProtoFuncImp::ObjectProtoFuncImp(ExecState * /*exec*/,
                                       FunctionPrototypeImp *funcProto,
                                       int i, int len, const Identifier &_ident)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
  ident = _ident;
}


bool ObjectProtoFuncImp::implementsCall() const
{
  return true;
}

// ECMA 15.2.4.2 + 15.2.4.3

Value ObjectProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  switch (id) {
  case ToString:
    // fall through
  case ToLocaleString:
    return String("[object "+thisObj.className()+"]");
  case ValueOf:
    return thisObj;
  case HasOwnProperty: {
    // Same as hasProperty() but without checking the prototype
    Identifier propertyName(args[0].toString(exec));
    Value tempProto(thisObj.imp()->prototype());
    thisObj.imp()->setPrototype(Value());
    bool exists = thisObj.hasProperty(exec,propertyName);
    thisObj.imp()->setPrototype(tempProto);
    return Value(exists ? BooleanImp::staticTrue : BooleanImp::staticFalse);
  }
  case IsPrototypeOf: {
    Value v = args[0];
    for (; v.isValid() && v.isA(ObjectType); v = Object::dynamicCast(v).prototype()) {
      if (v.imp() == thisObj.imp())
	return Value(BooleanImp::staticTrue);
    }
    return Value(BooleanImp::staticFalse);
  }
  case PropertyIsEnumerable: {
    Identifier propertyName(args[0].toString(exec));
    ObjectImp *obj = static_cast<ObjectImp*>(thisObj.imp());

    int attributes;
    ValueImp *v = obj->_prop.get(propertyName,attributes);
    if (v)
      return Value((attributes & DontEnum) ?
		   BooleanImp::staticFalse : BooleanImp::staticTrue);

    if (propertyName == specialPrototypePropertyName)
      return Value(BooleanImp::staticFalse);

    const HashEntry *entry = obj->findPropertyHashEntry(propertyName);
    return Value((entry && !(entry->attr & DontEnum)) ?
		 BooleanImp::staticTrue : BooleanImp::staticFalse);
  }
  }

  return Undefined();
}

// ------------------------------ ObjectObjectImp --------------------------------

ObjectObjectImp::ObjectObjectImp(ExecState * /*exec*/,
                                 ObjectPrototypeImp *objProto,
                                 FunctionPrototypeImp *funcProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // ECMA 15.2.3.1
  putDirect(prototypePropertyName, objProto, DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  putDirect(lengthPropertyName, NumberImp::one(), ReadOnly|DontDelete|DontEnum);
}


bool ObjectObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.2.2
Object ObjectObjectImp::construct(ExecState *exec, const List &args)
{
  // if no arguments have been passed ...
  if (args.isEmpty()) {
    Object proto = exec->lexicalInterpreter()->builtinObjectPrototype();
    Object result(new ObjectImp(proto));
    return result;
  }

  Value arg = *(args.begin());
  Object obj = Object::dynamicCast(arg);
  if (obj.isValid())
    return obj;

  switch (arg.type()) {
  case StringType:
  case BooleanType:
  case NumberType:
    return arg.toObject(exec);
  default:
    assert(!"unhandled switch case in ObjectConstructor");
  case NullType:
  case UndefinedType:
    Object proto = exec->lexicalInterpreter()->builtinObjectPrototype();
    return Object(new ObjectImp(proto));
  }
}

bool ObjectObjectImp::implementsCall() const
{
  return true;
}

Value ObjectObjectImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  Value result;

  List argList;
  // Construct a new Object
  if (args.isEmpty()) {
    result = construct(exec,argList);
  } else {
    Value arg = args[0];
    if (arg.type() == NullType || arg.type() == UndefinedType) {
      argList.append(arg);
      result = construct(exec,argList);
    } else
      result = arg.toObject(exec);
  }
  return result;
}

