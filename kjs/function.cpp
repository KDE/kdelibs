/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#include "function.h"

#include "kjs.h"
#include "types.h"
#include "internal.h"
#include "operations.h"
#include "nodes.h"

using namespace KJS;

const TypeInfo FunctionImp::info = { "Function", FunctionType,
				      &ObjectImp::info, 0, 0 };
const TypeInfo InternalFunctionImp::info = { "InternalFunction",
					      InternalFunctionType,
					      &FunctionImp::info, 0, 0 };
const TypeInfo ConstructorImp::info = { "Constructor", ConstructorType,
					 &InternalFunctionImp::info, 0, 0 };

FunctionImp::FunctionImp(ParamList *p)
  : ObjectImp(/*TODO*/BooleanClass), param(p)
{
}

KJSO FunctionImp::thisValue() const
{
  return KJSO(Context::current()->thisValue());
}


// ECMA 10.1.3
void FunctionImp::processParameters(const List *args)
{
  KJSO variable = Context::current()->variableObject();

  assert(args);

  if (param) {
    ListIterator it = args->begin();
    for(int i = 0; i < param->count() && i < 100; i++)
      if (it != args->end()) {
	variable.put(param->at(i), *it);
	it++;
      } else
	variable.put(param->at(i), Undefined());
  }
}

KJSO FunctionImp::executeCall(Imp *thisV, const List *args)
{
  if (!args)
    args = List::empty();

  Context *save = Context::current();

  Context::setCurrent(new Context(codeType(), save, this, args, thisV));

  // assign user supplied arguments to parameters
  processParameters(args);

  Completion comp = execute(*args);

  delete Context::current();
  Context::setCurrent(save);

  if (comp.isValueCompletion())
    return comp.value();
  else
    return Undefined();
}

InternalFunctionImp::InternalFunctionImp()
  : FunctionImp(0)
{
}

Completion InternalFunctionImp::execute(const List &)
{
  return Completion(Normal, Undefined());
}

ConstructorImp::ConstructorImp() {
  //  setPrototype(KJScript::global().functionPrototype());
  // TODO ???  put("constructor", this);
  put("length", Number(1), DontEnum);
}

ConstructorImp::ConstructorImp(const KJSO &p, int len)
{
  setPrototype(p);
  // TODO ???  put("constructor", *this);
  put("length", Number(len), DontEnum);
}

ConstructorImp::~ConstructorImp() { }

Completion ConstructorImp::execute(const List &)
{
  /* TODO */
  return Completion(Normal, Null());
}

Function::Function(Imp *d)
  : KJSO(d)
{
  if (d) {
    static_cast<FunctionImp*>(rep)->attr = ImplicitNone;
    assert(Global::current().hasProperty("[[Function.prototype]]"));
    setPrototype(Global::current().functionPrototype());
  }
}

Completion Function::execute(const List &args)
{
  assert(rep);
  return static_cast<FunctionImp*>(rep)->execute(args);
}

bool Function::hasAttribute(FunctionAttribute a) const
{
  assert(rep);
  FunctionImp* fdata = static_cast<FunctionImp*>(rep);
  return (fdata->attr & a);
}

#if 0
InternalFunction::InternalFunction(Imp *d)
  : Function(d)
{
  param = 0L;
}

InternalFunction::~InternalFunction()
{
}
#endif

Constructor::Constructor(Imp *d)
  : Function(d)
{
  if (d) {
    assert(Global::current().hasProperty("[[Function.prototype]]"));
    setPrototype(Global::current().get("[[Function.prototype]]"));
    put("constructor", *this);
    put("length", 1, DontEnum);
  }
}

#if 0
Constructor::Constructor(const Object& proto, int len)
{
  setPrototype(proto);
  put("constructor", *this);
  put("length", len, DontEnum);
}
#endif

Constructor::~Constructor()
{
}

Completion Constructor::execute(const List &)
{
  /* TODO: call construct instead ? */
  return Completion(Normal, Undefined());
}

Object Constructor::construct(const List &args)
{
  assert(rep && rep->type() == ConstructorType);
  return ((ConstructorImp*)rep)->construct(args);
}

Constructor Constructor::dynamicCast(const KJSO &obj)
{
  // return null object on type mismatch
  if (!obj.isA(ConstructorType))
    return Constructor(0L);

  return Constructor(obj.imp());
}

KJSO Function::thisValue() const
{
  return KJSO(Context::current()->thisValue());
}
