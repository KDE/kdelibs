/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <assert.h>

#include "global.h"
#include "kjsstring.h"

namespace KJS {

union Value {
  bool b;
  double d;
  UString *s;
  Compl c;
};

// forward declarations
class KJSProperty;
class KJSArgList;
class KJSParamList;
class StatementNode;

class KJSO {
public:
  KJSO() { init(); }
  virtual ~KJSO() { /* TODO: delete String object */ }
  void init() { proto = 0L; prop = 0L; call = 0L; }
  virtual Type type() const { assert(!"Undefined type()"); return Undefined; };
  bool isA(Type t) const { return (type() == t); }
  bool isObject() const { return (type() >= Object); }

  // Properties
  KJSO *prototype() const { return proto; }
  Class getClass() const { return cl; }
  KJSO *get(const CString &p) const;
  bool hasProperty(const CString &p) const;
  void put(const CString &p, KJSO *v, int attr = None);
  bool canPut(const CString &p) const;
  void deleteProperty(const CString &p);
  KJSO defaultValue(Hint hint = NoneHint);
  void dump();
  virtual KJSO *construct() { return 0L; }

  // Reference
  KJSO *getBase();
  CString getPropertyName();
  KJSO *getValue();
  void putValue(KJSO *v);

  // internal value
  bool bVal() { assert(type()==Boolean); return val.b; }
  double dVal()  { assert(type()==Number); return val.d; }
  const UString sVal()  { assert(type()==String); return *(val.s); }
  Compl cVal() { assert(type()==Completion); return val.c; }
  bool isValueCompletion() { assert(type()==Completion); return (complVal); }
  KJSO *complValue() { assert(type()==Completion); return complVal; }

  // function call
  bool implementsCall() const { return (type() == InternalFunction ||
				  type() == DeclaredFunction ||
				  type() == AnonymousFunction); }
  KJSO *executeCall(KJSO *, KJSArgList *);
  KJSO* (*call)(KJSO*);

  // constructor
  bool implementsConstruct() const { return true; /* TODO */ }
  KJSO *construct(KJSO *) { /* TODO */ return 0L; }

protected:
  KJSO *proto;
  Class cl;
  KJSProperty *prop;

protected:
  Value val;

  // references:
protected:
  KJSO *base;
  CString propname;

  // completion:
protected:
  KJSO *complVal;
};

class KJSProperty {
public:
  KJSProperty(const CString &n, KJSO *o, int attr = None);
  ~KJSProperty();
  Type type() const { return Property; }
public:
  CString name;
  int attribute;
  KJSO *object;
  KJSProperty *next;
};


class KJSReference : public KJSO {
public:
  KJSReference(KJSO *b, const CString &s);
  Type type() const { return Reference; }
};

class KJSNull : public KJSO {
public:
  KJSNull() { }
  Type type() const { return Null; }
};

class KJSNumber : public KJSO {
public:
  KJSNumber(int i) { val.d = static_cast<double>(i); }
  KJSNumber(double d) { val.d = d; }
  Type type() const { return Number; }
};

class KJSString : public KJSO {
public:
  KJSString(const UString &s) { val.s = new UString(s); }
  Type type() const { return String; }
};

class KJSUndefined : public KJSO {
public:
  Type type() const { return Undefined; }

};

class KJSBoolean : public KJSO {
public:
  KJSBoolean(bool b) { val.b = b; }
  Type type() const { return Boolean; }
};

typedef KJSO* (*fPtr)();

class KJSFunction : public KJSO {
public:
  KJSFunction() { attr = ImplicitNone; }
  void processParameters(KJSArgList *);
  virtual KJSO* execute() = 0;
  virtual bool hasAttribute(FunctionAttribute a) const { return (attr & a); }
protected:
  FunctionAttribute attr;
  KJSParamList *param;
};

class KJSInternalFunction : public KJSFunction {
public:
  KJSInternalFunction(KJSO* (*f)()) { func = f; }
  Type type() const { return InternalFunction; }
  KJSO* execute() { return (*func)(); }
private:
  KJSO* (*func)();
};

class KJSDeclaredFunction : public KJSFunction {
public:
  KJSDeclaredFunction(KJSParamList *p, StatementNode *b);
  Type type() const { return DeclaredFunction; }
  KJSO* execute();
private:
  StatementNode *block;
};

class KJSAnonymousFunction : public KJSFunction {
public:
  KJSAnonymousFunction() { /* TODO */ }
  Type type() const { return AnonymousFunction; }
  KJSO* execute() { /* TODO */ }
};

class KJSCompletion : public KJSO {
public:
  KJSCompletion(Compl c, KJSO *v = 0L)
    { val.c = c; complVal = v; }
  Type type() const { return Completion; }
};

class KJSObject : public KJSO {
public:
  KJSObject() {}
  Type type() const { return Object; }
};

class KJSScope : public KJSO {
public:
  KJSScope(KJSO *o) : object(o), next(0L) {}
  Type type() const { return Scope; }
  void append(KJSO *o) { next = new KJSScope(o); next->next = 0L; }

  KJSO *object;
  KJSScope *next;
};

class KJSActivation : public KJSO {
public:
  KJSActivation(KJSFunction *f, KJSArgList *args);
  ~KJSActivation();
  Type type() const { return Activation; }
private:
  KJSFunction *func;
};

class KJSArguments : public KJSO {
public:
  KJSArguments(KJSFunction *func, KJSArgList *args);
};

class KJSContext {
public:
  KJSContext(CodeType type = GlobalCode, KJSContext *callingContext = 0L,
	     KJSFunction *func = 0L, KJSArgList *args = 0L);
  ~KJSContext();
  KJSScope *firstScope() const { return scopeChain; }
  void insertScope(KJSO *s);
  KJSScope *copyOfChain() { /* TODO */ return scopeChain; }

  KJSO *variableObject() const { return variable; }
public:
  KJSO *thisValue;
  KJSActivation *activation;
  KJSArguments *arguments;
private:
  KJSO *variable;
  KJSScope *scopeChain;
};

class KJSGlobal : public KJSO {
public:
  KJSGlobal();
  Type type() const { return Object; }
private:
  static KJSO* eval();
};

class KJSArgList;

class KJSArg {
  friend KJSArgList;
public:
  KJSArg(KJSO *o) : obj(o), next(0L) {}
  //  Type type() const { return ArgList; }
  KJSArg *nextArg() const { return next; }
  KJSO *object() const { return obj; }
private:
  KJSO *obj;
  KJSArg *next;
};

class KJSArgList : public KJSO {
public:
  KJSArgList() : first(0L) {}
  Type type() const { return ArgList; }
  KJSArgList *append(KJSO *o);
  KJSArg *firstArg() const { return first; }
  int count() const;
private:
  KJSArg *first;
};

class KJSParamList {
public:
  KJSParamList(int s) : size(s){ param = new CString[s]; }
  ~KJSParamList() { delete [] param; }
  int count() const { return size; }
  void insert(int i, const CString &s) { if (i<size) param[i] = s; }
  const char *at(int i) { if (i<size)
                            return param[i].ascii();
                          else
			    return 0L; }
private:
  int size;
  CString *param;
};


};

#endif
