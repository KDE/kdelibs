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

#ifndef _TYPES_H_
#define _TYPES_H_

#include "object.h"
#include "function.h"

namespace KJS {

  class StatementNode;
  class UString;

  class Null : public KJSO {
  public:
    Null() { }
    Type type() const { return NullType; }
  };

  class Number : public KJSO {
  public:
    Number(int i) { value.d = static_cast<double>(i); }
    Number(unsigned int u) { value.d = static_cast<double>(u); }
    Number(double d) { value.d = d; }
    Number(long unsigned int l) { value.d = static_cast<double>(l); }
    Type type() const { return NumberType; }
  };

  class String : public KJSO {
  public:
    String(const UString &s) { value.s = new UString(s); }
    Type type() const { return StringType; }
  };

  class Undefined : public KJSO {
  public:
    Type type() const { return UndefinedType; }
  };

  class Boolean : public KJSO {
  public:
    Boolean(bool b) { value.b = b; }
    Type type() const { return BooleanType; }
  };

  class Completion : public KJSO {
  public:
    Completion(Compl c, KJSO *v, const UString &t);
    virtual ~Completion() { if (complVal) complVal->deref(); }
    Type type() const { return CompletionType; }
  };

  class Reference : public KJSO {
  public:
    Reference(KJSO *b, const UString &s);
    ~Reference();
    Type type() const { return ReferenceType; }
  };

  class ParamList {
  public:
    ParamList(int s) : size(s){ param = new UString[s]; }
    ~ParamList() { delete [] param; }
    int count() const { return size; }
    void insert(int i, const UString &s) { if (i<size) param[i] = s; }
    UString at(int i) { if (i<size)
                          return param[i];
                        else
			  return UString::null; }
  private:
    int size;
    UString *param;
  };

  class Property {
  public:
    Property(const UString &n, KJSO *o, int attr = None);
    Type type() const { return PropertyType; }
  public:
    UString name;
    int attribute;
    Ptr object;
    Property *next;
  };

  class Activation : public KJSO {
  public:
    Activation(Function *f, const List *args);
    virtual ~Activation();
    Type type() const { return ObjectType; }
  private:
    Function *func;
  };

  /**
   * @short Execution context.
   */
  class Context {
  public:
    Context(CodeType type = GlobalCode, Context *callingContext = 0L,
	       Function *func = 0L, const List *args = 0L, KJSO *thisV = 0L);
    virtual ~Context();
    static Context *current();
    static void setCurrent(Context *c);
    const List *pScopeChain() const { return scopeChain; }
    void pushScope(KJSO *s);
    void popScope();
    List *copyOfChain() { /* TODO */ return scopeChain; }
    KJSO *variableObject() const { return variable; }
    KJSO *thisValue() const { return thisVal; }
  private:
    KJSO *thisVal;
    Activation *activation;
    KJSO *variable;
    List *scopeChain;
  };

  class DeclaredFunction : public Function {
  public:
    DeclaredFunction(ParamList *p, StatementNode *b);
    Type type() const { return DeclaredFunctionType; }
    KJSO* execute(const List &);
    CodeType codeType() const { return FunctionCode; }
  private:
    StatementNode *block;
  };

  class AnonymousFunction : public Function {
  public:
    AnonymousFunction() { /* TODO */ }
    Type type() const { return AnonymousFunctionType; }
    KJSO* execute(const List &);
    CodeType codeType() const { return AnonymousCode; }
  };

}; // namespace


#endif
