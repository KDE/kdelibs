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

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <assert.h>

#include "object.h"

namespace KJS {

  enum CodeType { GlobalCode,
		  EvalCode,
		  FunctionCode,
		  AnonymousCode,
		  HostCode };

  enum FunctionAttribute { ImplicitNone, ImplicitThis, ImplicitParents };

  class Context;
  class ParamList;

  class Function : public KJSO {
  public:
    Function() { attr = ImplicitNone; }
    void processParameters(List *);
    virtual KJSO* execute(Context *) = 0;
    virtual bool hasAttribute(FunctionAttribute a) const { return (attr & a); }
    virtual CodeType codeType() = 0;
  protected:
    FunctionAttribute attr;
    ParamList *param;
  };

  class InternalFunction : public Function {
  public:
    InternalFunction() { param = 0L; }
    virtual Type type() const { return InternalFunctionType; }
    virtual KJSO* execute(Context *) = 0;
    CodeType codeType() { return HostCode; }
  };

  class Constructor : public InternalFunction {
  public:
    Constructor();
    Constructor(Object *proto, int len);
    virtual Type type() const { return ConstructorType; }
    virtual KJSO* execute(Context *);
    virtual Object* construct(List *args) = 0;
  };

  class Activation;

  class Context {
  public:
    Context(CodeType type = GlobalCode, Context *callingContext = 0L,
	       Function *func = 0L, List *args = 0L, KJSO *thisV = 0L);
    virtual ~Context();
    const List *pScopeChain() const { return scopeChain; }
    void pushScope(KJSO *s);
    void popScope();
    List *copyOfChain() { /* TODO */ return scopeChain; }
    KJSO *variableObject() const { return variable; }
    KJSO *arg(int i);
    int numArgs();
  public:
    KJSO *thisValue;
  private:
    Activation *activation;
    KJSO *variable;
    List *scopeChain;
  };

  class DebugPrint : public InternalFunction {
  public:
    KJSO* execute(KJS::Context *);
  };

}; // namespace

#endif
