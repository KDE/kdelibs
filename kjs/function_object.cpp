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
 */

#include "function_object.h"

#include "lexer.h"
#include "nodes.h"
#include "error_object.h"

extern int kjsyyparse();

using namespace KJS;

FunctionObject::FunctionObject(const Object& funcProto)
  : ConstructorImp(funcProto, 1)
{
  // ECMA 15.3.3.1 Function.prototype
  setPrototypeProperty(funcProto);
}

// ECMA 15.3.1 The Function Constructor Called as a Function
Completion FunctionObject::execute(const List &args)
{
  return Completion(ReturnValue, construct(args));
}

// ECMA 15.3.2 The Function Constructor
Object FunctionObject::construct(const List &args)
{
  UString p("");
  UString body;
  int argsSize = args.size();
  if (argsSize == 0) {
    body = "";
  } else if (argsSize == 1) {
    body = args[0].toString().value();
  } else {
    p = args[0].toString().value();
    for (int k = 1; k < argsSize - 1; k++)
      p += "," + args[k].toString().value();
    body = args[argsSize-1].toString().value();
  }

  Lexer::curr()->setCode(body.data(), body.size());

  KJScriptImp::current()->pushStack();
  int yp = kjsyyparse();
  ProgramNode *progNode = KJScriptImp::current()->progNode();
  KJScriptImp::current()->popStack();
  if (yp) {
    /* TODO: free nodes */
    return ErrorObject::create(SyntaxError,
			       I18N_NOOP("Syntax error in function body"), -1);
  }

  List scopeChain;
  scopeChain.append(Global::current());
  FunctionBodyNode *bodyNode = progNode;
  FunctionImp *fimp = new DeclaredFunctionImp(UString::null, bodyNode,
					      &scopeChain);
  Object ret(fimp); // protect from GC

  // parse parameter list. throw syntax error on illegal identifiers
  int len = p.size();
  const UChar *c = p.data();
  int i = 0, params = 0;
  UString param;
  while (i < len) {
      while (*c == ' ' && i < len)
	  c++, i++;
      if (Lexer::isIdentLetter(c->unicode())) {  // else error
	  param = UString(c, 1);
	  c++, i++;
	  while (i < len && (Lexer::isIdentLetter(c->unicode()) ||
			     Lexer::isDecimalDigit(c->unicode()))) {
	      param += UString(c, 1);
	      c++, i++;
	  }
	  while (i < len && *c == ' ')
	      c++, i++;
	  if (i == len) {
	      fimp->addParameter(param);
	      params++;
	      break;
	  } else if (*c == ',') {
	      fimp->addParameter(param);
	      params++;
	      c++, i++;
	      continue;
	  } // else error
      }
      return ErrorObject::create(SyntaxError,
				 I18N_NOOP("Syntax error in parameter list"),
				 -1);
  }

  fimp->setLength(params);
  fimp->setPrototype(Global::current().functionPrototype());
  List consArgs;
  KJSO objCons = Global::current().get("Object");
  KJSO prototype = static_cast<ConstructorImp*>(objCons.imp())->construct(consArgs);
  prototype.setConstructor(fimp);
  fimp->setPrototypeProperty(prototype);
  fimp->put("arguments",Null());
  return ret;
}

// a hack to avoid returning "(Internal function)"
String FunctionObject::toString() const
{
  return FunctionImp::toString();
}

FunctionPrototype::FunctionPrototype(const Object &p)
    : FunctionImp()
{
  setPrototype(p);
  put("length",Number(0),DontDelete|ReadOnly|DontEnum);

  put("toString", new FunctionProtoFunc(p,FunctionProtoFunc::ToString, 0), DontEnum);
  put("apply",    new FunctionProtoFunc(p,FunctionProtoFunc::Apply,    2), DontEnum);
  put("call",     new FunctionProtoFunc(p,FunctionProtoFunc::Call,     1), DontEnum);
}

// ECMA 15.3.4 invoking Function.prototype() returns undefined
Completion FunctionPrototype::execute(const List &/*args*/)
{
  return Completion(ReturnValue, Undefined());
}

// ECMA 15.3.4
FunctionProtoFunc::FunctionProtoFunc(const Object &objProto, int i, int len)
  : id(i)
{
  setPrototype(objProto);
  put("length",Number(len),DontDelete|ReadOnly|DontEnum);
}

Completion FunctionProtoFunc::execute(const List &args)
{
  KJSO result;

  switch (id) {
  case ToString: {
    Object thisObj = Object::dynamicCast(thisValue());
    if (thisObj.isNull() || thisObj.getClass() != StringClass) {
      result = Error::create(TypeError);
      return Completion(ReturnValue, result);
    }
    FunctionImp *func = static_cast<FunctionImp*>(thisObj.imp());
    if (func->name().isNull())
      result = String("(Internal function)");
    else
      result = String("function " + func->name() + "()");
    }
    break;
  case Apply: {
    KJSO thisArg = args[0];
    KJSO argArray = args[1];
    KJSO func = thisValue();

    if (!func.implementsCall()) {
      result = Error::create(TypeError);
      return Completion(ReturnValue, result);
    }

    KJSO thisV;
    if (thisArg.isA(NullType) || thisArg.isA(UndefinedType))
      thisV = Global::current();
    else
      thisV = args[0];

    List applyArgs;
    if (!argArray.isA(NullType) && !argArray.isA(UndefinedType)) {
      if ((argArray.isA(ObjectType) &&
           Object::dynamicCast(argArray).getClass() == ArrayClass) ||
           argArray.isA(ArgumentsType)) {

        unsigned int length = argArray.get("length").toUInt32();
        for (uint i = 0; i < length; i++)
          applyArgs.append(argArray.get(UString::from(i)));
      }
      else {
        result = Error::create(TypeError);
        return Completion(ReturnValue, result);
      }
    }
    result = func.executeCall(thisV,&applyArgs);
    }
    break;
  case Call: {
    KJSO thisArg = args[0];
    KJSO func = thisValue();

    if (!func.implementsCall()) {
      result = Error::create(TypeError);
      return Completion(ReturnValue, result);
    }

    KJSO thisV;
    if (thisArg.isA(NullType) || thisArg.isA(UndefinedType))
      thisV = Global::current();
    else
      thisV = args[0];

    List *callArgs = args.copy();
    callArgs->removeFirst();
    result = func.executeCall(thisV,callArgs);
    delete callArgs;
    }
    break;
  }

  return Completion(ReturnValue, result);
}

