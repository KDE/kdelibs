/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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

#include "global_object.h"

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

using namespace KJS;

const TypeInfo GlobalImp::info = { "global", GlobalType, &ObjectImp::info, 0, 0 };

Global::Global()
  : Object(0L)
{
  rep = 0;
}

Global::Global(void *)
  : Object(0L)
{
  rep = 0;
}

Global::Global(GlobalImp *d)
  : Object(0L)
{
  rep = d;
  rep->ref();
}

Global::~Global()
{
  if (rep)
    rep->deref();
}

Global Global::current()
{
  // Applications that do not pass in their own global object to the
  // KJScript constructor will get a GlobalImp, and Global::current()
  // will return a global object. However, if the app uses it's own
  // global object, then this will return 0.
  assert(KJScriptImp::current());
  KJSO glob(KJScriptImp::current()->globalObject());
  if (glob.derivedFrom(GlobalType))
    return Global(static_cast<GlobalImp*>(glob.imp()));
  else
    return Global();
}

KJSO Global::objectPrototype() const
{
  return get("[[Object.prototype]]");
}

KJSO Global::functionPrototype() const
{
  return get("[[Function.prototype]]");
}

void Global::setFilter(const KJSO &f)
{
  static_cast<GlobalImp*>(rep)->filter = f.imp();
}

KJSO Global::filter() const
{
  Imp *f = static_cast<GlobalImp*>(rep)->filter;
  return f ? KJSO(f) : KJSO(Null());
}

void *Global::extra() const
{
  return static_cast<GlobalImp*>(rep)->extraData;
}

void Global::setExtra(void *e)
{
  static_cast<GlobalImp*>(rep)->extraData = e;
}

GlobalImp::GlobalImp()
  : ObjectImp(ObjectClass),
    filter(0L),
    extraData(0L)
{
}

void GlobalImp::init()
{
}

GlobalImp::~GlobalImp() { }

void GlobalImp::mark(Imp*)
{
  ObjectImp::mark();
  /* TODO: remove in next version */
  if (filter && !filter->marked())
    filter->mark();
}

void GlobalImp::put(const UString &p, const KJSO& v)
{
  // if we already have this property (added by init() or a variable
  // declaration) overwrite it. Otherwise pass it to the prototype.
  // Needed to get something like a browser's window object working.
  if (!prototype() || hasProperty(p, false) || Imp::hasProperty(p, false))
    Imp::put(p, v);
  else
    prototype()->put(p, v);
#if 0    
  if (filter)
    filter->put(p, v);   /* TODO: remove in next version */
  else
    Imp::put(p, v);
#endif  
}

GlobalFunc::GlobalFunc(int i, int len) : id(i)
{
  put("length",Number(len),DontDelete|ReadOnly|DontEnum);
}

CodeType GlobalFunc::codeType() const
{
  return id == Eval ? EvalCode : InternalFunctionImp::codeType();
}

Completion GlobalFunc::execute(const List &args)
{
  KJSO res;

  static const char non_escape[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz"
				   "0123456789@*_+-./";

  if (id == Eval) { // eval()
    KJSO x = args[0];
    if (x.type() != StringType)
      return Completion(ReturnValue, x);
    else {
      UString s = x.toString().value();

      ProgramNode *progNode = Parser::parse(s.data(),s.size());
      if (!progNode)
	return Completion(ReturnValue, Error::create(SyntaxError));

      // ### use correct script & context
      Completion c = progNode->execute(KJScriptImp::current(),Context::current());
      delete progNode;
      if (c.complType() == ReturnValue)
	  return c;
      else if (c.complType() == Normal) {
	  if (c.isValueCompletion())
	      return Completion(ReturnValue, c.value());
	  else
	      return Completion(ReturnValue, Undefined());
      } else
	  return c;
    }
  } else if (id == ParseInt) {
    String str = args[0].toString();
    int radix = args[1].toInt32();
    if (radix == 0)
      radix = 10;
    else if (radix < 2 || radix > 36) {
      res = Number(NaN);
      return Completion(ReturnValue, res);
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

  return Completion(ReturnValue, res);
}
