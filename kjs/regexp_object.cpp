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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

const ClassInfo RegExpPrototypeImp::info = {"RegExp", 0, 0, 0};

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
  static const Identifier compilePropertyName("compile");
  putDirect(compilePropertyName,
            new RegExpProtoFuncImp(exec,funcProto,RegExpProtoFuncImp::Compile,  1, compilePropertyName), DontEnum);
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
  if (!thisObj.inherits(&RegExpImp::info)) {
    if (thisObj.inherits(&RegExpPrototypeImp::info)) {
      switch (id) {
        case ToString: return String("//"); // FireFox returns /(?:)/
      }
    }
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

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
        return Null();
    }
    RegExpObjectImp* regExpObj = static_cast<RegExpObjectImp*>(exec->lexicalInterpreter()->builtinRegExp().imp());
    int **ovector = regExpObj->registerRegexp( re, s.value() );

    re->prepareMatch(s.value());
    str = re->match(s.value(), i, 0L, ovector);
    re->doneMatch();
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
    if (thisObj.get(exec,"global").toBoolean(exec)) {
      str += "g";
    }
    if (thisObj.get(exec,"ignoreCase").toBoolean(exec)) {
      str += "i";
    }
    if (thisObj.get(exec,"multiline").toBoolean(exec)) {
      str += "m";
    }
    return String(str);
  case Compile: {
      RegExp* newEngine = RegExpObjectImp::makeEngine(exec, args[0].toString(exec), args[1]);
      if (!newEngine)
        return exec->exception();
      reimp->setRegExp(newEngine);
      return Value(reimp);
    }
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

void RegExpImp::setRegExp(RegExp *r)
{
  delete reg;
  reg = r;

  Object protect(this);//Protect self from GC (we are allocating a StringImp, and may be new)
  putDirect("global", (r->flags() & RegExp::Global) ? BooleanImp::staticTrue : BooleanImp::staticFalse, 
            DontDelete | ReadOnly | DontEnum);
  putDirect("ignoreCase", (r->flags() & RegExp::IgnoreCase) ? BooleanImp::staticTrue : BooleanImp::staticFalse, 
            DontDelete | ReadOnly | DontEnum);
  putDirect("multiline", (r->flags() & RegExp::Multiline) ? BooleanImp::staticTrue : BooleanImp::staticFalse, 
            DontDelete | ReadOnly | DontEnum);

  putDirect("source", new StringImp(r->pattern()), DontDelete | ReadOnly | DontEnum);
  putDirect("lastIndex", NumberImp::zero(), DontDelete | DontEnum);
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
  Object arr = exec->lexicalInterpreter()->builtinArray().construct(exec, list);
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

RegExp* RegExpObjectImp::makeEngine(ExecState *exec, const UString &p, const Value &flagsInput)
{
  UString flags = flagsInput.type() == UndefinedType ? UString("") : flagsInput.toString(exec);

  // Check for validity of flags
  for (int pos = 0; pos < flags.size(); ++pos) {
    switch (flags[pos].unicode()) {
    case 'g':
    case 'i':
    case 'm':
      break;
    default: {
        Object err = Error::create(exec, SyntaxError,
                    "Invalid regular expression flags");
        exec->setException(err);
        return 0;
      }
    }
  }

  bool global = (flags.find("g") >= 0);
  bool ignoreCase = (flags.find("i") >= 0);
  bool multiline = (flags.find("m") >= 0);

  int reflags = RegExp::None;
  if (global)
      reflags |= RegExp::Global;
  if (ignoreCase)
      reflags |= RegExp::IgnoreCase;
  if (multiline)
      reflags |= RegExp::Multiline;

  RegExp *re = new RegExp(p, reflags);
  if (!re->isValid()) {
    Object err = Error::create(exec, SyntaxError,
                               "Invalid regular expression");
    exec->setException(err);
    delete re;
    return 0;
  }
  return re;
}

// ECMA 15.10.4
Object RegExpObjectImp::construct(ExecState *exec, const List &args)
{
  UString p;
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

  RegExp* re = makeEngine(exec, p, args[1]);
  if (!re)
    return exec->exception().toObject(exec);

  RegExpPrototypeImp *proto = static_cast<RegExpPrototypeImp*>(exec->lexicalInterpreter()->builtinRegExpPrototype().imp());
  RegExpImp *dat = new RegExpImp(proto);
  Object obj(dat); // protect from GC
  dat->setRegExp(re);

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
  // TODO: handle RegExp argument case (15.10.3.1)

  return construct(exec, args);
}
