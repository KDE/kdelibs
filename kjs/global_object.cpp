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

#include "kjs.h"
#include "object.h"
#include "operations.h"
#include "types.h"
#include "lexer.h"
#include "nodes.h"
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
class GlobalFunc : public InternalFunction {
public:
  GlobalFunc(int i) : id(i) { }
  KJSO *execute(const List &c);
  enum { Eval, ParseInt, ParseFloat };
private:
  int id;
};

Global::Global()
{
  // constructor properties. prototypes as Global's member variables first.
  objProto = new ObjectPrototype();
  funcProto = new FunctionPrototype();
  arrayProto = new ArrayPrototype(objProto);
  stringProto = new StringPrototype(objProto);
  booleanProto = new BooleanPrototype(objProto);
  numberProto = new NumberPrototype(objProto);
  dateProto = new DatePrototype(objProto);
  regexpProto = new RegExpPrototype(objProto);
  errorProto = new ErrorPrototype(objProto);

  Ptr objectObj = new ObjectObject(objProto);
  Ptr arrayObj = new ArrayObject(funcProto);
  Ptr boolObj = new BooleanObject(funcProto);
  Ptr stringObj = new StringObject(funcProto);
  Ptr numObj = new NumberObject(funcProto);
  Ptr dateObj = new DateObject(dateProto);
  Ptr regObj = new RegExpObject(regexpProto);
  Ptr errObj = new ErrorObject(errorProto);

  put("Object", objectObj, DontEnum);
  put("Array", arrayObj, DontEnum);
  put("Boolean", boolObj, DontEnum);
  put("String", stringObj, DontEnum);
  put("Number", numObj, DontEnum);
  put("Date", dateObj, DontEnum);
  put("RegExp", regObj, DontEnum);
  put("Error", errObj, DontEnum);

  objProto->setConstructor(objectObj);
  arrayProto->setConstructor(arrayObj);
  booleanProto->setConstructor(boolObj);
  stringProto->setConstructor(stringObj);
  numberProto->setConstructor(numObj);
  dateProto->setConstructor(dateObj);
  regexpProto->setConstructor(regObj);
  errorProto->setConstructor(errObj);

#if 0
  objProto->deref();
  funcProto->deref();
  arrayProto->deref();
  stringProto->deref();
  boolProto->deref();
  numProto->deref();
#endif

  // value properties
  put("NaN", NaN, DontEnum | DontDelete);
  put("Infinity", Inf, DontEnum | DontDelete);
  put("undefined", zeroRef(newUndefined()), DontEnum | DontDelete);
  put("eval", zeroRef(new GlobalFunc(GlobalFunc::Eval)));
  put("parseInt", zeroRef(new GlobalFunc(GlobalFunc::ParseInt)));
  put("parseFloat", zeroRef(new GlobalFunc(GlobalFunc::ParseFloat)));

  // other properties
  put("Math", zeroRef(new Math()), DontEnum);
}

KJSO *GlobalFunc::execute(const List &args)
{
  Ptr res;

  if (id == Eval) { // eval()
    Ptr x = args[0];
    if (x->type() != StringType)
      res = x;
    else {
      Lexer::curr()->setCode(x->stringVal().data(), x->stringVal().size());
      if (kjsyyparse()) {
	KJS::Node::deleteAllNodes();
	return newCompletion(Normal, newError(SyntaxError)); /* TODO: zeroRef */
      }

      res = KJS::Node::progNode()->evaluate();
      /* TODO: analyse completion value */
      res.release();
      if (error())
	error()->deref();

      //      if (KJS::Node::progNode())
      //	KJS::Node::progNode()->deleteStatements();

      res = newUndefined();
    }
  } else if (id == ParseInt) {
    Ptr str = toString(args[0]);
    int radix = toInt32(args[1]);
    if (radix == 0)
      radix = 10;
    else if (radix < 2 || radix > 36) {
      res = newNumber(NaN);
      return newCompletion(Normal, res);
    }
    /* TODO: use radix */
    int i = 0;
    sscanf(str->stringVal().ascii(), "%d", &i);
    res = newNumber(i);
  } else if (id == ParseFloat) {
    Ptr str = toString(args[0]);
    double d = 0.0;
    sscanf(str->stringVal().ascii(), "%lf", &d);
    res = newNumber(d);
  }

  return newCompletion(Normal, res);
}
