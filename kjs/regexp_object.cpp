// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
#include "lookup.h"

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

  static const Identifier execPropertyName("exec");
  putDirect(execPropertyName,
	    new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::Exec,     0, execPropertyName), DontEnum);
  static const Identifier testPropertyName("test");
  putDirect(testPropertyName,
	    new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::Test,     0, testPropertyName), DontEnum);
  putDirect(toStringPropertyName,
	    new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::ToString, 0, toStringPropertyName), DontEnum);
}

// ------------------------------ RegExpProtoFuncImp ---------------------------

RegExpProtoFuncImp::RegExpProtoFuncImp(ExecState * /*exec*/, FunctionPrototypeImp *funcProto,
                                       int i, int len, const Identifier &_ident)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
  ident = _ident;
}

bool RegExpProtoFuncImp::implementsCall() const
{
  return true;
}

Value RegExpProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( RegExpImp, thisObj );

  RegExpImp *reimp = static_cast<RegExpImp*>(thisObj.imp());
  RegExp *re = reimp->regExp();
  String s;
  UString str;
  switch (id) {
  case Exec:      // 15.10.6.2
  case Test:
  {
    s = args[0].toString(exec);
    int length = s.value().size();

    // Get values from the last time (in case of /g)
    Value lastIndex = thisObj.get(exec,"lastIndex");
    int i = lastIndex.isValid() ? lastIndex.toInt32(exec) : 0;
    bool globalFlag = thisObj.get(exec,"global").toBoolean(exec);
    if (!globalFlag)
      i = 0;
    if (i < 0 || i > length) {
      thisObj.put(exec,"lastIndex", Number(0), DontDelete | DontEnum);
      if (id == Test)
        return Boolean(false);
      else
        Null();
    }
    RegExpObjectImp* regExpObj = static_cast<RegExpObjectImp*>(exec->interpreter()->builtinRegExp().imp());
    int **ovector = regExpObj->registerRegexp( re, s.value() );

    str = re->match(s.value(), i, 0L, ovector);
    regExpObj->setSubPatterns(re->subPatterns());

    if (id == Test)
      return Boolean(!str.isNull());

    if (str.isNull()) // no match
    {
      if (globalFlag)
        thisObj.put(exec,"lastIndex",Number(0), DontDelete | DontEnum);
      return Null();
    }
    else // success
    {
      if (globalFlag)
        thisObj.put(exec,"lastIndex",Number( (*ovector)[1] ), DontDelete | DontEnum);
      return regExpObj->arrayOfMatches(exec,str);
    }
  }
  break;
  case ToString:
    s = thisObj.get(exec,"source").toString(exec);
    str = "/";
    str += s.value();
    str += "/";
    // TODO append the flags
    return String(str);
  }

  return Undefined();
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

RegExpObjectImp::RegExpObjectImp(ExecState * /*exec*/,
                                 FunctionPrototypeImp *funcProto,
                                 RegExpPrototypeImp *regProto)

  : InternalFunctionImp(funcProto), lastOvector(0L), lastNrSubPatterns(0)
{
  Value protect(this);
  // ECMA 15.10.5.1 RegExp.prototype
  putDirect(prototypePropertyName, regProto, DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  putDirect(lengthPropertyName, NumberImp::two(), ReadOnly|DontDelete|DontEnum);
}

RegExpObjectImp::~RegExpObjectImp()
{
  delete [] lastOvector;
}

int **RegExpObjectImp::registerRegexp( const RegExp* re, const UString& s )
{
  lastString = s;
  delete [] lastOvector;
  lastOvector = 0;
  lastNrSubPatterns = re->subPatterns();
  return &lastOvector;
}

Object RegExpObjectImp::arrayOfMatches(ExecState *exec, const UString &result) const
{
  List list;
  // The returned array contains 'result' as first item, followed by the list of matches
  list.append(String(result));
  if ( lastOvector )
    for ( unsigned int i = 1 ; i < lastNrSubPatterns + 1 ; ++i )
    {
      UString substring = lastString.substr( lastOvector[2*i], lastOvector[2*i+1] - lastOvector[2*i] );
      list.append(String(substring));
    }
  Object arr = exec->interpreter()->builtinArray().construct(exec, list);
  arr.put(exec, "index", Number(lastOvector[0]));
  arr.put(exec, "input", String(lastString));
  return arr;
}

Value RegExpObjectImp::get(ExecState *exec, const Identifier &p) const
{
  UString s = p.ustring();
  if (s[0] == '$' && lastOvector)
  {
    bool ok;
    unsigned long i = s.substr(1).toULong(&ok);
    if (ok)
    {
      if (i < lastNrSubPatterns + 1)
      {
        UString substring = lastString.substr( lastOvector[2*i], lastOvector[2*i+1] - lastOvector[2*i] );
        return String(substring);
      }
      return String("");
    }
  }
  return InternalFunctionImp::get(exec, p);
}

bool RegExpObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.10.4
Object RegExpObjectImp::construct(ExecState *exec, const List &args)
{
  UString p;
  UString flags = args[1].toString(exec);
  if (args.isEmpty()) {
      p = "";
  } else {
    Value a0 = args[0];
    if (a0.isA(ObjectType) && a0.toObject(exec).inherits(&RegExpImp::info)) {
      // It's a regexp. Check that no flags were passed.
      if (args.size() > 1 && args[1].type() != UndefinedType) {
          Object err = Error::create(exec,TypeError);
          exec->setException(err);
          return err;
      }
      RegExpImp *rimp = static_cast<RegExpImp*>(Object::dynamicCast(a0).imp());
      p = rimp->regExp()->pattern();
    } else {
      p = a0.toString(exec);
    }
  }

  RegExpPrototypeImp *proto = static_cast<RegExpPrototypeImp*>(exec->interpreter()->builtinRegExpPrototype().imp());
  RegExpImp *dat = new RegExpImp(proto);
  Object obj(dat); // protect from GC

  bool global = (flags.find("g") >= 0);
  bool ignoreCase = (flags.find("i") >= 0);
  bool multiline = (flags.find("m") >= 0);
  // TODO: throw a syntax error on invalid flags

  dat->putDirect("global", global ? BooleanImp::staticTrue : BooleanImp::staticFalse);
  dat->putDirect("ignoreCase", ignoreCase ? BooleanImp::staticTrue : BooleanImp::staticFalse);
  dat->putDirect("multiline", multiline ? BooleanImp::staticTrue : BooleanImp::staticFalse);

  dat->putDirect("source", new StringImp(p));
  dat->putDirect("lastIndex", NumberImp::zero(), DontDelete | DontEnum);

  int reflags = RegExp::None;
  if (global)
      reflags |= RegExp::Global;
  if (ignoreCase)
      reflags |= RegExp::IgnoreCase;
  if (multiline)
      reflags |= RegExp::Multiline;
  dat->setRegExp(new RegExp(p, reflags));

  return obj;
}

bool RegExpObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.10.3
Value RegExpObjectImp::call(ExecState *exec, Object &/*thisObj*/,
			    const List &args)
{
  return construct(exec, args);
}
