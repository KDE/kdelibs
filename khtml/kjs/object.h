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
class KJSObject;
class KJSPrototype;
class KJSProperty;
class KJSParamList;
class KJSConstructor;
class KJSList;
class Node;
class StatementNode;

// this is class is a terrible mess. Dynamic casts might
// allow some simplifications.
class KJSO {
public:
  KJSO() { init(); }
  virtual ~KJSO();
public:
  void init();
  virtual Type type() const { assert(!"Undefined type()"); return Undefined; };
  bool isA(Type t) const { return (type() == t); }
  bool isObject() const { return (type() >= Object); }

#ifdef KJS_DEBUG_MEM
  static int count;
  static KJSO* firstObject;
  KJSO* nextObject, *prevObject;
  int objId;
  static int lastId;
#endif

  // Properties
  void setPrototype(KJSPrototype *p);
  KJSPrototype *prototype() const { return proto; }
  virtual KJSO *get(const CString &p) const;
  bool hasProperty(const CString &p, bool recursive = true) const;
  virtual void put(const CString &p, KJSO *v, int attr = None);
  void put(const CString &p, double d, int attr = None);
  void put(const CString &p, int i, int attr = None);
  void put(const CString &p, unsigned int u, int attr = None);
  bool canPut(const CString &p) const;
  void deleteProperty(const CString &p);
  KJSO *defaultValue(Hint hint = NoneHint);
  void dump(int level = 0);
  virtual KJSObject *construct() { return 0L; }

private:
  void putArrayElement(const CString &p, KJSO *v);

public:
  int refCount;
public:
  KJSO *ref() { refCount++; return this; }
  void deref() { assert(refCount > 0); if(!--refCount) delete this; }

  // Reference
  KJSO *getBase();
  CString getPropertyName();
  KJSO *getValue();
  ErrorCode putValue(KJSO *v);

  // internal value
  bool bVal() { assert(type()==Boolean); return val.b; }
  double dVal()  { assert(type()==Number); return val.d; }
  const UString sVal()  { assert(type()==String); return *(val.s); }
  Compl cVal() { assert(type()==Completion); return val.c; }
  bool isValueCompletion() { assert(type()==Completion); return (complVal); }
  KJSO *complValue() { assert(type()==Completion); return complVal->ref(); }

  // function call
  bool implementsCall() const { return (type() == InternalFunction ||
				  type() == DeclaredFunction ||
				  type() == AnonymousFunction); }
  KJSO *executeCall(KJSO *thisV, KJSList *args);
  KJSO* (*call)(KJSO*);

  // constructor
  void setConstructor(KJSConstructor *c);
  KJSConstructor *constructor() const { return constr; }
  bool implementsConstruct() const { return (constr); }
  KJSObject *construct(KJSList *args);

private:
  KJSPrototype *proto;
  KJSConstructor *constr;
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

class Ptr {
public:
  Ptr() { obj = 0L; }
  Ptr(KJSO *o) { assert(o); obj = o; }
  ~Ptr() { if (obj) obj->deref(); }
  Ptr *operator=(KJSO *o) { if (obj) obj->deref(); obj = o; return this; }
  KJSO* operator->() { return obj; }

  operator KJSO*() { return obj; }
  void release() { obj->deref(); obj = 0L; }
  KJSO *ref() { obj->ref(); return obj; }
  Ptr(const Ptr &) { assert(0); }
private:
  Ptr& operator=(const Ptr &);
  KJSO *obj;
};

KJSO *zeroRef(KJSO *obj);

class KJSList;
class KJSListIterator;

class KJSListNode {
  friend KJSList;
  friend KJSListIterator;
  KJSListNode(KJSO *obj, KJSListNode *p, KJSListNode *n)
    : member(obj), prev(p), next(n) {};
  ~KJSListNode() { if (member) member->deref(); }

  KJSO *member;
  KJSListNode *prev, *next;
};

class KJSListIterator {
  friend KJSList;
public:
  KJSListIterator(KJSListNode *n) : node(n) { }

  KJSO* operator->() const { return node->member; }
  operator KJSO*() const { return node->member; }
  KJSListNode* operator++() { node = node->next; return node; }
  KJSListNode* operator++(int) { KJSListNode *n = node; ++*this; return n; }
  KJSListNode* operator--() { node = node->prev; return node; }
  KJSListNode* operator--(int) { KJSListNode *n = node; --*this; return n; }
  bool operator==(KJSListIterator i) const { return (node==i.node); }
  bool operator!=(KJSListIterator i) const { return (node!=i.node); }
private:
  KJSListNode *node;
};

class KJSList : public KJSO {
public:
  KJSList();
  ~KJSList();
  Type type() const { return List; }
  void append(KJSO *obj);
  void prepend(KJSO *obj);
  void removeFirst();
  void removeLast();
  void clear();
  KJSListIterator begin() const { return KJSListIterator(hook->next); }
  KJSListIterator end() const { return KJSListIterator(hook); }
  bool isEmpty() const { return (hook->prev == hook); }
  int size() const;
private:
  void erase(KJSListIterator it);
private:
  KJSListNode *hook;
};

class KJSProperty {
public:
  KJSProperty(const CString &n, KJSO *o, int attr = None);
  Type type() const { return Property; }
public:
  CString name;
  int attribute;
  Ptr object;
  KJSProperty *next;
};


class KJSReference : public KJSO {
public:
  KJSReference(KJSO *b, const CString &s);
  ~KJSReference();
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
  KJSNumber(unsigned int u) { val.d = static_cast<double>(u); }
  KJSNumber(double d) { val.d = d; }
  KJSNumber(long unsigned int l) { val.d = static_cast<double>(l); }
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

class KJSContext;

class KJSFunction : public KJSO {
public:
  KJSFunction() { attr = ImplicitNone; }
  void processParameters(KJSList *);
  virtual KJSO* execute(KJSContext *) = 0;
  virtual bool hasAttribute(FunctionAttribute a) const { return (attr & a); }
  virtual CodeType codeType() = 0;
protected:
  FunctionAttribute attr;
  KJSParamList *param;
};

class KJSInternalFunction : public KJSFunction {
public:
  KJSInternalFunction() { param = 0L; }
  Type type() const { return InternalFunction; }
  KJSO* execute(KJSContext *) = 0;
  CodeType codeType() { return HostCode; }
};

class KJSDeclaredFunction : public KJSFunction {
public:
  KJSDeclaredFunction(KJSParamList *p, StatementNode *b);
  Type type() const { return DeclaredFunction; }
  KJSO* execute(KJSContext *);
  CodeType codeType() { return FunctionCode; }
private:
  StatementNode *block;
};

class KJSAnonymousFunction : public KJSFunction {
public:
  KJSAnonymousFunction() { /* TODO */ }
  Type type() const { return AnonymousFunction; }
  KJSO* execute(KJSContext *);
  CodeType codeType() { return AnonymousCode; }
};

class KJSConstructor : public KJSO {
public:
  Type type() const { return Constructor; }
  virtual KJSObject* construct(KJSList *args) = 0;
};

class KJSCompletion : public KJSO {
public:
  KJSCompletion(Compl c, KJSO *v = 0L)
    { val.c = c; complVal = v ? v->ref() : 0L; }
  virtual ~KJSCompletion() { if (complVal) complVal->deref(); }
  Type type() const { return Completion; }
};

class KJSObject : public KJSO {
public:
  KJSObject() : cl(UndefClass), internVal(0L) { }
  ~KJSObject() { if (internVal) internVal->deref(); }
  Type type() const { return Object; }
  void setClass(Class c) { cl = c; } 
  Class getClass() const { return cl; }
  void setInternalValue(KJSO *v) { internVal = v->ref(); }
  KJSO *internalValue() { return internVal->ref(); }
private:
  Class cl;
  KJSO* internVal;
};

class KJSPrototype : public KJSObject {
  // for type safety
};

typedef KJSList KJSScopeChain;

class KJSActivation : public KJSO {
public:
  KJSActivation(KJSFunction *f, KJSList *args);
  virtual ~KJSActivation();
  Type type() const { return Object; }
private:
  KJSFunction *func;
};

class KJSArguments : public KJSO {
public:
  KJSArguments(KJSFunction *func, KJSList *args);
  Type type() const { return Object; }
};

class KJSContext {
public:
  KJSContext(CodeType type = GlobalCode, KJSContext *callingContext = 0L,
	     KJSFunction *func = 0L, KJSList *args = 0L, KJSO *thisV = 0L);
  virtual ~KJSContext();
  const KJSScopeChain *pScopeChain() const { return scopeChain; }
  void pushScope(KJSO *s) { scopeChain->prepend(s); }
  void popScope() { scopeChain->removeFirst(); }
  KJSScopeChain *copyOfChain() { /* TODO */ return scopeChain; }

  KJSO *variableObject() const { return variable; }
public:
  KJSO *thisValue;
  KJSActivation *activation;
private:
  KJSO *variable;
  KJSScopeChain *scopeChain;
};

class KJSGlobal : public KJSO {
public:
  KJSGlobal();
  Type type() const { return Object; }
  KJSPrototype *objProto, *funcProto, *arrayProto, *stringProto, *boolProto;
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

class KJSError : public KJSO {
public:
  KJSError(ErrorCode e, Node *n);
  KJSError(ErrorCode e, KJSO *o);
  Type type() const { return Error; }
  ErrorCode number() const { return errNo; }
private:
  ErrorCode errNo;
  int line;
};

  class HostObject : public KJSO {
  public:
    Type type() const { return Host; }
    virtual KJSO *get(const CString &p) const;
    virtual void put(const CString &p, KJSO *v, int attr = None);
  };

};


#endif
