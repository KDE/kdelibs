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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <assert.h>

#include "ustring.h"

/**
 * @short Main namespace
 */
namespace KJS {

  /**
   * Types of classes derived from KJSO
   */
  enum Type { // main types
              UndefinedType,
	      NullType,
	      BooleanType,
	      NumberType,
	      StringType,
	      ObjectType,
	      HostType,
	      ReferenceType,
	      ListType,
              CompletionType,
	      // extended types
	      PropertyType,
	      InternalFunctionType,
	      DeclaredFunctionType,
	      AnonymousFunctionType,
	      ConstructorType,
	      ActivationType,
	      ErrorType
  };

  /**
   * Property attributes.
   */
  enum Attribute { None       = 0,
		   ReadOnly   = 1 << 1,
		   DontEnum   = 1 << 2,
		   DontDelete = 1 << 3,
		   Internal   = 1 << 4 };

  /**
   * Types of classes derived from @ref Object.
   */
  enum Class { UndefClass,
	       ArrayClass,
	       StringClass,
	       BooleanClass,
	       NumberClass,
	       ObjectClass };

  enum Compl { Normal, Break, Continue, ReturnValue, Throw };

  enum ErrorCode { ErrOK,
		   ErrParser,
		   ErrUndefToObject,
		   ErrNullToObject,
		   ErrFuncNoObject,
		   ErrFuncNoCall,
		   ErrBaseIsNull,
		   ErrNoReference,
		   ErrBaseNoRef,
		   ErrExprNoObject,
		   ErrNoConstruct,
		   ErrResNoObject,
		   ErrNoDefault,
		   ErrInvalidThis
  };

  extern const double NaN;
  extern const double Inf;

  // forward declarations
  class Object;
  class Property;
  class List;
  class Node;

  /**
   * @short Main base class for every KJS object.
   */
  // this is class is a terrible mess. Dynamic casts might
  // allow some simplifications.
  class KJSO {
  public:
    /**
     * Constructor.
     */
    KJSO();
    /**
     * Destructor.
     */
    virtual ~KJSO();
    /**
     * @return the type of the object. One of the @ref KJS::Type enums.
     */
    virtual Type type() const = 0L;
    /**
     * Check whether object is of a certain type
     * @param t type to check for
     */
    bool isA(Type t) const { return (type() == t); }
    /**
     * Use this method when checking for objects. It's safer than checking
     * for a single object type with @ref isA().
     */
    bool isObject() const { return (type() >= ObjectType); }

#ifdef KJS_DEBUG_MEM
    static int count;
    static KJSO* firstObject;
    KJSO* nextObject, *prevObject;
    int objId;
    static int lastId;
#endif

    // factories
    static KJSO *newNull();
    static KJSO *newUndefined();
    static KJSO *newBoolean(bool b);
    static KJSO *newString(const UString &s);
    static KJSO *newNumber(double d);
    static KJSO *newNumber(int i);
    static KJSO *newNumber(unsigned int u);
    static KJSO *newNumber(long unsigned int l);
    static KJSO *newCompletion(Compl c, KJSO *v = 0L,
			       const UString &t = UString::null);
    static KJSO *newReference(KJSO *b, const UString &s);
    static KJSO *newError(ErrorCode e, Node *n);
    static KJSO *newError(ErrorCode e, KJSO *o = 0L);

    // Properties
    void setPrototype(Object *p);
    Object *prototype() const { return pprototype; }
    virtual KJSO *get(const UString &p);
    virtual bool hasProperty(const UString &p, bool recursive = true) const;
    virtual void put(const UString &p, KJSO *v);
    virtual bool canPut(const UString &p) const;
    virtual void deleteProperty(const UString &p);

    void put(const UString &p, KJSO *v, int attr);
    void put(const UString &p, double d, int attr = None);
    void put(const UString &p, int i, int attr = None);
    void put(const UString &p, unsigned int u, int attr = None);

    KJSO *defaultValue(Type hint = UndefinedType);
    void dump(int level = 0);

    // Reference
    KJSO *getBase();
    UString getPropertyName();
    KJSO *getValue();
    ErrorCode putValue(KJSO *v);

    // internal value
    bool boolVal() { assert(isA(BooleanType)); return value.b; }
    double doubleVal()  { assert(isA(NumberType)); return value.d; }
    int intVal() const { assert(isA(NumberType)); return (int) value.d; }
    const UString stringVal()  { assert(isA(StringType)); return *(value.s); }
    Compl complType() { assert(isA(CompletionType)); return value.c; }
    bool isValueCompletion() { assert(isA(CompletionType)); return complVal; }
    KJSO *complValue() { assert(isA(CompletionType)); return complVal->ref(); }

    // function call
    bool implementsCall() const;
    KJSO *executeCall(KJSO *thisV, List *args);

    // constructor
    void setConstructor(KJSO *c);
    bool implementsConstruct() const { return isA(ConstructorType); }

    // reference counting
    KJSO *ref() { refCount++; return this; }
    void deref() { assert(refCount > 0); if(!--refCount) delete this; }
    friend KJSO *zeroRef(KJSO *obj);

  protected:
    /**
     * @internal
     */
    // multi purpose
    union Value {
      bool b;
      double d;
      UString *s;
      Compl c;
      KJSO *base;
    };
    Value value;
    // for references and completion target
    UString strVal;
    // completion value
    KJSO *complVal;

  private:
    // disallow copy constructor and assignment operator
    KJSO(const KJSO &);
    KJSO& operator=(const KJSO &);
    void init();
    void putArrayElement(const UString &p, KJSO *v);
    int refCount;
    Object *pprototype;
    Property *prop;
  }; // end of KJSO

  /**
   * @short Smart pointer holding references to @ref KJS::KJSO.
   */
  class Ptr {
  public:
    Ptr() : obj(0L) { }
    Ptr(KJSO *o) { assert(o); obj = o; }
    Ptr(const Ptr &p) { /* TODO */ obj = p.obj->ref(); }
    Ptr& operator=(const Ptr &p) { if (obj) obj->deref(); obj = p.obj->ref();
                                   return *this;}
    ~Ptr() { if (obj) obj->deref(); }
    Ptr *operator=(KJSO *o) { if (obj) obj->deref(); obj = o; return this; }
    KJSO* operator->() const { return obj; }

    operator KJSO*() const { return obj; }
    void release() { obj->deref(); obj = 0L; }
    KJSO *ref() { obj->ref(); return obj; }
  private:
    KJSO *obj;
  };

  KJSO *zeroRef(KJSO *obj);

  class List;
  class ListIterator;

  /**
   * @internal
   */
  class ListNode {
    friend List;
    friend ListIterator;
    ListNode(KJSO *obj, ListNode *p, ListNode *n)
      : member(obj), prev(p), next(n) {};
    ~ListNode() { if (member) member->deref(); }

    KJSO *member;
    ListNode *prev, *next;
  };

  /**
   * @short Iterator for @ref KJS::List objects.
   */
  class ListIterator {
    friend List;
  public:
    ListIterator(ListNode *n) : node(n) { }

    KJSO* operator->() const { return node->member; }
    operator KJSO*() const { return node->member; }
    ListNode* operator++() { node = node->next; return node; }
    ListNode* operator++(int) { ListNode *n = node; ++*this; return n; }
    ListNode* operator--() { node = node->prev; return node; }
    ListNode* operator--(int) { ListNode *n = node; --*this; return n; }
    bool operator==(ListIterator i) const { return (node==i.node); }
    bool operator!=(ListIterator i) const { return (node!=i.node); }
  private:
    ListNode *node;
  };

  /**
   * @short Native list type.
   */
  class List : public KJSO {
  public:
    List();
    ~List();
    Type type() const { return ListType; }
    void append(KJSO *obj);
    void prepend(KJSO *obj);
    void removeFirst();
    void removeLast();
    void clear();
    ListIterator begin() const { return ListIterator(hook->next); }
    ListIterator end() const { return ListIterator(hook); }
    bool isEmpty() const { return (hook->prev == hook); }
    int size() const;
    KJSO *at(int i) const;
    KJSO *operator[](int i) const { return at(i); }
  private:
    void erase(ListIterator it);
  private:
    ListNode *hook;
  };

  /**
   * @short Object class encapsulating an internal value.
   */
  class Object : public KJSO {
  public:
    Object(Class c = UndefClass, KJSO *v = 0L, Object *p = 0L);
    ~Object() { if (objValue) objValue->deref(); }
    Type type() const { return ObjectType; }
    void setClass(Class c) { classType = c; }
    Class getClass() const { return classType; }
    void setInternalValue(KJSO *v) { objValue = v ? v->ref() : 0L; }
    KJSO *internalValue() { return objValue->ref(); }
    static Object *create(Class c, KJSO *val = 0L, Object *p = 0L);
  private:
    Class classType;
    KJSO* objValue;
  };

  /**
   * @short Base class for language extensions.
   */
  class HostObject : public KJSO {
  public:
    virtual Type type() const { return HostType; }
    virtual KJSO *get(const UString &p);
    virtual void put(const UString &p, KJSO *v);
  };

  /**
   * @short Unique global object containing initial native properties.
   */
  class Global : public KJSO {
  public:
    Global();
    Type type() const { return ObjectType; }
    Object *objProto;
    Object *funcProto;
    Object *arrayProto;
    Object *stringProto;
    Object *boolProto;
    Object *numProto;
  };

}; // namespace

#endif
