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

#ifndef _KJS_OBJECT_H_
#define _KJS_OBJECT_H_

#include <stdlib.h>

#include "ustring.h"

/**
 * @short Main namespace
 */
namespace KJS {

  /**
   * Types of classes derived from KJSO
   */
  enum Type { // main types
              AbstractType = 1,
              UndefinedType,
	      NullType,
	      BooleanType,
	      NumberType,
	      StringType,
	      ObjectType,
	      HostType,
	      ReferenceType,
              CompletionType,
	      // extended types
	      FunctionType,
	      InternalFunctionType,
	      DeclaredFunctionType,
	      AnonymousFunctionType,
	      ConstructorType,
	      ActivationType
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
	       ObjectClass,
	       DateClass,
	       RegExpClass,
	       ErrorClass };

  /**
   * Completion types.
   */
  enum Compl { Normal, Break, Continue, ReturnValue, Throw };

  /**
   * Error codes.
   */
  enum ErrorType { NoError = 0,
		   GeneralError,
		   EvalError,
		   RangeError,
		   ReferenceError,
		   SyntaxError,
		   TypeError,
		   URIError };

  extern const double NaN;
  extern const double Inf;

  // forward declarations
  class Imp;
  class Boolean;
  class Number;
  class String;
  class Object;
  struct Property;
  class List;

  /**
   * @short Type information.
   */
  struct TypeInfo {
    /**
     * A string denoting the type name. Example: "Number".
     */
    const char *name;
    /**
     * One of the @ref KJS::Type enums.
     */
    Type type;
    /**
     * Pointer to the type information of the base class.
     * NULL if there is none.
     */
    const TypeInfo *base;
    /**
     * Additional specifier for your own use.
     */
    int extra;
    /**
     * Reserved for future extensions (internal).
     */
    void *dummy;
  };

  /**
   * @short Main base class for every KJS object.
   */
  class KJSO {
  public:
    /**
     * Constructor.
     */
    KJSO();
    /**
     * @internal
     */
    KJSO(Imp *d);
    /**
     * Copy constructor.
     */
    KJSO(const KJSO &);
    /*
     * Assignment operator
     */
    KJSO& operator=(const KJSO &);
    /**
     * Destructor.
     */
    virtual ~KJSO();
    /**
     * @return True if this object is null, i.e. if there is no data attached
     * to this object. Don't confuse this with the Null object.
     */
    bool isNull() const;
    /**
     * @return the type of the object. One of the @ref KJS::Type enums.
     */
    Type type() const;
    /**
     * Check whether object is of a certain type
     * @param t type to check for
     */
    bool isA(Type t) const { return (type() == t); }
    /**
     * Check whether object is of a certain type. Allows checking of
     * host objects, too.
     * @param type name (Number, Boolean etc.)
     */
    bool isA(const char *s) const;
    /**
     * Use this method when checking for objects. It's safer than checking
     * for a single object type with @ref isA().
     */
    bool isObject() const;
    /**
     * Examine the inheritance structure of this object.
     * @param t Name of the base class.
     * @return True if object is of type t or a derived from such a type.
     */
    bool derivedFrom(const char *s) const;

    KJSO toPrimitive(Type preferred = UndefinedType) const; // ECMA 9.1
    Boolean toBoolean() const; // ECMA 9.2
    Number toNumber() const; // ECMA 9.3
    double round() const;
    Number toInteger() const; // ECMA 9.4
    int toInt32() const; // ECMA 9.5
    unsigned int toUInt32() const; // ECMA 9.6
    unsigned short toUInt16() const; // ECMA 9.7
    String toString() const; // ECMA 9.8
    Object toObject() const; // ECMA 9.9
    
    // Properties
    /**
     * Set the internal [[prototype]] property of this object.
     * @param p A prototype object.
     */
    void setPrototype(const KJSO& p);
    /**
     * @return The internal [[prototype]] property.
     */
    KJSO prototype() const;
    /**
     * The internal [[Get]] method.
     * @return The value of property p.
     */
    KJSO get(const UString &p) const;
    /**
     * The internal [[HasProperty]] method.
     * @param p Property name.
     * @param recursive Indicates whether prototypes are searched as well.
     * @return Boolean value indicating whether the object already has a
     * member with the given name p.
     */
    bool hasProperty(const UString &p, bool recursive = true) const;
    /**
     * The internal [[Put]] method. Sets the specified property to the value v.
     * @param p Property name.
     * @param v Value.
     */
    void put(const UString &p, const KJSO& v);
    /**
     * The internal [[CanPut]] method.
     * @param p Property name.
     * @return A boolean value indicating whether a [[Put]] operation with
     * p succeed.
     */
    bool canPut(const UString &p) const;
    /**
     * The internal [[Delete]] method. Removes the specified property from
     * the object.
     * @param p Property name.
     */
    void deleteProperty(const UString &p);

    void put(const UString &p, const KJSO& v, int attr);
    void put(const UString &p, double d, int attr = None);
    void put(const UString &p, int i, int attr = None);
    void put(const UString &p, unsigned int u, int attr = None);

    // Reference
    KJSO getBase() const;
    UString getPropertyName() const;
    KJSO getValue();
    ErrorType putValue(const KJSO& v);

    // function call
    bool implementsCall() const;
    KJSO executeCall(const KJSO &thisV, const List *args);

    // constructor
    void setConstructor(KJSO c);

    /**
     * @return A Pointer to the internal implementation.
     */
    Imp *imp() const { return rep; }

#ifdef KJS_DEBUG_MEM
    /**
     * @internal
     */
    static int count;
#endif
  protected:
    /**
     * Pointer to the internal implementation.
     */
    Imp *rep;

  private:
    void putArrayElement(const UString &p, const KJSO &v);
  }; // end of KJSO

  /**
   * @short Base for all implementation classes.
   */
  class Imp {
    friend KJSO;
    friend class Collector;
  public:
    Imp();
  public:
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const; // ECMA 9.1
    virtual Boolean toBoolean() const; // ECMA 9.2
    virtual Number toNumber() const; // ECMA 9.3
    virtual String toString() const; // ECMA 9.8
    virtual Object toObject() const; // ECMA 9.9

    // properties
    virtual KJSO get(const UString &p) const;
    virtual bool hasProperty(const UString &p, bool recursive = true) const;
    virtual void put(const UString &p, const KJSO& v);
    void put(const UString &p, const KJSO& v, int attr);
    virtual bool canPut(const UString &p) const;
    virtual void deleteProperty(const UString &p);
    virtual KJSO defaultValue(Type hint) const;

    bool implementsCall() const;

    /**
     * @internal Reserved for mark & sweep garbage collection
     */
    virtual void mark(Imp*);

    Type type() const { return typeInfo()->type; }
    /**
     * @return The TypeInfo struct describing this object.
     */
    virtual const TypeInfo* typeInfo() const { return &info; }
    
    void setPrototype(const KJSO& p);
    void setConstructor(const KJSO& c);

    void* operator new(size_t);
    void operator delete(void*, size_t);

#ifdef KJS_DEBUG_MEM
    /**
     * @internal
     */
    static int count;
#endif
  protected:
    virtual ~Imp();
  private:
    Imp(const Imp&);
    Imp& operator=(const Imp&);
    void putArrayElement(const UString &p, const KJSO& v);

    // reference counting mechanism
    inline Imp* ref() { refcount++; return this; }
    inline bool deref() { return (!--refcount); }
    unsigned int refcount;

    Property *prop;
    Imp *proto;
    static const TypeInfo info;
    
    // for future extensions
    class ImpInternal;
    ImpInternal *internal;
  };

  /**
   * @short General implementation class for Objects
   */
  class ObjectImp : public Imp {
    friend Object;
  public:
    ObjectImp(Class c);
    ObjectImp(Class c, const KJSO &v);
    ObjectImp(Class c, const KJSO &v, const KJSO &p);
    virtual ~ObjectImp();
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
    /**
     * @internal Reimplemenation of @ref Imp::mark().
     */
    virtual void mark(Imp*);
  private:
    Class cl;
    Imp *val;
  };

  /**
   * @short Object class encapsulating an internal value.
   */
  class Object : public KJSO {
  public:
    Object(Imp *d);
    Object(Class c = UndefClass);
    Object(Class c, const KJSO& v);
    Object(Class c, const KJSO& v, const Object& p);
    virtual ~Object();
    void setClass(Class c);
    Class getClass() const;
    void setInternalValue(const KJSO& v);
    KJSO internalValue();
    static Object create(Class c);
    static Object create(Class c, const KJSO& val);
    static Object create(Class c, const KJSO& val, const Object &p);
    static Object dynamicCast(const KJSO &obj);
  };

  /**
   * @short Implementation base class for Host Objects.
   */
  class HostImp : public Imp {
  public:
    virtual ~HostImp();
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  };

  class KJScriptImp;
  /**
   * @short Unique global object containing initial native properties.
   */
  class Global : public Object {
    friend KJScriptImp;
  public:
    Global();
    virtual ~Global();
    static Global current();
    KJSO objectPrototype() const;
    KJSO functionPrototype() const;
  private:
    Global(void *);
    void init();
  };

  /**
   * @short Factory methos for error objects.
   */
  class Error {
  public:
    static KJSO create(ErrorType e, const char *m = 0, int l = -1);
    static Object createObject(ErrorType e, const char *m = 0, int l = -1);
  };

}; // namespace

#endif
