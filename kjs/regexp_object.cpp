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

#include <stdio.h>

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "internal.h"
#include "regexp.h"
#include "regexp_object.h"
#include "error_object.h"

using namespace KJS;

// ------------------------------ RegExpPrototypeImp ---------------------------

// ECMA 15.9.4

RegExpPrototypeImp::RegExpPrototypeImp(ExecState *exec,
                                       ObjectPrototypeImp *objProto,
                                       FunctionPrototypeImp *funcProto)
  : ObjectImp(objProto)
{
  Value protect(this);
  setInternalValue(String(""));

  // The constructor will be added later in RegExpObject's constructor (?)

  put(exec,"exec",     new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::Exec,     0), DontEnum);
  put(exec,"test",     new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::Test,     0), DontEnum);
  put(exec,"toString", new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::ToString, 0), DontEnum);
}

// ------------------------------ RegExpProtoFuncImp ---------------------------

RegExpProtoFuncImp::RegExpProtoFuncImp(ExecState *exec,
                                       FunctionPrototypeImp *funcProto, int i, int len)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

bool RegExpProtoFuncImp::implementsCall() const
{
  return true;
}

Value RegExpProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  if (!thisObj.inherits(&RegExpImp::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

  RegExp *re = static_cast<RegExpImp*>(thisObj.imp())->regExp();
  String s;
  Value lastIndex, tmp;
  UString str;
  int length, i;
  switch (id) {
  case Exec:
  case Test:
    s = args[0].toString(exec);
    length = s.value().size();
    lastIndex = thisObj.get(exec,"lastIndex");
    i = lastIndex.isNull() ? 0 : lastIndex.toInt32(exec);
    tmp = thisObj.get(exec,"global");
    if (tmp.toBoolean(exec).value() == false)
      i = 0;
    if (i < 0 || i > length) {
      thisObj.put(exec,"lastIndex", 0);
      result = Null();
      break;
    }
    str = re->match(s.value(), i);
    if (id == Test) {
      result = Boolean(str.size() != 0);
      break;
    }
    // TODO complete
    result = String(str);
    break;
  case ToString:
    s = thisObj.get(exec,"source").toString(exec);
    str = "/";
    str += s.value();
    str += "/";
    result = String(str);
    break;
  }

  return result;
}

// ------------------------------ RegExpImp ------------------------------------

const ClassInfo RegExpImp::info = {"RegExp", 0, 0, 0};

RegExpImp::RegExpImp(RegExpPrototypeImp *regexpProto)
  : ObjectImp(regexpProto), reg(0L)
{
}

RegExpImp::~RegExpImp()
{
  delete reg;
}

// ------------------------------ RegExpObjectImp ------------------------------

RegExpObjectImp::RegExpObjectImp(ExecState *exec,
                                 RegExpPrototypeImp *regProto,
                                 FunctionPrototypeImp *funcProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // ECMA 15.10.5.1 RegExp.prototype
  put(exec,"prototype",regProto,DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  put(exec,"length", Number(2), ReadOnly|DontDelete|DontEnum);
}

bool RegExpObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.10.4
Object RegExpObjectImp::construct(ExecState *exec, const List &args)
{
  // TODO: regexp arguments
  String p = args[0].toString(exec);
  String f = args[1].toString(exec);
  UString flags = f.value();

  RegExpPrototypeImp *proto = static_cast<RegExpPrototypeImp*>(exec->interpreter()->builtinRegExpPrototype().imp());
  RegExpImp *dat = new RegExpImp(proto);
  Object obj(dat); // protect from GC

  bool global = (flags.find("g") >= 0);
  bool ignoreCase = (flags.find("i") >= 0);
  bool multiline = (flags.find("m") >= 0);
  // TODO: throw an error on invalid flags

  dat->put(exec, "global", Boolean(global));
  dat->put(exec, "ignoreCase", Boolean(ignoreCase));
  dat->put(exec, "multiline", Boolean(multiline));

  dat->put(exec, "source", String(p.value()));
  dat->put(exec, "lastIndex", Number(0), DontDelete | DontEnum);

  int reflags = RegExp::None;
  if (global)
      reflags |= RegExp::Global;
  if (ignoreCase)
      reflags |= RegExp::IgnoreCase;
  if (multiline)
      reflags |= RegExp::Multiline;
  dat->setRegExp(new RegExp(p.value(), reflags));

  return obj;
}

bool RegExpObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.10.3
Value RegExpObjectImp::call(ExecState */*exec*/, Object &/*thisObj*/, const List &/*args*/)
{
  // TODO: implement constructor
  return Undefined();
}
