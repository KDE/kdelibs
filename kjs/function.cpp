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
#include "types.h"
#include "operations.h"
#include "nodes.h"

using namespace KJS;

Constructor::Constructor()
{
  setPrototype(KJScript::global()->funcProto);
  put("constructor", this);
  put("length", 1, DontEnum);
}

Constructor::Constructor(Object *proto, int len)
{
  setPrototype(proto);
  put("constructor", this);
  put("length", len, DontEnum);
}

KJSO* Constructor::execute(const List &)
{
  /* TODO: call construct instead ? */
  return newUndefined();
}

// ECMA 10.1.7 (draft April 98, 10.1.6 previously)
Activation::Activation(Function *f, const List *args)
{
  func = f;

  /* TODO: solve deleting problem due to circular reference */
  // put("arguments", this, DontDelete | DontEnum);
  if (func->hasProperty("arguments"))
    put("OldArguments", func->get("arguments"));
  put("callee", func, DontEnum);
  if (args) {
    put("length", args->size(), DontEnum);
    ListIterator arg = args->begin();
    for (int i = 0; arg != args->end(); arg++, i++) {
      put(int2String(i), arg);
    }
  }
  /* TODO: solve deleting problem due to circular reference */
  //  func->put("arguments", this);
}

// ECMA 10.1.6
Activation::~Activation()
{
  if (!hasProperty("OldArguments"))
    func->deleteProperty("arguments");
  else
    func->put("arguments", get("OldArguments")); /* TODO: deep copy ? */
}

// ECMA 10.2
Context::Context(CodeType type, Context *callingContext,
		       Function *func, const List *args, KJSO *thisV)
{
  Global *glob = KJScript::global();
  assert(glob);

  // create and initialize activation object (ECMA 10.1.6)
  if (type == FunctionCode || type == AnonymousCode || type == HostCode) {
    activation = new Activation(func, args);
    variable = activation;
  } else {
    activation = 0L;
    variable = glob;
  }

  // ECMA 10.2
  switch(type) {
    case EvalCode:
      if (callingContext) {
	scopeChain = callingContext->copyOfChain();
	variable = callingContext->variableObject();
	thisVal = callingContext->thisValue();
	break;
      } // else same as GlobalCode
    case GlobalCode:
      scopeChain = new List();
      scopeChain->append(glob);
      variable = glob;
      thisVal = glob;
      break;
    case FunctionCode:
    case AnonymousCode:
      scopeChain = new List();
      scopeChain->append(activation);
      scopeChain->append(glob);
      variable = activation; /* TODO: DontDelete ? (ECMA 10.2.3) */
      if (thisV->isA(ObjectType)) {
	thisVal = thisV;
      }
      else
	thisVal = glob;
      break;
    case HostCode:
      if (thisV->isA(ObjectType))
	thisVal = thisV;
      else
	thisVal = glob;
      variable = activation; /* TODO: DontDelete (ECMA 10.2.4) */
      scopeChain = new List();
      scopeChain->append(activation);
      if (func->hasAttribute(ImplicitThis))
	scopeChain->append(thisVal);
      if (func->hasAttribute(ImplicitParents)) {
	/* TODO ??? */
      }
      scopeChain->append(glob);
      break;
    }
}

Context::~Context()
{
  scopeChain->deref();

  if (activation)
    activation->deref();
    // delete activation;
}

Context *Context::current()
{
  return KJScript::current()->con;
}

void Context::setCurrent(Context *c)
{
  KJScript::current()->con = c;
}

void Context::pushScope(KJSO *s)
{
  scopeChain->prepend(s);
}

void Context::popScope()
{
  scopeChain->removeFirst();
}

// ECMA 10.1.3
void Function::processParameters(const List *args)
{
  KJSO *variable = Context::current()->variableObject();

  assert(args);

  if (param) {
    ListIterator it = args->begin();
    for(int i = 0; i < param->count() && i < 100; i++)
      if (it != args->end()) {
	variable->put(param->at(i), it);
	it++;
      } else
	variable->put(param->at(i), zeroRef(newUndefined()));
  }
}

KJSO *Function::thisValue() const
{
  return Context::current()->thisValue();
}

DeclaredFunction::DeclaredFunction(ParamList *p, StatementNode *b)
  : block(b)
{
  param = p;
}

KJSO* DeclaredFunction::execute(const List &)
{
 /* TODO */
  return block->evaluate();
}

KJSO* AnonymousFunction::execute(const List &)
{
 /* TODO */
  return 0L;
}

KJSO *DebugPrint::execute(const List &args)
{
  Ptr v = args[0];
  Ptr s = toString(v);
  fprintf(stderr, "---> %s\n", s->stringVal().cstring().c_str());

  return newCompletion(Normal);
}
