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

#include <stdio.h>
#include <string.h>

#include "kjs.h"
#include "object.h"
#include "operations.h"
#include "internal.h"
#include "types.h"
#include "lexer.h"
#include "nodes.h"
#include "lexer.h"

#include "object_object.h"
#include "function_object.h"
#include "array_object.h"
#include "bool_object.h"
#include "string_object.h"
#include "number_object.h"
#include "math_object.h"
#include "date_object.h"
#include "regexp_object.h"
#include "error_object.h"

extern int kjsyyparse();

using namespace KJS;

class GlobalFunc : public InternalFunctionImp {
public:
  GlobalFunc(int i) : id(i) { }
  Completion execute(const List &c);
  enum { Eval, ParseInt, ParseFloat, IsNaN, IsFinite, Escape, UnEscape };
private:
  int id;
};

Global::Global()
  : Object(0L)
{
  init();
}

Global::Global(void *)
  : Object(0L)
{
}

Global::~Global()
{
}

void Global::init()
{
  GlobalImp *g = new GlobalImp();
  rep = g;
  g->init();
}

Global Global::current()
{
  assert(KJScriptImp::current());
  return KJScriptImp::current()->glob;
}

KJSO Global::objectPrototype() const
{
  return get("[[Object.prototype]]");
}

KJSO Global::functionPrototype() const
{
  return get("[[Function.prototype]]");
}

GlobalImp::GlobalImp()
  : ObjectImp(ObjectClass)
{
  // constructor properties. prototypes as Global's member variables first.
  Object objProto(new ObjectPrototype());
  Object funcProto(new FunctionPrototype());
  Object arrayProto(new ArrayPrototype(objProto));
  Object stringProto(new StringPrototype(objProto));
  Object booleanProto(new BooleanPrototype(objProto));
  Object numberProto(new NumberPrototype(objProto));
  Object dateProto(new DatePrototype(objProto));
  Object regexpProto(new RegExpPrototype(objProto));
  Object errorProto(new ErrorPrototype(objProto));

  put("[[Object.prototype]]", objProto);
  put("[[Function.prototype]]", funcProto);
  put("[[Array.prototype]]", arrayProto);
  put("[[String.prototype]]", stringProto);
  put("[[Boolean.prototype]]", booleanProto);
  put("[[Number.prototype]]", numberProto);
  put("[[Date.prototype]]", dateProto);
  put("[[RegExp.prototype]]", regexpProto);
  put("[[Error.prototype]]", errorProto);

  Object objectObj(new ObjectObject(objProto));
  Object arrayObj(new ArrayObject(funcProto));
  Object boolObj(new BooleanObject(funcProto));
  Object stringObj(new StringObject(funcProto));
  Object numObj(new NumberObject(funcProto));
  Object dateObj(new DateObject(dateProto));
  Object regObj(new RegExpObject(regexpProto));
  Object errObj(new ErrorObject(errorProto));

  put("Object", objectObj, DontEnum);
  put("Array", arrayObj, DontEnum);
  put("Boolean", boolObj, DontEnum);
  put("String", stringObj, DontEnum);
  put("Number", numObj, DontEnum);
  put("Date", dateObj, DontEnum);
  put("RegExp", regObj, DontEnum);
  put("Error", errObj, DontEnum);

  objProto.setConstructor(objectObj);
  arrayProto.setConstructor(arrayObj);
  booleanProto.setConstructor(boolObj);
  stringProto.setConstructor(stringObj);
  numberProto.setConstructor(numObj);
  dateProto.setConstructor(dateObj);
  regexpProto.setConstructor(regObj);
  errorProto.setConstructor(errObj);
};

void GlobalImp::init()
{
  /* TODO: hardcode them in a get() method */
  
  // value properties
  put("NaN", Number(NaN), DontEnum | DontDelete);
  put("Infinity", Number(Inf), DontEnum | DontDelete);
  put("undefined", Undefined(), DontEnum | DontDelete);
  put("eval", Function(new GlobalFunc(GlobalFunc::Eval)));
  put("parseInt", Function(new GlobalFunc(GlobalFunc::ParseInt)));
  put("parseFloat", Function(new GlobalFunc(GlobalFunc::ParseFloat)));
  put("isNaN", Function(new GlobalFunc(GlobalFunc::IsNaN)));
  put("isFinite", Function(new GlobalFunc(GlobalFunc::IsFinite)));
  put("escape", Function(new GlobalFunc(GlobalFunc::Escape)));
  put("unescape", Function(new GlobalFunc(GlobalFunc::UnEscape)));

  // other properties
  put("Math", Object(new Math()), DontEnum);
}

GlobalImp::~GlobalImp() { }

Completion GlobalFunc::execute(const List &args)
{
  KJSO res;

  static const char non_escape[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz"
				   "0123456789@*_+-";

  if (id == Eval) { // eval()
    KJSO x = args[0];
    if (x.type() != StringType)
      res = x;
    else {
      String s = x.toString();
      Lexer::curr()->setCode(s.value().data(), s.value().size());
      if (kjsyyparse()) {
	// TODO: stop this from growing (will be deleted at end of global eval)
	//	KJS::Node::deleteAllNodes();
	return Completion(Normal, Error::create(SyntaxError));
      }

      res = KJScriptImp::current()->progNode->evaluate();
      if (!res.isA(CompletionType))
	res = Undefined();
      else {
	Completion c(res.imp());
	if ((c.complType() == Normal && c.isValueCompletion())
	    || c.complType() != Normal)
	  return c;
	else
	  res = Undefined();
      }

      //      if (KJS::Node::progNode())
      //	KJS::Node::progNode()->deleteStatements();
    }
  } else if (id == ParseInt) {
    String str = args[0].toString();
    int radix = args[1].toInt32();
    if (radix == 0)
      radix = 10;
    else if (radix < 2 || radix > 36) {
      res = Number(NaN);
      return Completion(Normal, res);
    }
    /* TODO: use radix */
    int i = 0;
    sscanf(str.value().ascii(), "%d", &i);
    res = Number(i);
  } else if (id == ParseFloat) {
    String str = args[0].toString();
    double d = 0.0;
    sscanf(str.value().ascii(), "%lf", &d);
    res = Number(d);
  } else if (id == IsNaN) {
    res = Boolean(args[0].toNumber().isNaN());
  } else if (id == IsFinite) {
    Number n = args[0].toNumber();
    res = Boolean(!n.isNaN() && !n.isInf());
  } else if (id == Escape) {
    UString r = "", s, str = args[0].toString().value();
    const UChar *c = str.data();
    for (int k = 0; k < str.size(); k++, c++) {
      int u = c->unicode();
      if (u > 255) {
	char tmp[7];
	sprintf(tmp, "%%u%04x", u);
	s = UString(tmp);
      } else if (strchr(non_escape, (char)u)) {
	s = UString(c, 1);
      } else {
	char tmp[4];
	sprintf(tmp, "%%%02x", u);
	s = UString(tmp);
      }
      r += s;
    }
    res = String(r);
  } else if (id == UnEscape) {
    UString s, str = args[0].toString().value();
    int k = 0, len = str.size();
    while (k < len) {
      const UChar *c = str.data() + k;
      UChar u;
      if (*c == UChar('%') && k <= len - 6 && *(c+1) == UChar('u')) {
	u = Lexer::convertUnicode((c+2)->unicode(), (c+3)->unicode(),
				  (c+4)->unicode(), (c+5)->unicode());
	c = &u;
	k += 5;
      } else if (*c == UChar('%') && k <= len - 3) {
	u = UChar(Lexer::convertHex((c+1)->unicode(), (c+2)->unicode()));
	c = &u;
	k += 2;
      }
      k++;
      s += UString(c, 1);
    }
    res = String(s);
  }

  return Completion(Normal, res);
}
